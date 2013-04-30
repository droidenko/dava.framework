#include <QFile>

#include "Qt/MaterialBrowser/MaterialBrowser.h"

#include "ui_materialbrowser.h"

MaterialBrowser::MaterialBrowser(QWidget *parent) 
	: QDialog(parent)
	, ui(new Ui::MaterialBrowser)
{
    ui->setupUi(this);
	setWindowFlags(Qt::Window);

	treeModelScene = new MaterialTreeModel();
	viewModel = new MaterialViewModel();

	SetupModelTree();
	SetupModelView();

	posSaver.Attach(this);
}

MaterialBrowser::~MaterialBrowser()
{
	delete viewModel;
	delete treeModelScene;
    delete ui;
}

void MaterialBrowser::SetScene(DAVA::Scene *scene)
{
	treeModelScene->SetScene(scene);
}

void MaterialBrowser::SetupModelTree()
{
	ui->treeScene->setModel(treeModelScene);

	QObject::connect(ui->treeScene, SIGNAL(Selected(const QItemSelection &, const QItemSelection &)), this, SLOT(TreeItemSelected(const QItemSelection &, const QItemSelection &)));
	QObject::connect(ui->treeScene, SIGNAL(pressed(const QModelIndex &)), this, SLOT(TreeItemPressed(const QModelIndex &)));
}

void MaterialBrowser::SetupModelView()
{
	ui->materialView->setModel(viewModel);
}

void MaterialBrowser::TreeItemSelected(const QItemSelection &selected, const QItemSelection &deselected)
{ }

void MaterialBrowser::TreeItemPressed(const QModelIndex &index)
{
	MaterialTreeItem *item = treeModelScene->Item(index);
	if(NULL != item)
	{
		if(0 == item->ChildCount() && NULL != item->Parent())
		{
			item = item->Parent();
		}

		viewModel->SetTreeItem(item);
	}
}
