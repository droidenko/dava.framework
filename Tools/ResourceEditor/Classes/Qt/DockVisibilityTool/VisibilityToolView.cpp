#include "VisibilityToolView.h"
#include "ui_VisibilityToolView.h"

#include "Qt/Main/QtMainWindowHandler.h"

VisibilityToolView::VisibilityToolView(QWidget* parent)
:	QWidget(parent),
	ui(new Ui::VisibilityToolView)
{
	ui->setupUi(this);
	
	Init();
}

VisibilityToolView::~VisibilityToolView()
{
	delete ui;
}

void VisibilityToolView::Init()
{
	QtMainWindowHandler* handler = QtMainWindowHandler::Instance();

	connect(ui->buttonVisibilityToolEnable, SIGNAL(clicked()), handler, SLOT(ToggleVisibilityTool()));

	ui->buttonVisibilityToolSave->blockSignals(true);
	ui->buttonVisibilityToolSetArea->blockSignals(true);
	ui->buttonVisibilityToolSetPoint->blockSignals(true);
	ui->sliderVisibilityToolAreaSize->blockSignals(true);

	connect(ui->buttonVisibilityToolSave,		SIGNAL(clicked()),
			handler,							SLOT(SaveTextureVisibilityTool()));
	connect(ui->buttonVisibilityToolSetArea,	SIGNAL(clicked()),
			handler,							SLOT(SetVisibilityAreaVisibilityTool()));
	connect(ui->buttonVisibilityToolSetPoint,	SIGNAL(clicked()),
			handler,							SLOT(SetVisibilityPointVisibilityTool()));
	connect(ui->sliderVisibilityToolAreaSize,	SIGNAL(valueChanged(int)),
			handler,							SLOT(ChangleAreaSizeVisibilityTool(int)));

	handler->RegisterWidgetsVisibilityTool(ui->buttonVisibilityToolEnable,
										   ui->buttonVisibilityToolSave,
										   ui->buttonVisibilityToolSetPoint,
										   ui->buttonVisibilityToolSetArea,
										   ui->sliderVisibilityToolAreaSize);

	handler->SetWidgetsStateVisibilityTool(false);
}
