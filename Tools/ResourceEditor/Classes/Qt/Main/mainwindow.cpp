#include "mainwindow.h"

#include "DAVAEngine.h"
#include "Qt/Main/QtMainWindowHandler.h"
#include "Qt/Scene/SceneDataManager.h"
#include "SceneEditor/EditorSettings.h"
#include "SceneEditor/CommandLineTool.h"
#include "Qt/TextureBrowser/TextureConvertor.h"
#include "Qt/DockSceneGraph/PointerHolder.h"
#include "Qt/Project/ProjectManager.h"
#include "Qt/DockLibrary/LibraryModel.h"

#include <QToolBar>
#include <QToolButton>

#include "SceneEditor/SceneEditorScreenMain.h"
#include "SceneEditor/EditorBodyControl.h"
#include "SceneEditor/EditorConfig.h"
#include "SceneEditor/CommandLineTool.h"
#include "QT/SpritesPacker/SpritePackerHelper.h"
#include "QT/QResourceEditorProgressDialog/QResourceEditorProgressDialog.h"

#include <QApplication>
#include <QPixmap>

#include "ModificationWidget.h"


QtMainWindow::QtMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
	, convertWaitDialog(NULL)
	, oldDockSceneGraphMinSize(-1, -1)
	, oldDockSceneGraphMaxSize(-1, -1)
	, repackSpritesWaitDialog(NULL)
	, emitRepackAndReloadFinished(false)
{
	new ProjectManager();
	new SceneDataManager();
	new QtMainWindowHandler(this);

	ui->setupUi(this);
 
    qApp->installEventFilter(this);
	EditorConfig::Instance()->ParseConfig(EditorSettings::Instance()->GetProjectPath() + "EditorConfig.yaml");

	QtMainWindowHandler::Instance()->SetDefaultFocusWidget(ui->sceneTabWidget);
	QtMainWindowHandler::Instance()->SetResentMenu(ui->menuFile);
	QtMainWindowHandler::Instance()->RegisterStatusBar(ui->statusBar);
	QtMainWindowHandler::Instance()->RestoreDefaultFocus();

    RegisterBasePointerTypes();
   
	SetupActions();
    SetupMainMenu();
    SetupToolBars();
    SetupDocks();

	posSaver.Attach(this);
	posSaver.LoadState(this);
	
	ui->dockParticleEditor->installEventFilter(this);
	ChangeParticleDockVisible(false); //hide particle editor dock on start up
	ui->dockParticleEditorTimeLine->hide();

	// Open last project
	QObject::connect(ProjectManager::Instance(), SIGNAL(ProjectOpened(const QString &)), this, SLOT(ProjectOpened(const QString &)));
	OpenLastProject();
}

QtMainWindow::~QtMainWindow()
{
	posSaver.SaveState(this);

	delete ui;
    
	QtMainWindowHandler::Instance()->Release();
	SceneDataManager::Instance()->Release();
	ProjectManager::Instance()->Release();
}

Ui::MainWindow* QtMainWindow::GetUI()
{
	return ui;
}

