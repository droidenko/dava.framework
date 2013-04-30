#include "SetSwitchIndexView.h"
#include "ui_SetSwitchIndexView.h"
#include <stdlib.h> 
#include "Qt/Project/ProjectManager.h"
#include "Qt/Main/QtMainWindowHandler.h"
#include "SceneEditor/EditorConfig.h"

SetSwitchIndexView::SetSwitchIndexView(QWidget* parent)
:	QWidget(parent),
	ui(new Ui::SetSwitchIndexView)
{
	ui->setupUi(this);
	
	Init();
}

SetSwitchIndexView::~SetSwitchIndexView()
{
	delete ui;
}

void SetSwitchIndexView::Init()
{
	QtMainWindowHandler* handler = QtMainWindowHandler::Instance();
	connect(this, SIGNAL(Clicked(DAVA::uint32, DAVA::SetSwitchIndexHelper::eSET_SWITCH_INDEX)), handler, SLOT(ToggleSetSwitchIndex(DAVA::uint32, DAVA::SetSwitchIndexHelper::eSET_SWITCH_INDEX)));
	connect(ui->btnOK, SIGNAL(clicked()), this, SLOT(Clicked()));
	
	ui->btnOK->blockSignals(true);
	QtMainWindowHandler::Instance()->RegisterSetSwitchIndexWidgets(ui->spinBox,
		ui->rBtnSelection,
		ui->rBtnScene,
		ui->btnOK);

	handler->SetSwitchIndexWidgetsState(true);
	
}

void SetSwitchIndexView::Clicked()
{
	uint32 value = ui->spinBox->value();
	SetSwitchIndexHelper::eSET_SWITCH_INDEX state;
	if ( ui->rBtnSelection->isChecked())
	{
		state = SetSwitchIndexHelper::FOR_SELECTED;
	}
	else
	{
		state = SetSwitchIndexHelper::FOR_SCENE;
	}

	emit Clicked(value, state);
}
