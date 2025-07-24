#include "Hooks.h"

#include "../../Core/Core.h"
#include "../../Hooks/Direct3DDevice9.h"

CHook::CHook(const std::string& sName, void* pInitFunc)
{
	m_pInitFunc = pInitFunc;
	U::Hooks.m_mHooks[sName] = this;
}

bool CHooks::Initialize()
{
	MH_Initialize();

	WndProc::Initialize();
	for (auto& [_, pHook] : m_mHooks)
		reinterpret_cast<void(__cdecl*)()>(pHook->m_pInitFunc)();

	m_bFailed = MH_EnableHook(MH_ALL_HOOKS) != MH_OK;
	if (m_bFailed)
		U::Core.AppendFailText("MinHook failed to enable all hooks!");
	return !m_bFailed;
}

bool CHooks::Unload()
{
	m_bFailed = MH_Uninitialize() != MH_OK;
	if (m_bFailed)
		U::Core.AppendFailText("MinHook failed to unload all hooks!");
	WndProc::Unload();
	return !m_bFailed;
}