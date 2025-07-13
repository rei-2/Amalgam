#include "CrashLog.h"

#include "../../Features/Configs/Configs.h"

#include <ImageHlp.h>
#include <Psapi.h>
#include <deque>
#include <sstream>
#include <fstream>
#include <format>
#pragma comment(lib, "imagehlp.lib")

struct Frame
{
	std::string m_sModule = "";
	uintptr_t m_uBase = 0;
	uintptr_t m_uAddress = 0;
	std::string m_sFile = "";
	unsigned int m_uLine = 0;
	std::string m_sName = "";
};

static std::deque<Frame> StackTrace(PCONTEXT pContext)
{
	HANDLE hProcess = GetCurrentProcess();
	HANDLE hThread = GetCurrentThread();

	if (!SymInitialize(hProcess, nullptr, TRUE))
		return {};
	
	SymSetOptions(SYMOPT_LOAD_LINES);

	STACKFRAME64 tStackFrame = {};
	tStackFrame.AddrPC.Offset = pContext->Rip;
	tStackFrame.AddrFrame.Offset = pContext->Rbp;
	tStackFrame.AddrStack.Offset = pContext->Rsp;
	tStackFrame.AddrPC.Mode = AddrModeFlat;
	tStackFrame.AddrFrame.Mode = AddrModeFlat;
	tStackFrame.AddrStack.Mode = AddrModeFlat;

	std::deque<Frame> vTrace = {};
	while (StackWalk64(IMAGE_FILE_MACHINE_AMD64, hProcess, hThread, &tStackFrame, pContext, nullptr, SymFunctionTableAccess64, SymGetModuleBase64, nullptr))
	{
		Frame tFrame = {};
		tFrame.m_uAddress = tStackFrame.AddrPC.Offset;

		if (auto hBase = HINSTANCE(SymGetModuleBase64(hProcess, tStackFrame.AddrPC.Offset)))
		{
			tFrame.m_uBase = uintptr_t(hBase);

			char buffer[MAX_PATH];
			if (GetModuleBaseName(hProcess, hBase, buffer, sizeof(buffer) / sizeof(char)))
				tFrame.m_sModule = buffer;
			else
				tFrame.m_sModule = std::format("{:#x}", tFrame.m_uBase);
		}

		{
			DWORD dwOffset = 0;
			IMAGEHLP_LINE64 line = {};
			line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
			if (SymGetLineFromAddr64(hProcess, tStackFrame.AddrPC.Offset, &dwOffset, &line))
			{
				tFrame.m_sFile = line.FileName;
				tFrame.m_uLine = line.LineNumber;
				auto iFind = tFrame.m_sFile.rfind("\\");
				if (iFind != std::string::npos)
					tFrame.m_sFile.replace(0, iFind + 1, "");
			}
		}

		{
			uintptr_t dwOffset = 0;
			char buf[sizeof(IMAGEHLP_SYMBOL64) + 255];
			auto symbol = PIMAGEHLP_SYMBOL64(buf);
			symbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64) + 255;
			symbol->MaxNameLength = 254;
			if (SymGetSymFromAddr64(hProcess, tStackFrame.AddrPC.Offset, &dwOffset, symbol))
				tFrame.m_sName = symbol->Name;
		}

		vTrace.push_back(tFrame);
	}
	//if (!vTrace.empty())
	//	vTrace.pop_front();

	SymCleanup(hProcess);

	return vTrace;
}

static LONG APIENTRY ExceptionFilter(PEXCEPTION_POINTERS ExceptionInfo)
{
	static std::unordered_map<LPVOID, bool> mAddresses = {};
	static bool bException = false;

	if (ExceptionInfo->ExceptionRecord->ExceptionCode != EXCEPTION_ACCESS_VIOLATION
		/*|| !ExceptionInfo->ExceptionRecord->ExceptionAddress*/
		|| mAddresses.contains(ExceptionInfo->ExceptionRecord->ExceptionAddress)
		|| !Vars::Debug::CrashLogging.Value
		|| bException && GetAsyncKeyState(VK_SHIFT) & 0x8000 && GetAsyncKeyState(VK_RETURN) & 0x8000)
		return EXCEPTION_EXECUTE_HANDLER;
	mAddresses[ExceptionInfo->ExceptionRecord->ExceptionAddress] = true;

	std::stringstream ssErrorStream;
	ssErrorStream << std::format("Error: {:#x}\n\n", ExceptionInfo->ExceptionRecord->ExceptionCode);

	ssErrorStream << std::format("RIP: {:#x}\n", ExceptionInfo->ContextRecord->Rip);
	ssErrorStream << std::format("RAX: {:#x}\n", ExceptionInfo->ContextRecord->Rax);
	ssErrorStream << std::format("RCX: {:#x}\n", ExceptionInfo->ContextRecord->Rcx);
	ssErrorStream << std::format("RDX: {:#x}\n", ExceptionInfo->ContextRecord->Rdx);
	ssErrorStream << std::format("RBX: {:#x}\n", ExceptionInfo->ContextRecord->Rbx);
	ssErrorStream << std::format("RSP: {:#x}\n", ExceptionInfo->ContextRecord->Rsp);
	ssErrorStream << std::format("RBP: {:#x}\n", ExceptionInfo->ContextRecord->Rbp);
	ssErrorStream << std::format("RSI: {:#x}\n", ExceptionInfo->ContextRecord->Rsi);
	ssErrorStream << std::format("RDI: {:#x}\n\n", ExceptionInfo->ContextRecord->Rdi);

	auto vTrace = StackTrace(ExceptionInfo->ContextRecord);
	if (!vTrace.empty())
	{
		for (auto& tFrame : vTrace)
		{
			if (tFrame.m_uBase)
				ssErrorStream << std::format("{}+{:#x}", tFrame.m_sModule, tFrame.m_uAddress - tFrame.m_uBase);
			else
				ssErrorStream << std::format("{:#x}", tFrame.m_uAddress);
			if (!tFrame.m_sFile.empty())
				ssErrorStream << std::format(" ({} L{})", tFrame.m_sFile, tFrame.m_uLine);
			if (!tFrame.m_sName.empty())
				ssErrorStream << std::format(" ({})", tFrame.m_sName);
			ssErrorStream << "\n";
		}
		ssErrorStream << "\n";
	}

	ssErrorStream << "Built @ " __DATE__ ", " __TIME__ ", " __CONFIGURATION__ "\n";
	ssErrorStream << "Ctrl + C to copy. \n";
	if (bException)
		ssErrorStream << "Shift + Enter to skip repetitive exceptions. \n";
	bException = true;
	try
	{
		std::ofstream file;
		file.open(F::Configs.m_sConfigPath + "crash_log.txt", std::ios_base::app);
		file << ssErrorStream.str() + "\n\n\n";
		file.close();
		ssErrorStream << "Logged to Amalgam\\crash_log.txt. ";
	}
	catch (...) {}

	SDK::Output("Unhandled exception", ssErrorStream.str().c_str(), {}, false, true, false, false, false, false, MB_OK | MB_ICONERROR);
	return EXCEPTION_EXECUTE_HANDLER;
}

static PVOID pHandle;
void CrashLog::Initialize()
{
	pHandle = AddVectoredExceptionHandler(1, ExceptionFilter);
}
void CrashLog::Unload()
{
	RemoveVectoredExceptionHandler(pHandle);
}