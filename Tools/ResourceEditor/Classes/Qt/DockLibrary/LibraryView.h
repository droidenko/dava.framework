#ifndef __LIBRARY_VIEW_H__
#define __LIBRARY_VIEW_H__

#include <QTreeView>
#include <QAction>
#include "Qt/Scene/SceneDataManager.h"
#include "Commands/LibraryCommands.h"
#include "Commands/CommandsManager.h"
#include "Qt/DockLibrary/LibraryModel.h"

class ContextMenuAction : public QAction
{
	Q_OBJECT

public:
	ContextMenuAction(const QString &text, Command *command)
		: QAction(text, NULL)
		, curCommand(command)
	{ }

	~ContextMenuAction()
	{
		if(NULL != curCommand)
		{
			delete curCommand;
		}
	}

	void Exec()
	{
		if(NULL != curCommand)
		{
			CommandsManager::Instance()->Execute(curCommand);
		}
	}

protected:
	Command *curCommand;
};

class LibraryView : public QTreeView
{
	Q_OBJECT

public:
	LibraryView(QWidget *parent = 0);
	~LibraryView();

public slots:
	void ProjectOpened(const QString &path);
	void ProjectClosed(const QString &path);
	void LibraryFileTypesChanged(bool showDAEFiles, bool showSC2Files);

protected slots:
	void ModelRootPathChanged(const QString & newPath);
	void ShowContextMenu(const QPoint &point);
	void FileSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

private:
	LibraryModel *libModel;
};

#endif // __LIBRARY_VIEW_H__ 
