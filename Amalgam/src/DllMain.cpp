#include <Windows.h>
#include "Core/Core.h"
#include "Utils/CrashLog/CrashLog.h"
#include <format>

DWORD WINAPI MainThread(LPVOID lpParam)
{
	U::CrashLog.Initialize();

	U::Core.Load();
	U::Core.Loop();
	U::Core.Unload();

	U::CrashLog.Unload();

	FreeLibraryAndExitThread(static_cast<HMODULE>(lpParam), EXIT_SUCCESS);
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		if (const auto hThread = CreateThread(nullptr, 0, MainThread, hinstDLL, 0, nullptr))
			CloseHandle(hThread);
	}

	return TRUE;
}