#include "Hooks.h"

#include "../Assert/Assert.h"
#include "../../Hooks/Direct3DDevice9.h"

CHook::CHook(std::string sName, void* pInitFunc)
{
	this->m_pInitFunc = pInitFunc;
	U::Hooks.m_mHooks[sName] = this;
}

bool CHooks::Initialize()
{
	MH_Initialize();

	WndProc::Initialize();
	for (auto& [_, pHook] : m_mHooks)
		reinterpret_cast<void(__cdecl*)()>(pHook->m_pInitFunc)();

	m_bFailed = MH_EnableHook(MH_ALL_HOOKS) != MH_OK;
	AssertCustom(!m_bFailed, "MinHook failed to enable all hooks!");
	return !m_bFailed;
}

void CHooks::Unload()
{
	AssertCustom(MH_Uninitialize() == MH_OK, "MinHook failed to unload all hooks!");
	WndProc::Unload();
}