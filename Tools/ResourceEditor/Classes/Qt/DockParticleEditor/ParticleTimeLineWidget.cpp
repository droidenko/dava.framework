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

#include "ParticleTimeLineWidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QPushButton>
#include "Commands/ParticleEditorCommands.h"
#include "Commands/CommandsManager.h"
#include "../Scene/SceneDataManager.h"

#include "ParticlesEditorController.h"

#define LEFT_INDENT 20
#define TOP_INDENT 14
#define BOTTOM_INDENT 24
#define RIGHT_INDENT 104
#define LINE_STEP 16
#define RECT_SIZE 3
#define LINE_WIDTH 3

#define PARTICLES_INFO_CONTROL_OFFSET 8
#define PARTICLES_INFO_CONTROL_WIDTH 50

#define UPDATE_LAYERS_EXTRA_INFO_PERIOD 250 // in milliseconds

ParticleTimeLineWidget::ParticleTimeLineWidget(QWidget *parent/* = 0*/) :
	ScrollZoomWidget(parent),
	selectedPoint(-1, -1),
	emitterNode(NULL),
	effectNode(NULL),
	selectedLayer(NULL),
#ifdef Q_WS_WIN
	nameFont("Courier", 8, QFont::Normal)
#else
	nameFont("Courier", 12, QFont::Normal)
#endif
{
	backgroundBrush.setColor(Qt::white);
	backgroundBrush.setStyle(Qt::SolidPattern);
	
	gridStyle = GRID_STYLE_LIMITS;
	
	connect(ParticlesEditorController::Instance(),
			SIGNAL(EmitterSelected(Entity*, BaseParticleEditorNode*)),
			this,
			SLOT(OnNodeSelected(Entity*)));
	connect(ParticlesEditorController::Instance(),
			SIGNAL(LayerSelected(Entity*, ParticleLayer*, BaseParticleEditorNode*, bool)),
			this,
			SLOT(OnLayerSelected(Entity*, ParticleLayer*)));
	connect(ParticlesEditorController::Instance(),
			SIGNAL(ForceSelected(Entity*, ParticleLayer*, int32, BaseParticleEditorNode*)),
			this,
			SLOT(OnNodeSelected(Entity*)));

	connect(ParticlesEditorController::Instance(),
			SIGNAL(EffectSelected(Entity*)),
			this,
			SLOT(OnEffectNodeSelected(Entity*)));
	
	Init(0, 0);
	
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

	// Init and start updating the particles grid.
	this->countWidget = new ParticlesCountWidget(this, this);
	this->areaWidget = new ParticlesAreaWidget(this, this);

    connect(&updateTimer, SIGNAL(timeout()),
			this, SLOT(OnUpdateLayersExtraInfoNeeded()));
    updateTimer.start(UPDATE_LAYERS_EXTRA_INFO_PERIOD);
}

ParticleTimeLineWidget::~ParticleTimeLineWidget()
{
	SafeDelete(this->countWidget);
	SafeDelete(this->areaWidget);

	updateTimer.stop();
}

void ParticleTimeLineWidget::OnLayerSelected(Entity* node, ParticleLayer* layer)
{
	if (!node || !layer)
	{
		emit ChangeVisible(false);
		return;
	}
	
	HandleNodeSelected(node, layer);
}

void ParticleTimeLineWidget::OnNodeSelected(Entity* node)
{
	HandleNodeSelected(node, NULL);
}

