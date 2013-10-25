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

#ifndef __RESOURCEEDITORQT__SLIDERWIDGET__
#define __RESOURCEEDITORQT__SLIDERWIDGET__

#include <QWidget>

class QSlider;
class QLineEdit;
class QSpinBox;
class QLabel;

class SliderWidget: public QWidget
{
	Q_OBJECT

public:
	explicit SliderWidget(QWidget* parent = 0);
	~SliderWidget();

	void Init(bool symmetric, int max, int min, int value);

	void SetRange(int min, int max);

	void SetRangeMax(int max);
	int GetRangeMax();

	void SetRangeMin(int min);
	int GetRangeMin();

	void SetSymmetric(bool symmetric);
	bool IsSymmetric();

	void SetValue(int value);
	int GetValue();

	void SetRangeChangingBlocked(bool blocked);
	bool IsRangeChangingBlocked();

	void SetRangeVisible(bool visible);
	bool IsRangeVisible();

	void SetCurValueVisible(bool visible);
	bool IsCurValueVisible();

	void SetRangeBoundaries(int min, int max);

protected:
	virtual bool eventFilter(QObject* obj, QEvent* ev);

signals:
	void ValueChanged(int newValue);

private slots:
	void SliderValueChanged(int newValue);
	void RangeChanged(int newMinValue, int newMaxValue);
	void SpinValueChanged(int newValue);

	void OnValueReady(const QWidget* widget, int value);

private:
	static const int DEF_LOWEST_VALUE;
	static const int DEF_HIGHEST_VALUE;

	QLabel* labelMinValue;
	QLabel* labelMaxValue;
	QSpinBox* spinCurValue;
	QSlider* sliderValue;

	bool isSymmetric;
	bool isRangeChangingBlocked;
	bool isRangeVisible;

	int minValue;
	int maxValue;
	int currentValue;

	int rangeBoundMin;
	int rangeBoundMax;

	void EmitValueChanged();
	void ConnectToSignals();
	void UpdateControls();

	void InitUI();
};

#endif /* defined(__RESOURCEEDITORQT__SLIDERWIDGET__) */