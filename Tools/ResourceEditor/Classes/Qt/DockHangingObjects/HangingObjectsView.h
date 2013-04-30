#ifndef __RESOURCEEDITORQT__HANGING_OBJECTS_VIEW_H__
#define __RESOURCEEDITORQT__HANGING_OBJECTS_VIEW_H__

#include <QWidget>
#include "Base/BaseTypes.h"
//#include "Commands/SetSwitchIndexCommands.h"

namespace Ui
{
	class HangingObjectsView;
}

class HangingObjectsView: public QWidget
{
	Q_OBJECT

public:
	explicit HangingObjectsView(QWidget* parent = 0);
	~HangingObjectsView();

	void Init();
	
signals:
	void Clicked(float value, bool isEnabled);

private slots:
	void Clicked();
	void CheckBoxChangeState(int);

private:
	Ui::HangingObjectsView *ui;
};

#endif /* defined(__RESOURCEEDITORQT__HANGING_OBJECTS_VIEW_H__) */
