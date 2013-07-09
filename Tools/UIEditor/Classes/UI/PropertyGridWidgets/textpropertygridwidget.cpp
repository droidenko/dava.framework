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
#include "textpropertygridwidget.h"
#include "ui_uitextfieldpropertygridwidget.h"

#include "CommandsController.h"
#include "ChangePropertyCommand.h"
#include "UITextControlMetadata.h"
#include "PropertiesHelper.h"
#include "PropertiesGridController.h"
#include "FileSystem/LocalizationSystem.h"
#include "WidgetSignalsBlocker.h"
#include "fontmanagerdialog.h"
#include "PropertyNames.h"
#include "ResourcesManageHelper.h"
#include "BackgroundGridWidgetHelper.h"
#include "UIStaticTextMetadata.h"
#include "UITextFieldMetadata.h"
#include "UIButtonMetadata.h"

#include "StringUtils.h"

#include <QLabel>

using namespace DAVA;

static const QString TEXT_PROPERTY_BLOCK_NAME = "Text";

TextPropertyGridWidget::TextPropertyGridWidget(QWidget *parent) :
    UITextFieldPropertyGridWidget(parent)
{
    SetPropertyBlockName(TEXT_PROPERTY_BLOCK_NAME);
	InsertLocalizationFields();
	BasePropertyGridWidget::InstallEventFiltersForWidgets(this);
}

TextPropertyGridWidget::~TextPropertyGridWidget()
{
	delete localizationKeyNameLineEdit;
	delete localizationKeyTextLineEdit;
	delete localizationKeyNameLabel;
	delete localizationKeyTextLabel;
}

void TextPropertyGridWidget::Initialize(BaseMetadata* activeMetadata)
{
	BasePropertyGridWidget::Initialize(activeMetadata);
	FillComboboxes();
    
    PROPERTIESMAP propertiesMap = BuildMetadataPropertiesMap();

    // All these properties are state-aware.
    RegisterSpinBoxWidgetForProperty(propertiesMap, PropertyNames::FONT_SIZE_PROPERTY_NAME, ui->fontSizeSpinBox, false, true);
    RegisterPushButtonWidgetForProperty(propertiesMap, PropertyNames::FONT_PROPERTY_NAME, ui->fontSelectButton, false, true);
    RegisterColorButtonWidgetForProperty(propertiesMap, PropertyNames::FONT_COLOR_PROPERTY_NAME, ui->textColorPushButton, false, true);
    // Shadow properties are also state-aware
    RegisterSpinBoxWidgetForProperty(propertiesMap, PropertyNames::SHADOW_OFFSET_X, ui->shadowOffsetXSpinBox, false, true);
    RegisterSpinBoxWidgetForProperty(propertiesMap, PropertyNames::SHADOW_OFFSET_Y, ui->shadowOffsetYSpinBox, false, true);
    RegisterColorButtonWidgetForProperty(propertiesMap, PropertyNames::SHADOW_COLOR, ui->shadowColorButton, false, true);
    // Localized Text Key is handled through generic Property mechanism, but we need to update the
    // Localization Value widget each time Localization Key is changes.
    RegisterLineEditWidgetForProperty(propertiesMap, PropertyNames::LOCALIZED_TEXT_KEY_PROPERTY_NAME, localizationKeyNameLineEdit, false, true);
	RegisterComboBoxWidgetForProperty(propertiesMap, PropertyNames::TEXT_ALIGN_PROPERTY_NAME, ui->alignComboBox, false, true);

	bool enableTextAlignComboBox = (dynamic_cast<UIStaticTextMetadata*>(activeMetadata)	!= NULL||
									dynamic_cast<UITextFieldMetadata*>(activeMetadata)	!= NULL||
									dynamic_cast<UIButtonMetadata*>(activeMetadata)		!= NULL);
	ui->alignComboBox->setEnabled(enableTextAlignComboBox);

    UpdateLocalizationValue();

    RegisterGridWidgetAsStateAware();
}

