#include <Windows.h>
#include "Core/Core.h"
#include "Utils/CrashLog/CrashLog.h"

DWORD WINAPI MainThread(LPVOID lpParam)
{
	// File-based logging for startup debugging
	FILE* log_file = fopen("C:\\temp\\amalgam_debug.log", "a");
	if (log_file) {
		fprintf(log_file, "MainThread: Entry\n");
		fclose(log_file);
	}

	try
	{
		FILE* log_file1 = fopen("C:\\temp\\amalgam_debug.log", "a");
		if (log_file1) {
			fprintf(log_file1, "MainThread: About to call Core.Load()\n");
			fclose(log_file1);
		}

		U::Core.Load();

		FILE* log_file2 = fopen("C:\\temp\\amalgam_debug.log", "a");
		if (log_file2) {
			fprintf(log_file2, "MainThread: Core.Load() complete, starting Loop()\n");
			fclose(log_file2);
		}

		U::Core.Loop();

		FILE* log_file3 = fopen("C:\\temp\\amalgam_debug.log", "a");
		if (log_file3) {
			fprintf(log_file3, "MainThread: Core.Loop() complete, calling Unload()\n");
			fclose(log_file3);
		}

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
			// File-based logging for DLL attach debugging
			FILE* log_file = fopen("C:\\temp\\amalgam_debug.log", "a");
			if (log_file) {
				fprintf(log_file, "DllMain: DLL_PROCESS_ATTACH called\n");
				fclose(log_file);
			}

			CrashLog::Initialize();

			FILE* log_file1 = fopen("C:\\temp\\amalgam_debug.log", "a");
			if (log_file1) {
				fprintf(log_file1, "DllMain: CrashLog initialized, creating MainThread\n");
				fclose(log_file1);
			}

			if (const auto hMainThread = CreateThread(nullptr, 0, MainThread, hinstDLL, 0, nullptr))
			{
				FILE* log_file2 = fopen("C:\\temp\\amalgam_debug.log", "a");
				if (log_file2) {
					fprintf(log_file2, "DllMain: MainThread created successfully\n");
					fclose(log_file2);
				}
				CloseHandle(hMainThread);
			}
			else
			{
				FILE* log_file3 = fopen("C:\\temp\\amalgam_debug.log", "a");
				if (log_file3) {
					fprintf(log_file3, "DllMain: Failed to create MainThread\n");
					fclose(log_file3);
				}
			}
		}

		return TRUE;
	}
	catch (...)
	{
		// Silent failure - don't crash the target process
		return FALSE;
	}
}