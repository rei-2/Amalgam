#include <Windows.h>
#include "Core/Core.h"
#include "Utils/Minidump/Minidump.h"

DWORD WINAPI MainThread(LPVOID lpParam)
{
	U::Core.Load();
	U::Core.Loop();
	U::Core.Unload();

#ifndef _DEBUG
	SetUnhandledExceptionFilter(nullptr);
#endif

	FreeLibraryAndExitThread(static_cast<HMODULE>(lpParam), EXIT_SUCCESS);
}


BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
#ifndef _DEBUG
		SetUnhandledExceptionFilter(Minidump::ExceptionFilter);
#endif

		if (const auto hMainThread = CreateThread(nullptr, 0, MainThread, hinstDLL, 0, nullptr))
			CloseHandle(hMainThread);
	}

	return TRUE;
}