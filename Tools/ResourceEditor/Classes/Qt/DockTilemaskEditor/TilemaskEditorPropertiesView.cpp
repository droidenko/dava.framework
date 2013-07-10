/*==================================================================================
 Copyright (c) 2008, DAVA Consulting, LLC
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
 * Neither the name of the DAVA Consulting, LLC nor the
 names of its contributors may be used to endorse or promote products
 derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 =====================================================================================*/

#include "TilemaskEditorPropertiesView.h"
#include "ui_tilemaskEditorProperties.h"

#include "../Main/QtMainWindowHandler.h"

#include "Qt/Scene/SceneSignals.h"

TilemaskEditorPropertiesView::TilemaskEditorPropertiesView(QWidget* parent)
:	QWidget(parent)
,	ui(new Ui::TilemaskEditorPropertiesView)
{
	ui->setupUi(this);

	Init();
}

TilemaskEditorPropertiesView::~TilemaskEditorPropertiesView()
{
	delete ui;
}

void TilemaskEditorPropertiesView::Init()
{
	InitBrushImages();

	QtMainWindowHandler* handler = QtMainWindowHandler::Instance();

	ui->sliderBrushSize->setValue(120);
	ui->sliderStrength->setValue(15);

	connect(ui->buttonEnableTilemaskEditor, SIGNAL(clicked()), handler, SLOT(ToggleTilemaskEditor()));
	connect(ui->sliderBrushSize, SIGNAL(valueChanged(int)), handler, SLOT(SetTilemaskEditorBrushSize(int)));
	connect(ui->comboBrushImage, SIGNAL(currentIndexChanged(int)), handler, SLOT(SetTilemaskEditorToolImage(int)));
	connect(ui->sliderStrength, SIGNAL(valueChanged(int)), handler, SLOT(SetTilemaskEditorStrength(int)));
	connect(ui->comboTileTexture, SIGNAL(currentIndexChanged(int)), handler, SLOT(SetTilemaskDrawTexture(int)));

	QtMainWindowHandler::Instance()->RegisterTilemaskEditorWidgets(ui->buttonEnableTilemaskEditor,
																	ui->sliderBrushSize,
																	ui->comboBrushImage,
																	ui->sliderStrength,
																	ui->comboTileTexture);

	handler->SetTilemaskEditorWidgetsState(false);
}

void TilemaskEditorPropertiesView::InitBrushImages()
{
	QSize iconSize = ui->comboBrushImage->iconSize();
	iconSize = iconSize.expandedTo(QSize(32, 32));
	ui->comboBrushImage->setIconSize(iconSize);

	FilePath toolsPath("~res:/LandscapeEditor/Tools/");
	FileList *fileList = new FileList(toolsPath);
	for(int32 iFile = 0; iFile < fileList->GetCount(); ++iFile)
	{
		String filename = fileList->GetFilename(iFile);
		if(fileList->GetPathname(iFile).IsEqualToExtension(".png"))
		{
			String fullname = fileList->GetPathname(iFile).GetAbsolutePathname();

			FilePath f = fileList->GetPathname(iFile);
			f.ReplaceExtension("");

			QString qFullname = QString::fromStdString(fullname);
			QIcon toolIcon(qFullname);
			ui->comboBrushImage->addItem(toolIcon, f.GetFilename().c_str(), QVariant(qFullname));
		}
	}
}
