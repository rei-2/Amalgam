#include "Minidump.h"

#include <dbghelp.h>
#include <shlobj.h>
#include <tchar.h>
#include <ctime>

[[noreturn]] void ExitWithMessage(const TCHAR* msg)
{
	MessageBox(
		nullptr,
		msg,
		_T("Unhandled exception"),
		MB_OK | MB_ICONERROR
	);

	ExitProcess(0);
}

LONG __stdcall Minidump::ExceptionFilter(PEXCEPTION_POINTERS exPtr)
{
	const HMODULE hLib = LoadLibrary(_T("dbghelp"));
	if (!hLib)
	{
		ExitWithMessage(_T("Could not load dbghelp!"));
	}

	const auto pMiniDumpWriteDump = reinterpret_cast<decltype(&MiniDumpWriteDump)>(GetProcAddress(hLib, "MiniDumpWriteDump"));

	// Get the MiniDumpWriteDump function
	if (!pMiniDumpWriteDump)
	{
		ExitWithMessage(_T("Could not load MiniDumpWriteDump!"));
	}

	// Get the dump folde (Desktop)
	TCHAR buf[512];
	const auto gfpResult = SHGetFolderPath(nullptr, CSIDL_DESKTOP, nullptr, SHGFP_TYPE_CURRENT, buf);
	if (FAILED(gfpResult))
	{
		ExitWithMessage(_T("Failed to get folder path!"));
	}

	// Create a file handle
	TCHAR fileName[MAX_PATH];
	const int curTime = static_cast<int>(time(nullptr) % 100000);
	wsprintf(fileName, _T("%s\\Crash_Amalgam_%x.dmp"), buf, curTime);
	const HANDLE hFile = CreateFile(fileName, GENERIC_READ | GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		wsprintf(buf, _T("Could not create minidump file!\n\n- Error: %ul\n- File: %s"), GetLastError(), fileName);
		ExitWithMessage(buf);
	}

	// Write the minidump
	MINIDUMP_EXCEPTION_INFORMATION mdei = {
		GetCurrentThreadId(),
		exPtr,
		FALSE
	};

	if (pMiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &mdei, nullptr, nullptr))
	{
		wsprintf(buf, _T("Minidump created.\n\n- Build: " __TIMESTAMP__ "\n- File: %s"), fileName);
	}
	else
	{
		wsprintf(buf, _T("Could not create minidump!\n- Error: %ul\n- File: %s"), GetLastError(), fileName);
	}

	// Cleanup
	FreeLibrary(hLib);
	CloseHandle(hFile);

	ExitWithMessage(buf);
}