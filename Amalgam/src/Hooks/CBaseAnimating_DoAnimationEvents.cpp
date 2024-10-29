#include "../SDK/SDK.h"

MAKE_SIGNATURE(CBaseAnimating_DoAnimationEvents, "client.dll", "48 85 D2 0F 84 ? ? ? ? 53 41 56 48 83 EC ? 83 B9", 0x0);

MAKE_HOOK(CBaseAnimating_DoAnimationEvents, S::CBaseAnimating_DoAnimationEvents(), void, __fastcall,
	void* rcx, CStudioHdr* pStudioHdr)
{
	if (rcx == H::Entities.GetLocal())
		return;

	CALL_ORIGINAL(rcx, pStudioHdr);
}