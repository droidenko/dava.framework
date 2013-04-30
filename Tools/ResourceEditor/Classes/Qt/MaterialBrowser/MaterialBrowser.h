#ifndef __MATERIAL_BROWSER_H__
#define __MATERIAL_BROWSER_H__

#include <QDialog>
#include <QItemSelection>
#include "Qt/QtPosSaver/QtPosSaver.h"
#include "Qt/MaterialBrowser/MaterialTreeModel.h"
#include "Qt/MaterialBrowser/MaterialViewModel.h"

namespace Ui {
class MaterialBrowser;
}

class MaterialBrowser : public QDialog
{
    Q_OBJECT
    
public:
    explicit MaterialBrowser(QWidget *parent = 0);
    ~MaterialBrowser();

	void SetScene(DAVA::Scene *scene);

private:
    Ui::MaterialBrowser *ui;
	QtPosSaver posSaver;

	MaterialTreeModel *treeModelScene;
	MaterialViewModel *viewModel;

	void SetupModelTree();
	void SetupModelView();

private slots:
	void TreeItemSelected(const QItemSelection &selected, const QItemSelection &deselected);
	void TreeItemPressed(const QModelIndex &index);
};

#endif // __MATERIAL_BROWSER_H__
