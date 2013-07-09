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

#include "TextureScrollArea.h"

#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QGraphicsProxyWidget>
#include <QGraphicsLinearLayout>
#include <QProgressBar>
#include <QApplication>
#include <QWheelEvent>
#include <QScrollBar>
#include <QPainter>
#include <QImage>
#include <QPixmap>
#include <QMovie>
#include <QLabel>

TextureScrollArea::TextureScrollArea(QWidget* parent /* = 0 */)
	: QGraphicsView(parent)
	, textureColorMask((int) ChannelAll)
	, mouseInMoveState(false)
	, textureScene(NULL)
	, textureBorder(NULL)
	, zoomFactor(1.0)
	, tiledBgDoDraw(false)
	, warningVisible(false)
{
	// create and setup scene
	textureScene = new QGraphicsScene();
	setRenderHints((QPainter::RenderHints) 0);
	setScene(textureScene);

	// we can have complex background (if tiledBgDoDraw set to true),
	// so set mode to redraw it each time
	// and then prepare pixmap for our custom background
	setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
	sutupCustomTiledBg();

	// add items to scene
	texturePixmap = textureScene->addPixmap(QPixmap());
	textureBorder = textureScene->addRect(0, 0, 10, 10, QPen(QColor(255, 255, 0, 255)), QBrush(Qt::NoBrush));

	// add warning label
	warningLabel = new QLabel("No image");
	warningLabel->setAttribute(Qt::WA_NoSystemBackground, true);
	// label color
	QPalette palette = warningLabel->palette();
	palette.setColor(warningLabel->foregroundRole(), Qt::gray);
	warningLabel->setPalette(palette);
	// label font size
	QFont font = warningLabel->font();
	font.setPointSize(18);
	font.setBold(true);
	warningLabel->setFont(font);
	warningLabel->setAlignment(Qt::AlignCenter);
	// add it to scene
	warningProxy = textureScene->addWidget(warningLabel);
	warningProxy->setGeometry(QRectF(-150, -20, 150, 20));
	warningProxy->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);

	// add wait-bar to scene
	QProgressBar *progressBar = new QProgressBar();
	progressBar->setMinimum(0);
	progressBar->setMaximum(0);
	progressBar->setTextVisible(false);
	progressBar->setAttribute(Qt::WA_NoSystemBackground, true);
	// add to scene
	waitBar = textureScene->addWidget(progressBar);
	waitBar->setGeometry(QRectF(-120, -15, 120, 15));
	waitBar->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);

	adjustWidgetsPos();

	borderShow(false);
	bgmaskShow(false);
	waitbarShow(false);
}

TextureScrollArea::~TextureScrollArea()
{ }

void TextureScrollArea::setImage(const QImage &image)
{
	currentTextureImage = image;
	QImage::Format imgFormat = image.format();

	applyCurrentImageToScenePixmap();
	applyCurrentImageBorder();

	warningVisible = currentTextureImage.isNull();
	warningProxy->setVisible(warningVisible);

	adjustWidgetsPos();
}

void TextureScrollArea::setColorChannel(int mask)
{
	textureColorMask = mask;
	applyCurrentImageToScenePixmap();
}

float TextureScrollArea::getTextureZoom()
{
	return zoomFactor;
}

QColor TextureScrollArea::getPixelColor(QPoint pos)
{
	QRgb rgb = 0;

	if(pos.x() >= 0 && pos.x() < currentTextureImage.width() &&
		pos.y() >= 0 && pos.y() < currentTextureImage.height())
	{
		rgb = currentTextureImage.pixel(pos);
	}

	return QColor::fromRgba(rgb);
}

void TextureScrollArea::resetTexturePosZoom()
{
	setTextureZoom(1.0);
	setTexturePos(QPoint(0,0));
}

void TextureScrollArea::borderShow(bool show)
{
	if(show)
	{
		textureBorder->show();
	}
	else
	{
		textureBorder->hide();
	}
}

void TextureScrollArea::bgmaskShow(bool show)
{
	tiledBgDoDraw = show;

	// call this setBackgroundBrush function to force background redraw
	textureScene->setBackgroundBrush(QBrush(QColor(0, 0, 0)));
}

void TextureScrollArea::waitbarShow(bool show)
{
	if(show)
	{
		warningProxy->setVisible(false);
		waitBar->show();
	}
	else
	{
		warningProxy->setVisible(warningVisible);
		waitBar->hide();
	}

	adjustWidgetsPos();
}

void TextureScrollArea::setTextureZoom(const float &zoom)
{
	if(zoom != zoomFactor)
	{
		zoomFactor = zoom;


		resetTransform();
		scale(zoomFactor, zoomFactor);
		emit textureZoomChanged(zoomFactor);

		adjustWidgetsPos();
	}
}

void TextureScrollArea::setTexturePos(const QPoint &pos)
{
	horizontalScrollBar()->setValue(pos.x());
	verticalScrollBar()->setValue(pos.y());
}

void TextureScrollArea::scrollContentsBy(int dx, int dy)
{
	QGraphicsView::scrollContentsBy(dx, dy);
	emit texturePosChanged(QPoint(horizontalScrollBar()->value(), verticalScrollBar()->value()));
}

void TextureScrollArea::wheelEvent(QWheelEvent * e)
{
	emit mouseWheel(e->delta());
}

