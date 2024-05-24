#include "../SDK/SDK.h"

MAKE_SIGNATURE(CTFPlayerShared_InCond, "client.dll", "48 89 5C 24 ? 57 48 83 EC ? 8B DA 48 8B F9 83 FA ? 7D", 0x0);
MAKE_SIGNATURE(PlayerShouldDraw, "client.dll", "E8 ? ? ? ? 84 C0 74 ? 32 C0 48 8B 74 24", 0x5);
MAKE_SIGNATURE(WearableShouldDraw, "client.dll", "E8 ? ? ? ? 84 C0 0F 85 ? ? ? ? 41 BF", 0x5);
MAKE_SIGNATURE(HudScopeShouldDraw, "client.dll", "84 C0 74 ? 48 8B CB E8 ? ? ? ? 48 85 C0 74 ? 48 8B CB E8 ? ? ? ? 48 8B C8 48 8B 10 FF 92 ? ? ? ? 83 F8 ? 0F 94 C0", 0x0);

MAKE_HOOK(CTFPlayerShared_InCond, S::CTFPlayerShared_InCond(), bool, __fastcall,
	void* ecx, ETFCond nCond)
{
	static const auto dwPlayerShouldDraw = S::PlayerShouldDraw();
	static const auto dwWearableShouldDraw = S::WearableShouldDraw();
	static const auto dwHudScopeShouldDraw = S::HudScopeShouldDraw();
	const auto dwRetAddr = std::uintptr_t(_ReturnAddress());

	auto GetOuter = [&ecx]() -> CBaseEntity*
		{
			static const auto dwShared = U::NetVars.GetNetVar("CTFPlayer", "m_Shared");
			static const auto dwBombHeadStage = U::NetVars.GetNetVar("CTFPlayer", "m_nHalloweenBombHeadStage");
			static const auto dwOff = (dwBombHeadStage - dwShared) + 0x4;
			return *reinterpret_cast<CBaseEntity**>(std::uintptr_t(ecx) + dwOff);
		};

	switch (nCond)
	{
	case TF_COND_ZOOMED:
		if (dwRetAddr == dwPlayerShouldDraw || dwRetAddr == dwWearableShouldDraw || Vars::Visuals::Removals::Scope.Value && dwRetAddr == dwHudScopeShouldDraw)
			return false;
		break;
	case TF_COND_TAUNTING:
		if (Vars::Visuals::Removals::Taunts.Value)
		{
			auto pLocal = H::Entities.GetLocal();
			auto pEntity = GetOuter();

			// Compare team's, removing team's taunt is useless
			if (pLocal && pEntity && pEntity->m_iTeamNum() != pLocal->m_iTeamNum())
				return false;
		}
		break;
	case TF_COND_HALLOWEEN_KART:
		if (Vars::Misc::Automation::KartControl.Value && H::Entities.GetLocal() == GetOuter() && !G::AnimateKart)
			return false;
		break;
	case TF_COND_DISGUISED:
		if (Vars::Visuals::Removals::Disguises.Value && H::Entities.GetLocal() != GetOuter())
			return false;
	}

	return CALL_ORIGINAL(ecx, nCond);
}