void ParticleTimeLineWidget::HandleNodeSelected(Entity* node, ParticleLayer* layer)
{
	emitterNode = node;
	selectedLayer = layer;
	effectNode = NULL;
	
	float32 minTime = 0;
	float32 maxTime = 0;
	ParticleEmitter* emitter = NULL;
	if (node)
	{
		emitter = GetEmitter(node);
		if (!emitter)
		{
		    return;
		}

		maxTime = emitter->GetLifeTime();
	}

	Init(minTime, maxTime);
	if (emitter)
	{
		QColor colors[3] = {Qt::blue, Qt::darkGreen, Qt::red};
		uint32 colorsCount = sizeof(colors) / sizeof(*colors);

		if (!layer)
		{
			// No particular layer specified - add all ones.
			const Vector<ParticleLayer*> & layers = emitter->GetLayers();
			for (uint32 i = 0; i < layers.size(); ++i)
			{
				AddLayerLine(i, minTime, maxTime, colors[i % colorsCount], layers[i]);
			}
		}
		else
		{
			// Add the particular layer only.
			int layerIndex = 0;
			const Vector<ParticleLayer*> & layers = emitter->GetLayers();
			for (uint32 i = 0; i < layers.size(); i ++)
			{
				if (layers[i] == layer)
				{
					layerIndex = i;
					break;
				}
			}

			AddLayerLine(layerIndex, minTime, maxTime, colors[layerIndex % colorsCount], layer);
		}
	}

	UpdateSizePolicy();
	UpdateLayersExtraInfoPosition();

	if (lines.size())
	{
		emit ChangeVisible(true);
		update();
	}
	else
	{
		emit ChangeVisible(false);
	}
}

QRect ParticleTimeLineWidget::GetSliderRect() const
{
	QRect rect = GetIncreaseRect();
	rect.translate(-(ZOOM_SLIDER_LENGTH + 5), 0);
	rect.setWidth(ZOOM_SLIDER_LENGTH);
	rect.setHeight(rect.height() + 4);
	return rect;
}

QRect ParticleTimeLineWidget::GetIncreaseRect() const
{
	QRect rect = GetScaleRect();
	rect.translate(-12, 0);
	rect.setWidth (8);
	rect.setHeight(8);
	return rect;
}

QRect ParticleTimeLineWidget::GetScaleRect() const
{
	QRect rect = GetScrollBarRect();
	rect.translate(-SCALE_WIDTH, 0);
	return rect;
}

QRect ParticleTimeLineWidget::GetDecreaseRect() const
{
	QRect rect = GetSliderRect();
	rect.translate(-12, 0);
	rect.setWidth (8);
	rect.setHeight(8);
	return rect;
}

void ParticleTimeLineWidget::OnEffectNodeSelected(Entity* node)
{
	emitterNode = NULL;
	effectNode = node;
	
	float32 minTime = 0;
	float32 maxTime = 0;
	if (node)
	{
		int32 count = node->GetChildrenCount();
		for (int32 i = 0; i < count; ++i)
		{
			Entity* emitterNode = dynamic_cast<Entity*>(node->GetChild(i));
			if (emitterNode)
			{
				ParticleEmitter * emitter = GetEmitter(emitterNode);
				if (!emitter)
				{
					continue;
				}

				if (emitter)
					maxTime = Max(maxTime, emitter->GetLifeTime());
			}
		}
	}
	Init(minTime, maxTime);
	if (node)
	{
		QColor colors[3] = {Qt::blue, Qt::darkGreen, Qt::red};
		int32 count = node->GetChildrenCount();
		int32 iLines = 0;
		for (int32 iEmitter = 0; iEmitter < count; ++iEmitter)
		{
			Entity* emitterNode = dynamic_cast<Entity*>(node->GetChild(iEmitter));
			if (emitterNode)
			{
				ParticleEmitter * emitter = GetEmitter(emitterNode);
				if (!emitter)
				{
					continue;
				}

				if (emitter)
				{
					const Vector<ParticleLayer*> & layers = emitter->GetLayers();
					for (uint32 iLayer = 0; iLayer < layers.size(); ++iLayer)
					{
						float32 startTime = Max(minTime, layers[iLayer]->startTime);
						float32 endTime = Min(maxTime, layers[iLayer]->endTime);
						AddLine(iLines, startTime, endTime, colors[iLines % 3],
								QString::fromStdString(layers[iLayer]->layerName), layers[iLayer]);
						iLines++;
					}
				}
			}
		}
	}
	
	UpdateSizePolicy();
	if (lines.size())
	{
		emit ChangeVisible(true);
		update();
	}
	else
	{
		emit ChangeVisible(false);
	}
}

