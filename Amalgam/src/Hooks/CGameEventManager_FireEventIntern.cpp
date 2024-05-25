#include "../SDK/SDK.h"

#include "../Features/Backtrack/Backtrack.h"
#include "../Features/CheaterDetection/CheaterDetection.h"
#include "../Features/CritHack/CritHack.h"
#include "../Features/Records/Records.h"
#include "../Features/Misc/Misc.h"
#include "../Features/PacketManip/AntiAim/AntiAim.h"
#include "../Features/Resolver/Resolver.h"
#include "../Features/Visuals/Visuals.h"

MAKE_SIGNATURE(CGameEventManager_FireEventIntern, "engine.dll", "44 88 44 24 ? 48 89 4C 24 ? 55 57", 0x0);

MAKE_HOOK(CGameEventManager_FireEventIntern, S::CGameEventManager_FireEventIntern(), bool, __fastcall,
	void* ecx, IGameEvent* pEvent, bool bServerOnly, bool bClientOnly)
{
	SDK::Output("Event", std::format("{}: {}, {}", pEvent ? pEvent->GetName() : "unknown", bServerOnly, bClientOnly).c_str(), {255, 255, 255, 255}, Vars::Debug::Logging.Value, false, true);

	if (!pEvent || I::EngineClient->IsPlayingTimeDemo() || !bClientOnly)
		return CALL_ORIGINAL(ecx, pEvent, bServerOnly, bClientOnly);

	auto pLocal = H::Entities.GetLocal();
	auto uHash = FNV1A::Hash(pEvent->GetName());

	F::Records.Event(pEvent, uHash, pLocal);
	F::CritHack.Event(pEvent, uHash, pLocal);
	F::Misc.Event(pEvent, uHash);
	switch (uHash)
	{
	case FNV1A::HashConst("player_hurt"):
		F::Resolver.OnPlayerHurt(pEvent);
		F::CheaterDetection.ReportDamage(pEvent);
		break;
	case FNV1A::HashConst("player_spawn"):
		F::Backtrack.SetLerp(pEvent);
		break;
	case FNV1A::HashConst("item_pickup"): // this is never sent!!!
	{
		if (!Vars::Visuals::UI::PickupTimers.Value)
			break;

		if (auto pEntity = I::ClientEntityList->GetClientEntity(I::EngineClient->GetPlayerForUserID(pEvent->GetInt("userid"))))
		{
			auto itemName = pEvent->GetString("item");
			if (std::strstr(itemName, "medkit"))
				F::Visuals.PickupDatas.push_back({ 1, I::EngineClient->Time(), pEntity->GetAbsOrigin() });
			else if (std::strstr(itemName, "ammopack"))
				F::Visuals.PickupDatas.push_back({ 0, I::EngineClient->Time(), pEntity->GetAbsOrigin() });
		}

		break;
	}
	case FNV1A::HashConst("revive_player_notify"):
	{
		if (!Vars::Misc::MannVsMachine::InstantRevive.Value)
			break;

		if (pEvent->GetInt("entindex") != I::EngineClient->GetLocalPlayer())
			break;

		auto kv = new KeyValues("MVM_Revive_Response");
		kv->SetInt("accepted", 1);
		I::EngineClient->ServerCmdKeyValues(kv);
	}
	}

	return CALL_ORIGINAL(ecx, pEvent, bServerOnly, bClientOnly);
}