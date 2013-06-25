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
#include <QProgressDialog>
#include "Base/Singleton.h"
#include "QtPosSaver/QtPosSaver.h"
#include "ui_mainwindow.h"

class LibraryModel;
class QtMainWindow : public QMainWindow, public DAVA::Singleton<QtMainWindow>
{
    Q_OBJECT
    
public:
	explicit QtMainWindow(QWidget *parent = 0);
	~QtMainWindow();

	Ui::MainWindow* GetUI();
    
    virtual bool eventFilter(QObject *, QEvent *);
    
private:
	void OpenLastProject();

	void SetupActions();
    void SetupMainMenu();
	void SetupToolBars();
    void SetupDocks();

    void SetupCustomColorsDock();
	void SetupVisibilityToolDock();
    
    void SetCustomColorsDockControlsEnabled(bool enabled);

	void UpdateLibraryFileTypes();
	void UpdateLibraryFileTypes(bool showDAEFiles, bool showSC2Files);

public slots:
	void ShowActionWithText(QToolBar *toolbar, QAction *action, bool showText);

	void ChangeParticleDockVisible(bool visible);
	void ChangeParticleDockTimeLineVisible(bool visible);
	void returnToOldMaxMinSizesForDockSceneGraph();

	//return true if conversion has been started
	void UpdateParticleSprites();
	void RepackAndReloadScene();

	void Undo2();
	void Redo2();

private slots:
	void ProjectOpened(const QString &path);
	void LibraryFileTypesChanged();
	
	//reference
	void ApplyReferenceNodeSuffix();
	void RepackSpritesWaitDone(QObject *destroyed);

signals:
	// Library File Types.
	void LibraryFileTypesChanged(bool showDAEFiles, bool showSC2Files);
	void RepackAndReloadFinished();

private:
    Ui::MainWindow *ui;
	QtPosSaver posSaver;

	QProgressDialog *repackSpritesWaitDialog;
    
	QSize oldDockSceneGraphMaxSize;
	QSize oldDockSceneGraphMinSize;

	bool emitRepackAndReloadFinished;
};


#endif // MAINWINDOW_H
