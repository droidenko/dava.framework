/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "logger.h"
#include <QMessageBox>
#include <QComboBox>
#include <QSet>
#include "processhelper.h"
#include "settings.h"

#define LAUNCER_VER "0.87.5"

#define COLUMN_NAME 0
#define COLUMN_CUR_VER 1
#define COLUMN_NEW_VER 2

#define ST_TAB 0
#define TM_TAB 1
#define QA_TAB 2
#define DEV_TAB 3
#define DEP_TAB 4

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setWindowTitle(tr("Launcher %1").arg(QString(LAUNCER_VER)));

    m_pInstaller = new Installer(this);
    connect(m_pInstaller, SIGNAL(SoftWareAvailable(AvailableSoftWare)), this, SLOT(AvailableSoftWareUpdated(AvailableSoftWare)));
    connect(m_pInstaller, SIGNAL(StartDownload()), this, SLOT(OnDownloadStarted()));
    connect(m_pInstaller, SIGNAL(DownloadFinished()), this, SLOT(OnDownloadFinished()));
    connect(m_pInstaller, SIGNAL(DownloadProgress(int)), this, SLOT(OnDownloadProgress(int)));
    connect(m_pInstaller, SIGNAL(WebPageUpdated(QString)), this, SLOT(UpdateWebPage(QString)));

    m_pUpdateTimer = new QTimer(this);
    connect(m_pUpdateTimer, SIGNAL(timeout()), this, SLOT(on_btnRefresh_clicked()));
    m_pUpdateTimer->start(Settings::GetInstance()->GetUpdateTimerInterval());

    ui->stableTable->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    ui->toMasterTable->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    ui->testTable->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    ui->developmentTable->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    ui->dependenciesTable->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    ui->stableTable->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    ui->toMasterTable->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    ui->testTable->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    ui->developmentTable->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    ui->dependenciesTable->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);

    connect(ui->stableTable, SIGNAL(itemSelectionChanged()), this, SLOT(UpdateBtn()));
    connect(ui->toMasterTable, SIGNAL(itemSelectionChanged()), this, SLOT(UpdateBtn()));
    connect(ui->testTable, SIGNAL(itemSelectionChanged()), this, SLOT(UpdateBtn()));
    connect(ui->dependenciesTable, SIGNAL(itemSelectionChanged()), this, SLOT(UpdateBtn()));
    connect(ui->developmentTable, SIGNAL(itemSelectionChanged()), this, SLOT(UpdateBtn()));
    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(UpdateBtn()));

    connect(ui->stableTable, SIGNAL(itemClicked(QTableWidgetItem *)), this, SLOT(tableItemClicked(QTableWidgetItem *)));
    connect(ui->stableTable, SIGNAL(itemChanged(QTableWidgetItem *)), this, SLOT(tableItemChanged(QTableWidgetItem *)));
    connect(ui->toMasterTable, SIGNAL(itemClicked(QTableWidgetItem *)), this, SLOT(tableItemClicked(QTableWidgetItem *)));
    connect(ui->toMasterTable, SIGNAL(itemChanged(QTableWidgetItem *)), this, SLOT(tableItemChanged(QTableWidgetItem *)));
    connect(ui->testTable, SIGNAL(itemClicked(QTableWidgetItem *)), this, SLOT(tableItemClicked(QTableWidgetItem *)));
    connect(ui->testTable, SIGNAL(itemChanged(QTableWidgetItem *)), this, SLOT(tableItemChanged(QTableWidgetItem *)));
    connect(ui->developmentTable, SIGNAL(itemClicked(QTableWidgetItem *)), this, SLOT(tableItemClicked(QTableWidgetItem *)));
    connect(ui->developmentTable, SIGNAL(itemChanged(QTableWidgetItem *)), this, SLOT(tableItemChanged(QTableWidgetItem *)));

    connect(ui->stableTable, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(on_btnRun_clicked()));
    connect(ui->toMasterTable, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(on_btnRun_clicked()));
    connect(ui->testTable, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(on_btnRun_clicked()));
    connect(ui->developmentTable, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(on_btnRun_clicked()));

    connect(Logger::GetInstance(), SIGNAL(LogAdded(QString)), this, SLOT(OnLogAdded(QString)));

    ui->downloadProgress->setVisible(false);
    ui->downloadProgress->setRange(0, 100);

    isTablesFilling = false;
    m_bBusy = false;
    UpdateBtn();
    m_pInstaller->Init();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::tableItemClicked(QTableWidgetItem * item)
{
    if(!isTablesFilling)
        lastSelectedValue = item->text();
}

void MainWindow::tableItemChanged(QTableWidgetItem * item)
{
    if(!isTablesFilling)
        item->setText(lastSelectedValue);
}

