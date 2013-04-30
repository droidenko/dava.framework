#include "Qt/DockLibrary/LibraryView.h"
#include "Qt/Project/ProjectManager.h"
#include <QFileSystemModel>
#include <QMenu>

#include "Qt/Scene/SceneDataManager.h"
#include "Qt/Scene/SceneData.h"
#include "LandscapeEditor/LandscapesController.h"

#include "SceneEditor/SceneEditorScreenMain.h"
#include "SceneEditor/EditorBodyControl.h"
#include "AppScreens.h"


LibraryView::LibraryView(QWidget *parent /* = 0 */)
	: QTreeView(parent)
{
	libModel = new LibraryModel();

	QObject::connect(libModel, SIGNAL(rootPathChanged(const QString &)), this, SLOT(ModelRootPathChanged(const QString &)));

	setModel(libModel);

	QObject::connect(ProjectManager::Instance(), SIGNAL(ProjectOpened(const QString &)), this, SLOT(ProjectOpened(const QString &)));
	QObject::connect(ProjectManager::Instance(), SIGNAL(ProjectClosed(const QString &)), this, SLOT(ProjectClosed(const QString &)));

	QObject::connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(ShowContextMenu(const QPoint &)));
	QObject::connect(selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), this, SLOT(FileSelectionChanged(const QItemSelection &, const QItemSelection &)));

	// hide columns with size/modif date etc.
	for(int32 i = 1; i < libModel->columnCount(); ++i)
	{
		setColumnHidden(i, true);
	}
}

LibraryView::~LibraryView()
{
	delete libModel;
}

void LibraryView::ProjectOpened(const QString &path)
{
	libModel->SetLibraryPath(ProjectManager::Instance()->CurProjectDataSourcePath());
}

void LibraryView::ProjectClosed(const QString &path)
{
	libModel->SetLibraryPath("");
}

void LibraryView::ShowContextMenu(const QPoint &point)
{
	QModelIndex index = indexAt(point);

	if(index.isValid())
	{
		QFileInfo fileInfo = libModel->fileInfo(index);
		QString fileExtension = fileInfo.suffix();
		if(fileInfo.isFile())
		{
			QMenu contextMenu(this);

			if(0 == fileExtension.compare("sc2", Qt::CaseInsensitive))
			{
				SceneData *activeScene = SceneDataManager::Instance()->SceneGetActive();
				LandscapesController *landsacpesController = activeScene->GetLandscapesController();

				SceneEditorScreenMain *screen = static_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen(SCREEN_MAIN_OLD));
				EditorBodyControl *c = screen->FindCurrentBody()->bodyControl;

				bool canChangeScene = !landsacpesController->EditorLandscapeIsActive() && !c->LandscapeEditorActive();
				if(canChangeScene)
				{
					contextMenu.addAction(new ContextMenuAction(QString("Add"), new CommandAddScene(fileInfo.filePath().toAscii().constData())));
				}

				contextMenu.addAction(new ContextMenuAction(QString("Edit"), new CommandEditScene(fileInfo.filePath().toAscii().constData())));

				if(canChangeScene)
				{
					contextMenu.addAction(new ContextMenuAction(QString("Reload"), new CommandReloadScene(fileInfo.filePath().toAscii().constData())));
				}

			}
			else if(0 == fileExtension.compare("dae", Qt::CaseInsensitive))
			{
				contextMenu.addAction(new ContextMenuAction(QString("Convert"), new CommandConvertScene(fileInfo.filePath().toAscii().constData())));
			}

			ContextMenuAction* action = (ContextMenuAction *) contextMenu.exec(mapToGlobal(point));
			if(NULL != action)
			{
				action->Exec();
			}
		}
	}
}

void LibraryView::ModelRootPathChanged(const QString & newPath)
{
	setRootIndex(libModel->index(newPath));
}

void LibraryView::FileSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected)
{
	DAVA::String previewPath;
	const QModelIndex index = selected.indexes().first();

	if(index.isValid())
	{
		QFileInfo fileInfo = libModel->fileInfo(index);
		if(0 == fileInfo.suffix().compare("sc2", Qt::CaseInsensitive))
		{
			previewPath = fileInfo.filePath().toAscii().constData();
		}
	}

	SceneDataManager::Instance()->SceneShowPreview(previewPath);
}

void LibraryView::LibraryFileTypesChanged(bool showDAEFiles, bool showSC2Files)
{
	libModel->SetFileNameFilters(showDAEFiles, showSC2Files);
}