/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA, INC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA, INC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "mainwindow.h"

#include "DAVAEngine.h"
#include "Classes/Qt/Main/QtMainWindowHandler.h"
#include "Classes/Qt/Scene/SceneDataManager.h"
#include "Classes/SceneEditor/EditorSettings.h"
#include "Classes/CommandLine/CommandLineManager.h"
#include "Classes/Qt/TextureBrowser/TextureConvertor.h"
#include "Classes/Qt/DockSceneGraph/PointerHolder.h"
#include "DockConsole/Console.h"
#include "Classes/Qt/Project/ProjectManager.h"
#include "DockLibrary/LibraryModel.h"

#include <QToolBar>
#include <QToolButton>

#include "../SceneEditor/SceneEditorScreenMain.h"
#include "../SceneEditor/EditorBodyControl.h"
#include "../SceneEditor/EditorConfig.h"
#include "SpritesPacker/SpritePackerHelper.h"
#include "Tools/QResourceEditorProgressDialog/QResourceEditorProgressDialog.h"

#include <QApplication>
#include <QPixmap>

#include "ModificationWidget.h"


QtMainWindow::QtMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
	, oldDockSceneGraphMinSize(-1, -1)
	, oldDockSceneGraphMaxSize(-1, -1)
	, repackSpritesWaitDialog(NULL)
	, emitRepackAndReloadFinished(false)
{
	// initialize console
	Console::Instance();

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
	ChangeParticleDockVisible(false, true); //hide particle editor dock on start up
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
    
    ui->actionExportPVRIOS->setData(GPU_POWERVR_IOS);
    ui->actionExportPVRAndroid->setData(GPU_POWERVR_ANDROID);
    ui->actionExportTegra->setData(GPU_TEGRA);
    ui->actionExportMali->setData(GPU_MALI);
    ui->actionExportAdreno->setData(GPU_ADRENO);
    ui->actionExportPNG->setData(GPU_UNKNOWN);
	connect(ui->menuExport, SIGNAL(triggered(QAction *)), actionHandler, SLOT(ExportMenuTriggered(QAction *)));
    
    
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
    connect(ui->actionSquareTextures, SIGNAL(triggered()), actionHandler, SLOT(SquareTextures()));
    connect(ui->actionShowMipmapLevel, SIGNAL(triggered()), actionHandler, SLOT(ReplaceZeroMipmaps()));
    
#if defined (__DAVAENGINE_MACOS__)
    ui->menuTools->removeAction(ui->actionBeast);
#else //#if defined (__DAVAENGINE_MACOS__)
	connect(ui->actionBeast, SIGNAL(triggered()), actionHandler, SLOT(Beast()));
#endif //#if defined (__DAVAENGINE_MACOS__)
    
	//Edit
	connect(ui->actionConvertToShadow, SIGNAL(triggered()), actionHandler, SLOT(ConvertToShadow()));
    
    ui->actionEnableCameraLight->setChecked(EditorSettings::Instance()->GetShowEditorCamerLight());
	connect(ui->actionEnableCameraLight, SIGNAL(triggered()), actionHandler, SLOT(CameraLightTrigerred()));

    //Help
    connect(ui->actionHelp, SIGNAL(triggered()), actionHandler, SLOT(OpenHelp()));
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
	QAction *actionSceneTree = ui->dockSceneTree->toggleViewAction();
	QAction *actionConsole = ui->dockConsole->toggleViewAction();

    ui->menuView->addAction(actionSceneInfo);
    ui->menuView->addAction(actionToolBar);
    ui->menuView->addAction(actionLibrary);
    ui->menuView->addAction(actionProperties);
	ui->menuView->addAction(actionReferences);
    ui->menuView->addAction(actionSceneGraph);
    ui->menuView->addAction(actionCustomColors);
	ui->menuView->addAction(actionVisibilityCheckTool);
	ui->menuView->addAction(actionParticleEditor);
	ui->menuView->addAction(actionParticleEditorTimeLine);
	ui->menuView->addAction(actionHangingObjects);
	ui->menuView->addAction(actionSetSwitchIndex);
	ui->menuView->addAction(actionSceneTree);
	ui->menuView->addAction(actionConsole);

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

    ui->actionReloadPoverVRIOS->setData(GPU_POWERVR_IOS);
    ui->actionReloadPoverVRAndroid->setData(GPU_POWERVR_ANDROID);
    ui->actionReloadTegra->setData(GPU_TEGRA);
    ui->actionReloadMali->setData(GPU_MALI);
    ui->actionReloadAdreno->setData(GPU_ADRENO);
    ui->actionReloadPNG->setData(GPU_UNKNOWN);
	connect(ui->menuTexturesForGPU, SIGNAL(triggered(QAction *)), actionHandler, SLOT(ReloadMenuTriggered(QAction *)));
    actionHandler->RegisterTextureGPUActions(GPU_FAMILY_COUNT + 1, ui->actionReloadPoverVRIOS, ui->actionReloadPoverVRAndroid,
                                             ui->actionReloadTegra, ui->actionReloadMali, ui->actionReloadAdreno, ui->actionReloadPNG);

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

	QAction *undoSceneEditor2 = new QAction("Undo2", this);
	QAction *redoSceneEditor2 = new QAction("Redo2", this);

	QObject::connect(undoSceneEditor2, SIGNAL(triggered()), this, SLOT(Undo2()));
	QObject::connect(redoSceneEditor2, SIGNAL(triggered()), this, SLOT(Redo2()));
	ui->viewModeToolBar->addAction(undoSceneEditor2);
	ui->viewModeToolBar->addAction(redoSceneEditor2);


	// <-
}

