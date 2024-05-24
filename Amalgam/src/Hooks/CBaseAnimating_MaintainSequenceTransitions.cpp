#include "../SDK/SDK.h"

MAKE_SIGNATURE(CBaseAnimating_MaintainSequenceTransitions, "client.dll", "4C 89 4C 24 ? 41 56", 0x0);

MAKE_HOOK(C_BaseAnimating_MaintainSequenceTransitions, S::CBaseAnimating_MaintainSequenceTransitions(), void, __fastcall,
	void* ecx, void* boneSetup, float flCycle, Vec3 pos[], Vector4D q[])
{
	return;
}