void QtMainWindow::SetupActions()
{
	QtMainWindowHandler *actionHandler = QtMainWindowHandler::Instance();

	//File
	connect(ui->menuFile, SIGNAL(triggered(QAction *)), actionHandler, SLOT(FileMenuTriggered(QAction *)));
	connect(ui->actionNewScene, SIGNAL(triggered()), actionHandler, SLOT(NewScene()));
	connect(ui->actionOpenScene, SIGNAL(triggered()), actionHandler, SLOT(OpenScene()));
	connect(ui->actionOpenProject, SIGNAL(triggered()), actionHandler, SLOT(OpenProject()));
	connect(ui->actionSaveScene, SIGNAL(triggered()), actionHandler, SLOT(SaveScene()));
	connect(ui->actionSaveToFolder, SIGNAL(triggered()), actionHandler, SLOT(SaveToFolderWithChilds()));
	connect(ui->actionPNG, SIGNAL(triggered()), actionHandler, SLOT(ExportAsPNG()));
	connect(ui->actionPVR, SIGNAL(triggered()), actionHandler, SLOT(ExportAsPVR()));
	connect(ui->actionDXT, SIGNAL(triggered()), actionHandler, SLOT(ExportAsDXT()));
	connect(ui->actionReloadAll, SIGNAL(triggered()), actionHandler, SLOT(RepackAndReloadTextures()));

	//View
	connect(ui->actionRestoreViews, SIGNAL(triggered()), actionHandler, SLOT(RestoreViews()));

	//Tools
	connect(ui->actionMaterialEditor, SIGNAL(triggered()), actionHandler, SLOT(Materials()));
	connect(ui->actionTextureConverter, SIGNAL(triggered()), actionHandler, SLOT(ConvertTextures()));
	connect(ui->actionHeightMapEditor, SIGNAL(triggered()), actionHandler, SLOT(HeightmapEditor()));
	connect(ui->actionTileMapEditor, SIGNAL(triggered()), actionHandler, SLOT(TilemapEditor()));
	connect(ui->actionRulerTool, SIGNAL(triggered()), actionHandler, SLOT(RulerTool()));
	connect(ui->actionShowSettings, SIGNAL(triggered()), actionHandler, SLOT(ShowSettings()));
    
#if defined (__DAVAENGINE_MACOS__)
    ui->menuTools->removeAction(ui->actionBeast);
#else //#if defined (__DAVAENGINE_MACOS__)
	connect(ui->actionBeast, SIGNAL(triggered()), actionHandler, SLOT(Beast()));
#endif //#if defined (__DAVAENGINE_MACOS__)
    
	//Edit
	connect(ui->actionConvertToShadow, SIGNAL(triggered()), actionHandler, SLOT(ConvertToShadow()));
}

