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
#include "ui_mainwindow.h"

#include "DAVAEngine.h"

#include "HierarchyTreeController.h"
#include "rectpropertygridwidget.h"
#include "createplatformdlg.h"
#include "createplatformdlg.h"
#include "createscreendlg.h"
#include "Dialogs/createaggregatordlg.h"
#include "fontmanagerdialog.h"
#include "Dialogs/localizationeditordialog.h"
#include "ItemsCommand.h"
#include "CommandsController.h"
#include "FileSystem/FileSystem.h"
#include "ResourcesManageHelper.h"
#include "Dialogs/importdialog.h"
#include "ImportCommands.h"

#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include <QSettings>

#define SPIN_SCALE 10.f

const QString APP_NAME = "UIEditor";
const QString APP_COMPANY = "DAVA";
const QString APP_GEOMETRY = "geometry";
const QString APP_STATE = "windowstate";

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
	screenChangeUpdate = false;
	
    ui->setupUi(this);
    
	ui->davaGlWidget->setFocus();
    
	this->setWindowTitle(ResourcesManageHelper::GetProjectTitle());

	ui->scaleSlider->setMinimum((int)(ui->scaleSpin->minimum() * SPIN_SCALE));
	ui->scaleSlider->setMaximum((int)(ui->scaleSpin->maximum() * SPIN_SCALE));
	
	connect(ui->verticalScrollBar, SIGNAL(valueChanged(int)), this, SLOT(OnSliderMoved()));
	connect(ui->horizontalScrollBar, SIGNAL(valueChanged(int)), this, SLOT(OnSliderMoved()));

	ui->menuView->addAction(ui->hierarchyDockWidget->toggleViewAction());
	ui->menuView->addAction(ui->libraryDockWidget->toggleViewAction());
	ui->menuView->addAction(ui->propertiesDockWidget->toggleViewAction());

    connect(ui->actionFontManager, SIGNAL(triggered()), this, SLOT(OnOpenFontManager()));
    connect(ui->actionLocalizationManager, SIGNAL(triggered()), this, SLOT(OnOpenLocalizationManager()));

	connect(HierarchyTreeController::Instance(),
			SIGNAL(ProjectCreated()),
			this,
			SLOT(OnProjectCreated()));
	
	connect(HierarchyTreeController::Instance(),
			SIGNAL(ProjectClosed()),
			this,
			SLOT(OnProjectCreated()));
	
	connect(HierarchyTreeController::Instance(),
			SIGNAL(ProjectLoaded()),
			this,
			SLOT(OnProjectCreated()));
	
	connect(HierarchyTreeController::Instance(),
			SIGNAL(SelectedScreenChanged(const HierarchyTreeScreenNode*)),
			this,
			SLOT(OnSelectedScreenChanged()));
	
	connect(ui->hierarchyDockWidget->widget(),
			SIGNAL(CreateNewScreen()),
			this,
			SLOT(OnNewScreen()));
	
	connect(ui->hierarchyDockWidget->widget(),
			SIGNAL(CreateNewAggregator()),
			this,
			SLOT(OnNewAggregator()));

	connect(ui->hierarchyDockWidget->widget(),
			SIGNAL(ImportScreenOrAggregator()),
			this,
			SLOT(OnImportScreenOrAggregator()));

	connect(ScreenWrapper::Instance(),
			SIGNAL(UpdateScaleRequest(float)),
			this,
			SLOT(OnUpdateScaleRequest(float)));
	
	connect(ScreenWrapper::Instance(),
			SIGNAL(UpdateScreenPositionRequest(const QPoint&)),
			this,
			SLOT(OnUpdateScreenPositionRequest(const QPoint&)));
	
	this->ui->actionUndo->setEnabled(false);
	this->ui->actionRedo->setEnabled(false);
	connect(CommandsController::Instance(),
			SIGNAL(UndoRedoAvailabilityChanged()),
			this,
			SLOT(OnUndoRedoAvailabilityChanged()));
	
	connect(CommandsController::Instance(),
			SIGNAL(UndoRedoAvailabilityChanged()),
			this,
			SLOT(OnChangePropertySucceeded()));

	connect(CommandsController::Instance(),
			SIGNAL(UnsavedChangesNumberChanged()),
			this,
			SLOT(OnUnsavedChangesNumberChanged()));

	InitMenu();
	RestoreMainWindowState();
	CreateHierarchyDockWidgetToolbar();
}

