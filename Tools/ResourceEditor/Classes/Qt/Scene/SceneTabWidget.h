#ifndef __SCENE_TAB_WIDGET_H__
#define __SCENE_TAB_WIDGET_H__

#include <QMap>
#include <QTabBar>
#include <QWidget>
#include <QMetaType>

#include "Qt/Main/davaglwidget.h"

#include "UI/UIScreen.h"
#include "UI/UI3DView.h"

// old ui. should be removed later -->
class SceneEditorScreenMain;
// <--
class SceneEditorProxy;
class DAVAUI3DView;

Q_DECLARE_METATYPE(SceneEditorProxy *);

class SceneTabWidget : public QWidget
{
	Q_OBJECT

public:
	SceneTabWidget(QWidget *parent);
	~SceneTabWidget();

	int OpenTab();
	int OpenTab(const DAVA::FilePath &scenePapth);

	void CloseTab(int index);

	int GetCurrentTab();
	void SetCurrentTab(int index);
	
public slots:
	// this slot redirects any UIEvent to the active sceneProxy for processing
	void ProcessDAVAUIEvent(DAVA::UIEvent *event);

	// tab switched by user
	void TabBarCurrentChanged(int index);

	// tab request close
	void TabBarCloseRequest(int index);

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
	//int currentTabIndex;
	//int currentTabID;

	DavaGLWidget *davaWidget;
	DAVA::UIScreen *davaUIScreen;
	DAVA::UI3DView *dava3DView;
	const int davaUIScreenID;
	const int dava3DViewMargin;

	//QMap<int, SceneEditorProxy* > tabIDtoSceneMap;

	void InitDAVAUI();
	void ReleaseDAVAUI();

	//int GetTabIndex(int tabID);
	//int GetTabID(int index);
	SceneEditorProxy* GetTabScene(int index);
	void SetTabScene(int index, SceneEditorProxy* scene);

	virtual void resizeEvent(QResizeEvent * event);

private:
	int newSceneCounter;
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
