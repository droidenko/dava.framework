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

#ifndef __SCENE_TAB_WIDGET_H__
#define __SCENE_TAB_WIDGET_H__

#include <QMap>
#include <QTabBar>
#include <QWidget>
#include <QMetaType>

#include "Qt/Main/davaglwidget.h"
#include "Qt/Scene/EntityGroup.h"
#include "Qt/Scene/SceneSignals.h"
#include "Qt/Scene/SceneTypes.h"

#include "UI/UIScreen.h"
#include "UI/UI3DView.h"

// old ui. should be removed later -->
class SceneEditorScreenMain;
// <--
class SceneEditor2;
class DAVAUI3DView;

Q_DECLARE_METATYPE(SceneEditor2 *);

class SceneTabWidget : public QWidget
{
	Q_OBJECT

public:
	SceneTabWidget(QWidget *parent);
	~SceneTabWidget();

	int OpenTab();
	int OpenTab(const DAVA::FilePath &scenePapth);
	void CloseTab(int index);

	int GetCurrentTab() const;
	void SetCurrentTab(int index);

	ST_ModifMode GetModifMode() const;
	void SetModifMode(ST_ModifMode mode);

	ST_PivotPoint GetPivotPoint() const;
	void SetPivotPoint(ST_PivotPoint pivotpoint);

	ST_Axis GetModifAxis() const;
	void SetModifAxis(ST_Axis axis);

	int GetSelectionDrawMode() const;
	void SetSelectionDrawMode(int mode);

	int GetCollisionDrawMode() const;
	void SetCollisionDrawMode(int mode);
	
	SceneEditor2* GetSceneEditorForCurrentTab() const;

public slots:
	// this slot redirects any UIEvent to the active sceneProxy for processing
	void ProcessDAVAUIEvent(DAVA::UIEvent *event);

	// tab switched by user
	void TabBarCurrentChanged(int index);

	// tab request close
	void TabBarCloseRequest(int index);

	// scene mouse over selected object
	void MouseOverSelectedEntities(SceneEditor2* scene, const EntityGroup *entities);

// old ui. should be removed later -->
protected:
	SceneEditorScreenMain * sceneEditorScreenMain;
	const int oldScreenID;
	bool oldInput;

	void InitOldUI();
	void ReleaseOldUI();
// <--

protected:
	QTabBar *tabBar;
	DavaGLWidget *davaWidget;
	DAVA::UIScreen *davaUIScreen;
	DAVA::UI3DView *dava3DView;
	const int davaUIScreenID;
	const int dava3DViewMargin;

	void InitDAVAUI();
	void ReleaseDAVAUI();

	SceneEditor2* GetTabScene(int index) const;
	void SetTabScene(int index, SceneEditor2* scene);

	virtual void resizeEvent(QResizeEvent * event);

private:
	int newSceneCounter;

	SceneEditor2 *curScene;
	ST_Axis curModifAxis;
	ST_ModifMode curModifMode;
	ST_PivotPoint curPivotPoint;
	int curSelDrawMode;
	int curColDrawMode;
};

// this is helper class
// it is only used to grab all UIEvents from 3dView and
// redirect them to the specified SceneTabWidget
class DAVAUI3DView : public DAVA::UI3DView
{
public:
	DAVAUI3DView(SceneTabWidget *tw, const DAVA::Rect &rect) 
		: DAVA::UI3DView(rect)
		, tabWidget(tw)
	{
		SetInputEnabled(true);
	}

	virtual void Input(DAVA::UIEvent *event)
	{
		if(NULL != tabWidget)
		{
			tabWidget->ProcessDAVAUIEvent(event);
		}
	}

protected:
	SceneTabWidget *tabWidget;
};

#endif // __SCENE_TAB_WIDGET_H__