void QtMainWindow::SetupMainMenu()
{
    QtMainWindowHandler *actionHandler = QtMainWindowHandler::Instance();

    QAction *actionSceneGraph = ui->dockSceneGraph->toggleViewAction();
    QAction *actionProperties = ui->dockProperties->toggleViewAction();
    QAction *actionLibrary = ui->dockLibrary->toggleViewAction();
	QAction *actionReferences = ui->dockReferences->toggleViewAction();
    QAction *actionToolBar = ui->mainToolBar->toggleViewAction();
    QAction *actionCustomColors = ui->dockCustomColors->toggleViewAction();
	QAction *actionVisibilityCheckTool = ui->dockVisibilityTool->toggleViewAction();
	QAction *actionHangingObjects = ui->dockHangingObjects->toggleViewAction();
	QAction *actionSetSwitchIndex = ui->dockSetSwitchIndex->toggleViewAction();
	QAction *actionParticleEditor = ui->dockParticleEditor->toggleViewAction();
	QAction *actionParticleEditorTimeLine = ui->dockParticleEditorTimeLine->toggleViewAction();
    QAction *actionSceneInfo = ui->dockSceneInfo->toggleViewAction();

    ui->menuView->insertAction(ui->actionRestoreViews, actionSceneInfo);
    ui->menuView->insertAction(actionSceneInfo, actionToolBar);
    ui->menuView->insertAction(actionToolBar, actionLibrary);
    ui->menuView->insertAction(actionLibrary, actionProperties);
	ui->menuView->insertAction(actionProperties, actionReferences);
    ui->menuView->insertAction(actionReferences, actionSceneGraph);
    ui->menuView->insertAction(actionSceneGraph, actionCustomColors);
	ui->menuView->insertAction(actionCustomColors, actionVisibilityCheckTool);
	ui->menuView->insertAction(actionVisibilityCheckTool, actionParticleEditor);
	ui->menuView->insertAction(actionParticleEditor, actionParticleEditorTimeLine);
	ui->menuView->insertAction(actionParticleEditorTimeLine, actionHangingObjects);
	ui->menuView->insertAction(actionHangingObjects, actionSetSwitchIndex);
    
    ui->menuView->insertSeparator(ui->actionRestoreViews);
    ui->menuView->insertSeparator(actionToolBar);
    ui->menuView->insertSeparator(actionProperties);

    actionHandler->RegisterDockActions(ResourceEditor::HIDABLEWIDGET_COUNT,
                                       actionSceneGraph,
                                       actionProperties, actionLibrary, actionToolBar,
									   actionReferences, actionCustomColors, actionVisibilityCheckTool, 
									   actionParticleEditor, actionHangingObjects, actionSetSwitchIndex, actionSceneInfo);


    ui->dockProperties->hide();
    
    
    //CreateNode
    actionHandler->RegisterNodeActions(ResourceEditor::NODE_COUNT,
                                       ui->actionLandscape,
                                       ui->actionLight,
                                       ui->actionServiceNode,
                                       ui->actionCamera,
                                       ui->actionImposter,
                                       ui->actionParticleEmitter,
                                       ui->actionUserNode,
									   ui->actionSwitchNode,
									   ui->actionParticleEffectNode
                                       );
    connect(ui->menuCreateNode, SIGNAL(triggered(QAction *)), actionHandler, SLOT(CreateNodeTriggered(QAction *)));

    
    //TODO: need enable flag in future
    ui->actionHeightMapEditor->setCheckable(false);
    ui->actionTileMapEditor->setCheckable(false);
    //ENDOFTODO
    
    
    //Viewport
    connect(ui->menuViewPort, SIGNAL(triggered(QAction *)), actionHandler, SLOT(ViewportTriggered(QAction *)));
    actionHandler->RegisterViewportActions(ResourceEditor::VIEWPORT_COUNT,
                                           ui->actionIPhone,
                                           ui->actionRetina,
                                           ui->actionIPad,
                                           ui->actionDefault
                                       );

	//Edit Options
	connect(ui->actionUndo, SIGNAL(triggered()), actionHandler, SLOT(UndoAction()));
	connect(ui->actionRedo, SIGNAL(triggered()), actionHandler, SLOT(RedoAction()));
	actionHandler->RegisterEditActions(ResourceEditor::EDIT_COUNT, ui->actionUndo, ui->actionRedo);

    //View Options
    connect(ui->actionShowNotPassableLandscape, SIGNAL(triggered()), actionHandler, SLOT(ToggleNotPassableTerrain()));
    connect(ui->actionReloadAsPNG, SIGNAL(triggered()), actionHandler, SLOT(ReloadAsPNG()));
    connect(ui->actionReloadAsPVR, SIGNAL(triggered()), actionHandler, SLOT(ReloadAsPVR()));
    connect(ui->actionReloadAsDXT, SIGNAL(triggered()), actionHandler, SLOT(ReloadAsDXT()));
    actionHandler->RegisterTextureFormatActions(FILE_FORMAT_COUNT, ui->actionReloadAsPNG, ui->actionReloadAsPVR, ui->actionReloadAsDXT);

	//Modifications Options
	connect(ui->actionModifySelect, SIGNAL(triggered()), actionHandler, SLOT(ModificationSelect()));
	connect(ui->actionModifyMove, SIGNAL(triggered()), actionHandler, SLOT(ModificationMove()));
	connect(ui->actionModifyRotate, SIGNAL(triggered()), actionHandler, SLOT(ModificationRotate()));
	connect(ui->actionModifyScale, SIGNAL(triggered()), actionHandler, SLOT(ModificationScale()));
	connect(ui->actionModifyPlaceOnLandscape, SIGNAL(triggered()), actionHandler, SLOT(ModificationPlaceOnLand()));
	connect(ui->actionModifySnapToLandscape, SIGNAL(triggered()), actionHandler, SLOT(ModificationSnapToLand()));
	connect(ui->actionModifyReset, SIGNAL(triggered()), actionHandler, SLOT(OnResetModification()));

	actionHandler->RegisterModificationActions(ResourceEditor::MODIFY_COUNT,
											   ui->actionModifySelect,
											   ui->actionModifyMove,
											   ui->actionModifyRotate,
											   ui->actionModifyScale,
											   ui->actionModifyPlaceOnLandscape,
											   ui->actionModifySnapToLandscape);

	//Reference
	connect(ui->applyReferenceSuffixButton, SIGNAL(clicked()), this, SLOT(ApplyReferenceNodeSuffix()));

	actionHandler->MenuViewOptionsWillShow();
}

