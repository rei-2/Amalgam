#include "Events.h"

#include "../../Core/Core.h"
#include "../../Features/Aimbot/AutoHeal/AutoHeal.h"
#include "../../Features/Backtrack/Backtrack.h"
#include "../../Features/CheaterDetection/CheaterDetection.h"
#include "../../Features/CritHack/CritHack.h"
#include "../../Features/Misc/Misc.h"
#include "../../Features/PacketManip/AntiAim/AntiAim.h"
#include "../../Features/Output/Output.h"
#include "../../Features/Resolver/Resolver.h"
#include "../../Features/Visuals/Visuals.h"
#include "../../Features/Visuals/CritHeals/CritHeals.h"
#include "../../Features/Visuals/FocusFire/FocusFire.h"
#include "../../Features/Visuals/AmmoTracker/AmmoTracker.h"

bool CEventListener::Initialize()
{
	std::vector<const char*> vEvents = { 
		"client_beginconnect", "client_connected", "client_disconnect", "game_newmap", "teamplay_round_start", "scorestats_accumulated_update", "mvm_reset_stats", "player_connect_client", "player_spawn", "player_changeclass", "player_hurt", "vote_cast", "item_pickup", "revive_player_notify"
	};

	for (auto szEvent : vEvents)
	{
		I::GameEventManager->AddListener(this, szEvent, false);

		if (!I::GameEventManager->FindListener(this, szEvent))
		{
			U::Core.AppendFailText(std::format("Failed to add listener: {}", szEvent).c_str());
			m_bFailed = true;
		}
	}

	return !m_bFailed;
}

void CEventListener::Unload()
{
	I::GameEventManager->RemoveListener(this);
}

void CEventListener::FireGameEvent(IGameEvent* pEvent)
{
	if (!pEvent)
		return;

	auto pLocal = H::Entities.GetLocal();
	auto uHash = FNV1A::Hash32(pEvent->GetName());

	F::Output.Event(pEvent, uHash, pLocal);
	if (I::EngineClient->IsPlayingDemo())
		return;

	F::CritHack.Event(pEvent, uHash, pLocal);
	F::AutoHeal.Event(pEvent, uHash);
	F::Misc.Event(pEvent, uHash);
	F::Visuals.Event(pEvent, uHash);
	F::AmmoTracker.Event(pEvent, uHash);
	switch (uHash)
	{
	case FNV1A::Hash32Const("player_hurt"):
		F::Resolver.PlayerHurt(pEvent);
		F::CheaterDetection.ReportDamage(pEvent);
		F::CritHeals.OnPlayerHurt(I::EngineClient->GetPlayerForUserID(pEvent->GetInt("userid")));
		F::FocusFire.OnPlayerHurt(I::EngineClient->GetPlayerForUserID(pEvent->GetInt("userid")), I::EngineClient->GetPlayerForUserID(pEvent->GetInt("attacker")));
		break;
	case FNV1A::Hash32Const("player_spawn"):
		F::Backtrack.SetLerp(pEvent);
		break;
	case FNV1A::Hash32Const("revive_player_notify"):
	{
		if (!Vars::Misc::MannVsMachine::InstantRevive.Value || pEvent->GetInt("entindex") != I::EngineClient->GetLocalPlayer())
			break;

		KeyValues* kv = new KeyValues("MVM_Revive_Response");
		kv->SetInt("accepted", 1);
		I::EngineClient->ServerCmdKeyValues(kv);
	}
	}

	return;
}