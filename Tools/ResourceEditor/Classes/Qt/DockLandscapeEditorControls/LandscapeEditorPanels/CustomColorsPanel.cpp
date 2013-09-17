#include "CustomColorsPanel.h"
#include "../../Scene/SceneSignals.h"
#include "../../Scene/SceneEditor2.h"
#include "../../SliderWidget/SliderWidget.h"
#include "../../../SceneEditor/EditorConfig.h"
#include "Project/ProjectManager.h"
#include "Constants.h"
#include "Main/QtUtils.h"

#include <QLayout>
#include <QComboBox>
#include <QPushButton>
#include <QSpacerItem>
#include <QFileDialog>

CustomColorsPanel::CustomColorsPanel(QWidget* parent)
:	LandscapeEditorBasePanel(parent)
,	comboColor(NULL)
,	sliderWidgetBrushSize(NULL)
,	buttonSaveTexture(NULL)
,	buttonLoadTexture(NULL)
{
	InitUI();
	ConnectToSignals();
}

CustomColorsPanel::~CustomColorsPanel()
{
}

bool CustomColorsPanel::GetEditorEnabled()
{
	return GetActiveScene()->customColorsSystem->IsLandscapeEditingEnabled();
}

void CustomColorsPanel::SetWidgetsState(bool enabled)
{
	comboColor->setEnabled(enabled);
	sliderWidgetBrushSize->setEnabled(enabled);
	buttonSaveTexture->setEnabled(enabled);
	buttonLoadTexture->setEnabled(enabled);
}

void CustomColorsPanel::BlockAllSignals(bool block)
{
	comboColor->blockSignals(block);
	sliderWidgetBrushSize->blockSignals(block);
	buttonSaveTexture->blockSignals(block);
	buttonLoadTexture->blockSignals(block);
}

void CustomColorsPanel::StoreState()
{
	KeyedArchive* customProperties = GetActiveScene()->GetCustomProperties();
	if (customProperties)
	{
		customProperties->SetInt32(ResourceEditor::CUSTOM_COLORS_BRUSH_SIZE_MIN, sliderWidgetBrushSize->GetRangeMin());
		customProperties->SetInt32(ResourceEditor::CUSTOM_COLORS_BRUSH_SIZE_MAX, sliderWidgetBrushSize->GetRangeMax());
	}
}

void CustomColorsPanel::RestoreState()
{
	SceneEditor2* sceneEditor = GetActiveScene();
	
	bool enabled = sceneEditor->customColorsSystem->IsLandscapeEditingEnabled();
	int32 brushSize = BrushSizeSystemToUI(sceneEditor->customColorsSystem->GetBrushSize());
	int32 colorIndex = sceneEditor->customColorsSystem->GetColor();
	
	int32 brushRangeMin = DEF_BRUSH_MIN_SIZE;
	int32 brushRangeMax = DEF_BRUSH_MAX_SIZE;
	
	KeyedArchive* customProperties = sceneEditor->GetCustomProperties();
	if (customProperties)
	{
		brushRangeMin = customProperties->GetInt32(ResourceEditor::CUSTOM_COLORS_BRUSH_SIZE_MIN, DEF_BRUSH_MIN_SIZE);
		brushRangeMax = customProperties->GetInt32(ResourceEditor::CUSTOM_COLORS_BRUSH_SIZE_MAX, DEF_BRUSH_MAX_SIZE);
	}
	
	SetWidgetsState(enabled);
	
	BlockAllSignals(true);
	sliderWidgetBrushSize->SetRangeMin(brushRangeMin);
	sliderWidgetBrushSize->SetRangeMax(brushRangeMax);
	sliderWidgetBrushSize->SetValue(brushSize);
	comboColor->setCurrentIndex(colorIndex);
	BlockAllSignals(!enabled);
}

void CustomColorsPanel::InitUI()
{
	QVBoxLayout* layout = new QVBoxLayout(this);
	
	comboColor = new QComboBox(this);
	sliderWidgetBrushSize = new SliderWidget(this);
	buttonSaveTexture = new QPushButton(this);
	buttonLoadTexture = new QPushButton(this);
	QSpacerItem* spacer = new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Expanding);
	
	layout->addWidget(comboColor);
	layout->addWidget(sliderWidgetBrushSize);
	layout->addWidget(buttonLoadTexture);
	layout->addWidget(buttonSaveTexture);
	layout->addSpacerItem(spacer);
	
	setLayout(layout);
	
	SetWidgetsState(false);
	BlockAllSignals(true);
	
	sliderWidgetBrushSize->Init(ResourceEditor::CUSTOM_COLORS_BRUSH_SIZE_CAPTION.c_str(),
								false, DEF_BRUSH_MAX_SIZE, DEF_BRUSH_MIN_SIZE, DEF_BRUSH_MIN_SIZE);
	buttonSaveTexture->setText(ResourceEditor::CUSTOM_COLORS_SAVE_CAPTION.c_str());
	buttonLoadTexture->setText(ResourceEditor::CUSTOM_COLORS_LOAD_CAPTION.c_str());
}