MainWindow::~MainWindow()
{
    for(int32 i = 0; i < EditorSettings::RECENT_FILES_COUNT; ++i)
    {
        SafeDelete(recentPojectActions[i]);
    }
	
	SaveMainWindowState();
	
    delete ui;
}

void MainWindow::SaveMainWindowState()
{
	QSettings settings(APP_COMPANY, APP_NAME);
    settings.setValue(APP_GEOMETRY, saveGeometry());
    settings.setValue(APP_STATE, saveState());
}

void MainWindow::RestoreMainWindowState()
{
	QSettings settings(APP_COMPANY, APP_NAME);
	// Check settings befor applying it
	if (!settings.value(APP_GEOMETRY).isNull() && settings.value(APP_GEOMETRY).isValid())
	{
    	restoreGeometry(settings.value(APP_GEOMETRY).toByteArray());
	}
	if (!settings.value(APP_STATE).isNull() && settings.value(APP_STATE).isValid())
	{
    	restoreState(settings.value(APP_STATE).toByteArray());
	}
}

void MainWindow::CreateHierarchyDockWidgetToolbar()
{
	QMainWindow *window = new QMainWindow(0);
 	QToolBar *toolBar = new QToolBar(window);
	// Change size of icons to 16x16 (default is 32x32)
	toolBar->setIconSize(QSize(16, 16));
	// Set actions for toolbar
 	toolBar->addAction(ui->actionNew_platform);
	toolBar->addAction(ui->actionNew_screen);
	toolBar->addAction(ui->actionNew_aggregator);
	toolBar->addAction(ui->actionImport_Platform);
	// Disable moving of toolbar
	toolBar->setMovable(false);
	// Set toolbar position
	window->setCentralWidget(ui->hierarchyDockWidgetContents);
	window->addToolBar(toolBar);
	window->setParent(ui->hierarchyDockWidget);
	// link toolbar to hierarchyDockWidget
 	ui->hierarchyDockWidget->setWidget(window);
}

void MainWindow::OnUpdateScaleRequest(float scaleDelta)
{
	ui->scaleSpin->setValue(ui->scaleSpin->value() + scaleDelta);
}

void MainWindow::on_scaleSpin_valueChanged(double arg1)
{
	ui->scaleSlider->setValue(int(arg1 * SPIN_SCALE));
	ScreenWrapper::Instance()->SetScale((float)arg1);
	UpdateSliders();
	UpdateScreenPosition();
}

void MainWindow::on_scaleSlider_valueChanged(int value)
{
	double spinValue = value / SPIN_SCALE;
	if (Abs(spinValue - ui->scaleSpin->value()) > 0.01)
		ui->scaleSpin->setValue(spinValue);
}

void MainWindow::OnSliderMoved()
{
	UpdateScreenPosition();
}

void MainWindow::resizeEvent(QResizeEvent * event)
{
	QMainWindow::resizeEvent(event);

	UpdateSliders();
	UpdateScreenPosition();
}

void MainWindow::closeEvent(QCloseEvent * event)
{
	// Ask user to save the project before closing.
	if (!CloseProject())
	{
		event->ignore();
	}
}

void MainWindow::showEvent(QShowEvent * event)
{
	QMainWindow::showEvent(event);
	
	UpdateSliders();
	UpdateScreenPosition();
}

void MainWindow::OnSelectedScreenChanged()
{
	screenChangeUpdate = true;
	if (HierarchyTreeController::Instance()->GetActiveScreen())
	{
		ui->scaleSpin->setValue(HierarchyTreeController::Instance()->GetActiveScreen()->GetScale());

		UpdateSliders();
		ui->horizontalScrollBar->setValue(HierarchyTreeController::Instance()->GetActiveScreen()->GetPosX());
		ui->verticalScrollBar->setValue(HierarchyTreeController::Instance()->GetActiveScreen()->GetPosY());
		// Enable library widget for selected screen
		ui->libraryDockWidget->setEnabled(true);
	}
	else
	{	// Disable library widget if no screen is selected
		ui->libraryDockWidget->setEnabled(false);
	}
	
	screenChangeUpdate = false;
	
	UpdateScreenPosition();
}

