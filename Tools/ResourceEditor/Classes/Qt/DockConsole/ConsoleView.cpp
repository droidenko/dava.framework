#include "DockConsole/ConsoleView.h"

ConsoleView::ConsoleView(QWidget *parent /*= 0*/)
	: QPlainTextEdit(parent)
	, lineCount(0)
{
	/*
	QPalette p = palette();
	p.setColor(QPalette::Base, Qt::black);
	p.setColor(QPalette::Text, Qt::gray);
	setPalette(p);
	*/

	for(size_t i = 0; i < Console::Instance()->GetLineCount(); ++i)
	{
		appendPlainText(Console::Instance()->GetLine(i).c_str());
		lineCount++;
	}

	connect(Console::Instance(), SIGNAL(OnClear()), this, SLOT(OnClear()));
	connect(Console::Instance(), SIGNAL(OnEcho()), this, SLOT(OnEcho()));
}

ConsoleView::~ConsoleView()
{

}

void ConsoleView::OnClear()
{
	clear();
	lineCount = 0;
}

void ConsoleView::OnEcho()
{
	for(size_t i = lineCount; i < Console::Instance()->GetLineCount(); ++i)
	{
		appendPlainText(Console::Instance()->GetLine(i).c_str());
		lineCount++;
	}
}
