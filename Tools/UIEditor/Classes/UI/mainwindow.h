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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include "ScreenWrapper.h"
#include "EditorSettings.h"

namespace Ui {
class MainWindow;
}

class QFileDialog;
class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
protected:
	virtual void resizeEvent(QResizeEvent *);
	virtual void showEvent(QShowEvent * event);
	virtual void closeEvent(QCloseEvent * event);

private slots:
    void on_scaleSpin_valueChanged(double arg1);
	void OnSliderMoved();
    void on_scaleSlider_valueChanged(int value);
    void OnOpenFontManager();
    void OnOpenLocalizationManager();
    void OnShowHelpContents();
	
	void OnNewProject();
	void OnSaveProject();
	void OnSaveProjectAll();
    void OnOpenProject();
	void OnCloseProject();
	void OnExitApplication();
	
	void OnNewPlatform();
	void OnNewScreen();
	void OnNewAggregator();

	void OnImportPlatform();
	void OnImportScreenOrAggregator();

	void OnProjectCreated();
	void OnSelectedScreenChanged();
	
	void OnUpdateScaleRequest(float scaleDelta);
	void OnUpdateScreenPositionRequest(const QPoint& posDelta);
	
	void FileMenuTriggered(QAction *resentScene);
	void MenuFileWillShow();

	void OnUndoRequested();
	void OnRedoRequested();
	
	void OnZoomInRequested();
	void OnZoomOutRequested();
	
	void OnUndoRedoAvailabilityChanged();
	void OnChangePropertySucceeded();

	void OnUnsavedChangesNumberChanged();

private:
	bool CloseProject();
	
	void UpdateSliders();
	void UpdateScreenPosition();
	
	void InitMenu();
	void UpdateMenu();
	void UpdateProjectSettings(const QString& filename);
	// Save/restore positions of DockWidgets and main window geometry
	void SaveMainWindowState();
	void RestoreMainWindowState();
	// Create toolbar for HierarchyTreeDockWidget
	void CreateHierarchyDockWidgetToolbar();

	// Save the full project or changes only.
	void DoSaveProject(bool changesOnly);

private:
    Ui::MainWindow *ui;
	QAction *recentPojectActions[EditorSettings::RECENT_FILES_COUNT];
	
	bool screenChangeUpdate;
};

#endif // MAINWINDOW_H
