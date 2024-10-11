#include "../SDK/SDK.h"

// don't really see any benefit to keeping this

MAKE_SIGNATURE(CSequenceTransitioner_CheckForSequenceChange, "client.dll", "48 85 D2 0F 84 ? ? ? ? 48 89 6C 24 ? 48 89 74 24 ? 48 89 7C 24", 0x0);

MAKE_HOOK(CSequenceTransitioner_CheckForSequenceChange, S::CSequenceTransitioner_CheckForSequenceChange(), void, __fastcall,
	void* rcx, CStudioHdr* hdr, int nCurSequence, bool bForceNewSequence, bool bInterpolate)
{
	CALL_ORIGINAL(rcx, hdr, nCurSequence, bForceNewSequence, Vars::Visuals::Removals::Interpolation.Value ? false : bInterpolate);
}