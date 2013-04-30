#ifndef __RESOURCEEDITORQT__SET_SWITCH_INDEX_VIEW_H__
#define __RESOURCEEDITORQT__SET_SWITCH_INDEX_VIEW_H__

#include <QWidget>
#include "Base/BaseTypes.h"
#include "Commands/SetSwitchIndexCommands.h"

namespace Ui
{
	class SetSwitchIndexView;
}

class SetSwitchIndexView: public QWidget
{
	Q_OBJECT

public:
	explicit SetSwitchIndexView(QWidget* parent = 0);
	~SetSwitchIndexView();

	void Init();

signals:
	void Clicked(DAVA::uint32 value, DAVA::SetSwitchIndexHelper::eSET_SWITCH_INDEX selectionState);

private slots:
	void Clicked();

private:
	Ui::SetSwitchIndexView *ui;
};

#endif /* defined(__RESOURCEEDITORQT__SET_SWITCH_INDEX_VIEW_H__) */