void QtMainWindow::OpenLastProject()
{
    if(CommandLineManager::Instance() && !CommandLineManager::Instance()->IsCommandLineModeEnabled())
    {
        DAVA::FilePath projectPath = EditorSettings::Instance()->GetProjectPath();

        if(projectPath.IsEmpty())
        {
			projectPath = FilePath(ProjectManager::Instance()->ProjectOpenDialog().toStdString());
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
	
	// Yuri Coder. Automatic show/hide of the Particles Timeline
	// is disabled due to DF-1421.
	//connect(ui->dockParticleEditor->widget(), SIGNAL(ChangeVisible(bool)), this, SLOT(ChangeParticleDockVisible(bool)));

	connect(ui->dockParticleEditorTimeLine->widget(), SIGNAL(ChangeVisible(bool)), this, SLOT(ChangeParticleDockTimeLineVisible(bool)));
	connect(ui->dockParticleEditorTimeLine->widget(), SIGNAL(ValueChanged()), ui->dockParticleEditor->widget(), SLOT(OnUpdate()));
	connect(ui->dockParticleEditor->widget(), SIGNAL(ValueChanged()), ui->dockParticleEditorTimeLine->widget(), SLOT(OnUpdate()));
	
	connect(ui->cbShowDAEFiles, SIGNAL(stateChanged(int)), this, SLOT(LibraryFileTypesChanged()));
	connect(ui->cbShowSC2Files, SIGNAL(stateChanged(int)), this, SLOT(LibraryFileTypesChanged()));
	
	connect(this, SIGNAL(LibraryFileTypesChanged(bool, bool)), ui->libraryView, SLOT(LibraryFileTypesChanged(bool, bool)));
}

void QtMainWindow::ChangeParticleDockVisible(bool visible, bool forceUpdate)
{
	if (!forceUpdate && (ui->dockParticleEditor->isVisible() == visible))
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
	// Yuri Coder. Automatic show/hide of the Particles Timeline
	// is disabled due to DF-1234.
	// ui->dockParticleEditorTimeLine->setVisible(visible);
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

void QtMainWindow::Undo2()
{
	SceneEditor2* sceneEditor = ui->sceneTabWidget->GetCurrentScene();
	if(NULL != sceneEditor)
	{
		sceneEditor->Undo();
	}
}

void QtMainWindow::Redo2()
{
	SceneEditor2* sceneEditor = ui->sceneTabWidget->GetCurrentScene();
	if(NULL != sceneEditor)
	{
		sceneEditor->Redo();
	}
}
