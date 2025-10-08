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
	for (auto& Interface : m_vInterfaces)
	{
		const char* sModule = nullptr;
		std::vector<std::string> vModules;
		boost::split(vModules, Interface->m_sDLL, boost::is_any_of(", "));
		if (vModules.size() == 1)
			sModule = vModules.front().c_str();
		else
		{
			for (auto& sName : vModules)
			{
				if (GetModuleHandle(sName.c_str()))
				{
					sModule = sName.c_str();
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

		switch (Interface->m_nType)
		{
		case 0:
		{
			*Interface->m_pPtr = U::Memory.FindInterface(sModule, Interface->m_sName);
			break;
		}
		case 1:
		{
			*Interface->m_pPtr = U::Memory.GetModuleExport<void*>(sModule, Interface->m_sName);
			break;
		}
		case 2:
		{
			auto dwDest = U::Memory.FindSignature(sModule, Interface->m_sName);
			if (!dwDest)
			{
				U::Core.AppendFailText(std::format("CInterfaces::Initialize() failed to find signature").c_str());
				break;
			}

			auto dwAddress = U::Memory.RelToAbs(dwDest);
			*Interface->m_pPtr = reinterpret_cast<void*>(dwAddress + Interface->m_nOffset);
			break;
		}
		}

		for (int n = 0; n < Interface->m_nDereferenceCount; n++)
		{
			if (Interface->m_pPtr)
				*Interface->m_pPtr = *reinterpret_cast<void**>(*Interface->m_pPtr);
		}

		if (!*Interface->m_pPtr)
		{
			U::Core.AppendFailText(std::format("CInterfaces::Initialize() failed to initialize:\n  {}\n  {}", sModule, Interface->m_sName).c_str());
			m_bFailed = true;
		}
	}

	if (!H::Interfaces.Initialize())
		m_bFailed = true; // Initialize any null interfaces

	return !m_bFailed;
}