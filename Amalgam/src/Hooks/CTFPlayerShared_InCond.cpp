#include "../SDK/SDK.h"

MAKE_SIGNATURE(CTFPlayerShared_InCond, "client.dll", "48 89 5C 24 ? 57 48 83 EC ? 8B DA 48 8B F9 83 FA ? 7D", 0x0);
MAKE_SIGNATURE(CTFPlayer_ShouldDraw_InCond_Call, "client.dll", "84 C0 74 ? 32 C0 48 8B 74 24", 0x0);
MAKE_SIGNATURE(CTFWearable_ShouldDraw_InCond_Call, "client.dll", "84 C0 0F 85 ? ? ? ? 41 BF", 0x0);
MAKE_SIGNATURE(CHudScope_ShouldDraw_InCond_Call, "client.dll", "84 C0 74 ? 48 8B CB E8 ? ? ? ? 48 85 C0 74 ? 48 8B CB E8 ? ? ? ? 48 8B C8 48 8B 10 FF 92 ? ? ? ? 83 F8 ? 0F 94 C0", 0x0);
MAKE_SIGNATURE(CTFPlayer_CreateMove_InCondTaunt_Call, "client.dll", "84 C0 75 ? BA ? ? ? ? 48 8D 8E ? ? ? ? E8 ? ? ? ? 84 C0 75 ? 45 32 FF", 0x0);
MAKE_SIGNATURE(CTFPlayer_CreateMove_InCondKart_Call, "client.dll", "84 C0 74 ? 4C 8B C3", 0x0);
MAKE_SIGNATURE(CTFInput_ApplyMouse_InCond_Call, "client.dll", "84 C0 74 ? F3 0F 10 9B", 0x0);

MAKE_HOOK(CTFPlayerShared_InCond, S::CTFPlayerShared_InCond(), bool,
	void* rcx, ETFCond nCond)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::CTFPlayerShared_InCond[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx, nCond);
#endif

	static const auto dwZoomPlayer = S::CTFPlayer_ShouldDraw_InCond_Call();
	static const auto dwZoomWearable = S::CTFWearable_ShouldDraw_InCond_Call();
	static const auto dwZoomHudScope = S::CHudScope_ShouldDraw_InCond_Call();
	static const auto dwTaunt = S::CTFPlayer_CreateMove_InCondTaunt_Call();
	static const auto dwKart1 = S::CTFPlayer_CreateMove_InCondKart_Call();
	static const auto dwKart2 = S::CTFInput_ApplyMouse_InCond_Call();
	const auto dwRetAddr = uintptr_t(_ReturnAddress());

	auto GetOuter = [&rcx]() -> CBaseEntity*
		{
			static const auto iShared = U::NetVars.GetNetVar("CTFPlayer", "m_Shared");
			static const auto iBombHeadStage = U::NetVars.GetNetVar("CTFPlayer", "m_nHalloweenBombHeadStage");
			static const auto iOffset = iBombHeadStage - iShared + 0x4;
			return *reinterpret_cast<CBaseEntity**>(uintptr_t(rcx) + iOffset);
		};

	switch (nCond)
	{
	case TF_COND_ZOOMED:
		if (dwRetAddr == dwZoomPlayer || dwRetAddr == dwZoomWearable || Vars::Visuals::Removals::Scope.Value && dwRetAddr == dwZoomHudScope)
			return false;
		break;
	case TF_COND_DISGUISED:
		if (Vars::Visuals::Removals::Disguises.Value && H::Entities.GetLocal() != GetOuter())
			return false;
		break;
	case TF_COND_TAUNTING:
		if (Vars::Misc::Automation::TauntControl.Value && dwRetAddr == dwTaunt)
			return false;
		if (Vars::Visuals::Removals::Taunts.Value && H::Entities.GetLocal() != GetOuter())
			return false;
		break;
	case TF_COND_HALLOWEEN_KART:
		if (Vars::Misc::Automation::KartControl.Value && (dwRetAddr == dwKart1 || dwRetAddr == dwKart2))
			return false;
		break;
	case TF_COND_FREEZE_INPUT:
		if (!CALL_ORIGINAL(rcx, TF_COND_HALLOWEEN_KART) || Vars::Misc::Automation::KartControl.Value)
			return false;
	}

	return CALL_ORIGINAL(rcx, nCond);
}