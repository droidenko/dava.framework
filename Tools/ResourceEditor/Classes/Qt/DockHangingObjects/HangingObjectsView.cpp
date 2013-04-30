#include "HangingObjectsView.h"
#include "ui_HangingObjectsView.h"
#include <stdlib.h> 
#include "Qt/Project/ProjectManager.h"
#include "Qt/Main/QtMainWindowHandler.h"
#include "SceneEditor/EditorConfig.h"

HangingObjectsView::HangingObjectsView(QWidget* parent)
:	QWidget(parent),
	ui(new Ui::HangingObjectsView)
{
	ui->setupUi(this);
	
	Init();
}

HangingObjectsView::~HangingObjectsView()
{
	delete ui;
}

void HangingObjectsView::Init()
{
	QtMainWindowHandler* handler = QtMainWindowHandler::Instance();
	connect(this, SIGNAL(Clicked(float,bool)), handler, SLOT(ToggleHangingObjects(float,bool)));
	connect(ui->btnUpdate, SIGNAL(clicked()), this, SLOT(Clicked()));
	connect(ui->checkBoxEnable, SIGNAL(stateChanged(int)), this, SLOT(CheckBoxChangeState(int )));
	
	ui->btnUpdate->blockSignals(true);
	QtMainWindowHandler::Instance()->RegisterHangingObjectsWidgets(ui->checkBoxEnable,
		ui->doubleSpinBoxHeight,
		ui->btnUpdate);

	handler->SetHangingObjectsWidgetsState(false);
	ui->checkBoxEnable->setEnabled(true);
}


void HangingObjectsView::Clicked()
{
	float value = (float)ui->doubleSpinBoxHeight->value();
	emit Clicked(value, ui->checkBoxEnable->isChecked());
}

void HangingObjectsView::CheckBoxChangeState(int newState)
{
	if(newState == Qt::Unchecked)
	{
		ui->doubleSpinBoxHeight->setEnabled(false);
		ui->btnUpdate->setEnabled(false);
	}
	if(newState == Qt::Checked)
	{
		ui->doubleSpinBoxHeight->setEnabled(true);
		ui->btnUpdate->setEnabled(true);
	}
	this->Clicked();
}
