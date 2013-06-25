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

#ifndef __ResourceEditorQt__ParticleTimeLineWidget__
#define __ResourceEditorQt__ParticleTimeLineWidget__

#include <QWidget>
#include <QDialog>
#include <QLabel>
#include <QTimer>

#include <DAVAEngine.h>
#include "ScrollZoomWidget.h"
#include "../CustomControls/EventFilterDoubleSpinBox.h"

using namespace DAVA;

class ParticlesCountWidget;
class ParticlesAreaWidget;
class ParticleTimeLineWidget : public ScrollZoomWidget
{
	Q_OBJECT
	friend class ParticlesExtraInfoWidget;

public:
	explicit ParticleTimeLineWidget(QWidget *parent = 0);
	~ParticleTimeLineWidget();
	
	void Init(float32 minTime, float32 maxTime);

	struct LINE
	{
		float32 startTime;
		float32 endTime;
		QColor color;
		QString legend;
		ParticleLayer* layer;
	};
	typedef DAVA::Map<uint32, LINE> LINE_MAP;

signals:
	void ChangeVisible(bool visible);
	
protected slots:
	void OnNodeSelected(Entity* node);
	void OnEffectNodeSelected(Entity* node);
	void OnLayerSelected(Entity* node, ParticleLayer* layer);
	void OnUpdate();

	void OnUpdateLayersExtraInfoNeeded();

protected:
	virtual void paintEvent(QPaintEvent *);
	virtual void mouseMoveEvent(QMouseEvent *);
	virtual void mousePressEvent(QMouseEvent *);
	virtual void mouseReleaseEvent(QMouseEvent *);
	virtual void mouseDoubleClickEvent(QMouseEvent *);

private:
	bool GetLineRect(uint32 id, QRect& startPoint, QRect& endPoint) const;
	QRect GetGraphRect() const;
	QPoint GetPoint(const QPoint&) const;
	
	void AddLayerLine(uint32 layerLineID, float32 minTime, float32 maxTime,
					  const QColor& layerColor, ParticleLayer* layer);
	void AddLine(uint32 lineId, float32 startTime, float32 endTime, const QColor& color, const QString& legend, ParticleLayer* layer);

	void OnValueChanged(int lineId);
	void UpdateSizePolicy();

	void UpdateLayersExtraInfoPosition();
	void UpdateLayersExtraInfoValues();

	// Handle situation when the Particle Emitter Node is selected (including
	// case when separate Layer node is selected.
	void HandleNodeSelected(Entity* node, ParticleLayer* layer);
	
	virtual QRect GetSliderRect() const;
	virtual QRect GetIncreaseRect() const;
	virtual QRect GetScaleRect() const;
	virtual QRect GetDecreaseRect() const;

private:
	// Get the width/height for particle counter label.
	void GetParticlesCountWidthHeight(const LINE& line, int32& width, int32& height);

	LINE_MAP lines;
	QFont nameFont;
	
	QPoint selectedPoint;
	Entity* emitterNode;
	Entity* effectNode;
	ParticleLayer* selectedLayer;
	
	QTimer updateTimer;
	ParticlesCountWidget* countWidget;
	ParticlesAreaWidget* areaWidget;

	class SetPointValueDlg: public QDialog
	{
		//Q_OBJECT
		
	public:
		explicit SetPointValueDlg(float32 value, float32 minValue, float32 maxValue, QWidget *parent = 0);
		
		float32 GetValue() const;
		
	private:
		EventFilterDoubleSpinBox* valueSpin;
	};
};

////////////////////////////////////////////////////////////////////////////////////
// A specific class to display the per-layer particles info.
const int32 EXTRA_INFO_LEFT_PADDING = 3;

class ParticlesExtraInfoWidget : public QWidget
{
	Q_OBJECT
	
public:
	explicit ParticlesExtraInfoWidget(const ParticleTimeLineWidget* timeLineWidget,
									  QWidget *parent = 0);

protected:
	virtual void paintEvent(QPaintEvent *);
	
	// These methods are to be overriden for derived classes.
	// Get an extra information to be displayed near the line.
	virtual QString GetExtraInfoForLayerLine(const ParticleTimeLineWidget::LINE& line) {return QString();};

	// Get the extra info for the header/footer.
	virtual QString GetExtraInfoHeader() {return QString(); };
	virtual QString GetExtraInfoFooter() {return QString(); };

	// In case some information should be accumulated during the loop,
	// these methods are called just before the loop and just after it.
	virtual void OnBeforeGetExtraInfoLoop() {};
	virtual void OnAfterGetExtraInfoLoop() {};

	// The timeline widget being used.
	const ParticleTimeLineWidget* timeLineWidget;
};

// Particles Count information.
class ParticlesCountWidget : public ParticlesExtraInfoWidget
{
	Q_OBJECT
public:
	explicit ParticlesCountWidget(const ParticleTimeLineWidget* timeLineWidget,
									  QWidget *parent = 0);
protected:
	virtual void OnBeforeGetExtraInfoLoop();
	virtual QString GetExtraInfoForLayerLine(const ParticleTimeLineWidget::LINE& line);

	virtual QString GetExtraInfoHeader();
	virtual QString GetExtraInfoFooter();

private:
	int32 totalParticlesCount;
};

// Particles Area information.
class ParticlesAreaWidget : public ParticlesExtraInfoWidget
{
	Q_OBJECT
public:
	explicit ParticlesAreaWidget(const ParticleTimeLineWidget* timeLineWidget,
								 QWidget *parent = 0);
protected:
	virtual void OnBeforeGetExtraInfoLoop();
	virtual QString GetExtraInfoForLayerLine(const ParticleTimeLineWidget::LINE& line);
	
	virtual QString GetExtraInfoHeader();
	virtual QString GetExtraInfoFooter();
	
private:
	QString FormatFloat(float32 value);

	float32 totalParticlesArea;
};

#endif /* defined(__ResourceEditorQt__ParticleTimeLineWidget__) */
