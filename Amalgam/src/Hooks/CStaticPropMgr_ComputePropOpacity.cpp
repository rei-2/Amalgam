#include "../SDK/SDK.h"

MAKE_SIGNATURE(CStaticPropMgr_ComputePropOpacity, "engine.dll", "48 89 5C 24 ? 48 89 7C 24 ? 41 54 41 56 41 57 48 83 EC ? 48 8B 05", 0x0);

MAKE_HOOK(CStaticPropMgr_ComputePropOpacity, S::CStaticPropMgr_ComputePropOpacity(), void,
	void* rcx, CStaticProp* pProp)
{
	DEBUG_RETURN(CStaticPropMgr_ComputePropOpacity, rcx, pProp);

	if (Vars::Visuals::World::NoPropFade.Value && pProp)
	{
		pProp->m_Alpha = 255;
		return;
	}

	CALL_ORIGINAL(rcx, pProp);
}