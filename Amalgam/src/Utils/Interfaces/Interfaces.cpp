#include "Interfaces.h"
#include "../Memory/Memory.h"
#include "../../Core/Core.h"
#include "../Assert/Assert.h"
#include "../../SDK/Definitions/Interfaces.h"
#include <TlHelp32.h>
#include <string>
#include <format>

#pragma warning (disable: 4172)

static const char* SearchForDLL(const char* pszDLLSearch)
{
	HANDLE hProcessSnap = INVALID_HANDLE_VALUE;
	PROCESSENTRY32 pe32;
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)
		return pszDLLSearch;

	pe32.dwSize = sizeof(PROCESSENTRY32);
	if (!Process32First(hProcessSnap, &pe32))
	{
		CloseHandle(hProcessSnap);
		return pszDLLSearch;
	}

	do
	{
		if (pe32.szExeFile == strstr(pe32.szExeFile, "tf_win64.exe"))
		{
			HANDLE hModuleSnap = INVALID_HANDLE_VALUE;
			MODULEENTRY32 me32;
			hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pe32.th32ProcessID);
			if (hModuleSnap == INVALID_HANDLE_VALUE)
				break;

			me32.dwSize = sizeof(MODULEENTRY32);
			if (!Module32First(hModuleSnap, &me32))
			{
				CloseHandle(hModuleSnap);
				break;
			}

			do
			{
				if (strstr(me32.szModule, pszDLLSearch))
				{
					CloseHandle(hProcessSnap);
					CloseHandle(hModuleSnap);
					return me32.szModule;
				}
			} while (Module32Next(hModuleSnap, &me32));

			CloseHandle(hModuleSnap);
			break;
		}
	} while (Process32Next(hProcessSnap, &pe32));

	CloseHandle(hProcessSnap);
	return pszDLLSearch;
}

InterfaceInit_t::InterfaceInit_t(void** pPtr, const char* sDLLName, const char* sVersion, int nOffset, int nDereferenceCount, bool bSearchDLL)
{
	m_pPtr = pPtr;
	m_pszDLLName = sDLLName;
	m_pszVersion = sVersion;
	m_nOffset = nOffset;
	m_nDereferenceCount = nDereferenceCount;
	m_bSearchDLL = bSearchDLL;

	U::Interfaces.AddInterface(this);
}

bool CInterfaces::Initialize()
{
	for (auto& Interface : m_vecInterfaces)
	{
		if (!Interface->m_pPtr || !Interface->m_pszDLLName || !Interface->m_pszVersion)
			continue;

		if (Interface->m_bSearchDLL)
			Interface->m_pszDLLName = SearchForDLL(Interface->m_pszDLLName);

		if (Interface->m_nOffset == -1)
			*Interface->m_pPtr = U::Memory.FindInterface(Interface->m_pszDLLName, Interface->m_pszVersion);
		else
		{
			auto dwDest = U::Memory.FindSignature(Interface->m_pszDLLName, Interface->m_pszVersion);
			if (!dwDest)
			{
				U::Core.AppendFailText(std::format("CInterfaces::Initialize() failed to find signature:\n  {}\n  {}", Interface->m_pszDLLName, Interface->m_pszVersion).c_str());
				m_bFailed = true;
				continue;
			}

			auto dwAddress = U::Memory.RelToAbs(dwDest);
			*Interface->m_pPtr = reinterpret_cast<void*>(dwAddress + Interface->m_nOffset);

			for (int n = 0; n < Interface->m_nDereferenceCount; n++)
			{
				if (Interface->m_pPtr)
					*Interface->m_pPtr = *reinterpret_cast<void**>(*Interface->m_pPtr);
			}
		}

		if (!*Interface->m_pPtr)
		{
			U::Core.AppendFailText(std::format("CInterfaces::Initialize() failed to initialize:\n  {}\n  {}", Interface->m_pszDLLName, Interface->m_pszVersion).c_str());
			m_bFailed = true;
		}
	}

	if (!H::Interfaces.Initialize())
		m_bFailed = true; // Initialize any null interfaces

	return !m_bFailed;
}