void TextPropertyGridWidget::InsertLocalizationFields()
{
	ui->textLineEdit->setEnabled(false);
	ui->textLineEdit->setVisible(false);
	ui->textLabel->setVisible(false);
	
	this->resize(300, 302);
    this->setMinimumSize(QSize(300, 302));

	ui->groupBox->resize(300, 302);
    ui->groupBox->setMinimumSize(QSize(300, 302));
	
	localizationKeyNameLabel = new QLabel(ui->groupBox);
	localizationKeyNameLabel->setObjectName(QString::fromUtf8("localizationKeyNameLabel"));
	localizationKeyNameLabel->setGeometry(QRect(10, 25, 281, 16));
	localizationKeyNameLabel->setText(QString::fromUtf8("Localization Key:"));
	localizationKeyNameLineEdit = new QLineEdit(ui->groupBox);
	localizationKeyNameLineEdit->setObjectName(QString::fromUtf8("localizationKeyNameLineEdit"));
	localizationKeyNameLineEdit->setGeometry(QRect(10, 45, 281, 22));
	
	localizationKeyTextLabel = new QLabel(ui->groupBox);
	localizationKeyTextLabel->setObjectName(QString::fromUtf8("localizationKeyTextLabel"));
	localizationKeyTextLabel->setGeometry(QRect(10, 75, 281, 16));
	localizationKeyTextLabel->setText(QString::fromUtf8("Localization Value for current language:"));
	localizationKeyTextLineEdit = new QLineEdit(ui->groupBox);
	localizationKeyTextLineEdit->setObjectName(QString::fromUtf8("localizationKeyTextLineEdit"));
	localizationKeyTextLineEdit->setEnabled(false);
	localizationKeyTextLineEdit->setGeometry(QRect(10, 95, 281, 22));
	localizationKeyTextLineEdit->setReadOnly(true);

	ui->fontNameLabel->setGeometry(QRect(10, 135, 31, 16));
	ui->fontSizeSpinBox->setGeometry(QRect(234, 131, 57, 25));
	ui->fontSelectButton->setGeometry(QRect(50, 126, 181, 38));
	ui->fontColorLabel->setGeometry(QRect(10, 180, 71, 16));
	ui->textColorPushButton->setGeometry(QRect(85, 178, 205, 21));
	ui->shadowOffsetLabel->setGeometry(QRect(10, 212, 91, 16));
	ui->offsetYLabel->setGeometry(QRect(210, 212, 16, 16));
	ui->shadowColorLabel->setGeometry(QRect(10, 242, 91, 16));
	ui->shadowOffsetXSpinBox->setGeometry(QRect(140, 208, 57, 25));
	ui->shadowOffsetYSpinBox->setGeometry(QRect(230, 208, 57, 25));
	ui->shadowColorButton->setGeometry(QRect(105, 240, 185, 21));
	ui->offsetXLabel->setGeometry(QRect(120, 212, 16, 16));
	ui->AlignLabel->setGeometry(QRect(10, 276, 62, 16));
	ui->alignComboBox->setGeometry(QRect(80, 270, 209, 26));
}

void TextPropertyGridWidget::Cleanup()
{
    UnregisterGridWidgetAsStateAware();
    UnregisterLineEditWidget(localizationKeyNameLineEdit);
    UITextFieldPropertyGridWidget::Cleanup();
}

void TextPropertyGridWidget::UpdateLocalizationValue()
{
    if ( !this->activeMetadata )
    {
        return;
    }
    
    // Key is known now - determine and set the value.
    QString localizationKey = this->localizationKeyNameLineEdit->text();
    WideString localizationValue = LocalizationSystem::Instance()->GetLocalizedString(QStrint2WideString(localizationKey));
	this->localizationKeyTextLineEdit->setText(WideString2QStrint(localizationValue));
    
    // Also update the "dirty" style for the "Value"
    PROPERTYGRIDWIDGETSITER iter = this->propertyGridWidgetsMap.find(this->localizationKeyNameLineEdit);
    if (iter != this->propertyGridWidgetsMap.end())
    {
        UpdateWidgetPalette(this->localizationKeyTextLineEdit, iter->second.getProperty().name());
    }
}

void TextPropertyGridWidget::HandleSelectedUIControlStatesChanged(const Vector<UIControl::eControlState>& newStates)
{
    // When the UI Control State is changed. we need to update Localization Key/Value.
    BasePropertyGridWidget::HandleSelectedUIControlStatesChanged(newStates);
    UpdateLocalizationValue();
}

void TextPropertyGridWidget::HandleChangePropertySucceeded(const QString& propertyName)
{
    BasePropertyGridWidget::HandleChangePropertySucceeded(propertyName);
    
    if (IsWidgetBoundToProperty(this->localizationKeyNameLineEdit, propertyName))
    {
        // Localization Key is updated - update the value.
        UpdateLocalizationValue();
    }
}

void TextPropertyGridWidget::HandleChangePropertyFailed(const QString& propertyName)
{
    BasePropertyGridWidget::HandleChangePropertyFailed(propertyName);

    if (IsWidgetBoundToProperty(this->localizationKeyNameLineEdit, propertyName))
    {
        // Localization Key is updated - update the value.
        UpdateLocalizationValue();
    }
}
