#include "Interfaces.h"
#include "../Memory/Memory.h"
#include "../../Core/Core.h"
#include "../../SDK/Definitions/Interfaces.h"
#include <TlHelp32.h>
#include <string>
#include <format>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#pragma warning (disable: 4172)

InterfaceInit_t::InterfaceInit_t(void** pPtr, const char* sDLL, const char* sName, int8_t nType, int8_t nOffset, int8_t nDereferenceCount)
{
	m_pPtr = pPtr;
	m_sDLL = sDLL;
	m_sName = sName;
	m_nType = nType;
	m_nOffset = nOffset;
	m_nDereferenceCount = nDereferenceCount;

	U::Interfaces.AddInterface(this);
}

bool CInterfaces::Initialize()
{
	// File-based logging for interface debugging
	FILE* log_file = fopen("C:\\temp\\amalgam_debug.log", "a");
	if (log_file) {
		fprintf(log_file, "Interfaces::Initialize: Starting interface loop, %zu interfaces to process\n", m_vInterfaces.size());
		fclose(log_file);
	}

	int interfaceIndex = 0;
	for (auto& Interface : m_vInterfaces)
	{
		FILE* log_file1 = fopen("C:\\temp\\amalgam_debug.log", "a");
		if (log_file1) {
			fprintf(log_file1, "Interfaces::Initialize: Processing interface %d - DLL: %s, Name: %s, Type: %d\n",
				interfaceIndex, Interface->m_sDLL ? Interface->m_sDLL : "NULL",
				Interface->m_sName ? Interface->m_sName : "NULL", Interface->m_nType);
			fclose(log_file1);
		}

		const char* sModule = nullptr;
		std::vector<std::string> vModules;

		FILE* log_file2 = fopen("C:\\temp\\amalgam_debug.log", "a");
		if (log_file2) {
			fprintf(log_file2, "Interfaces::Initialize: About to call boost::split for interface %d\n", interfaceIndex);
			fclose(log_file2);
		}

		boost::split(vModules, Interface->m_sDLL, boost::is_any_of(", "));

		FILE* log_file3 = fopen("C:\\temp\\amalgam_debug.log", "a");
		if (log_file3) {
			fprintf(log_file3, "Interfaces::Initialize: boost::split complete, %zu modules found\n", vModules.size());
			fclose(log_file3);
		}

		if (vModules.size() == 1)
			sModule = vModules.front().c_str();
		else
		{
			FILE* log_file4 = fopen("C:\\temp\\amalgam_debug.log", "a");
			if (log_file4) {
				fprintf(log_file4, "Interfaces::Initialize: Searching for module in multiple options\n");
				fclose(log_file4);
			}

			for (auto& sName : vModules)
			{
				FILE* log_file5 = fopen("C:\\temp\\amalgam_debug.log", "a");
				if (log_file5) {
					fprintf(log_file5, "Interfaces::Initialize: Checking module: %s\n", sName.c_str());
					fclose(log_file5);
				}

				if (GetModuleHandle(sName.c_str()))
				{
					sModule = sName.c_str();
					FILE* log_file6 = fopen("C:\\temp\\amalgam_debug.log", "a");
					if (log_file6) {
						fprintf(log_file6, "Interfaces::Initialize: Found module: %s\n", sName.c_str());
						fclose(log_file6);
					}
					break;
				}
			}
			if (!sModule)
			{
				std::stringstream ssModuleStream;
				for (auto& sName : vModules)
					ssModuleStream << std::format("{}{}", !ssModuleStream.str().empty() ? ", " : "", sName);

				U::Core.AppendFailText(std::format("CInterfaces::Initialize() failed to find module:\n  {}\n  {}", ssModuleStream.str(), Interface->m_sName).c_str());
				m_bFailed = true;
				continue;
			}
		}

		FILE* log_file7 = fopen("C:\\temp\\amalgam_debug.log", "a");
		if (log_file7) {
			fprintf(log_file7, "Interfaces::Initialize: Module resolved to: %s, about to process type %d\n", sModule, Interface->m_nType);
			fclose(log_file7);
		}

		switch (Interface->m_nType)
		{
		case 0:
		{
			FILE* log_file8 = fopen("C:\\temp\\amalgam_debug.log", "a");
			if (log_file8) {
				fprintf(log_file8, "Interfaces::Initialize: Type 0 - About to call FindInterface for %s::%s\n", sModule, Interface->m_sName);
				fclose(log_file8);
			}

			*Interface->m_pPtr = U::Memory.FindInterface(sModule, Interface->m_sName);

			FILE* log_file9 = fopen("C:\\temp\\amalgam_debug.log", "a");
			if (log_file9) {
				fprintf(log_file9, "Interfaces::Initialize: Type 0 - FindInterface returned %p\n", *Interface->m_pPtr);
				fclose(log_file9);
			}
			break;
		}
		case 1:
		{
			FILE* log_file10 = fopen("C:\\temp\\amalgam_debug.log", "a");
			if (log_file10) {
				fprintf(log_file10, "Interfaces::Initialize: Type 1 - About to call GetModuleExport for %s::%s\n", sModule, Interface->m_sName);
				fclose(log_file10);
			}

			*Interface->m_pPtr = U::Memory.GetModuleExport<void*>(sModule, Interface->m_sName);

			FILE* log_file11 = fopen("C:\\temp\\amalgam_debug.log", "a");
			if (log_file11) {
				fprintf(log_file11, "Interfaces::Initialize: Type 1 - GetModuleExport returned %p\n", *Interface->m_pPtr);
				fclose(log_file11);
			}
			break;
		}
		case 2:
		{
			FILE* log_file12 = fopen("C:\\temp\\amalgam_debug.log", "a");
			if (log_file12) {
				fprintf(log_file12, "Interfaces::Initialize: Type 2 - About to call FindSignature for %s::%s\n", sModule, Interface->m_sName);
				fclose(log_file12);
			}

			auto dwDest = U::Memory.FindSignature(sModule, Interface->m_sName);
			if (!dwDest)
			{
				FILE* log_file13 = fopen("C:\\temp\\amalgam_debug.log", "a");
				if (log_file13) {
					fprintf(log_file13, "Interfaces::Initialize: Type 2 - FindSignature FAILED for %s::%s\n", sModule, Interface->m_sName);
					fclose(log_file13);
				}
				U::Core.AppendFailText(std::format("CInterfaces::Initialize() failed to find signature").c_str());
				break;
			}

			FILE* log_file14 = fopen("C:\\temp\\amalgam_debug.log", "a");
			if (log_file14) {
				fprintf(log_file14, "Interfaces::Initialize: Type 2 - FindSignature returned 0x%llx, calling RelToAbs\n", dwDest);
				fclose(log_file14);
			}

			auto dwAddress = U::Memory.RelToAbs(dwDest);

			FILE* log_file15 = fopen("C:\\temp\\amalgam_debug.log", "a");
			if (log_file15) {
				fprintf(log_file15, "Interfaces::Initialize: Type 2 - RelToAbs returned 0x%llx, applying offset %d\n", dwAddress, Interface->m_nOffset);
				fclose(log_file15);
			}

			*Interface->m_pPtr = reinterpret_cast<void*>(dwAddress + Interface->m_nOffset);

			FILE* log_file16 = fopen("C:\\temp\\amalgam_debug.log", "a");
			if (log_file16) {
				fprintf(log_file16, "Interfaces::Initialize: Type 2 - Final pointer set to %p\n", *Interface->m_pPtr);
				fclose(log_file16);
			}
			break;
		}
		}

		FILE* log_file17 = fopen("C:\\temp\\amalgam_debug.log", "a");
		if (log_file17) {
			fprintf(log_file17, "Interfaces::Initialize: About to dereference %d times for interface %d\n", Interface->m_nDereferenceCount, interfaceIndex);
			fclose(log_file17);
		}

		for (int n = 0; n < Interface->m_nDereferenceCount; n++)
		{
			FILE* log_file18 = fopen("C:\\temp\\amalgam_debug.log", "a");
			if (log_file18) {
				fprintf(log_file18, "Interfaces::Initialize: Dereference %d/%d - Current ptr: %p\n", n+1, Interface->m_nDereferenceCount, *Interface->m_pPtr);
				fclose(log_file18);
			}

			if (*Interface->m_pPtr)  // Check if the POINTER VALUE is valid, not just the pointer-to-pointer
				*Interface->m_pPtr = *reinterpret_cast<void**>(*Interface->m_pPtr);

			FILE* log_file19 = fopen("C:\\temp\\amalgam_debug.log", "a");
			if (log_file19) {
				fprintf(log_file19, "Interfaces::Initialize: After dereference %d - New ptr: %p\n", n+1, *Interface->m_pPtr);
				fclose(log_file19);
			}
		}

		FILE* log_file20 = fopen("C:\\temp\\amalgam_debug.log", "a");
		if (log_file20) {
			fprintf(log_file20, "Interfaces::Initialize: Final check - Interface %d ptr: %p\n", interfaceIndex, *Interface->m_pPtr);
			fclose(log_file20);
		}

		if (!*Interface->m_pPtr)
		{
			U::Core.AppendFailText(std::format("CInterfaces::Initialize() failed to initialize:\n  {}\n  {}", sModule, Interface->m_sName).c_str());
			m_bFailed = true;
		}

		FILE* log_file21 = fopen("C:\\temp\\amalgam_debug.log", "a");
		if (log_file21) {
			fprintf(log_file21, "Interfaces::Initialize: Interface %d completed successfully\n", interfaceIndex);
			fclose(log_file21);
		}

		interfaceIndex++;
	}

	FILE* log_file22 = fopen("C:\\temp\\amalgam_debug.log", "a");
	if (log_file22) {
		fprintf(log_file22, "Interfaces::Initialize: All interfaces processed, about to call H::Interfaces.Initialize()\n");
		fclose(log_file22);
	}

	if (!H::Interfaces.Initialize())
	{
		FILE* log_file23 = fopen("C:\\temp\\amalgam_debug.log", "a");
		if (log_file23) {
			fprintf(log_file23, "Interfaces::Initialize: H::Interfaces.Initialize() FAILED\n");
			fclose(log_file23);
		}
		m_bFailed = true; // Initialize any null interfaces
	}
	else
	{
		FILE* log_file24 = fopen("C:\\temp\\amalgam_debug.log", "a");
		if (log_file24) {
			fprintf(log_file24, "Interfaces::Initialize: H::Interfaces.Initialize() SUCCESS\n");
			fclose(log_file24);
		}
	}

	FILE* log_file25 = fopen("C:\\temp\\amalgam_debug.log", "a");
	if (log_file25) {
		fprintf(log_file25, "Interfaces::Initialize: Returning %s\n", !m_bFailed ? "SUCCESS" : "FAILURE");
		fclose(log_file25);
	}

	return !m_bFailed;
}