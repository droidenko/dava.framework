/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA, INC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA, INC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "SelectPathWidget.h"
#include "ui_SelectPathWidget.h"
#include "SceneEditor/EditorSettings.h"
#include "./../Qt/Main/QtUtils.h"
#include "./../Qt/DockSceneTree/SceneTreeModel.h"
#include "./../Qt/Tools/QMimeDataHelper/QMimeDataHelper.h"


#include <QFileInfo>
#include <QKeyEvent>
#include <QUrl>
#include <QFileDialog>

SelectPathWidget::SelectPathWidget(QWidget* parent)
:	QWidget(parent),
ui(new Ui::SelectPathWidget)
{
	ui->setupUi(this);
    setAcceptDrops(true);
    
    connect(ui->EraseBtn, SIGNAL(clicked()), this, SLOT(EraseClicked()));
    
    connect(ui->SelectDialogBtn, SIGNAL(clicked()), this, SLOT(OpenClicked()));
}

SelectPathWidget::~SelectPathWidget()
{
	delete ui;
}

void SelectPathWidget::dragEnterEvent(QDragEnterEvent* event)
{
    if(QMimeDataHelper::IsMimeDataTypeSupported(event->mimeData()))
    {
        event->acceptProposedAction();
    }
    
}

void SelectPathWidget::dropEvent(QDropEvent* event)
{
	const QMimeData* sendedMimeData = event->mimeData();
		
	List<String> nameList;
	
	bool success = QMimeDataHelper::GetItemNamesFromMime(sendedMimeData, nameList);
	if(!success)
	{
		return;
	}
	String itemName = *nameList.begin();
	FilePath filePath(itemName);
	if(filePath.Exists())
	{
		SetPathText(QString(filePath.GetAbsolutePathname().c_str()));
		ui->FilePathBox->setEnabled(true);
	}
	else
	{
		SetPathText(QString(itemName.c_str()));
		ui->FilePathBox->setEnabled(false);
	}
	
	mimeData.clear();
	foreach(const QString & format, event->mimeData()->formats())
	{
		mimeData.setData(format, event->mimeData()->data(format));
	}
	event->acceptProposedAction();
}


void SelectPathWidget::EraseClicked()
{
	ui->FilePathBox->setText("");
}

void SelectPathWidget::OpenClicked()
{
	FilePath presentPath(ui->FilePathBox->text().toStdString());
	FilePath dialogString = EditorSettings::Instance()->GetDataSourcePath();
	if(presentPath.GetDirectory().Exists())
	{
		dialogString = presentPath.GetDirectory();
	}
	
	((QWidget*)parent())->setWindowFlags(Qt::WindowStaysOnBottomHint);
	((QWidget*)parent())->showNormal();
	QString retString = QFileDialog::getOpenFileName(this, "Open Scene File",
                                                    QString(dialogString.GetAbsolutePathname().c_str()),
                                                    "Scene File (*.sc2)");

	((QWidget*)parent())->setWindowFlags(Qt::WindowStaysOnTopHint);
	((QWidget*)parent())->showNormal();
	SetPathText(retString);
	FilePath filePath(retString.toStdString());
	List<FilePath> urls;
	urls.push_back(filePath);
	if(!QMimeDataHelper::ConvertToMime(urls, &mimeData))
	{
		mimeData.clear();
	}
}


void SelectPathWidget::SetDiscriptionText(const QString& discription)
{
    ui->DiscriptionLabel->setText(discription);
}

QString SelectPathWidget::GetDiscriptionText()
{
    return ui->DiscriptionLabel->text();
}

void SelectPathWidget::SetPathText(const QString& filePath)
{
    ui->FilePathBox->setText(ConvertToRelativPath(filePath));
}

QString SelectPathWidget::GetPathText()
{
    return ui->FilePathBox->text();
}

void SelectPathWidget::SetRelativePath(const QString& newRelativPath)
{
    relativPath = newRelativPath;
    QString existingPath = ui->FilePathBox->text();
    if(!existingPath.isEmpty())
    {
        ui->FilePathBox->setText(ConvertToRelativPath(existingPath));
    }
}

QString SelectPathWidget::ConvertToRelativPath(const QString& path)
{
    //QString retValue = path;
    int index = path.indexOf(relativPath);
    QString retValue = path.right(path.length() - index);
    return retValue;
}