void ParticleTimeLineWidget::Init(float32 minTime, float32 maxTime)
{
	ScrollZoomWidget::Init(minTime, maxTime);
	lines.clear();
}

void ParticleTimeLineWidget::AddLayerLine(uint32 layerLineID, float32 minTime, float32 maxTime,
										  const QColor& layerColor, ParticleLayer* layer)
{
	if (!layer)
	{
		return;
	}
	
	float32 startTime = Max(minTime, layer->startTime);
	float32 endTime = Min(maxTime, layer->endTime);
	
	AddLine(layerLineID, startTime, endTime, layerColor, QString::fromStdString(layer->layerName), layer);
}

void ParticleTimeLineWidget::AddLine(uint32 lineId, float32 startTime, float32 endTime, const QColor& color, const QString& legend, ParticleLayer* layer)
{
	LINE line;
	line.startTime = startTime;
	line.endTime = endTime;
	line.color = color;
	line.legend = legend;
	line.layer = layer;
	lines[lineId] = line;
}

void ParticleTimeLineWidget::paintEvent(QPaintEvent *e)
{
	QPainter painter(this);
	
	QFont font("Courier", 11, QFont::Normal);
	painter.setFont(font);
	
	painter.fillRect(this->rect(), backgroundBrush);
	
	QRect graphRect = GetGraphRect();
	
	//draw grid
	{
		painter.setPen(Qt::gray);
	
		float step = 18;
		float steps = graphRect.width() / step;
		float valueStep = (maxTime - minTime) / steps;
		bool drawText = false;
		for (int i = 0; i <= steps; i++)
		{
			int x = graphRect.left() + i * step;
			painter.drawLine(x, graphRect.top(), x, graphRect.bottom());
			drawText = !drawText;
			if (!drawText)
				continue;
			
			if (gridStyle == GRID_STYLE_ALL_POSITION)
			{
				float value = minTime + i * valueStep;
				QString strValue = float2QString(value);
				int textWidth = painter.fontMetrics().width(strValue);
				QRect textRect(x - textWidth / 2, graphRect.bottom(), textWidth, BOTTOM_INDENT);
				painter.drawText(textRect, Qt::AlignCenter, strValue);
			}
		}

		if (gridStyle == GRID_STYLE_LIMITS)
		{
			QRect textRect(graphRect.left(), graphRect.bottom(), graphRect.width(), BOTTOM_INDENT);
			painter.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, float2QString(minTime));
			painter.drawText(textRect, Qt::AlignRight | Qt::AlignVCenter, float2QString(maxTime));
		}
	}
	
	painter.setFont(nameFont);
	
	painter.setPen(Qt::black);
	painter.drawRect(graphRect);

	uint32 i = 0;
	for (LINE_MAP::const_iterator iter = lines.begin(); iter != lines.end(); ++iter, ++i)
	{
		const LINE& line = iter->second;
		
		QRect startRect;
		QRect endRect;
		bool drawStartRect = true;
		bool drawEndRect = true;
		GetLineRect(iter->first, startRect, endRect);
		ePositionRelativelyToDrawRect startPosition =  GetPointPositionFromDrawingRect(startRect.center());
		ePositionRelativelyToDrawRect endPosition =  GetPointPositionFromDrawingRect(endRect.center());
		if(startPosition == POSITION_LEFT)
		{
			drawStartRect = false;
			startRect.moveTo(graphRect.x() - RECT_SIZE,startRect.y());

		}else if (startPosition == POSITION_RIGHT)
		{
			drawStartRect = false;
			startRect.moveTo(graphRect.x() + graphRect.width() - RECT_SIZE, startRect.y());
		}

		if(endPosition == POSITION_LEFT)
		{
			drawEndRect = false;
			endRect.moveTo(graphRect.x() - RECT_SIZE, endRect.y());
		}else if (endPosition == POSITION_RIGHT)
		{
			drawEndRect = false;
			endRect.moveTo(graphRect.x() + graphRect.width() - RECT_SIZE, endRect.y());
		}

		painter.setPen(QPen(line.color, 1));
		painter.drawLine(QPoint(graphRect.left(), startRect.center().y()), QPoint(graphRect.right(), startRect.center().y()));
		
		int textMaxWidth = graphRect.left() - LEFT_INDENT - painter.fontMetrics().width("WW");
		QString legend;
		for (int i = 0; i < line.legend.length(); ++i)
		{
			legend += line.legend.at(i);
			int textWidth = painter.fontMetrics().width(legend);
			if (textWidth > textMaxWidth)
			{
				legend.remove(legend.length() - 3, 3);
				legend += "...";
				break;
			}
		}
		painter.drawText(QPoint(LEFT_INDENT, startRect.bottom()), legend);
	
		painter.setPen(QPen(line.color, LINE_WIDTH));
		if (selectedPoint.x() == iter->first)
		{
			QBrush brush(line.color);
			if (selectedPoint.y() == 0)
				painter.fillRect(startRect, brush);
			else
				painter.fillRect(endRect, brush);
		}
		
		QPoint startPoint(startRect.center());
		startPoint.setX(startPoint.x() + 3);
		QPoint endPoint(endRect.center());
		endPoint.setX(endPoint.x() - 3);

		if(drawStartRect)
		{
			painter.drawRect(startRect);
		}
		if(drawEndRect)
		{
			painter.drawRect(endRect);
		}
		if(!(startPosition == endPosition && startPosition != POSITION_INSIDE))
		{
			painter.drawLine(startPoint, endPoint);
		}

		UpdateLayersExtraInfoPosition();
	}

	ScrollZoomWidget::paintEvent(e, painter);
}