void CustomColorsPanel::ConnectToSignals()
{
	connect(ProjectManager::Instance(), SIGNAL(ProjectOpened(const QString &)), this, SLOT(ProjectOpened(const QString &)));

	connect(SceneSignals::Instance(), SIGNAL(CustomColorsTextureShouldBeSaved(SceneEditor2*)),
			this, SLOT(NeedSaveCustomColorsTexture(SceneEditor2*)));
	connect(SceneSignals::Instance(), SIGNAL(CustomColorsToggled(SceneEditor2*)),
			this, SLOT(EditorToggled(SceneEditor2*)));

	connect(sliderWidgetBrushSize, SIGNAL(ValueChanged(int)), this, SLOT(SetBrushSize(int)));
	connect(comboColor, SIGNAL(currentIndexChanged(int)), this, SLOT(SetColor(int)));
	connect(buttonSaveTexture, SIGNAL(clicked()), this, SLOT(SaveTexture()));
	connect(buttonLoadTexture, SIGNAL(clicked()), this, SLOT(LoadTexture()));
}

void CustomColorsPanel::ProjectOpened(const QString &path)
{
	InitColors();
}

void CustomColorsPanel::InitColors()
{
	comboColor->clear();
	
	QSize iconSize = comboColor->iconSize();
	iconSize = iconSize.expandedTo(QSize(100, 0));
	comboColor->setIconSize(iconSize);
	
	Vector<Color> customColors = EditorConfig::Instance()->GetColorPropertyValues(ResourceEditor::CUSTOM_COLORS_PROPERTY_COLORS);
	Vector<String> customColorsDescription = EditorConfig::Instance()->GetComboPropertyValues(ResourceEditor::CUSTOM_COLORS_PROPERTY_DESCRIPTION);
	for(size_t i = 0; i < customColors.size(); ++i)
	{
		QColor color = QColor::fromRgbF(customColors[i].r, customColors[i].g, customColors[i].b, customColors[i].a);
		
		QImage image(iconSize, QImage::Format_ARGB32);
		image.fill(color);
		
		QPixmap pixmap(iconSize);
		pixmap.convertFromImage(image, Qt::ColorOnly);
		
		QIcon icon(pixmap);
		String description = (i >= customColorsDescription.size()) ? "" : customColorsDescription[i];
		comboColor->addItem(icon, description.c_str());
	}
}

// these functions are designed to convert values from sliders in ui
// to the values suitable for custom colors system
int32 CustomColorsPanel::BrushSizeUIToSystem(int32 uiValue)
{
	int32 systemValue = uiValue * ResourceEditor::LANDSCAPE_BRUSH_SIZE_UI_TO_SYSTEM_COEF;
	return systemValue;
}

int32 CustomColorsPanel::BrushSizeSystemToUI(int32 systemValue)
{
	int32 uiValue = systemValue / ResourceEditor::LANDSCAPE_BRUSH_SIZE_UI_TO_SYSTEM_COEF;
	return uiValue;
}
// end of convert functions ==========================

void CustomColorsPanel::SetBrushSize(int brushSize)
{
	GetActiveScene()->customColorsSystem->SetBrushSize(BrushSizeUIToSystem(brushSize));
}

void CustomColorsPanel::SetColor(int color)
{
	GetActiveScene()->customColorsSystem->SetColor(color);
}

void CustomColorsPanel::SaveTexture()
{
	SceneEditor2* sceneEditor = GetActiveScene();
	
	FilePath selectedPathname = sceneEditor->customColorsSystem->GetCurrentSaveFileName();
	if (selectedPathname.IsEmpty())
	{
		selectedPathname = sceneEditor->GetScenePath().GetDirectory();
	}
	
	QString filePath = QFileDialog::getSaveFileName(NULL, QString(ResourceEditor::CUSTOM_COLORS_SAVE_CAPTION.c_str()),
													QString(selectedPathname.GetAbsolutePathname().c_str()),
													QString(ResourceEditor::CUSTOM_COLORS_FILE_FILTER.c_str()));
	selectedPathname = PathnameToDAVAStyle(filePath);
	
	if (!selectedPathname.IsEmpty())
	{
		sceneEditor->customColorsSystem->SaveTexture(selectedPathname);
	}
}

void CustomColorsPanel::LoadTexture()
{
	SceneEditor2* sceneEditor = GetActiveScene();
	
	FilePath currentPath = sceneEditor->customColorsSystem->GetCurrentSaveFileName();
	if (currentPath.IsEmpty())
	{
		currentPath = sceneEditor->GetScenePath().GetDirectory();
	}
	
	FilePath selectedPathname = GetOpenFileName(ResourceEditor::CUSTOM_COLORS_LOAD_CAPTION,
												currentPath,
												ResourceEditor::CUSTOM_COLORS_FILE_FILTER);
	if(!selectedPathname.IsEmpty())
	{
		sceneEditor->customColorsSystem->LoadTexture(selectedPathname);
	}
}

void CustomColorsPanel::NeedSaveTexture(SceneEditor2* scene)
{
	if (scene != GetActiveScene())
	{
		return;
	}

	FilePath selectedPathname = scene->customColorsSystem->GetCurrentSaveFileName();
	if(!selectedPathname.IsEmpty())
	{
		scene->customColorsSystem->SaveTexture(selectedPathname);
	}
	else
	{
		SaveTexture();
	}
}