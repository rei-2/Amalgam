#include "CrashLog.h"

#include "../../Features/Configs/Configs.h"

#include <ImageHlp.h>
#include <deque>
#include <sstream>
#include <fstream>
#include <format>
#pragma comment(lib, "imagehlp.lib")

struct Frame
{
	std::string m_sModule = "";
	uintptr_t m_pAddress = 0;
	std::string m_sFile = "";
	unsigned int m_uLine = 0;
	std::string m_sName = "";
};

static std::deque<Frame> StackTrace(PCONTEXT context)
{
	HANDLE hProcess = GetCurrentProcess();
	HANDLE hThread = GetCurrentThread();

		OutputDebugStringA("StackTrace\n");
	if (!SymInitialize(hProcess, nullptr, TRUE))
		return {};
	
		OutputDebugStringA("Frame\n");
	SymSetOptions(SYMOPT_LOAD_LINES);

	STACKFRAME64 frame = {};
	frame.AddrPC.Offset = context->Rip;
	frame.AddrFrame.Offset = context->Rbp;
	frame.AddrStack.Offset = context->Rsp;
	frame.AddrPC.Mode = AddrModeFlat;
	frame.AddrFrame.Mode = AddrModeFlat;
	frame.AddrStack.Mode = AddrModeFlat;

	std::deque<Frame> vTrace = {};
	while (StackWalk64(IMAGE_FILE_MACHINE_AMD64, hProcess, hThread, &frame, context, nullptr, SymFunctionTableAccess64, SymGetModuleBase64, nullptr))
	{
			OutputDebugStringA("StackWalk\n");
		Frame tFrame = {};

		tFrame.m_pAddress = frame.AddrPC.Offset;

		{
			auto base = HINSTANCE(SymGetModuleBase64(hProcess, frame.AddrPC.Offset));
			char buf[MAX_PATH];
			if (base && GetModuleFileNameA(base, buf, MAX_PATH))
			{
				tFrame.m_sModule = std::format("{}, {:#x}", buf, uintptr_t(base));
				auto find = tFrame.m_sModule.rfind("\\");
				if (find != std::string::npos)
					tFrame.m_sModule.replace(0, find + 1, "");
			}
		}

		{
			DWORD offset = 0;
			IMAGEHLP_LINE64 line = {};
			line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
			if (SymGetLineFromAddr64(hProcess, frame.AddrPC.Offset, &offset, &line))
			{
				tFrame.m_sFile = line.FileName;
				tFrame.m_uLine = line.LineNumber;
				auto find = tFrame.m_sFile.rfind("\\");
				if (find != std::string::npos)
					tFrame.m_sFile.replace(0, find + 1, "");
			}
		}

		{
			uintptr_t offset = 0;
			char buf[sizeof(IMAGEHLP_SYMBOL64) + 255];
			auto symbol = PIMAGEHLP_SYMBOL64(buf);
			symbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64) + 255;
			symbol->MaxNameLength = 254;
			if (SymGetSymFromAddr64(hProcess, frame.AddrPC.Offset, &offset, symbol))
				tFrame.m_sName = symbol->Name;
		}

		vTrace.push_back(tFrame);
	}
	//if (!vTrace.empty())
	//	vTrace.pop_front();

	SymCleanup(hProcess);

	return vTrace;
}

LONG APIENTRY CrashLog::ExceptionFilter(PEXCEPTION_POINTERS Info)
{
	if (Info->ExceptionRecord->ExceptionCode != EXCEPTION_ACCESS_VIOLATION)
		return EXCEPTION_EXECUTE_HANDLER;

	std::stringstream error;
	error << std::format("Error: {:#X}\n", Info->ExceptionRecord->ExceptionCode);
	error << std::format("Address: {:#X}\n\n", uintptr_t(Info->ExceptionRecord->ExceptionAddress));
	error << std::format("RIP: {:#x}\n", Info->ContextRecord->Rip);
	error << std::format("EFLAGS: {:#x}\n", Info->ContextRecord->EFlags);
	error << std::format("RAX: {:#x}\n", Info->ContextRecord->Rax);
	error << std::format("RCX: {:#x}\n", Info->ContextRecord->Rcx);
	error << std::format("RDX: {:#x}\n", Info->ContextRecord->Rdx);
	error << std::format("RBX: {:#x}\n", Info->ContextRecord->Rbx);
	error << std::format("RSP: {:#x}\n", Info->ContextRecord->Rsp);
	error << std::format("RBP: {:#x}\n", Info->ContextRecord->Rbp);
	error << std::format("RSI: {:#x}\n", Info->ContextRecord->Rsi);
	error << std::format("RDI: {:#x}\n\n", Info->ContextRecord->Rdi);

	auto vTrace = StackTrace(Info->ContextRecord);
	if (!vTrace.empty())
	{
		for (auto& tFrame : vTrace)
		{
			error << std::format("{:#x}", tFrame.m_pAddress);
			if (!tFrame.m_sModule.empty())
				error << std::format(" ({})", tFrame.m_sModule);
			if (!tFrame.m_sFile.empty())
				error << std::format(" ({} L{})", tFrame.m_sFile, tFrame.m_uLine);
			if (!tFrame.m_sName.empty())
				error << std::format(" ({})", tFrame.m_sName);
			error << "\n";
		}
		error << "\n";
	}

	error << "Ctrl + C to copy. Logged to Amalgam\\crash_log.txt. ";

	MessageBox(nullptr, error.str().c_str(), "Unhandled exception", MB_OK | MB_ICONERROR);

	error << "\n\n\n\n";
	std::ofstream file;
	file.open(F::Configs.sConfigPath + "\\crash_log.txt", std::ios_base::app);
	file << error.str();
	file.close();

	return EXCEPTION_EXECUTE_HANDLER;
}