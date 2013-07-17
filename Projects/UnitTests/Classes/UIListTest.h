//
//  UIListTest.h
//  TemplateProjectMacOS
//
//  Created by Denis Bespalov on 4/9/13.
//
//

#ifndef __UILIST_TEST_H__
#define __UILIST_TEST_H__

#include "DAVAEngine.h"

using namespace DAVA;

#include "TestTemplate.h"

class UIListTestDelegate: public UIControl, public UIListDelegate
{

public:
	UIListTestDelegate(const Rect &rect = Rect(), bool rectInAbsoluteCoordinates = false);
	virtual ~UIListTestDelegate();

	// UIListDelegate
    virtual int32 ElementsCount(UIList *list);
	virtual UIListCell *CellAtIndex(UIList *list, int32 index);
	virtual int32 CellHeight(UIList *list, int32 index);

private:
	Vector2 cellSize;
};


class UIListTest: public TestTemplate<UIListTest>
{

public:
	UIListTest();

	virtual void LoadResources();
	virtual void UnloadResources();
	virtual bool RunTest(int32 testNum);
	
	virtual void DidAppear();	
	virtual void Update(float32 timeElapsed);
	
	void TestFunction(PerfFuncData * data);
	
private:
	void ButtonPressed(BaseObject *obj, void *data, void *callerData);
	
private:
	UIButton* finishTestBtn;
	bool testFinished;
		
	float32 onScreenTime;
};

#endif /* defined(__UILIST_TEST_H__) */