void MainWindow::UpdateSliders()
{
	QRect widgetRect = ui->davaGlWidget->rect();
	QRect viewRect = ScreenWrapper::Instance()->GetRect();

	int valueV = viewRect.height() - widgetRect.height();
	if (valueV < 0) valueV = 0;
	int valueH = viewRect.width() - widgetRect.width();
	if (valueH < 0) valueH = 0;
	if (ui->verticalScrollBar->maximum() != valueV ||
		ui->horizontalScrollBar-> maximum() != valueH)
	{
		ui->verticalScrollBar->setMinimum(0);
		ui->verticalScrollBar->setMaximum(valueV);
		ui->horizontalScrollBar->setMinimum(0);
		ui->horizontalScrollBar->setMaximum(valueH);
	}
}

void MainWindow::UpdateScreenPosition()
{
	if (screenChangeUpdate)
		return;
	
	int valueV = ui->verticalScrollBar->value();
	int valueH = ui->horizontalScrollBar->value();
	ScreenWrapper::Instance()->SetViewPos(valueH, valueV, ui->davaGlWidget->rect());
}

void MainWindow::OnUpdateScreenPositionRequest(const QPoint& posDelta)
{
	ui->horizontalScrollBar->setValue(ui->horizontalScrollBar->value() + posDelta.x());
	ui->verticalScrollBar->setValue(ui->verticalScrollBar->value() + posDelta.y());
}

void MainWindow::OnOpenFontManager()
{
    FontManagerDialog *fontManagerDialog = new FontManagerDialog();
    if (fontManagerDialog->exec())
    {
        delete fontManagerDialog;
    }
}

void MainWindow::OnOpenLocalizationManager()
{
    LocalizationEditorDialog *localizationManagerDialog = new LocalizationEditorDialog();
    if (localizationManagerDialog->exec())
    {
        delete localizationManagerDialog;
    }
}

void MainWindow::OnShowHelpContents()
{
    //Get help contents file absolute path
    QString helpPath = "file:///";
    helpPath += ResourcesManageHelper::GetHelpContentsPath();
    //Open help file in default browser new window
    QDesktopServices::openUrl(QUrl(helpPath));
}

void MainWindow::InitMenu()
{
	ui->menuView->addAction(ui->hierarchyDockWidget->toggleViewAction());
	ui->menuView->addAction(ui->libraryDockWidget->toggleViewAction());
	ui->menuView->addAction(ui->propertiesDockWidget->toggleViewAction());
    ui->menuView->addAction(ui->actionZoomIn);
    ui->menuView->insertSeparator(ui->actionZoomIn);
    ui->menuView->addAction(ui->actionZoomOut);

	connect(ui->actionNew_project, SIGNAL(triggered()), this, SLOT(OnNewProject()));
	connect(ui->actionSave_project, SIGNAL(triggered()), this, SLOT(OnSaveProject()));
	connect(ui->actionSave_All, SIGNAL(triggered()), this, SLOT(OnSaveProjectAll()));
    connect(ui->actionOpen_project, SIGNAL(triggered()), this, SLOT(OnOpenProject()));
	connect(ui->actionClose_project, SIGNAL(triggered()), this, SLOT(OnCloseProject()));

	connect(ui->actionNew_platform, SIGNAL(triggered()), this, SLOT(OnNewPlatform()));
	connect(ui->actionNew_screen, SIGNAL(triggered()), this, SLOT(OnNewScreen()));
	connect(ui->actionNew_aggregator, SIGNAL(triggered()), this, SLOT(OnNewAggregator()));
	connect(ui->actionImport_Platform, SIGNAL(triggered()), this, SLOT(OnImportPlatform()));

	connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(OnExitApplication()));
	
	connect(ui->menuFile, SIGNAL(aboutToShow()), this, SLOT(MenuFileWillShow()));
    connect(ui->menuFile, SIGNAL(triggered(QAction *)), this, SLOT(FileMenuTriggered(QAction *)));

    connect(ui->actionZoomIn, SIGNAL(triggered()), this, SLOT(OnZoomInRequested()));
    connect(ui->actionZoomOut, SIGNAL(triggered()), this, SLOT(OnZoomOutRequested()));
	// Remap zoom in/out shorcuts for windows platform
#if defined(__DAVAENGINE_WIN32__)
	QList<QKeySequence> shortcuts;
	shortcuts.append(QKeySequence(Qt::CTRL + Qt::Key_Equal));
	shortcuts.append(QKeySequence(Qt::CTRL + Qt::Key_Plus));
	ui->actionZoomIn->setShortcuts(shortcuts);