void MainWindow::AvailableSoftWareUpdated(const AvailableSoftWare& software) {
    isTablesFilling = true;
    m_SoftWare = software;
    FillTableSoft(ui->stableTable, software.m_Stable);
    FillTableSoft(ui->toMasterTable, software.m_toMaster);
    FillTableSoft(ui->testTable, software.m_Test);
    FillTableSoft(ui->developmentTable, software.m_Development);
    FillTableDependencies(ui->dependenciesTable, software.m_Dependencies);
    isTablesFilling = false;
}

void MainWindow::OnComboBoxValueChanged(const QString& value) {
    m_SelectedAppVersion = value;
    UpdateBtn();
}

void MainWindow::FillTableSoft(QTableWidget* table, const AvailableSoftWare::SoftWareMap& soft) {
    AvailableSoftWare::SoftWareMap::const_iterator iter;
    table->setRowCount(0);
    for (iter = soft.begin(); iter != soft.end(); ++iter) {
        const QString& name = iter.key();
        const SoftWare& config = iter.value();

        int i = table->rowCount();
        table->insertRow(i);
        table->setItem(i, COLUMN_NAME, new QTableWidgetItem(name));
        table->setItem(i, COLUMN_CUR_VER, new QTableWidgetItem(config.m_CurVersion));
        if (config.m_AvailableVersion.size() == 1) {
            table->setItem(i, COLUMN_NEW_VER, new QTableWidgetItem(*(config.m_AvailableVersion.begin())));
        } else {
            QComboBox* pComboBox = new QComboBox(table);
            connect(pComboBox,
                    SIGNAL(currentIndexChanged(QString)),
                    this,
                    SLOT(OnComboBoxValueChanged(QString)));

            QList<QString> versionsList = config.m_AvailableVersion.toList();
            qSort(versionsList);

            for (QList<QString>::const_iterator iter = versionsList.begin();
                 iter != versionsList.end();
                 ++iter) {
                pComboBox->addItem(*iter);
            }
            pComboBox->setCurrentIndex(pComboBox->count() - 1);
            table->setCellWidget(i, COLUMN_NEW_VER, pComboBox);
        }
    }
    table->resizeRowsToContents();
}

void MainWindow::FillTableDependencies(QTableWidget* table, const AvailableSoftWare::SoftWareMap& soft) {
    AvailableSoftWare::SoftWareMap::const_iterator iter;
    table->setRowCount(0);
    for (iter = soft.begin(); iter != soft.end(); ++iter) {
        const QString& name = iter.key();
        const SoftWare& config = iter.value();

        int i = table->rowCount();
        table->insertRow(i);
        table->setItem(i, COLUMN_NAME, new QTableWidgetItem(name));
        if (config.m_CurVersion.isEmpty())
            table->setItem(i, COLUMN_CUR_VER, new QTableWidgetItem("Not installed"));
        else if (config.m_CurVersion == config.m_NewVersion)
            table->setItem(i, COLUMN_CUR_VER, new QTableWidgetItem("Installed latest"));
        else
            table->setItem(i, COLUMN_CUR_VER, new QTableWidgetItem("Installed not latest"));
    }
    table->resizeRowsToContents();
}

void MainWindow::UpdateSelectedApp(QTableWidget* table) {
    QList<QTableWidgetItem *> items = table->selectedItems();
    if (!items.size())
        return;

    for (int i = 0; i < items.size(); i++) {
        if (items[i]->column() == COLUMN_NAME)
            m_SelectedApp = items[i]->text();
        else if (items[i]->column() == COLUMN_NEW_VER)
            m_SelectedAppVersion = items[i]->text();
    }
    if (m_SelectedAppVersion.isEmpty()) {
        QWidget* pWidget = table->cellWidget(items[0]->row(), COLUMN_NEW_VER);
        QComboBox* pComboBox = dynamic_cast<QComboBox*>(pWidget);
        if (pComboBox)
            m_SelectedAppVersion = pComboBox->currentText();
    }
}

