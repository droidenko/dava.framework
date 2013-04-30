#include "CustomColorsView.h"
#include "ui_CustomColorsView.h"

#include "Qt/Project/ProjectManager.h"
#include "Qt/Main/QtMainWindowHandler.h"
#include "SceneEditor/EditorConfig.h"

CustomColorsView::CustomColorsView(QWidget* parent)
:	QWidget(parent),
	ui(new Ui::CustomColorsView)
{
	ui->setupUi(this);
	
	Init();
}

CustomColorsView::~CustomColorsView()
{
	delete ui;
}

void CustomColorsView::Init()
{
	QObject::connect(ProjectManager::Instance(), SIGNAL(ProjectOpened(const QString &)), this, SLOT(ProjectOpened(const QString &)));

	QtMainWindowHandler* handler = QtMainWindowHandler::Instance();
	connect(ui->buttonCustomColorsEnable, SIGNAL(clicked()), handler, SLOT(ToggleCustomColors()));

	ui->buttonCustomColorsSave->blockSignals(true);
	ui->sliderCustomColorBrushSize->blockSignals(true);
	ui->comboboxCustomColors->blockSignals(true);

	connect(ui->buttonCustomColorsSave, SIGNAL(clicked()), handler, SLOT(SaveTextureCustomColors()));
	connect(ui->sliderCustomColorBrushSize, SIGNAL(valueChanged(int)), handler, SLOT(ChangeBrushSizeCustomColors(int)));
	connect(ui->comboboxCustomColors, SIGNAL(currentIndexChanged(int)), handler, SLOT(ChangeColorCustomColors(int)));
	connect(ui->buttonCustomColorsLoad, SIGNAL(clicked()), handler, SLOT(LoadTextureCustomColors()));

	QtMainWindowHandler::Instance()->RegisterCustomColorsWidgets(ui->buttonCustomColorsEnable,
																 ui->buttonCustomColorsSave,
																 ui->sliderCustomColorBrushSize,
																 ui->comboboxCustomColors,
																 ui->buttonCustomColorsLoad);

	handler->SetCustomColorsWidgetsState(false);
}

void CustomColorsView::InitColors()
{
	QSize iconSize = ui->comboboxCustomColors->iconSize();
	iconSize = iconSize.expandedTo(QSize(100, 0));
	ui->comboboxCustomColors->setIconSize(iconSize);

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
		ui->comboboxCustomColors->addItem(icon, description.c_str());
	}
}

void CustomColorsView::ProjectOpened(const QString& path)
{
	InitColors();
}
