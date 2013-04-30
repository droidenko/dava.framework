#ifndef __RESOURCEEDITORQT__MATERIAL_VIEW_OPTIONS_H__
#define __RESOURCEEDITORQT__MATERIAL_VIEW_OPTIONS_H__

#include <QWidget>
#include "Base/BaseTypes.h"
#include "Commands/MaterialViewOptionsCommands.h"

namespace Ui
{
	class MaterialViewOptions;
}

class MaterialViewOptions: public QWidget
{
	Q_OBJECT

public:
	explicit MaterialViewOptions(QWidget* parent = 0);
	~MaterialViewOptions();

	void Init();

private:
	Ui::MaterialViewOptions *ui;
};

#endif /* defined(__RESOURCEEDITORQT__MATERIAL_VIEW_OPTIONS_H__) */
