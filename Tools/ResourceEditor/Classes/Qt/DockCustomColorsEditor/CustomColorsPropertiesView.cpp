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

#include "CustomColorsPropertiesView.h"
#include "ui_CustomColorsProperties.h"

#include "Project/ProjectManager.h"
#include "Classes/Qt/Main/QtMainWindowHandler.h"
#include "../SceneEditor/EditorConfig.h"
#include "../Scene/SceneSignals.h"

CustomColorsPropertiesView::CustomColorsPropertiesView(QWidget* parent)
:	QWidget(parent),
ui(new Ui::CustomColorsPropertiesView)
{
	ui->setupUi(this);

	Init();
}

CustomColorsPropertiesView::~CustomColorsPropertiesView()
{
	delete ui;
}

void CustomColorsPropertiesView::Init()
{
	QObject::connect(ProjectManager::Instance(), SIGNAL(ProjectOpened(const QString &)), this, SLOT(ProjectOpened(const QString &)));

	QtMainWindowHandler* handler = QtMainWindowHandler::Instance();
	connect(ui->buttonEnableCustomColorsEditor, SIGNAL(clicked()), handler, SLOT(ToggleCustomColorsEditor()));
	connect(SceneSignals::Instance(), SIGNAL(NeedSaveCustomColorsTexture(SceneEditor2*)),
			handler, SLOT(NeedSaveCustomColorsTexture(SceneEditor2*)));
	
	ui->buttonSaveTexture->blockSignals(true);
	ui->sliderBrushSize->blockSignals(true);
	ui->comboColor->blockSignals(true);

	connect(ui->buttonSaveTexture, SIGNAL(clicked()), handler, SLOT(SaveCustomColorsTexture()));
	connect(ui->sliderBrushSize, SIGNAL(valueChanged(int)), handler, SLOT(SetCustomColorsBrushSize(int)));
	connect(ui->comboColor, SIGNAL(currentIndexChanged(int)), handler, SLOT(SetCustomColorsColor(int)));
	connect(ui->buttonLoadTexture, SIGNAL(clicked()), handler, SLOT(LoadCustomColorsTexture()));

	QtMainWindowHandler::Instance()->RegisterCustomColorsEditorWidgets(ui->buttonEnableCustomColorsEditor,
																 ui->buttonSaveTexture,
																 ui->sliderBrushSize,
																 ui->comboColor,
																 ui->buttonLoadTexture);

	handler->SetCustomColorsEditorWidgetsState(false);
}

void CustomColorsPropertiesView::InitColors()
{
	QSize iconSize = ui->comboColor->iconSize();
	iconSize = iconSize.expandedTo(QSize(100, 0));
	ui->comboColor->setIconSize(iconSize);

	Vector<Color> customColors = EditorConfig::Instance()->GetColorPropertyValues("LandscapeCustomColors");
	Vector<String> customColorsDescription = EditorConfig::Instance()->GetComboPropertyValues("LandscapeCustomColorsDescription");
	for(size_t i = 0; i < customColors.size(); ++i)
	{
		QColor color = QColor::fromRgbF(customColors[i].r, customColors[i].g, customColors[i].b, customColors[i].a);

		QImage image(iconSize, QImage::Format_ARGB32);
		image.fill(color);

		QPixmap pixmap(iconSize);
		pixmap.convertFromImage(image, Qt::ColorOnly);

		QIcon icon(pixmap);
		String description = (i >= customColorsDescription.size()) ? "" : customColorsDescription[i];
		ui->comboColor->addItem(icon, description.c_str());
	}
}

void CustomColorsPropertiesView::ProjectOpened(const QString& path)
{
	InitColors();
}
