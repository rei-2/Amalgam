#include "ExceptionHandler.h"

#include "../../Features/Configs/Configs.h"

#include <ImageHlp.h>
#include <Psapi.h>
#include <deque>
#include <sstream>
#include <fstream>
#include <format>
#pragma comment(lib, "imagehlp.lib")

#define STATUS_RUNTIME_ERROR             ((DWORD   )0xE06D7363L)
#define DBG_THREAD_NAMING                ((DWORD   )0x406D1388L)

struct Frame_t
{
	std::string m_sModule = "";
	uintptr_t m_uBase = 0;
	uintptr_t m_uAddress = 0;
	std::string m_sFile = "";
	unsigned int m_uLine = 0;
	std::string m_sName = "";
};

static PVOID s_pHandle;
static LPVOID s_lpParam;
static int s_iExceptions = 0;

// our own image range, used to attribute frames when the image is not in the loader's module list (e.g. manual mapping)
static uintptr_t s_uImageBase = 0;
static uintptr_t s_uImageEnd = 0;
static PRUNTIME_FUNCTION s_pFunctionTable = nullptr;
static DWORD s_nFunctionTableEntries = 0;

static void RegisterImageInformation(uintptr_t uBase)
{
	const auto pDosHeader = reinterpret_cast<const IMAGE_DOS_HEADER*>(uBase);
	if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
		return;

	const auto pNtHeaders = reinterpret_cast<const IMAGE_NT_HEADERS*>(uBase + pDosHeader->e_lfanew);
	if (pNtHeaders->Signature != IMAGE_NT_SIGNATURE)
		return;

	s_uImageBase = uBase;
	s_uImageEnd = uBase + pNtHeaders->OptionalHeader.SizeOfImage;

	// the OS has no unwind data for images that are not in the loader's module list,
	// causing stack traces to die at our first frame, so register ours manually
	HMODULE hModule = nullptr;
	if (!GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, LPCSTR(uBase), &hModule))
	{
		const auto& tDirectory = pNtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION];
		if (tDirectory.VirtualAddress && tDirectory.Size >= sizeof(RUNTIME_FUNCTION))
		{
			s_pFunctionTable = reinterpret_cast<PRUNTIME_FUNCTION>(uBase + tDirectory.VirtualAddress);
			s_nFunctionTableEntries = tDirectory.Size / sizeof(RUNTIME_FUNCTION);
			RtlAddFunctionTable(s_pFunctionTable, s_nFunctionTableEntries, uBase);
		}
	}
}

static inline bool IsAddressInImage(uintptr_t uAddress)
{
	return s_uImageBase && uAddress >= s_uImageBase && uAddress < s_uImageEnd;
}

static inline std::deque<Frame_t> StackTrace(PCONTEXT pContext)
{
	std::deque<Frame_t> vTrace = {};

	HANDLE hProcess = GetCurrentProcess();
	HANDLE hThread = GetCurrentThread();

	if (!SymInitialize(hProcess, nullptr, TRUE))
		return vTrace;

	SymSetOptions(SYMOPT_LOAD_LINES);

	STACKFRAME64 tStackFrame = {};
	tStackFrame.AddrPC.Offset = pContext->Rip;
	tStackFrame.AddrFrame.Offset = pContext->Rbp;
	tStackFrame.AddrStack.Offset = pContext->Rsp;
	tStackFrame.AddrPC.Mode = AddrModeFlat;
	tStackFrame.AddrFrame.Mode = AddrModeFlat;
	tStackFrame.AddrStack.Mode = AddrModeFlat;

	CONTEXT tContext = *pContext;

	while (StackWalk64(IMAGE_FILE_MACHINE_AMD64, hProcess, hThread, &tStackFrame, &tContext, nullptr, SymFunctionTableAccess64, SymGetModuleBase64, nullptr))
	{
		vTrace.push_back({ .m_uAddress = tStackFrame.AddrPC.Offset });
		Frame_t& tFrame = vTrace.back();

		if (auto hBase = HINSTANCE(SymGetModuleBase64(hProcess, tStackFrame.AddrPC.Offset)))
		{
			tFrame.m_uBase = uintptr_t(hBase);

			char buffer[MAX_PATH];
			if (GetModuleBaseName(hProcess, hBase, buffer, sizeof(buffer) / sizeof(char)))
				tFrame.m_sModule = buffer;
			else
				tFrame.m_sModule = std::format("{:#x}", tFrame.m_uBase);
		}
		else if (IsAddressInImage(tStackFrame.AddrPC.Offset))
		{	// manually mapped images are not in the module list, attribute their frames ourselves
			tFrame.m_uBase = s_uImageBase;
			tFrame.m_sModule = "Amalgam";
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
			DWORD64 dwOffset = 0;
			char buf[sizeof(IMAGEHLP_SYMBOL64) + 255];
			auto symbol = PIMAGEHLP_SYMBOL64(buf);
			symbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64) + 255;
			symbol->MaxNameLength = 254;
			if (SymGetSymFromAddr64(hProcess, tStackFrame.AddrPC.Offset, &dwOffset, symbol))
				tFrame.m_sName = symbol->Name;
		}
	}

	SymCleanup(hProcess);

	return vTrace;
}

