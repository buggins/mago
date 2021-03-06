// mago-mi.cpp: entry point


#include <windows.h>
#include "miutils.h"
#include "cmdline.h"
#include "debugger.h"

#define FORCE_LOG_FILE_IN_RELEASE_MODE


int wmain(int argc, wchar_t* argv[])
{
	std::wstring logFileName = L"mago-mi-debug.log";
	std::wstring exename = argv[0];
	std::wstring dirname = getDirName(exename);
	if (!dirname.empty())
		logFileName = dirname + L"\\" + logFileName;
#ifdef FORCE_LOG_FILE_IN_RELEASE_MODE
	CRLog::setFileLogger(toUtf8(logFileName).c_str(), true);
#ifdef _DEBUG
	CRLog::setLogLevel(CRLog::LL_TRACE);
#else
	CRLog::setLogLevel(CRLog::LL_DEBUG);
#endif
	for (int i = 0; i < argc; i++)
		CRLog::debug("argv[%d]: `%s`", i, toUtf8(argv[i]).c_str());
#endif

	parseCommandLine(argc, argv);

#ifdef _DEBUG
	if (!CRLog::isLoggerSet()) {
		CRLog::setFileLogger(toUtf8(logFileName).c_str(), true);
		CRLog::setLogLevel(CRLog::LL_TRACE);
	}
#else
#ifdef FORCE_LOG_FILE_IN_RELEASE_MODE
	if (!CRLog::isLoggerSet()) {
		CRLog::setFileLogger(toUtf8(logFileName).c_str(), true);
		CRLog::setLogLevel(CRLog::LL_DEBUG);
	}
#endif
#endif

	//testEngine();

	Debugger debugger;
	int res = debugger.enterCommandLoop();

	params.clear();
	CRLog::trace("mago-mi leaving wmain(), exit code: %d", res);
	return res;
}

