#include "TransparentWebViewTest.h"

static const float TRANSPARENT_WEB_VIEW_TEST_AUTO_CLOSE_TIME = 30.0f;

TransparentWebViewTest::TransparentWebViewTest()
:	TestTemplate<TransparentWebViewTest>("TransparentWebViewTest")
,	webView1(NULL)
,	webView2(NULL)
{
	RegisterFunction(this, &TransparentWebViewTest::TestFunction, Format("TransparentWebViewTest"), NULL);
}

void TransparentWebViewTest::LoadResources()
{
	webView1 = new UIWebView(Rect(5, 5, 500, 500));
	webView1->SetVisible(true);
	webView1->OpenURL("http://www.davaconsulting.com/");
	AddControl(webView1);

	FilePath srcDir("~res:/TestData/TransparentWebViewTest/");
	FilePath cpyDir = FileSystem::Instance()->GetCurrentDocumentsDirectory() + "TransparentWebViewTest/";
	FileSystem::Instance()->DeleteDirectory(cpyDir);
	FileSystem::Instance()->CreateDirectory(cpyDir);
	FilePath srcFile = srcDir + "test.html";
	FilePath cpyFile = cpyDir + "test.html";
	FileSystem::Instance()->CopyFile(srcFile, cpyFile);
	String url = "file:///" + cpyFile.GetAbsolutePathname();

	webView2 = new UIWebView(Rect(30, 150, 200, 100));
	webView2->SetVisible(true);
	webView2->SetBackgroundTransparency(true);
	webView2->OpenURL(url);
	AddControl(webView2);
}

void TransparentWebViewTest::UnloadResources()
{
	RemoveAllControls();

	SafeRelease(webView1);
	SafeRelease(webView2);
}

bool TransparentWebViewTest::RunTest(int32 testNum)
{
	TestTemplate<TransparentWebViewTest>::RunTest(testNum);
	return testFinished;
}

void TransparentWebViewTest::DidAppear()
{
	onScreenTime = 0.f;
}

void TransparentWebViewTest::Update(float32 timeElapsed)
{
	onScreenTime += timeElapsed;
	if(onScreenTime > TRANSPARENT_WEB_VIEW_TEST_AUTO_CLOSE_TIME)
	{
		testFinished = true;
	}

	TestTemplate<TransparentWebViewTest>::Update(timeElapsed);
}

void TransparentWebViewTest::TestFunction(PerfFuncData * data)
{
	return;
}
