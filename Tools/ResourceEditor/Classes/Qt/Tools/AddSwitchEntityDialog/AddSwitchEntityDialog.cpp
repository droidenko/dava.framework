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

#include "AddSwitchEntityDialog.h"
#include "ui_AddSwitchEntityDialog.h"
#include "./../Qt/Tools/MimeDataHelper/MimeDataHelper.h"

#include <QKeyEvent>

AddSwitchEntityDialog::AddSwitchEntityDialog(QWidget* parent)
:	QDialog(parent),
ui(new Ui::AddSwitchEntityDialog)
{
	ui->setupUi(this);
	setAcceptDrops(false);
	
	setWindowFlags(windowFlags() | Qt::Tool );
	setAttribute( Qt::WA_MacAlwaysShowToolWindow);
	ui->firstSelectionWidget->SetDiscriptionText("Select first entity:");

	ui->secondSelectionWidget->SetDiscriptionText("Select second entity:");

}

void AddSwitchEntityDialog::SetRelativePath(const DAVA::String& rPath, bool forFirstWidget )
{
	if(forFirstWidget)
	{
		ui->firstSelectionWidget->SetRelativePath(rPath);
	}
	else
	{
		ui->firstSelectionWidget->SetRelativePath(rPath);
	}
}

void AddSwitchEntityDialog::SetOpenDialogsDefaultPath(const DAVA::String& path)
{
	ui->firstSelectionWidget->SetOpenDialogDefaultPath(path);
	ui->secondSelectionWidget->SetOpenDialogDefaultPath(path);
}

void AddSwitchEntityDialog::GetSelectedEntities(DAVA::Entity** firstChild, DAVA::Entity** secondChild, SceneEditor2* editor)
{
	*firstChild  = ui->firstSelectionWidget->GetOutputEntity(editor);
	*secondChild = ui->secondSelectionWidget->GetOutputEntity(editor);
}

void AddSwitchEntityDialog::ErasePathWidgets()
{
	ui->firstSelectionWidget->EraseWidget();
	ui->secondSelectionWidget->EraseWidget();
}

AddSwitchEntityDialog::~AddSwitchEntityDialog()
{
	delete ui;
}