#endif
	//Create empty actions for recent projects files
	for(int32 i = 0; i < EditorSettings::RECENT_FILES_COUNT; ++i)
    {
        recentPojectActions[i] = new QAction(this);
        recentPojectActions[i]->setObjectName(QString::fromUtf8(Format("recentPojectActions[%d]", i)));
    }
	
	//Help contents dialog
    connect(ui->actionHelpContents, SIGNAL(triggered()), this, SLOT(OnShowHelpContents()));
	
	// Undo/Redo.
	connect(ui->actionUndo, SIGNAL(triggered()), this, SLOT(OnUndoRequested()));
	connect(ui->actionRedo, SIGNAL(triggered()), this, SLOT(OnRedoRequested()));

	UpdateMenu();
}

void MainWindow::UpdateMenu()
{
	//ui->actionNew
	ui->actionSave_project->setEnabled(HierarchyTreeController::Instance()->GetTree().IsProjectCreated());
	ui->actionSave_All->setEnabled(HierarchyTreeController::Instance()->GetTree().IsProjectCreated());
	ui->actionClose_project->setEnabled(HierarchyTreeController::Instance()->GetTree().IsProjectCreated());
	ui->menuProject->setEnabled(HierarchyTreeController::Instance()->GetTree().IsProjectCreated());
	ui->actionNew_platform->setEnabled(HierarchyTreeController::Instance()->GetTree().IsProjectCreated());
	ui->actionNew_screen->setEnabled(HierarchyTreeController::Instance()->GetTree().GetPlatforms().size());
	ui->actionNew_aggregator->setEnabled(HierarchyTreeController::Instance()->GetTree().GetPlatforms().size());
	ui->actionImport_Platform->setEnabled(HierarchyTreeController::Instance()->GetTree().GetPlatforms().size());
}

void MainWindow::OnNewProject()
{
	if (!CloseProject())
		return;
	
	QString projectDir = QFileDialog::getExistingDirectory(this, tr("Choose new project folder"),
											ResourcesManageHelper::GetDefaultDirectory());
				
	if (projectDir.isNull() || projectDir.isEmpty())
		return;
	// Convert directory path into Unix-style path
	projectDir = ResourcesManageHelper::ConvertPathToUnixStyle(projectDir);

	CommandsController::Instance()->CleanupUndoRedoStack();
	if (!HierarchyTreeController::Instance()->NewProject(projectDir))
	{
		QMessageBox msgBox;
		msgBox.setText(tr("Error while creating project"));
		msgBox.exec();
	}
}

void MainWindow::OnProjectCreated()
{
	UpdateMenu();
	// Release focus from Dava GL widget, so after the first click to it
	// it will lock the keyboard and will process events successfully.
	ui->hierarchyDockWidget->setFocus();
}

void MainWindow::OnNewPlatform()
{
	CreatePlatformDlg dlg(this);
	if (dlg.exec() == QDialog::Accepted)
	{
		CreatePlatformCommand* cmd = new CreatePlatformCommand(dlg.GetPlatformName(), Vector2(dlg.GetWidth(), dlg.GetHeight()));
		CommandsController::Instance()->ExecuteCommand(cmd);
		SafeRelease(cmd);
		UpdateMenu();
	}
}

void MainWindow::OnNewScreen()
{
	HierarchyTreeNode::HIERARCHYTREENODEID id = HierarchyTreeNode::HIERARCHYTREENODEID_EMPTY;
	HierarchyTreeNode* node = HierarchyTreeController::Instance()->GetActivePlatform();
	if (node)
		id = node->GetId();

	CreateScreenDlg dlg(this);
	dlg.SetDefaultPlatform(id);
	if (dlg.exec() == QDialog::Accepted)
	{
		CreateScreenCommand* cmd = new CreateScreenCommand(dlg.GetScreenName(), dlg.GetPlatformId());
		CommandsController::Instance()->ExecuteCommand(cmd);
		SafeRelease(cmd);
	}
}

void MainWindow::OnNewAggregator()
{
	HierarchyTreeNode::HIERARCHYTREENODEID id = HierarchyTreeNode::HIERARCHYTREENODEID_EMPTY;
	HierarchyTreeNode* node = HierarchyTreeController::Instance()->GetActivePlatform();
	if (node)
		id = node->GetId();
	
	CreateAggregatorDlg dlg(this);
	dlg.SetDefaultPlatform(id);
	if (dlg.exec() == QDialog::Accepted)
	{
		CreateAggregatorCommand* cmd = new CreateAggregatorCommand(dlg.GetName(), dlg.GetPlatformId(), dlg.GetRect());
		CommandsController::Instance()->ExecuteCommand(cmd);
		SafeRelease(cmd);
	}
}

