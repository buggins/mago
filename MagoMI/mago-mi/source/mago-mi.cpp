// mago-mi.cpp: entry point


#include <windows.h>
#include "cmdline.h"
#include "debugger.h"

int wmain(int argc, wchar_t* argv[])
{
#ifdef _DEBUG
	CRLog::setFileLogger("mago-mi-debug.log", true);
	CRLog::setLogLevel(CRLog::LL_TRACE);
#endif
	CRLog::trace("mago-mi entered wmain()");
	parseCommandLine(argc, argv);
	executableInfo.dumpParams();

	//testEngine();

	Debugger debugger;
	int res = debugger.enterCommandLoop();

	executableInfo.clear();
	CRLog::trace("mago-mi leaving wmain(), exit code: %d", res);
	return res;
}

