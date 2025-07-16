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
		
		// Check if this is manual mapping or native injection
		HMODULE hModule = static_cast<HMODULE>(lpParam);
		if (hModule && GetModuleHandleA(nullptr) != hModule)
		{
			// Native injection - use FreeLibraryAndExitThread
			FreeLibraryAndExitThread(hModule, EXIT_SUCCESS);
		}
		else
		{
			// Manual mapping - use ExitThread only
			ExitThread(EXIT_SUCCESS);
		}
	}
	catch (...)
	{
		// Silent failure - handle both injection methods
		HMODULE hModule = static_cast<HMODULE>(lpParam);
		if (hModule && GetModuleHandleA(nullptr) != hModule)
		{
			FreeLibraryAndExitThread(hModule, EXIT_FAILURE);
		}
		else
		{
			ExitThread(EXIT_FAILURE);
		}
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