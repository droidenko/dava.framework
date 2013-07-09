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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include "installer.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private slots:
    void AvailableSoftWareUpdated(const AvailableSoftWare&);
    void UpdateBtn();
    void OnLogAdded(const QString& log);
    void UpdateWebPage(const QString& url);

    void on_btnInstall_clicked();
    void on_btnRefresh_clicked();
    void on_btnRemove_clicked();
    void on_btnRun_clicked();

    void on_btnReinstall_clicked();

    void OnDownloadStarted();
    void OnDownloadProgress(int nPercent);
    void OnDownloadFinished();

    void on_btnCancel_clicked();

    void tableItemClicked(QTableWidgetItem * item);
    void tableItemChanged(QTableWidgetItem * item);

    void OnComboBoxValueChanged(const QString& value);

private:
    void FillTableSoft(QTableWidget* table, const AvailableSoftWare::SoftWareMap& soft);
    void FillTableDependencies(QTableWidget* table, const AvailableSoftWare::SoftWareMap& soft);
    void UpdateSelectedApp(QTableWidget* table);

private:
    Ui::MainWindow *ui;
    Installer* m_pInstaller;

    AvailableSoftWare m_SoftWare;

    eAppType m_SelectedAppType;
    QString m_SelectedApp;
    QString m_SelectedAppVersion;

    QTimer* m_pUpdateTimer;

    bool m_bBusy;

    QString lastSelectedValue;
    bool isTablesFilling;
};

#endif // MAINWINDOW_H
