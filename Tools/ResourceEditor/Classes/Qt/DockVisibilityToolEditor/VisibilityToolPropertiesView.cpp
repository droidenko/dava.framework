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

#include "VisibilityToolPropertiesView.h"
#include "ui_VisibilityToolPropertiesView.h"

#include "../Main/QtMainWindowHandler.h"
#include "../Scene/SceneSignals.h"

VisibilityToolPropertiesView::VisibilityToolPropertiesView(QWidget* parent)
:	QWidget(parent),
ui(new Ui::VisibilityToolPropertiesView)
{
	ui->setupUi(this);

	Init();
}

VisibilityToolPropertiesView::~VisibilityToolPropertiesView()
{
	delete ui;
}

void VisibilityToolPropertiesView::Init()
{
	QtMainWindowHandler* handler = QtMainWindowHandler::Instance();

	connect(SceneSignals::Instance(), SIGNAL(UpdateVisibilityButtonsState(SceneEditor2*)),
			handler, SLOT(SetVisibilityToolButtonsState(SceneEditor2*)));

	connect(ui->buttonEnableVisibilityTool, SIGNAL(clicked()), handler, SLOT(ToggleVisibilityToolEditor()));
	connect(ui->buttonSaveTexture, SIGNAL(clicked()), handler, SLOT(SaveVisibilityToolTexture()));
	connect(ui->buttonSetVisibilityPoint, SIGNAL(clicked()), handler, SLOT(SetVisibilityPoint()));
	connect(ui->buttonSetVisibilityArea, SIGNAL(clicked()), handler, SLOT(SetVisibilityArea()));
	connect(ui->sliderBrushSize, SIGNAL(valueChanged(int)), handler, SLOT(SetVisibilityToolAreaSize(int)));

	handler->RegisterVisibilityToolWidgets(ui->buttonEnableVisibilityTool,
										   ui->buttonSaveTexture,
										   ui->buttonSetVisibilityPoint,
										   ui->buttonSetVisibilityArea,
										   ui->sliderBrushSize);

	handler->SetVisibilityToolWidgetsState(false);
}