void MainWindow::OnImportPlatform()
{
	QString platformsPath = ResourcesManageHelper::GetPlatformRootPath(HierarchyTreeController::Instance()->GetTree().GetRootNode()->GetProjectDir());

	QString selectedDir = QFileDialog::getExistingDirectory(this, tr("Select platform to import"),
															platformsPath,
															QFileDialog::DontResolveSymlinks |
															QFileDialog::ReadOnly |
															QFileDialog::ShowDirsOnly);

	if (selectedDir.isEmpty())
	{
		return;
	}

	if (!selectedDir.startsWith(platformsPath))
	{
		QMessageBox::critical(this, tr("Import error"),
							  tr("Only the platforms inside current project directory could be imported"));
		return;
	}

	FilePath selectedDirPath(selectedDir.toStdString());
	String platformName = selectedDirPath.GetFilename();

    selectedDirPath.MakeDirectoryPathname();
	ImportDialog importDlg(ImportDialog::IMPORT_PLATFORM, this, selectedDirPath);
	if (importDlg.exec() == QDialog::Accepted)
	{
		QSize size = importDlg.GetPlatformSize();
		Vector<ImportDialog::FileItem> files = importDlg.GetFiles();

		ImportPlatformCommand* cmd = new ImportPlatformCommand(selectedDir,
															   QString::fromStdString(platformName),
															   size, files);
		CommandsController::Instance()->ExecuteCommand(cmd);
		SafeRelease(cmd);
	}
}

void MainWindow::OnImportScreenOrAggregator()
{
	ImportDialog importDlg(ImportDialog::IMPORT_SCREEN, this);
	if (importDlg.exec() == QDialog::Accepted)
	{
		HierarchyTreeNode::HIERARCHYTREENODEID platformId = importDlg.GetPlatformId();
		Vector<ImportDialog::FileItem> files = importDlg.GetFiles();

		ImportScreensCommand* cmd = new ImportScreensCommand(platformId, files);
		CommandsController::Instance()->ExecuteCommand(cmd);
		SafeRelease(cmd);
	}
}

void MainWindow::MenuFileWillShow()
{
	// Delete old list of recent project actions
	for(DAVA::int32 i = 0; i < EditorSettings::RECENT_FILES_COUNT; ++i)
    {
        if(recentPojectActions[i]->parentWidget())
        {
            ui->menuFile->removeAction(recentPojectActions[i]);
        }
    }
    // Get up to date count of recent project actions
    int32 projectCount = EditorSettings::Instance()->GetLastOpenedCount();
    if(projectCount > 0)
    {
        QList<QAction *> recentActions;
        for(int32 i = 0; i < projectCount; ++i)
        {
            recentPojectActions[i]->setText(QString(EditorSettings::Instance()->GetLastOpenedFile(i).c_str()));
            recentActions.push_back(recentPojectActions[i]);
        }
        // Insert recent project actions into file menu
        ui->menuFile->insertActions(ui->actionExit, recentActions);
        ui->menuFile->insertSeparator(ui->actionExit);
    }
}

void MainWindow::FileMenuTriggered(QAction *resentScene)
{
    for(int32 i = 0; i < EditorSettings::RECENT_FILES_COUNT; ++i)
    {
		// Check if user clicked on one of the recent project link
        if(resentScene == recentPojectActions[i])
        {
			// Close and save current project if any
			if (!CloseProject())
				return;
		
			QString projectPath = QString::fromStdString(EditorSettings::Instance()->GetLastOpenedFile(i));
			if (projectPath.isNull())
				return;

			if (HierarchyTreeController::Instance()->Load(projectPath))
			{
				// Update project title if project was successfully loaded
				UpdateProjectSettings(projectPath);				
			}
			else
			{
				QMessageBox msgBox;
				msgBox.setText(tr("Error while loading project"));
				msgBox.exec();
			}
			// We don't need to continue circle when selected menu item action ended
			return;
        }
    }
}

