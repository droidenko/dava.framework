#ifndef __TEMPLATEPROJECTIPHONE__TRANSPARENTWEBVIEWTEST__
#define __TEMPLATEPROJECTIPHONE__TRANSPARENTWEBVIEWTEST__

#include "DAVAEngine.h"
#include "UI/UIWebView.h"

using namespace DAVA;

#include "TestTemplate.h"

class TransparentWebViewTest: public TestTemplate<TransparentWebViewTest>
{
public:
	TransparentWebViewTest();

	virtual void LoadResources();
	virtual void UnloadResources();
	virtual bool RunTest(int32 testNum);

	virtual void DidAppear();
	virtual void Update(float32 timeElapsed);

	void TestFunction(PerfFuncData * data);

private:
	UIWebView* webView1;
	UIWebView* webView2;

	bool testFinished;
	float onScreenTime;
};

#endif /* defined(__TEMPLATEPROJECTIPHONE__TRANSPARENTWEBVIEWTEST__) */
