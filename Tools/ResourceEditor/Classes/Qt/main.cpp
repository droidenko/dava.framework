#include <QtGui/QApplication>
#include "Qt/Main/mainwindow.h"
#include "DAVAEngine.h"

#if defined (__DAVAENGINE_MACOS__)
#include "Platform/Qt/MacOS/QtLayerMacOS.h"
#elif defined (__DAVAENGINE_WIN32__)
#include "Platform/Qt/Win32/QtLayerWin32.h"
#include "Platform/Qt/Win32/CorePlatformWin32.h"
#endif //#if defined (__DAVAENGINE_MACOS__)

int main(int argc, char *argv[])
{
	int ret = 0;
    QApplication a(argc, argv);

#if defined (__DAVAENGINE_MACOS__)
    DAVA::Core::Run(argc, argv);
	new DAVA::QtLayerMacOS();
#elif defined (__DAVAENGINE_WIN32__)
	HINSTANCE hInstance = (HINSTANCE)::GetModuleHandle(NULL);
	DAVA::Core::Run(argc, argv, hInstance);
	new DAVA::QtLayerWin32();
#else
	DVASSERT(false && "Wrong platform")
#endif


    new QtMainWindow();
    QtMainWindow::Instance()->show();

	ret = a.exec();

	QtMainWindow::Instance()->Release();
	DAVA::QtLayer::Instance()->Release();

	DAVA::Core::Instance()->ReleaseSingletons();
	DAVA::Core::Instance()->Release();

	return ret;
}