void QtMainWindow::SetupToolBars()
{
	// add combobox to select current view mode
	// TODO:
	// ... ->
	QComboBox *cbox = new QComboBox(NULL);
	cbox->addItem("Normal mode");
	cbox->addItem("Particles mode");
	cbox->addItem("Landscape mode");
	ui->viewModeToolBar->addWidget(cbox);
	// <-

	// add special modifications widget into modificationToolBar
	ModificationWidget* modificationWidget = new ModificationWidget(this);
	ui->modificationToolBar->insertWidget(ui->actionModifyReset, modificationWidget);
	connect(modificationWidget, SIGNAL(ApplyModification(double, double, double)), QtMainWindowHandler::Instance(), SLOT(OnApplyModification(double, double, double)));

	// let "reload" action from mainToolBar be displayed as icon+text
	ShowActionWithText(ui->mainToolBar, ui->actionReloadAll, true);

	// align part of actions from SceneGraph toolbar to the right
	QWidget* spacer = new QWidget(); 
	spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	ui->scenegraphToolBar->insertWidget(ui->actionAddNewEntity, spacer);
	ui->scenegraphToolBar->insertSeparator(ui->actionAddNewEntity);

	// let "add new entity" action from sceneGraphToolBar be displayed as icon+text
	ShowActionWithText(ui->scenegraphToolBar, ui->actionAddNewEntity, true);

	// align part of actions from Properties toolbar to the right
	spacer = new QWidget(); 
	spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	ui->propertiesToolBar->insertWidget(ui->actionAddNewComponent, spacer);
	ui->propertiesToolBar->insertSeparator(ui->actionAddNewComponent);

	// let "add new component" action from propertiesToolBar be displayed as icon+text
	ShowActionWithText(ui->propertiesToolBar, ui->actionAddNewComponent, true);

	// TODO:
	// remove this ->
	// (is done before merging development to to_master)

	cbox->setEnabled(false);
	ui->actionAddNewEntity->setEnabled(false);
	ui->actionRemoveEntity->setEnabled(false);
	ui->actionAddNewComponent->setEnabled(false);
	ui->actionRemoveComponent->setEnabled(false);

	// <-
}

void QtMainWindow::OpenLastProject()
{
    if(     !CommandLineTool::Instance()->CommandIsFound(String("-sceneexporter"))
       &&   !CommandLineTool::Instance()->CommandIsFound(String("-imagesplitter"))
       &&   !CommandLineTool::Instance()->CommandIsFound(String("-scenesaver")))
    {
        DAVA::FilePath projectPath = EditorSettings::Instance()->GetProjectPath();

        if(projectPath.IsEmpty())
        {
			projectPath = FilePath(ProjectManager::Instance()->ProjectOpenDialog().toAscii().constData());
        }

        if(projectPath.IsEmpty())
		{
			QtLayer::Instance()->Quit();
		}
		else
		{
			ProjectManager::Instance()->ProjectOpen(QString(projectPath.GetAbsolutePathname().c_str()));
		}
    }
}

void QtMainWindow::SetupDocks()
{
    connect(ui->actionRefreshSceneGraph, SIGNAL(triggered()), QtMainWindowHandler::Instance(), SLOT(RefreshSceneGraph()));
	connect(ui->dockParticleEditor->widget(), SIGNAL(ChangeVisible(bool)), this, SLOT(ChangeParticleDockVisible(bool)));
	connect(ui->dockParticleEditorTimeLine->widget(), SIGNAL(ChangeVisible(bool)), this, SLOT(ChangeParticleDockTimeLineVisible(bool)));
	connect(ui->dockParticleEditorTimeLine->widget(), SIGNAL(ValueChanged()), ui->dockParticleEditor->widget(), SLOT(OnUpdate()));
	connect(ui->dockParticleEditor->widget(), SIGNAL(ValueChanged()), ui->dockParticleEditorTimeLine->widget(), SLOT(OnUpdate()));
	
	connect(ui->cbShowDAEFiles, SIGNAL(stateChanged(int)), this, SLOT(LibraryFileTypesChanged()));
	connect(ui->cbShowSC2Files, SIGNAL(stateChanged(int)), this, SLOT(LibraryFileTypesChanged()));
	
	connect(this, SIGNAL(LibraryFileTypesChanged(bool, bool)), ui->libraryView, SLOT(LibraryFileTypesChanged(bool, bool)));
}

