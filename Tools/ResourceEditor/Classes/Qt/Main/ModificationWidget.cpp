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

#include "ModificationWidget.h"
#include "ui_ModificationWidget.h"

#include <QKeyEvent>

ModificationWidget::ModificationWidget(QWidget* parent)
:	QWidget(parent),
ui(new Ui::ModificationWidget)
{
	ui->setupUi(this);

	connect(ui->xAxisModify, SIGNAL(editingFinished()), this, SLOT(OnEditingFinished()));
	connect(ui->yAxisModify, SIGNAL(editingFinished()), this, SLOT(OnEditingFinished()));
	connect(ui->zAxisModify, SIGNAL(editingFinished()), this, SLOT(OnEditingFinished()));
}

ModificationWidget::~ModificationWidget()
{
	delete ui;
}

void ModificationWidget::OnEditingFinished()
{
	double x = ui->xAxisModify->value();
	double y = ui->yAxisModify->value();
	double z = ui->zAxisModify->value();

	if (x != 0.f || y != 0.f || z != 0.f)
	{
		emit ApplyModification(x, y, z);
		ResetSpinBoxes();
	}
}

void ModificationWidget::ResetSpinBoxes()
{
	this->blockSignals(true);
	ui->xAxisModify->setValue(0.0);
	ui->yAxisModify->setValue(0.0);
	ui->zAxisModify->setValue(0.0);
	this->blockSignals(false);
}

void ModificationWidget::keyPressEvent(QKeyEvent* event)
{
	if(event->key() == Qt::Key_Escape)
	{
		ResetSpinBoxes();
	}
}
