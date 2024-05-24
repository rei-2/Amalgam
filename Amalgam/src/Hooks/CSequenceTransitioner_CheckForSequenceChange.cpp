#include "../SDK/SDK.h"

MAKE_SIGNATURE(CSequenceTransitioner_CheckForSequenceChange, "client.dll", "48 85 D2 0F 84 ? ? ? ? 48 89 6C 24 ? 48 89 74 24 ? 48 89 7C 24", 0x0);

MAKE_HOOK(CSequenceTransitioner_CheckForSequenceChange, S::CSequenceTransitioner_CheckForSequenceChange(), void, __fastcall,
	void* ecx, CStudioHdr* hdr, int nCurSequence, bool bForceNewSequence, bool bInterpolate)
{
	return CALL_ORIGINAL(ecx, hdr, nCurSequence, bForceNewSequence, Vars::Visuals::Removals::Interpolation.Value ? false : bInterpolate);
}