void QtMainWindow::ChangeParticleDockVisible(bool visible)
{
	if (ui->dockParticleEditor->isVisible() == visible)
		return;

	// ui magic :)
	bool isNeedEmitSignal = false;
	if (oldDockSceneGraphMaxSize.width() < 0)
	{
		oldDockSceneGraphMinSize = ui->dockSceneGraph->minimumSize();
		oldDockSceneGraphMaxSize = ui->dockSceneGraph->maximumSize();
		isNeedEmitSignal = true;
	}
	
	int minWidthParticleDock = ui->dockParticleEditor->minimumWidth();
	int maxWidthParticleDock = ui->dockParticleEditor->maximumWidth();
	
	ui->dockSceneGraph->setFixedWidth(ui->dockSceneGraph->width());
	ui->dockParticleEditor->setFixedWidth(ui->dockParticleEditor->width());
	
	ui->dockParticleEditor->setVisible(visible);
	
	ui->dockParticleEditor->setMinimumWidth(minWidthParticleDock);
	ui->dockParticleEditor->setMaximumWidth(maxWidthParticleDock);
	
	ui->dockSceneGraph->setFixedWidth(ui->dockSceneGraph->width());
	ui->dockParticleEditor->setVisible(visible);
	
	if (isNeedEmitSignal)
		QTimer::singleShot(1, this, SLOT(returnToOldMaxMinSizesForDockSceneGraph()));
}

void QtMainWindow::returnToOldMaxMinSizesForDockSceneGraph()
{
	ui->dockSceneGraph->setMinimumSize(oldDockSceneGraphMinSize);
	ui->dockSceneGraph->setMaximumSize(oldDockSceneGraphMaxSize);
	
	oldDockSceneGraphMinSize = QSize(-1, -1);
	oldDockSceneGraphMaxSize = QSize(-1, -1);
}

void QtMainWindow::ChangeParticleDockTimeLineVisible(bool visible)
{
	ui->dockParticleEditorTimeLine->setVisible(visible);
}

void QtMainWindow::ApplyReferenceNodeSuffix()
{
	QString qStr = ui->referenceSuffixEdit->text();
	QByteArray array = qStr.toLatin1();
	char * chars = array.data();
	String str(chars);

	SceneEditorScreenMain * screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
	Scene * scene = screen->FindCurrentBody()->bodyControl->GetScene();
	scene->SetReferenceNodeSuffix(str);
}

void QtMainWindow::ShowActionWithText(QToolBar *toolbar, QAction *action, bool showText)
{
	if(NULL != toolbar && NULL != action)
	{
		QToolButton *toolBnt = dynamic_cast<QToolButton *>(toolbar->widgetForAction(action));
		if(NULL != toolBnt)
		{
			if(showText)
			{
				toolBnt->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
			}
			else
			{
				toolBnt->setToolButtonStyle(Qt::ToolButtonIconOnly);
			}
		}
	}
}

bool QtMainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if(qApp == obj && ProjectManager::Instance()->IsOpened())
    {
        if(QEvent::ApplicationActivate == event->type())
        {
//            Logger::Debug("QEvent::ApplicationActivate");
            
            if(QtLayer::Instance())
            {
                QtLayer::Instance()->OnResume();
                Core::Instance()->GetApplicationCore()->OnResume();
            }

			// RepackAndReloadScene();			
        }
        else if(QEvent::ApplicationDeactivate == event->type())
        {
//            Logger::Debug("QEvent::ApplicationDeactivate");
            if(QtLayer::Instance())
            {
                QtLayer::Instance()->OnSuspend();
                Core::Instance()->GetApplicationCore()->OnSuspend();
            }
        }
    }
	else if (obj == ui->dockParticleEditor)
	{
		if (QEvent::Close == event->type())
		{
			event->ignore();
			ChangeParticleDockVisible(false);
			return true;
		}
	}
    
    return QMainWindow::eventFilter(obj, event);
}

void QtMainWindow::ProjectOpened(const QString &path)
{
	DAVA::String frameworkTitle = DAVA::Core::Instance()->GetOptions()->GetString("title");
	QString strVer = QString(frameworkTitle.c_str());

	if(!strVer.isEmpty())
	{
		strVer += " | ";
	}

	this->setWindowTitle(strVer + QString("Project - ") + path);
	UpdateLibraryFileTypes();
	UpdateParticleSprites();
}

