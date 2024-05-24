#include "../SDK/SDK.h"

MAKE_SIGNATURE(CBaseEntity_ResetLatched, "client.dll", "40 56 48 83 EC ? 48 8B 01 48 8B F1 FF 90 ? ? ? ? 84 C0 75", 0x0);

MAKE_HOOK(CBaseEntity_ResetLatched, S::CBaseEntity_ResetLatched(), void, __fastcall,
	void* ecx)
{
	if (Vars::Misc::Game::PredictionErrorJitterFix.Value)
		return;

	CALL_ORIGINAL(ecx);
}