void MainWindow::UpdateBtn() {
    //update buttons
    ui->btnRemove->setEnabled(false);
    //ui->btnInstall->setEnabled(false);
    ui->btnInstall->setVisible(false);
    ui->btnRun->setVisible(false);
    ui->btnReinstall->setVisible(false);
    ui->btnCancel->setVisible(false);

    if (m_bBusy) {
        ui->btnCancel->setVisible(true);
        return;
    }

    m_SelectedApp.clear();
    m_SelectedAppVersion.clear();

    const AvailableSoftWare::SoftWareMap* pSelectedMap = NULL;
    switch (ui->tabWidget->currentIndex()) {
    case ST_TAB: {
        pSelectedMap = &m_SoftWare.m_Stable;
        m_SelectedAppType = eAppTypeStable;
        UpdateSelectedApp(ui->stableTable);
    }break;
    case TM_TAB: {
        pSelectedMap = &m_SoftWare.m_toMaster;
        m_SelectedAppType = eAppTypeToMaster;
        UpdateSelectedApp(ui->toMasterTable);
    }break;
    case QA_TAB: {
        pSelectedMap = &m_SoftWare.m_Test;
        m_SelectedAppType = eAppTypeTest;
        UpdateSelectedApp(ui->testTable);
    }break;
    case DEV_TAB: {
        pSelectedMap = &m_SoftWare.m_Development;
        m_SelectedAppType = eAppTypeDevelopment;
        UpdateSelectedApp(ui->developmentTable);
    }break;
    case DEP_TAB: {
        pSelectedMap = &m_SoftWare.m_Dependencies;
        m_SelectedAppType = eAppTypeDependencies;
        UpdateSelectedApp(ui->dependenciesTable);
    }break;
    default:
        return;
    }

    if (pSelectedMap == NULL || m_SelectedApp.isEmpty())
        return;

    if (pSelectedMap->value(m_SelectedApp).m_CurVersion.isEmpty()) {
        //install
        ui->btnInstall->setVisible(true);
    } else {
        ui->btnInstall->setVisible(false);
        ui->btnRemove->setEnabled(true);
        ui->btnRun->setEnabled(!pSelectedMap->value(m_SelectedApp).m_RunPath.isEmpty());

        switch (ui->tabWidget->currentIndex())
        {
        case DEV_TAB: {
            if (pSelectedMap->value(m_SelectedApp).m_CurVersion != m_SelectedAppVersion)
            {
                ui->btnInstall->setEnabled(true);
                ui->btnInstall->setVisible(true);
            }
        };
        case ST_TAB:
        case TM_TAB:
        case QA_TAB: {
            ui->btnRun->setVisible(true);
        } break;

        case DEP_TAB: {
            ui->btnRemove->setEnabled(false);
            ui->btnInstall->setVisible(false);
            ui->btnRun->setVisible(false);
            ui->btnReinstall->setVisible(false);
            ui->btnCancel->setVisible(false);
        }break;
        }

        /*if (ui->tabWidget->currentIndex() != DEP_TAB)
            ui->btnRun->setVisible(true);
        else
            ui->btnReinstall->setVisible(true);
        ui->btnRemove->setEnabled(true);
        ui->btnRun->setEnabled(!pSelectedMap->value(m_SelectedApp).m_RunPath.isEmpty());*/
    }
}

void MainWindow::OnLogAdded(const QString &log) {
    ui->label->setText(log);
    ui->listWidget->addItem(log);
}

void MainWindow::on_btnInstall_clicked() {
    m_pInstaller->Install(m_SelectedApp, m_SelectedAppVersion, m_SelectedAppType);
}

void MainWindow::on_btnRefresh_clicked() {
    m_pInstaller->CheckForUpdate();
}

void MainWindow::on_btnCancel_clicked() {
    m_pInstaller->AbortCurInstallation();
}

void MainWindow::on_btnRemove_clicked() {
    if (0 == QMessageBox::information(this,
                                     tr("Confirmation"),
                                     tr("Are you sure you want to remove %1.").arg(m_SelectedApp),
                                     tr("OK"),
                                     tr("Cancel"))) {
        m_pInstaller->Delete(m_SelectedApp, m_SelectedAppType);
    }
}

void MainWindow::on_btnRun_clicked() {
    QString path = m_pInstaller->GetRunPath(m_SelectedApp, m_SelectedAppType);
    if (ProcessHelper::IsProcessRuning(path)) {
        ProcessHelper::SetActiveProcess(path);
    } else {
        ProcessHelper::RunProcess(path);
    }
}

void MainWindow::on_btnReinstall_clicked() {
    if (m_pInstaller->Delete(m_SelectedApp, m_SelectedAppType)) {
        m_pInstaller->Install(m_SelectedApp, m_SelectedAppVersion, m_SelectedAppType);
    }
}

void MainWindow::OnDownloadStarted() {
    m_bBusy = true;
    ui->downloadProgress->setValue(0);
    ui->downloadProgress->setVisible(true);
    UpdateBtn();
}

void MainWindow::OnDownloadProgress(int nPercent) {
    ui->downloadProgress->setValue(nPercent);
}

void MainWindow::OnDownloadFinished() {
    m_bBusy = false;
    ui->downloadProgress->setVisible(false);
    UpdateBtn();
}

void MainWindow::UpdateWebPage(const QString& url) {
    ui->webView->setUrl(url);
}