bool ParticleTimeLineWidget::GetLineRect(uint32 id, QRect& startRect, QRect& endRect) const
{
	uint32 i = 0;
	QRect grapRect = GetGraphRect();
	for (LINE_MAP::const_iterator iter = lines.begin(); iter != lines.end(); ++iter, ++i)
	{
		if (iter->first != id)
			continue;
		
		const LINE& line = iter->second;
		
		QPoint startPoint(grapRect.left() + (line.startTime - minTime) / (maxTime - minTime) * grapRect.width(), grapRect.top() + (i + 1) * LINE_STEP);
		QPoint endPoint(grapRect.left() + (line.endTime - minTime) / (maxTime - minTime) * grapRect.width(), grapRect.top() + (i + 1) * LINE_STEP);
		startRect = QRect(startPoint - QPoint(RECT_SIZE, RECT_SIZE), startPoint + QPoint(RECT_SIZE, RECT_SIZE));
		endRect = QRect(endPoint - QPoint(RECT_SIZE, RECT_SIZE), endPoint + QPoint(RECT_SIZE, RECT_SIZE));
		return true;
	}
	return false;
}

QRect ParticleTimeLineWidget::GetGraphRect() const
{
	QFontMetrics metrics(nameFont);

	int legendWidth = 0;
	for (LINE_MAP::const_iterator iter = lines.begin(); iter != lines.end(); ++iter)
	{
		int width = metrics.width(iter->second.legend);
		width += LEFT_INDENT;
		width += metrics.width(" ");
		legendWidth = Max(legendWidth, width);
	}
	legendWidth = Min(legendWidth, (width() - LEFT_INDENT * 2) / 6);
	

	QRect rect = QRect(QPoint(LEFT_INDENT + legendWidth, TOP_INDENT),
					   QSize(width() - LEFT_INDENT * 2 - legendWidth - RIGHT_INDENT,
							 height() - (BOTTOM_INDENT + TOP_INDENT) + LINE_STEP / 2 - SCROLL_BAR_HEIGHT));

	return rect;
}

