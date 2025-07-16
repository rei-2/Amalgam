#include <Windows.h>
#include "Core/Core.h"
#include "Utils/CrashLog/CrashLog.h"

// Force manual mapping only - no LoadLibrary fallback

DWORD WINAPI MainThread(LPVOID lpParam)
{
	try
	{
		U::Core.Load();
		U::Core.Loop();
		U::Core.Unload();

		CrashLog::Unload();
		
		// Manual mapping only - use ExitThread instead of FreeLibraryAndExitThread
		ExitThread(EXIT_SUCCESS);
	}
	catch (...)
	{
		// Silent failure - just exit thread (manual mapping only)
		ExitThread(EXIT_FAILURE);
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