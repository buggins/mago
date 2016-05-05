// mago-mi.cpp: определяет точку входа для консольного приложения.
//

#include "readline.h"
#include <stdio.h>
#include <stdlib.h>
#include "cmdline.h"

#include "Common.h"
#include "DebuggerProxy.h"
//#include "Module.h"

void InitDebug()
{
	int f = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
	f |= _CRTDBG_LEAK_CHECK_DF;     // should always use in debug build
	f |= _CRTDBG_CHECK_ALWAYS_DF;   // check on free AND alloc
	_CrtSetDbgFlag(f);

	//_CrtSetAllocHook( LocalMemAllocHook );
	//SetLocalMemWorkingSetLimit( 550 );
}

class _EventCallback : public IEventCallback
{
	Exec*       mExec;
	IModule*    mMod;
	bool        mHitBp;

public:
	_EventCallback()
		: mExec(NULL),
		mMod(NULL),
		mHitBp(false)
	{
	}

	~_EventCallback()
	{
		if (mMod != NULL)
			mMod->Release();
	}

	void SetExec(Exec* exec)
	{
		mExec = exec;
	}

	bool GetModule(IModule*& mod)
	{
		mod = mMod;
		mod->AddRef();
		return mod != NULL;
	}

	bool GetHitBp()
	{
		return mHitBp;
	}

	virtual void AddRef()
	{
	}

	virtual void Release()
	{
	}

	virtual void OnProcessStart(IProcess* process)
	{
	}

	virtual void OnProcessExit(IProcess* process, DWORD exitCode)
	{
	}

	virtual void OnThreadStart(IProcess* process, Thread* thread)
	{
	}

	virtual void OnThreadExit(IProcess* process, DWORD threadId, DWORD exitCode)
	{
	}

	virtual void OnModuleLoad(IProcess* process, IModule* module)
	{
		char*   macName = "";

		switch (module->GetMachine())
		{
		case IMAGE_FILE_MACHINE_I386: macName = "x86"; break;
		case IMAGE_FILE_MACHINE_IA64: macName = "ia64"; break;
		case IMAGE_FILE_MACHINE_AMD64: macName = "x64"; break;
		}

		if (sizeof(Address) == sizeof(uintptr_t))
			printf("  %p %d %s '%ls'\n", module->GetImageBase(), module->GetSize(), macName, module->GetPath());
		else
			printf("  %08I64x %d %s '%ls'\n", module->GetImageBase(), module->GetSize(), macName, module->GetPath());

		if (mMod == NULL)
		{
			mMod = module;
			mMod->AddRef();
		}
	}

	virtual void OnModuleUnload(IProcess* process, Address baseAddr)
	{
		if (sizeof(Address) == sizeof(uintptr_t))
			printf("  %p\n", baseAddr);
		else
			printf("  %08I64x\n", baseAddr);
	}

	virtual void OnOutputString(IProcess* process, const wchar_t* outputString)
	{
		printf("  '%ls'\n", outputString);
	}

	virtual void OnLoadComplete(IProcess* process, DWORD threadId)
	{
		UINT_PTR    baseAddr = (UINT_PTR)mMod->GetImageBase();

		// 0x003C137A, 0x003C1395
		// 1137A, 11395

#if 0
		mExec->SetBreakpoint(process, baseAddr + 0x0001137A, (void*)33);
		mExec->SetBreakpoint(process, baseAddr + 0x00011395, (void*)17);

		//mExec->SetBreakpoint( process, 0x003C137A, (void*) 257 );

		//mExec->RemoveBreakpoint( process, 0x003C137A, (void*) 33 );
		//mExec->RemoveBreakpoint( process, 0x003C137A, (void*) 257 );

		//mExec->RemoveBreakpoint( process, 0x003C1395, (void*) 33 );

		//mExec->RemoveBreakpoint( process, 0x003C1395, (void*) 17 );
#endif
	}

	virtual RunMode OnException(IProcess* process, DWORD threadId, bool firstChance, const EXCEPTION_RECORD* exceptRec)
	{
		if (sizeof(Address) == sizeof(uintptr_t))
			printf("  %p %08x\n", exceptRec->ExceptionAddress, exceptRec->ExceptionCode);
		else
			printf("  %08I64x %08x\n", exceptRec->ExceptionAddress, exceptRec->ExceptionCode);
		return RunMode_Break;
	}

	virtual RunMode OnBreakpoint(IProcess* process, uint32_t threadId, Address address, bool embedded)
	{
		if (sizeof(Address) == sizeof(uintptr_t))
			printf("  breakpoint at %p\n", address);
		else
			printf("  breakpoint at %08I64x\n", address);

		mHitBp = true;

		UINT_PTR    baseAddr = (UINT_PTR)mMod->GetImageBase();

		//mExec->RemoveBreakpoint( process, baseAddr + 0x0001137A, (void*) 257 );
		//mExec->SetBreakpoint( process, baseAddr + 0x0001137A, (void*) 257 );
		//mExec->RemoveBreakpoint( process, baseAddr + 0x00011395, (void*) 129 );
		//mExec->SetBreakpoint( process, baseAddr + 0x00011395, (void*) 129 );

		return RunMode_Break;
	}

	virtual void OnStepComplete(IProcess* process, uint32_t threadId)
	{
		printf("  Step complete\n");
	}

	virtual void OnAsyncBreakComplete(IProcess* process, uint32_t threadId)
	{
	}

	virtual void OnError(IProcess* process, HRESULT hrErr, EventCode event)
	{
		printf("  *** ERROR: %08x while %d\n", hrErr, event);
	}

	virtual ProbeRunMode OnCallProbe(
		IProcess* process, uint32_t threadId, Address address, AddressRange& thunkRange)
	{
		return ProbeRunMode_Run;
	}
};

int main(int argc, char *argv[])
{
	parseCommandLine(argc, argv);
	executableInfo.dumpParams();
	if (executableInfo.exename && !fileExists(executableInfo.exename)) {
		fprintf(stderr, "%s: no such file or directory", executableInfo.exename);
		//exit(4);
	}

	InitDebug();
	_EventCallback   callback;
	Exec        exec;
	HRESULT     hr = S_OK;
	LaunchInfo  info = { 0 };
	MagoCore::DebuggerProxy debuggerProxy;

	//Mago::Module * module = new Mago::Module();
	//delete module;
	callback.SetExec(&exec);
	hr = exec.Init(&callback, &debuggerProxy);
	if (FAILED(hr)) {
		fprintf(stderr, "Cannot start debugging");
		//goto Error;
	}

	char *line;

	printf("\nType exit to quit the test\n\n");
	while ((line = readline("(gdb) "))
		&& (strncmp(line, "exit", 4))) {
		printf("string=%s\n", line);
		add_history(line);
		free(line);
	}

	return 0;
}

