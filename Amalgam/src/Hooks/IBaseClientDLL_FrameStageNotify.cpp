#include "../SDK/SDK.h"

#include "../Features/Backtrack/Backtrack.h"
#include "../Features/CheaterDetection/CheaterDetection.h"
#include "../Features/CritHack/CritHack.h"
#include "../Features/Players/PlayerUtils.h"
#include "../Features/Resolver/Resolver.h"
#include "../Features/Simulation/MovementSimulation/MovementSimulation.h"
#include "../Features/Visuals/Visuals.h"
#include "../Features/Visuals/ESP/ESP.h"
#include "../Features/Visuals/Chams/Chams.h"
#include "../Features/Visuals/Glow/Glow.h"
#include "../Features/Spectate/Spectate.h"

MAKE_HOOK(IBaseClientDLL_FrameStageNotify, U::Memory.GetVFunc(I::BaseClientDLL, 35), void,
	void* rcx, ClientFrameStage_t curStage)
{
	switch (curStage)
	{
	case FRAME_RENDER_START:
	{
		if (auto pLocal = H::Entities.GetLocal())
			F::Resolver.FrameStageNotify(pLocal);
		// CRASH: Exception thrown at 0x00007FFCD004E223 (client.dll) in tf_win64.exe: 0xC0000005: Access violation reading location 0x0000025800000000.
		// CRASH: Exception thrown at 0x00007FFC5A09EED0 (client.dll) in tf_win64.exe: 0xC0000005: Access violation reading location 0x000001F636472562.
		// crashes likely not fsn related
	}
	}

	CALL_ORIGINAL(rcx, curStage);

	switch (curStage)
	{
	case FRAME_NET_UPDATE_START:
	{
		H::Entities.Clear();
		break;
	}
	case FRAME_NET_UPDATE_END:
	{
		H::Entities.Store();
		F::PlayerUtils.UpdatePlayers();

		for (int n = 1; n <= I::EngineClient->GetMaxClients(); n++)
		{
			if (n == I::EngineClient->GetLocalPlayer())
				continue;

			auto pEntity = I::ClientEntityList->GetClientEntity(n)->As<CTFPlayer>();
			if (pEntity && pEntity->IsPlayer() && !pEntity->IsDormant())
			{
				G::VelocityMap[n].push_front({ pEntity->m_vecOrigin() + Vec3(0, 0, pEntity->m_vecMaxs().z - pEntity->m_vecMins().z), pEntity->m_flSimulationTime() });
				if (G::VelocityMap[n].size() > Vars::Aimbot::Projectile::VelocityAverageCount.Value)
					G::VelocityMap[n].pop_back();
			}
			else
				G::VelocityMap[n].clear();
		}

		if (Vars::Visuals::Removals::Interpolation.Value)
		{
			for (auto& pEntity : H::Entities.GetGroup(EGroupType::PLAYERS_ALL))
			{
				auto pPlayer = pEntity->As<CTFPlayer>();
				if (pPlayer->entindex() == I::EngineClient->GetLocalPlayer() && !I::EngineClient->IsPlayingDemo() || pPlayer->IsDormant() || !pPlayer->IsAlive())
					continue; // local player managed in CPrediction_RunCommand

				if (auto iDeltaTicks = TIME_TO_TICKS(H::Entities.GetDeltaTime(pPlayer->entindex())))
				{
					float flOldFrameTime = I::GlobalVars->frametime;
					I::GlobalVars->frametime = I::Prediction->m_bEnginePaused ? 0.f : TICK_INTERVAL;
					for (int i = 0; i < iDeltaTicks; i++)
					{
						G::UpdatingAnims = true;
						pPlayer->UpdateClientSideAnimation();
						G::UpdatingAnims = false;
					}
					I::GlobalVars->frametime = flOldFrameTime;
				}
			}
		}

		F::Backtrack.FrameStageNotify();
		F::MoveSim.Store();
		F::CritHack.Store();
		F::Visuals.Store();

		auto pLocal = H::Entities.GetLocal();
		F::ESP.Store(pLocal);
		F::Chams.Store(pLocal);
		F::Glow.Store(pLocal);

		F::CheaterDetection.Run();
		F::Spectate.NetUpdateEnd(pLocal);

		break;
	}
	case FRAME_RENDER_START:
	{
		if (!G::Unload)
			F::Visuals.Modulate();
	}
	}
}