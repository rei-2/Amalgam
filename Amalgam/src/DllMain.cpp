#include <Windows.h>
#include "Core/Core.h"
#include "Utils/CrashLog/CrashLog.h"

DWORD WINAPI MainThread(LPVOID lpParam)
{
	try
	{
		U::Core.Load();
		U::Core.Loop();
		U::Core.Unload();

		CrashLog::Unload();
		FreeLibraryAndExitThread(static_cast<HMODULE>(lpParam), EXIT_SUCCESS);
	}
	catch (...)
	{
		// Silent failure - just exit thread
		FreeLibraryAndExitThread(static_cast<HMODULE>(lpParam), EXIT_FAILURE);
	}
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	try
	{
		if (fdwReason == DLL_PROCESS_ATTACH)
		{
			CrashLog::Initialize();

			if (const auto hMainThread = CreateThread(nullptr, 0, MainThread, hinstDLL, 0, nullptr))
				CloseHandle(hMainThread);
		}

		return TRUE;
	}
	catch (...)
	{
		// Silent failure - don't crash the target process
		return FALSE;
	}
}