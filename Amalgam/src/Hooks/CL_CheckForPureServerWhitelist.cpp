#include "../SDK/SDK.h"

MAKE_SIGNATURE(CL_CheckForPureServerWhitelist, "engine.dll", "40 56 48 83 EC ? 83 3D ? ? ? ? ? 48 8B F1 0F 8E", 0x0);

MAKE_HOOK(CL_CheckForPureServerWhitelist, S::CL_CheckForPureServerWhitelist(), void, __cdecl,
	void **pFilesToReload)
{
	if (Vars::Misc::Exploits::BypassPure.Value)
		return;

	CALL_ORIGINAL(pFilesToReload);
}