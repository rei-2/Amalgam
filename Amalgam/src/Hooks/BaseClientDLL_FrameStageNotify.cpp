#include "../SDK/SDK.h"

#include "../Features/Backtrack/Backtrack.h"
#include "../Features/CheaterDetection/CheaterDetection.h"
#include "../Features/CritHack/CritHack.h"
#include "../Features/Misc/Misc.h"
#include "../Features/Players/PlayerUtils.h"
#include "../Features/Resolver/Resolver.h"
#include "../Features/Simulation/MovementSimulation/MovementSimulation.h"
#include "../Features/Visuals/Visuals.h"

MAKE_HOOK(BaseClientDLL_FrameStageNotify, U::Memory.GetVFunc(I::BaseClientDLL, 35), void, __fastcall,
	void* ecx, ClientFrameStage_t curStage)
{
	switch (curStage)
	{
	case FRAME_RENDER_START:
		G::PunchAngles = Vec3();
		if (auto pLocal = H::Entities.GetLocal())
		{
			G::PunchAngles = pLocal->m_vecPunchAngle(); // use in aimbot
			if (Vars::Visuals::Removals::ViewPunch.Value)
				pLocal->m_vecPunchAngle() = Vec3(); // visual no-recoil
			F::Resolver.FrameStageNotify(pLocal);
			// CRASH: Exception thrown at 0x00007FFCD004E223 (client.dll) in tf_win64.exe: 0xC0000005: Access violation reading location 0x0000025800000000.
			// CRASH: Exception thrown at 0x00007FFC5A09EED0 (client.dll) in tf_win64.exe: 0xC0000005: Access violation reading location 0x000001F636472562.
			// crashes likely not fsn related
		}
	}

	CALL_ORIGINAL(ecx, curStage);

	switch (curStage)
	{
	case FRAME_NET_UPDATE_START:
		H::Entities.Clear();
		break;
	case FRAME_NET_UPDATE_END:
		H::Entities.Fill();
		for (auto& pEntity : H::Entities.GetGroup(EGroupType::PLAYERS_ALL))
		{
			if (pEntity->entindex() == I::EngineClient->GetLocalPlayer())
				continue; // local player managed in CPrediction_RunCommand

			G::VelocityMap[pEntity->entindex()] = { pEntity->m_vecOrigin(), pEntity->m_vecMaxs().z - pEntity->m_vecMins().z, pEntity->m_flSimulationTime() };

			static auto sv_maxusrcmdprocessticks = U::ConVars.FindVar("sv_maxusrcmdprocessticks");
			const int iTicks = sv_maxusrcmdprocessticks ? sv_maxusrcmdprocessticks->GetInt() : 24;
			if (auto iDifference = std::min(TIME_TO_TICKS(pEntity->m_flSimulationTime() - pEntity->m_flOldSimulationTime()), iTicks))
			{
				float flOldFrameTime = I::GlobalVars->frametime;
				I::GlobalVars->frametime = I::Prediction->m_bEnginePaused ? 0.f : TICK_INTERVAL;
				for (int i = 0; i < iDifference; i++)
				{
					G::UpdatingAnims = true;
					pEntity->As<CTFPlayer>()->UpdateClientSideAnimation();
					G::UpdatingAnims = false;
				}
				I::GlobalVars->frametime = flOldFrameTime;
			}
		}

		F::PlayerUtils.UpdatePlayers();
		F::Misc.DetectChoke();

		F::Backtrack.FrameStageNotify();
		F::MoveSim.FillVelocities();
		F::CritHack.Store();
		F::Visuals.StoreSightlines();

		F::CheaterDetection.Fill(); // maybe switch this to run instead

		break;
	case FRAME_RENDER_START:
		if (G::Unload)
			break;
		F::Visuals.SkyboxChanger();
		F::Visuals.Modulate();
	}
}