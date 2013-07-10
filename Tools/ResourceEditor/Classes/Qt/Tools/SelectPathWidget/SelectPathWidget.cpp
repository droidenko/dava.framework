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
#include "./../Qt/Tools/MimeDataHelper/MimeDataHelper.h"


#include <QFileInfo>
#include <QKeyEvent>
#include <QUrl>
#include <QFileDialog>

SelectPathWidget::SelectPathWidget(QWidget* _parent, DAVA::String _openDialogDefualtPath, DAVA::String _relativPath)
:	QWidget(_parent),
ui(new Ui::SelectPathWidget)
{
	Init(_openDialogDefualtPath, _relativPath);
}

SelectPathWidget::~SelectPathWidget()
{
	delete ui;
}

void SelectPathWidget::Init(DAVA::String& _openDialogDefualtPath, DAVA::String& _relativPath)
{
	ui->setupUi(this);
	setAcceptDrops(true);
	
	connect(ui->eraseBtn, SIGNAL(clicked()), this, SLOT(EraseClicked()));
	connect(ui->selectDialogBtn, SIGNAL(clicked()), this, SLOT(OpenClicked()));
	
	relativePath = DAVA::FilePath(_relativPath);
	openDialogDefaultPath = _openDialogDefualtPath;
}

void SelectPathWidget::dragEnterEvent(QDragEnterEvent* event)
{
	if(DAVA::MimeDataHelper::IsMimeDataTypeSupported(event->mimeData()))
	{
		event->acceptProposedAction();
	}
}

void SelectPathWidget::dropEvent(QDropEvent* event)
{
	const QMimeData* sendedMimeData = event->mimeData();
		
	DAVA::List<DAVA::String> nameList;
	
	DAVA::MimeDataHelper::GetItemNamesFromMimeData(sendedMimeData, nameList);
	if(nameList.size() == 0)
	{
		return;
	}
	
	mimeData.clear();

	
	foreach(const QString & format, event->mimeData()->formats())
	{
		if(DAVA::MimeDataHelper::IsMimeDataTypeSupported(format.toStdString()))
		{
			mimeData.setData(format, event->mimeData()->data(format));
		}
	}
	
	DAVA::String itemName = *nameList.begin();
	DAVA::FilePath filePath(itemName);
	if(filePath.Exists())// check is it item form scene tree or file system
	{
		SetPathText(filePath.GetAbsolutePathname().c_str());
	}
	else
	{
		SetPathText(itemName.c_str());
	}
	
	event->acceptProposedAction();
}

void SelectPathWidget::EraseWidget()
{
	ui->filePathBox->setText("");
	mimeData.clear();
}

void SelectPathWidget::EraseClicked()
{
	EraseWidget();
}

void SelectPathWidget::OpenClicked()
{
	DAVA::FilePath presentPath(ui->filePathBox->text().toStdString());
	DAVA::FilePath dialogString(openDialogDefaultPath);
	if(presentPath.GetDirectory().Exists())//check if file text box clean
	{
		dialogString = presentPath.GetDirectory();
	}
	
	DAVA::String retString = QFileDialog::getOpenFileName(this, "Open Scene File",
                                                    QString(dialogString.GetAbsolutePathname().c_str()),
                                                    "Scene File (*.sc2)").toStdString();

	if(!retString.empty())
	{
		SetPathText(retString);
		
		DAVA::FilePath fullPath(retString);
		
		DVASSERT(fullPath.Exists());
		DAVA::List<DAVA::FilePath> urls;
		urls.push_back(fullPath);
		DAVA::MimeDataHelper::ConvertToMimeData(urls, &mimeData);
	}
}


void SelectPathWidget::SetDiscriptionText(const DAVA::String& discription)
{
	ui->descriptionLabel->setText(QString(discription.c_str()));
}

DAVA::String SelectPathWidget::GetDiscriptionText()
{
	return ui->descriptionLabel->text().toStdString();
}

void SelectPathWidget::SetPathText(const DAVA::String& filePath)
{
	ui->filePathBox->setText(QString(ConvertToRelativPath(filePath).c_str()));
}

DAVA::String SelectPathWidget::GetPathText()
{
	return ui->filePathBox->text().toStdString();
}

void SelectPathWidget::SetRelativePath(const DAVA::String& newRelativPath)
{
	relativePath = DAVA::FilePath(newRelativPath);
	DAVA::String existingPath = ui->filePathBox->text().toStdString();
	if(!existingPath.empty())
	{
		ui->filePathBox->setText(QString(ConvertToRelativPath(existingPath).c_str()));
	}
}

DAVA::Entity* SelectPathWidget::GetOutputEntity(SceneEditor2* editor)
{
	DAVA::List<DAVA::Entity*> retList;
	DAVA::MimeDataHelper::ConvertFromMimeData(&mimeData, retList, editor);
	DAVA::Entity* retEntity = retList.size() > 0 ? *retList.begin(): NULL;
	return retEntity;
}

DAVA::String SelectPathWidget::ConvertToRelativPath(const DAVA::String& path)
{
	DAVA::FilePath fullPath(path);
	if(fullPath.Exists())
	{
		return fullPath.GetRelativePathname(relativePath);
	}
	else
	{
		return path;
	}
}