void ParticleTimeLineWidget::UpdateLayersExtraInfoPosition()
{
	if (lines.size() == 0)
	{
		countWidget->setVisible(false);
		areaWidget->setVisible(false);
		return;
	}
	
	QRect graphRect = GetGraphRect();
	QRect extraInfoRect(graphRect.right() + PARTICLES_INFO_CONTROL_OFFSET,
					   0,
					   PARTICLES_INFO_CONTROL_WIDTH + 1,
					   graphRect.height() + TOP_INDENT + 1);
	countWidget->setGeometry(extraInfoRect);
	countWidget->setVisible(true);
	
	extraInfoRect.moveRight(extraInfoRect.right() + PARTICLES_INFO_CONTROL_WIDTH);
	areaWidget->setGeometry(extraInfoRect);
	areaWidget->setVisible(true);
}

void ParticleTimeLineWidget::UpdateLayersExtraInfoValues()
{
	// Just invalidate and repaint the counter widgets.
	this->countWidget->repaint();
	this->areaWidget->repaint();
}

void ParticleTimeLineWidget::UpdateSizePolicy()
{
	int height = (lines.size() + 1) * LINE_STEP + BOTTOM_INDENT + TOP_INDENT + PARTICLES_INFO_CONTROL_OFFSET;
	setMinimumHeight(height);
}

void ParticleTimeLineWidget::mouseMoveEvent(QMouseEvent * event)
{
	if (selectedPoint.x() == -1)
	{
		ScrollZoomWidget::mouseMoveEvent(event);
		return;
	}
	
	LINE_MAP::iterator iter = lines.find(selectedPoint.x());
	if (iter == lines.end())
		return;
	
	LINE& line = iter->second;
	
	QRect graphRect = GetGraphRect();
	float32 value = (event->pos().x() - graphRect.left()) / (float32)graphRect.width() * (maxTime - minTime) + minTime;
	value = Max(minTime, Min(maxTime, value));
	if (selectedPoint.y() == 0) //start point selected
	{
		line.startTime = Min(value, line.endTime);
	}
	else
	{
		line.endTime = Max(value, line.startTime);
	}
	update();
}

void ParticleTimeLineWidget::mousePressEvent(QMouseEvent * event)
{
	selectedPoint = GetPoint(event->pos());

	ScrollZoomWidget::mousePressEvent(event);
	update();
}

void ParticleTimeLineWidget::mouseReleaseEvent(QMouseEvent * e)
{
	if (selectedPoint.x() != -1 &&
		selectedPoint.y() != -1)
	{
		OnValueChanged(selectedPoint.x());
	}
		
	selectedPoint = QPoint(-1, -1);
	ScrollZoomWidget::mouseReleaseEvent(e);
	update();
}

void ParticleTimeLineWidget::mouseDoubleClickEvent(QMouseEvent * event)
{
	QPoint point = GetPoint(event->pos());
	LINE_MAP::iterator iter = lines.find(point.x());
	if (iter != lines.end())
	{
		LINE& line = iter->second;
		float32 value = point.y() == 0 ? line.startTime : line.endTime;
		float32 minValue = point.y() == 0 ? minTime : line.startTime;
		float32 maxValue = point.y() == 0 ? line.endTime : maxTime;
		SetPointValueDlg dlg(value, minValue, maxValue, this);
		if (dlg.exec())
		{
			if (point.y() == 0)
				line.startTime = dlg.GetValue();
			else
				line.endTime = dlg.GetValue();
			
			OnValueChanged(iter->first);
		}
	}
	update();
}

