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


#include "LodCorrectionDialog.h"

#include "ui_LodCorrectionDialog.h"

#include "Tools/EventFilterDoubleSpinBox/EventFilterDoubleSpinBox.h"
#include "DAVAEngine.h"

using namespace DAVA;

static EventFilterDoubleSpinBox *spinBoxes[DAVA::LodComponent::MAX_LOD_LAYERS];

LodCorrectionDialog::LodCorrectionDialog(DAVA::Scene *_scene, QWidget *parent)
	: QDialog(parent)
	, ui(new Ui::LodCorrectionDialog)
{
    ui->setupUi(this);
    
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowCloseButtonHint | Qt::CustomizeWindowHint);
    setWindowModality(Qt::ApplicationModal);
    setModal(true);

    spinBoxes[0] = ui->lod0Correction;
    spinBoxes[1] = ui->lod1Correction;
    spinBoxes[2] = ui->lod2Correction;
    spinBoxes[3] = ui->lod3Correction;

    for(int32 i = 0; i < DAVA::LodComponent::MAX_LOD_LAYERS; ++i)
    {
        connect(spinBoxes[i], SIGNAL(valueChanged(double)), SLOT(valueChanged(double)));
    }

	posSaver.Attach(this);
    
    scene = NULL;
    InitializeWithScene(_scene);
}

LodCorrectionDialog::~LodCorrectionDialog()
{
    SafeRelease(scene);
    delete ui;
}

void LodCorrectionDialog::InitializeWithScene(DAVA::Scene *_scene)
{
    DVASSERT(_scene);
    
    SafeRelease(scene);
    scene = SafeRetain(_scene);
    
    const Vector<float32> & lodCorrection = scene->GetLodLayersCorrection();
    for(uint32 i = 0; i < DAVA::LodComponent::MAX_LOD_LAYERS; ++i)
    {
        spinBoxes[i]->setValue(lodCorrection[i] * 100.f); //convert to persents
        UpdateFontColor(spinBoxes[i]);
    }
}

void LodCorrectionDialog::accept()
{
    if(scene)
    {
        for(uint32 i = 0; i < DAVA::LodComponent::MAX_LOD_LAYERS; ++i)
        {
            scene->SetLodLayersCorrection(spinBoxes[i]->value() / 100.f, i); //convert from persents
        }
        
        scene->ApplyLodLayerCorrection();
        
        SafeRelease(scene);
    }

    QDialog::accept();
}

void LodCorrectionDialog::reject()
{
    SafeRelease(scene);

    QDialog::reject();
}


void LodCorrectionDialog::valueChanged(double value)
{
    QDoubleSpinBox *spinBox = dynamic_cast<QDoubleSpinBox *>(sender());
    if(spinBox)
    {
        UpdateFontColor(spinBox);
    }
}


void LodCorrectionDialog::UpdateFontColor(QDoubleSpinBox *spinBox)
{
    QPalette* palette = new QPalette();
    
    if(spinBox->value() > 0)
    {
        palette->setColor(QPalette::Text,Qt::red);
    }
    else
    {
        palette->setColor(QPalette::Text,Qt::green);
    }
    
    
    spinBox->setPalette(*palette);
}

