#include "Qt/QtPropertyEditor/QtPropertyWidgets/QtColorLineEdit.h"

#include <QHBoxLayout>
#include <QEvent>
#include <QStyle>
#include <QColorDialog>
#include <QRegExpValidator>
#include <QFocusEvent>
#include <QApplication>

QtColorLineEdit::QtColorLineEdit(QWidget * parent)
	: QLineEdit(parent)
{
	QRegExpValidator *validator = new QRegExpValidator();
	validator->setRegExp(QRegExp("#[A-F0-9]{8}", Qt::CaseInsensitive));

	setValidator(validator);

	QObject::connect(this, SIGNAL(editingFinished()), this, SLOT(EditFinished()));
}

void QtColorLineEdit::SetColor(const QColor &color)
{
	QString s;
	curColor = color;
	setText(s.sprintf("#%02x%02x%02x%02x", curColor.red(), curColor.green(), curColor.blue(), curColor.alpha()));
}

QColor QtColorLineEdit::GetColor() const
{
	return curColor;
}

void QtColorLineEdit::EditFinished()
{
	int r = 0, g = 0, b = 0, a = 255;
	if(4 == sscanf(text().toAscii().constData(), "#%02x%02x%02x%02x", &r, &g, &b, &a))
	{
		curColor = QColor(r, g, b, a);
	}
}
