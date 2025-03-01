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
#include "../Features/Binds/Binds.h"

MAKE_HOOK(IBaseClientDLL_FrameStageNotify, U::Memory.GetVFunc(I::BaseClientDLL, 35), void,
	void* rcx, ClientFrameStage_t curStage)
{
	if (G::Unload)
		return CALL_ORIGINAL(rcx, curStage);

	switch (curStage)
	{
	case FRAME_RENDER_START:
		for (int iKey = 0; iKey < 256; iKey++)
		{
			// don't drop inputs for binds
			auto& tKey = F::Binds.m_mKeyStorage[iKey];

			bool bOldIsDown = tKey.m_bIsDown;
			bool bOldIsPressed = tKey.m_bIsPressed;
			bool bOldIsDouble = tKey.m_bIsDouble;
			bool bOldIsReleased = tKey.m_bIsReleased;

			U::KeyHandler.StoreKey(iKey, &tKey);

			tKey.m_bIsDown = tKey.m_bIsDown || bOldIsDown;
			tKey.m_bIsPressed = tKey.m_bIsPressed || bOldIsPressed;
			tKey.m_bIsDouble = tKey.m_bIsDouble || bOldIsDouble;
			tKey.m_bIsReleased = tKey.m_bIsReleased || bOldIsReleased;
		}
	}

	CALL_ORIGINAL(rcx, curStage);

	switch (curStage)
	{
	case FRAME_NET_UPDATE_START:
	{
		auto pLocal = H::Entities.GetLocal();
		F::Spectate.NetUpdateStart(pLocal);

		H::Entities.Clear();
		break;
	}
	case FRAME_NET_UPDATE_END:
	{
		H::Entities.Store();
		F::PlayerUtils.UpdatePlayers();
		F::Resolver.FrameStageNotify();

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

		for (auto& pEntity : H::Entities.GetGroup(EGroupType::PLAYERS_ALL))
		{
			auto pPlayer = pEntity->As<CTFPlayer>();
			if (pPlayer->entindex() == I::EngineClient->GetLocalPlayer() && !I::EngineClient->IsPlayingDemo() || pPlayer->IsDormant() || !pPlayer->IsAlive())
				continue; // local player managed in CreateMove

			bool bResolver = F::Resolver.GetAngles(pPlayer);
			if (!(Vars::Visuals::Removals::Interpolation.Value || bResolver))
				continue;

			if (int iDeltaTicks = TIME_TO_TICKS(H::Entities.GetDeltaTime(pPlayer->entindex())))
			{
				float flOldFrameTime = I::GlobalVars->frametime;
				I::GlobalVars->frametime = I::Prediction->m_bEnginePaused ? 0.f : TICK_INTERVAL;
				for (int i = 0; i < iDeltaTicks; i++)
				{
					G::UpdatingAnims = true;

					if (bResolver)
					{
						float flYaw, flPitch;
						F::Resolver.GetAngles(pPlayer, &flYaw, &flPitch, nullptr, i + 1 == iDeltaTicks);

						float flOriginalYaw = pPlayer->m_angEyeAnglesY(), flOriginalPitch = pPlayer->m_angEyeAnglesX();
						pPlayer->m_angEyeAnglesY() = flYaw, pPlayer->m_angEyeAnglesX() = flPitch;
						pPlayer->UpdateClientSideAnimation();
						pPlayer->m_angEyeAnglesY() = flOriginalYaw, pPlayer->m_angEyeAnglesX() = flOriginalPitch;
					}
					else
						pPlayer->UpdateClientSideAnimation();

					G::UpdatingAnims = false;
				}
				I::GlobalVars->frametime = flOldFrameTime;
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