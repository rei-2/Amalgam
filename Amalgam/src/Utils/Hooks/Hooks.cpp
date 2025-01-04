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

	bool bFail{false};
	for (auto& [_, pHook] : m_mHooks)
	{
		if (!reinterpret_cast<bool(__cdecl*)()>(pHook->m_pInitFunc)())
			bFail = true;
	}

	const bool bEnableAll{MH_EnableHook(MH_ALL_HOOKS) == MH_OK};
	AssertCustom(bEnableAll, "MH failed to enable all hooks!");

	bFail = (bFail || !bEnableAll);
	return !bFail;
}

void CHooks::Unload()
{
	AssertCustom(MH_Uninitialize() == MH_OK, "MH failed to unload all hooks!");
	WndProc::Unload();
}