bool QtMainWindow::TextureCheckConvetAndWait(bool forceConvertAll)
{
	bool ret = false;
	if(     CommandLineTool::Instance()
       &&   !CommandLineTool::Instance()->CommandIsFound(String("-sceneexporter"))
       &&   !CommandLineTool::Instance()->CommandIsFound(String("-scenesaver"))
       &&   !CommandLineTool::Instance()->CommandIsFound(String("-imagesplitter")) && NULL == convertWaitDialog)
	{
		// check if we have textures to convert - 
		// if we have function will return true and conversion will start in new thread
		// signal 'readyAll' will be emited when convention finishes
		if(TextureConvertor::Instance()->checkAndCompressAll(forceConvertAll))
		{
			ret = true;
			convertWaitDialog = new QProgressDialog(this);
			QObject::connect(TextureConvertor::Instance(), SIGNAL(readyAll()), convertWaitDialog, SLOT(close()));
			QObject::connect(TextureConvertor::Instance(), SIGNAL(convertStatus(const QString &, int, int)), this, SLOT(ConvertWaitStatus(const QString &, int, int)));
			QObject::connect(convertWaitDialog, SIGNAL(destroyed(QObject *)), this, SLOT(ConvertWaitDone(QObject *)));
			convertWaitDialog->setModal(true);
			convertWaitDialog->setCancelButton(NULL);
			convertWaitDialog->setAttribute(Qt::WA_DeleteOnClose);
			convertWaitDialog->setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::CustomizeWindowHint | Qt::WindowMinimizeButtonHint);
			convertWaitDialog->show();
		}
	}
	return ret;
}

void QtMainWindow::UpdateParticleSprites()
{
	if(repackSpritesWaitDialog != NULL)
	{
		return;
	}

	repackSpritesWaitDialog = new QResourceEditorProgressDialog(this, 0, true);

	SpritePackerHelper::Instance()->UpdateParticleSpritesAsync();
	
	QObject::connect(SpritePackerHelper::Instance(), SIGNAL(readyAll()), repackSpritesWaitDialog, SLOT(close()));
	QObject::connect(repackSpritesWaitDialog, SIGNAL(destroyed(QObject *)), this, SLOT(RepackSpritesWaitDone(QObject *)));

	repackSpritesWaitDialog->setModal(true);
	repackSpritesWaitDialog->setCancelButton(NULL);
	repackSpritesWaitDialog->setAttribute(Qt::WA_DeleteOnClose);
	repackSpritesWaitDialog->setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::CustomizeWindowHint | Qt::WindowMinimizeButtonHint);
	repackSpritesWaitDialog->setLabelText("Repack sprites...");
	repackSpritesWaitDialog->setRange(0, 100);
	repackSpritesWaitDialog->setValue(0);
	repackSpritesWaitDialog->show();
}

void QtMainWindow::RepackAndReloadScene()
{
	emitRepackAndReloadFinished = true;
	if(!TextureCheckConvetAndWait())
	{
		// conversion hasn't been started, run repack immediately 
		// in another case repack will be invoked in finishing callback (ConvertWaitDone)
		UpdateParticleSprites();
	}
}

void QtMainWindow::ConvertWaitDone(QObject *destroyed)
{
	convertWaitDialog = NULL;
	UpdateParticleSprites();
}

void QtMainWindow::RepackSpritesWaitDone(QObject *destroyed)
{
	if(emitRepackAndReloadFinished)
	{
		emit RepackAndReloadFinished();
	}

	emitRepackAndReloadFinished = false;
	repackSpritesWaitDialog = NULL;
}

void QtMainWindow::ConvertWaitStatus(const QString &curPath, int curJob, int jobCount)
{
	if(NULL != convertWaitDialog)
	{
		convertWaitDialog->setRange(0, jobCount);
		convertWaitDialog->setValue(curJob);
		convertWaitDialog->setLabelText(curPath);
	}
}

void QtMainWindow::LibraryFileTypesChanged()
{
	UpdateLibraryFileTypes();
}

void QtMainWindow::UpdateLibraryFileTypes()
{
	UpdateLibraryFileTypes(ui->cbShowDAEFiles->isChecked(), ui->cbShowSC2Files->isChecked());
}

void QtMainWindow::UpdateLibraryFileTypes(bool showDAEFiles, bool showSC2Files)
{
	emit LibraryFileTypesChanged(showDAEFiles, showSC2Files);
}