void TextureScrollArea::mouseMoveEvent(QMouseEvent *event)
{
	QGraphicsView::mouseMoveEvent(event);

	if(mouseInMoveState)
	{
		int mouseDx = event->pos().x() - mousePressPos.x();
		int mouseDy = event->pos().y() - mousePressPos.y();

		horizontalScrollBar()->setValue(mousePressScrollPos.x() - mouseDx);
		verticalScrollBar()->setValue(mousePressScrollPos.y() - mouseDy);
	}
	else
	{
		QPointF scenePos = mapToScene(event->pos().x(), event->pos().y());
		emit mouseOverPixel(QPoint((int) scenePos.x(), (int) scenePos.y()));
	}
}

void TextureScrollArea::mousePressEvent(QMouseEvent *event)
{
	QGraphicsView::mousePressEvent(event);

	mouseInMoveState = true;
	mousePressPos = event->pos();
	mousePressScrollPos.setX(horizontalScrollBar()->value());
	mousePressScrollPos.setY(verticalScrollBar()->value());
	viewport()->setProperty("cursor", QVariant(QCursor(Qt::ClosedHandCursor)));
}

void TextureScrollArea::mouseReleaseEvent(QMouseEvent *event)
{
	QGraphicsView::mouseReleaseEvent(event);

	mouseInMoveState = false;
	viewport()->setProperty("cursor", QVariant(QCursor(Qt::ArrowCursor)));
}

void TextureScrollArea::applyCurrentImageToScenePixmap()
{
	// TODO: optimize this code

	if(~textureColorMask)
	{
		int mask = 0xFFFFFFFF;
		int maskA = 0;

		QImage tmpImage = currentTextureImage.convertToFormat(QImage::Format_ARGB32);

		if(!(textureColorMask & ChannelR)) mask &= 0xFF00FFFF;
		if(!(textureColorMask & ChannelG)) mask &= 0xFFFF00FF;
		if(!(textureColorMask & ChannelB)) mask &= 0xFFFFFF00;
		if(!(textureColorMask & ChannelA)) maskA |= 0xFF000000;

		if(mask == 0xFF000000)
		{
			maskA ^= 0xFF000000;

			// only alpha, so show it
			for (int y = 0; y < tmpImage.height(); y++) 
			{
				QRgb *line = (QRgb *) tmpImage.scanLine(y);
				for (int x = 0; x < tmpImage.width(); x++) 
				{
					int c = (line[x] & 0xFF000000) >> 24;
					line[x] = (maskA | c << 16 | c << 8 | c);
				}
			}
		}
		else
		{
			for (int y = 0; y < tmpImage.height(); y++) 
			{
				QRgb *line = (QRgb *) tmpImage.scanLine(y);
				for (int x = 0; x < tmpImage.width(); x++) 
				{
					line[x] &= mask;
					line[x] |= maskA;
				}
			}
		}

		QPixmap pixmap = QPixmap::fromImage(tmpImage);
		textureScene->setSceneRect(pixmap.rect());
		texturePixmap->setPixmap(pixmap);
	}
	else
	{
		QPixmap pixmap = QPixmap::fromImage(currentTextureImage);
		textureScene->setSceneRect(pixmap.rect());
		texturePixmap->setPixmap(pixmap);
	}
}

void TextureScrollArea::applyCurrentImageBorder()
{
	QRectF r(currentTextureImage.rect());

	if(r.width() != 0 && r.height() != 0)
	{
		r.adjust(-0.1, -0.1, 0.1, 0.1);
	}

	textureBorder->setRect(r);
	r.adjust(0, 0, 1, 1);
	textureScene->setSceneRect(r);
}

void TextureScrollArea::adjustWidgetsPos()
{
	// apply to waitBar inverted transform - so it will be always same size
	// waitBar->setTransform(transform().inverted());

	// calculate new waitBar pos
	qreal scaleX = transform().m11();
	qreal scaleY = transform().m22();
	QRectF rect = waitBar->sceneBoundingRect();
	QPointF viewCenter = mapToScene(width() / 2.0, height() / 2.0);
	qreal x = viewCenter.x() - rect.width() / 2.0 / scaleX;
	qreal y = viewCenter.x() - rect.height() / 2.0 / scaleY;
	waitBar->setPos(x, y);

	// calculate new warning pos
	rect = warningProxy->sceneBoundingRect();
	viewCenter = mapToScene(width() / 2.0, height() / 2.0);
	x = viewCenter.x() - rect.width() / 2.0 / scaleX;
	y = viewCenter.x() - rect.height() / 2.0 / scaleY;
	warningProxy->setPos(x, y);
}

void TextureScrollArea::drawBackground(QPainter * painter, const QRectF & rect)
{
	if(tiledBgDoDraw)
	{
		painter->resetTransform();
		painter->drawTiledPixmap(this->rect(), tiledBgPixmap);
	}
	else
	{
		QGraphicsView::drawBackground(painter, rect);
	}
}

void TextureScrollArea::sutupCustomTiledBg()
{
	tiledBgPixmap = QPixmap(30, 30);
	QPainter p(&tiledBgPixmap);
	p.setBrush(QBrush(QColor(150,150,150)));
	p.setPen(Qt::NoPen);
	p.drawRect(QRect(0,0,30,30));
	p.setBrush(QBrush(QColor(200,200,200)));
	p.drawRect(QRect(0,0,15,15));
	p.drawRect(QRect(15,15,15,15));
}

QImage TextureScrollArea::getImage()
{
	return currentTextureImage;
}
