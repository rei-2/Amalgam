#include "../SDK/SDK.h"

MAKE_SIGNATURE(CBaseAnimating_SetSequence, "client.dll", "40 53 48 83 EC ? 48 8B D9 39 91 ? ? ? ? 74 ? 89 91", 0x0);

MAKE_HOOK(CBaseAnimating_SetSequence, S::CBaseAnimating_SetSequence(), void,
	void* rcx, int nSequence)
{
	auto pEntity = reinterpret_cast<CBaseAnimating*>(rcx);
	if (pEntity->m_nSequence() != nSequence && !pEntity->m_bSequenceLoops())
		pEntity->m_flCycle() = 0.f; // set on the server but not client

	CALL_ORIGINAL(rcx, nSequence);
}