QPoint ParticleTimeLineWidget::GetPoint(const QPoint& pos) const
{
	QPoint point = QPoint(-1, -1);
	for (LINE_MAP::const_iterator iter = lines.begin(); iter != lines.end(); ++iter)
	{
		QRect startRect;
		QRect endRect;
		if (!GetLineRect(iter->first, startRect, endRect))
			continue;
		
		//startRect.translate(-LINE_WIDTH, -LINE_WIDTH);
		//endRect.translate(-LINE_WIDTH, -LINE_WIDTH);
		startRect.adjust(-LINE_WIDTH, -LINE_WIDTH, LINE_WIDTH, LINE_WIDTH);
		endRect.adjust(-LINE_WIDTH, -LINE_WIDTH, LINE_WIDTH, LINE_WIDTH);
		point.setX(iter->first);
		if (startRect.contains(pos))
		{
			point.setY(0);
			break;
		}
		
		if (endRect.contains(pos))
		{
			point.setY(1);
			break;
		}
	}
	if (point.y() == -1)
		point.setX(-1);
	return point;
}

float32 ParticleTimeLineWidget::SetPointValueDlg::GetValue() const
{
	return valueSpin->value();
}

void ParticleTimeLineWidget::OnValueChanged(int lineId)
{
	LINE_MAP::iterator iter = lines.find(lineId);
	if (iter == lines.end())
		return;
	
	CommandUpdateParticleLayerTime* cmd = new CommandUpdateParticleLayerTime(iter->second.layer);
	cmd->Init(iter->second.startTime, iter->second.endTime);
	CommandsManager::Instance()->ExecuteAndRelease(cmd,
												   SceneDataManager::Instance()->SceneGetActive()->GetScene());
	
	emit ValueChanged();
}

void ParticleTimeLineWidget::OnUpdate()
{
	if (selectedLayer && emitterNode)
		OnLayerSelected(emitterNode, selectedLayer);
	else if (emitterNode)
		OnNodeSelected(emitterNode);
	else if (effectNode)
		OnEffectNodeSelected(effectNode);
}

void ParticleTimeLineWidget::OnUpdateLayersExtraInfoNeeded()
{
	UpdateLayersExtraInfoValues();
}

ParticleTimeLineWidget::SetPointValueDlg::SetPointValueDlg(float32 value, float32 minValue, float32 maxValue, QWidget *parent) :
	QDialog(parent)
{
	setWindowTitle("Set time");
	// DF-1248 fix - Remove help button
	Qt::WindowFlags flags = windowFlags();
	flags &= ~Qt::WindowContextHelpButtonHint; 
	setWindowFlags(flags);
	
	QVBoxLayout* mainBox = new QVBoxLayout;
	setLayout(mainBox);
	
	valueSpin = new QDoubleSpinBox(this);
	mainBox->addWidget(valueSpin);
	
	QHBoxLayout* btnBox = new QHBoxLayout;
	QPushButton* btnCancel = new QPushButton("Cancel", this);
	QPushButton* btnOk = new QPushButton("Ok", this);
	btnBox->addWidget(btnCancel);
	btnBox->addWidget(btnOk);
	mainBox->addLayout(btnBox);
	
	valueSpin->setMinimum(minValue);
	valueSpin->setMaximum(maxValue);
	valueSpin->setValue(value);
	valueSpin->setSingleStep(0.01);
	
	connect(btnOk,
			SIGNAL(clicked(bool)),
			this,
			SLOT(accept()));
	connect(btnCancel,
			SIGNAL(clicked(bool)),
			this,
			SLOT(reject()));
	
	btnOk->setDefault(true);
	valueSpin->setFocus();
	valueSpin->selectAll();	
}

////////////////////////////////////////////////////////////////////////////////////
ParticlesExtraInfoWidget::ParticlesExtraInfoWidget(const ParticleTimeLineWidget* timeLineWidget,
											   QWidget *parent) :
	QWidget(parent)
{
	this->timeLineWidget = timeLineWidget;
}