void MainWindow::DoSaveProject(bool changesOnly)
{
	QString projectPath = HierarchyTreeController::Instance()->GetTree().GetActiveProjectPath();

	if (projectPath.isNull() || projectPath.isEmpty())
		return;

	HierarchyTreeController* controller = HierarchyTreeController::Instance();
	bool saveSucceeded = changesOnly ? controller->SaveOnlyChangedScreens(projectPath) :
		controller->SaveAll(projectPath);

	if (saveSucceeded)
	{
		// If project was successfully saved - we should save new project file path
		// and add this project to recent files list
		UpdateProjectSettings(ResourcesManageHelper::GetProjectFilePath(projectPath));
	}
	else
	{
		QMessageBox msgBox;
		msgBox.setText(tr("Error while saving project"));
		msgBox.exec();
	}
}

void MainWindow::OnOpenProject()
{
	QString projectPath = QFileDialog::getOpenFileName(this, tr("Select a project file"),
														ResourcesManageHelper::GetDefaultDirectory(),
														tr( "Project (*.uieditor)"));
    if (projectPath.isNull() || projectPath.isEmpty())
        return;

	// Convert file path into Unix-style path
	projectPath = ResourcesManageHelper::ConvertPathToUnixStyle(projectPath);
        
	CommandsController::Instance()->CleanupUndoRedoStack();
	if (HierarchyTreeController::Instance()->Load(projectPath))
	{
		// If project was successfully loaded - we should save project path
		// and add this project to recent files list
		UpdateProjectSettings(projectPath);
	}
	else
	{
		QMessageBox msgBox;
		msgBox.setText(tr("Error while loading project"));
		msgBox.exec();
	}
}

void MainWindow::OnSaveProject()
{
	DoSaveProject(true);
}

void MainWindow::OnSaveProjectAll()
{
	DoSaveProject(false);
}

void MainWindow::OnCloseProject()
{
	CloseProject();
}

void MainWindow::OnExitApplication()
{
	if (CloseProject())
	{
		QCoreApplication::exit();
	}
}

bool MainWindow::CloseProject()
{
	bool lastChangeSaved = !HierarchyTreeController::Instance()->HasUnsavedChanges();
	if (!lastChangeSaved)
	{
		int ret = QMessageBox::warning(this, qApp->applicationName(),
									   tr("The project has been modified.\n"
										  "Do you want to save your changes?"),
									   QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
									   QMessageBox::Save);
		if (ret == QMessageBox::Cancel)
			return false;
		else if (ret == QMessageBox::Save)
			OnSaveProject();
	}
	
	HierarchyTreeController::Instance()->CloseProject();
	// Update project title
	this->setWindowTitle(ResourcesManageHelper::GetProjectTitle());
	
	return true;
}

void MainWindow::UpdateProjectSettings(const QString& projectPath)
{
	// Add file to recent project files list
	EditorSettings::Instance()->AddLastOpenedFile(projectPath.toStdString());
	
	// Save to settings default project directory
	QFileInfo fileInfo(projectPath);
	QString projectDir = fileInfo.absoluteDir().absolutePath();
	EditorSettings::Instance()->SetProjectPath(projectDir.toStdString());

	// Update window title
	this->setWindowTitle(ResourcesManageHelper::GetProjectTitle(projectPath));
}

void MainWindow::OnUndoRequested()
{
	CommandsController::Instance()->Undo();
}

void MainWindow::OnRedoRequested()
{
	CommandsController::Instance()->Redo();
}

void MainWindow::OnZoomInRequested()
{
	OnUpdateScaleRequest(ui->scaleSpin->singleStep());
}

void MainWindow::OnZoomOutRequested()
{
	OnUpdateScaleRequest(ui->scaleSpin->singleStep() * (-1));
}

void MainWindow::OnUndoRedoAvailabilityChanged()
{
	this->ui->actionUndo->setEnabled(CommandsController::Instance()->IsUndoAvailable());
	this->ui->actionRedo->setEnabled(CommandsController::Instance()->IsRedoAvailable());
}

void MainWindow::OnChangePropertySucceeded()
{
	OnSelectedScreenChanged();
}

void MainWindow::OnUnsavedChangesNumberChanged()
{
	QString projectTitle = ResourcesManageHelper::GetProjectTitle();
	if (HierarchyTreeController::Instance()->HasUnsavedChanges())
	{
		projectTitle += " *";
	}
	setWindowTitle(projectTitle);
}