static LONG APIENTRY ExceptionFilter(PEXCEPTION_POINTERS ExceptionInfo)
{
	const char* sError = "UNKNOWN";
	switch (ExceptionInfo->ExceptionRecord->ExceptionCode)
	{
	case STATUS_ACCESS_VIOLATION: sError = "ACCESS VIOLATION"; break;
	case STATUS_STACK_OVERFLOW: sError = "STACK OVERFLOW"; break;
	case STATUS_HEAP_CORRUPTION: sError = "HEAP CORRUPTION"; break;
	case STATUS_RUNTIME_ERROR:
	case EXCEPTION_BREAKPOINT:
	case DBG_PRINTEXCEPTION_C:
	case DBG_PRINTEXCEPTION_WIDE_C:
	case DBG_THREAD_NAMING: return EXCEPTION_CONTINUE_SEARCH;
	}

	if (!Vars::Debug::CrashLogging.Value)
		return EXCEPTION_CONTINUE_SEARCH;

	std::stringstream ssErrorStream;
	ssErrorStream << std::format("Error: {} (0x{:X}) ({})\n", sError, ExceptionInfo->ExceptionRecord->ExceptionCode, ++s_iExceptions);
	ssErrorStream << "Built @ " __DATE__ ", " __TIME__ ", " __CONFIGURATION__ "\n";
	ssErrorStream << std::format("Time @ {}, {}\n", SDK::GetDate(), SDK::GetTime());
	ssErrorStream << std::format("Thread: {}\n", GetCurrentThreadId());

	if (ExceptionInfo->ExceptionRecord->ExceptionCode == STATUS_ACCESS_VIOLATION
		&& ExceptionInfo->ExceptionRecord->NumberParameters >= 2)
	{
		const auto uOperation = ExceptionInfo->ExceptionRecord->ExceptionInformation[0];
		const auto uTarget = ExceptionInfo->ExceptionRecord->ExceptionInformation[1];
		const char* sOperation = "ACCESS";
		if (uOperation == 0)
			sOperation = "READ of";
		else if (uOperation == 1)
			sOperation = "WRITE to";
		else if (uOperation == 8)
			sOperation = "EXECUTE of (DEP)";
		ssErrorStream << std::format("Fault: {} {:#x} ({})\n", sOperation, uTarget, U::Memory.GetModuleOffset(uTarget));
	}

	ssErrorStream << "\n";
	if (U::Memory.GetOffsetFromBase(s_lpParam) == uintptr_t(-1))
		ssErrorStream << std::format("This: {} (unregistered image, injected manually?)\n", U::Memory.GetModuleOffset(s_lpParam));
	ssErrorStream << std::format("RIP: {:#x} ({})\n", ExceptionInfo->ContextRecord->Rip, U::Memory.GetModuleOffset(ExceptionInfo->ContextRecord->Rip));
	ssErrorStream << std::format("RAX: {:#x}\n", ExceptionInfo->ContextRecord->Rax);
	ssErrorStream << std::format("RCX: {:#x}\n", ExceptionInfo->ContextRecord->Rcx);
	ssErrorStream << std::format("RDX: {:#x}\n", ExceptionInfo->ContextRecord->Rdx);
	ssErrorStream << std::format("RBX: {:#x}\n", ExceptionInfo->ContextRecord->Rbx);
	ssErrorStream << std::format("RSP: {:#x}\n", ExceptionInfo->ContextRecord->Rsp);
	ssErrorStream << std::format("RBP: {:#x}\n", ExceptionInfo->ContextRecord->Rbp);
	ssErrorStream << std::format("RSI: {:#x}\n", ExceptionInfo->ContextRecord->Rsi);
	ssErrorStream << std::format("RDI: {:#x}\n", ExceptionInfo->ContextRecord->Rdi);

	ssErrorStream << "\n";
	if (auto vTrace = StackTrace(ExceptionInfo->ContextRecord);
		!vTrace.empty())
	{
		for (int i = 0; i < vTrace.size(); i++)
		{
			Frame_t& tFrame = vTrace[i];

			ssErrorStream << std::format("{}: ", i + 1);
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
	}
	else
	{
		ssErrorStream << U::Memory.GetModuleOffset(ExceptionInfo->ExceptionRecord->ExceptionAddress);
		ssErrorStream << "\n";
	}

	try
	{
		std::ofstream file;
		file.open(F::Configs.m_sConfigPath + "crash_log.txt", std::ios_base::app);
		file << ssErrorStream.str() + "\n\n\n";
		file.close();

		ssErrorStream << "\n";
		ssErrorStream << "Ctrl + C to copy. \n";
		ssErrorStream << "Logged to Amalgam\\crash_log.txt. ";
	}
	catch (...) {}

	switch (ExceptionInfo->ExceptionRecord->ExceptionCode)
	{
	case STATUS_ACCESS_VIOLATION:
	case STATUS_STACK_OVERFLOW:
	case STATUS_HEAP_CORRUPTION:
		SDK::Output("Unhandled exception", ssErrorStream.str().c_str(), {}, OUTPUT_DEBUG, nullptr, MB_OK | MB_ICONERROR);
	}

	return EXCEPTION_CONTINUE_SEARCH;
}

void CExceptionHandler::Initialize(LPVOID lpParam)
{
	s_pHandle = AddVectoredExceptionHandler(1, ExceptionFilter);
	s_lpParam = lpParam;
	RegisterImageInformation(uintptr_t(lpParam));
}
void CExceptionHandler::Unload()
{
	RemoveVectoredExceptionHandler(s_pHandle);

	if (s_pFunctionTable)
	{
		RtlRemoveFunctionTable(s_pFunctionTable, s_nFunctionTableEntries, s_uImageBase);
		s_pFunctionTable = nullptr;
		s_nFunctionTableEntries = 0;
	}
}