void ParticlesExtraInfoWidget::paintEvent(QPaintEvent *)
{
	if (!this->timeLineWidget)
	{
		return;
	}

	QPainter painter(this);
	painter.setPen(Qt::black);
	
	QRect ourRect = rect();
	ourRect.adjust(0, 0, -1, -1);
	painter.drawRect(ourRect);

	// Draw the header.
	painter.setFont(timeLineWidget->nameFont);
	painter.setPen(Qt::black);
	QRect textRect(0, 0, rect().width(), TOP_INDENT);
	painter.drawRect(textRect);
	painter.drawText(textRect, Qt::AlignHCenter | Qt::AlignVCenter, GetExtraInfoHeader());

	// Draw the per-layer particles count.
	OnBeforeGetExtraInfoLoop();

	QFontMetrics fontMetrics(timeLineWidget->nameFont);
	painter.setFont(timeLineWidget->nameFont);

	int32 i = 0;
	for (ParticleTimeLineWidget::LINE_MAP::const_iterator iter = timeLineWidget->lines.begin();
		 iter != timeLineWidget->lines.end(); ++iter, ++i)
	{
		const ParticleTimeLineWidget::LINE& line = iter->second;

		painter.setPen(QPen(line.color, LINE_WIDTH));
		int startY = i * LINE_STEP + LINE_STEP / 2;
		QRect textRect (EXTRA_INFO_LEFT_PADDING, TOP_INDENT + startY,
						rect().width() - EXTRA_INFO_LEFT_PADDING, LINE_STEP);
		painter.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter,
						 GetExtraInfoForLayerLine(line));
	}

	OnAfterGetExtraInfoLoop();

	// Draw the "Total" box.
	QPoint totalPoint(EXTRA_INFO_LEFT_PADDING, rect().bottom() - 3);
	QFont totalFont = timeLineWidget->nameFont;
	totalFont.setBold(true);

	painter.setPen(QPen(Qt::black, LINE_WIDTH));
	painter.drawText(totalPoint, GetExtraInfoFooter());
}

////////////////////////////////////////////////////////////////////////////////////
ParticlesCountWidget::ParticlesCountWidget(const ParticleTimeLineWidget* timeLineWidget,
										   QWidget *parent) :
	ParticlesExtraInfoWidget(timeLineWidget, parent)
{
	this->totalParticlesCount = 0;
}

void ParticlesCountWidget::OnBeforeGetExtraInfoLoop()
{
	this->totalParticlesCount = 0;
}

QString ParticlesCountWidget::GetExtraInfoForLayerLine(const ParticleTimeLineWidget::LINE& line)
{
	if (!line.layer)
	{
		return QString();
	}
	
	int32 particlesNumber = line.layer->GetActiveParticlesCount();
	this->totalParticlesCount += particlesNumber;

	return QString::number(particlesNumber);
}

QString ParticlesCountWidget::GetExtraInfoHeader()
{
	return "Count";
}

QString ParticlesCountWidget::GetExtraInfoFooter()
{
	return QString::number(this->totalParticlesCount);
}

////////////////////////////////////////////////////////////////////////////////////
ParticlesAreaWidget::ParticlesAreaWidget(const ParticleTimeLineWidget* timeLineWidget,
									   QWidget *parent) :
ParticlesExtraInfoWidget(timeLineWidget, parent)
{
	this->totalParticlesArea = 0.0f;
}

void ParticlesAreaWidget::OnBeforeGetExtraInfoLoop()
{
	this->totalParticlesArea = 0;
}

QString ParticlesAreaWidget::GetExtraInfoForLayerLine(const ParticleTimeLineWidget::LINE& line)
{
	if (!line.layer)
	{
		return QString();
	}
	
	float32 area = line.layer->GetActiveParticlesArea();
	this->totalParticlesArea += area;

	return FormatFloat(area);
}

QString ParticlesAreaWidget::GetExtraInfoHeader()
{
	return "Area";
}

QString ParticlesAreaWidget::GetExtraInfoFooter()
{
	return FormatFloat(this->totalParticlesArea);
}

QString ParticlesAreaWidget::FormatFloat(float32 value)
{
	QString strValue;
	if (fabs(value) < 10)
	{
		strValue = "%.4f";
	}
	else if (fabs(value) < 100)
	{
		strValue = "%.2f";
	}
	else
	{
		strValue = "%.0f";
	}

	strValue.sprintf(strValue.toAscii(), value);
	return strValue;
}
