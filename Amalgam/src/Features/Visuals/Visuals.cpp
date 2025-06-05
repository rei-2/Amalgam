#include "Visuals.h"

#include "../Aimbot/Aimbot.h"
#include "../Ticks/Ticks.h"
#include "../Backtrack/Backtrack.h"
#include "../PacketManip/AntiAim/AntiAim.h"
#include "../Simulation/ProjectileSimulation/ProjectileSimulation.h"
#include "CameraWindow/CameraWindow.h"
#include "../Players/PlayerUtils.h"
#include "../Spectate/Spectate.h"

MAKE_SIGNATURE(CBaseAnimating_DrawServerHitboxes, "server.dll", "44 88 44 24 ? 53 48 81 EC", 0x0);
MAKE_SIGNATURE(GetServerAnimating, "server.dll", "48 83 EC ? 8B D1 85 C9 7E ? 48 8B 05", 0x0);
MAKE_SIGNATURE(CTFPlayer_FireEvent, "client.dll", "48 89 5C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 4C 89 64 24 ? 55 41 56 41 57 48 8D 6C 24", 0x0);
MAKE_SIGNATURE(CWeaponMedigun_UpdateEffects, "client.dll", "40 57 48 81 EC ? ? ? ? 8B 91 ? ? ? ? 48 8B F9 85 D2 0F 84 ? ? ? ? 48 89 B4 24", 0x0);
MAKE_SIGNATURE(CWeaponMedigun_StopChargeEffect, "client.dll", "40 53 48 83 EC ? 44 0F B6 C2", 0x0);
MAKE_SIGNATURE(CWeaponMedigun_ManageChargeEffect, "client.dll", "48 89 5C 24 ? 48 89 74 24 ? 57 48 81 EC ? ? ? ? 48 8B F1 E8 ? ? ? ? 48 8B D8", 0x0);

static std::vector<Vec3> SplashTrace(Vec3 vOrigin, float flRadius, Vec3 vNormal = { 0, 0, 1 }, bool bTrace = true, int iSegments = 100)
{
	if (!flRadius)
		return {};

	Vec3 vAngles; Math::VectorAngles(vNormal, vAngles);
	Vec3 vRight, vUp; Math::AngleVectors(vAngles, nullptr, &vRight, &vUp);

	std::vector<Vec3> vPoints = {};
	for (float i = 0.f; i < iSegments; i++)
	{
		Vec3 vPoint = vOrigin + (vRight * cos(2 * PI * i / iSegments) + vUp * sin(2 * PI * i / iSegments)) * flRadius;
		if (bTrace)
		{
			CGameTrace trace = {};
			CTraceFilterWorldAndPropsOnly filter = {};
			SDK::Trace(vOrigin, vPoint, MASK_SHOT, &filter, &trace);
			vPoint = trace.endpos;
		}
		vPoints.push_back(vPoint);
	}
	vPoints.push_back(vPoints.front());

	return vPoints;
}

void CVisuals::ProjectileTrace(CTFPlayer* pPlayer, CTFWeaponBase* pWeapon, const bool bQuick)
{
	if (bQuick)
		F::CameraWindow.m_bShouldDraw = false;
	if (bQuick ? !Vars::Visuals::Simulation::TrajectoryPath.Value && !Vars::Visuals::Simulation::ProjectileCamera.Value : !Vars::Visuals::Simulation::ShotPath.Value)
		return;

	Vec3 vAngles = bQuick ? I::EngineClient->GetViewAngles() : G::CurrentUserCmd->viewangles;
	int iFlags = bQuick ? ProjSimEnum::Trace | ProjSimEnum::InitCheck | ProjSimEnum::Quick : ProjSimEnum::Trace | ProjSimEnum::InitCheck;
	if (bQuick && F::Spectate.m_iTarget != -1)
	{
		pPlayer = I::ClientEntityList->GetClientEntity(I::EngineClient->GetPlayerForUserID(F::Spectate.m_iTarget))->As<CTFPlayer>();
		if (!pPlayer || pPlayer->IsDormant())
			return;

		pWeapon = pPlayer->m_hActiveWeapon().Get()->As<CTFWeaponBase>();
		if (I::Input->CAM_IsThirdPerson())
			vAngles = pPlayer->GetEyeAngles();

		pPlayer->m_vecViewOffset() = pPlayer->GetViewOffset();
	}
	if (!pPlayer || !pWeapon || pWeapon->GetWeaponID() == TF_WEAPON_FLAMETHROWER)
		return;

	ProjectileInfo tProjInfo = {};
	if (!F::ProjSim.GetInfo(pPlayer, pWeapon, vAngles, tProjInfo, iFlags, (bQuick && Vars::Aimbot::Projectile::AutoRelease.Value) ? Vars::Aimbot::Projectile::AutoRelease.Value / 100 : -1.f)
		|| !F::ProjSim.Initialize(tProjInfo))
		return;

	CGameTrace trace = {};
	CTraceFilterProjectile filter = {}; filter.pSkip = pPlayer;
	Vec3* pNormal = nullptr;

	for (int n = 1; n <= TIME_TO_TICKS(tProjInfo.m_flLifetime); n++)
	{
		Vec3 Old = F::ProjSim.GetOrigin();
		F::ProjSim.RunTick(tProjInfo);
		Vec3 New = F::ProjSim.GetOrigin();

		SDK::TraceHull(Old, New, tProjInfo.m_vHull * -1, tProjInfo.m_vHull, MASK_SOLID, &filter, &trace);
		if (trace.DidHit())
		{
			pNormal = &trace.plane.normal;
			if (n == 1 && trace.startsolid)
				*pNormal = F::ProjSim.GetVelocity().Normalized();
			break;
		}
	}
	
	if (tProjInfo.m_vPath.empty())
		return;

	tProjInfo.m_vPath.push_back(trace.endpos);

	std::vector<Vec3> vPoints = {};
	if ((bQuick ? Vars::Visuals::Simulation::TrajectoryPath.Value : Vars::Visuals::Simulation::ShotPath.Value) && Vars::Visuals::Simulation::SplashRadius.Value & Vars::Visuals::Simulation::SplashRadiusEnum::Simulation)
	{
		float flRadius = 0.f;
		switch (pWeapon->GetWeaponID())
		{
		case TF_WEAPON_ROCKETLAUNCHER:
		case TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT:
		case TF_WEAPON_PARTICLE_CANNON:
			if (Vars::Visuals::Simulation::SplashRadius.Value & Vars::Visuals::Simulation::SplashRadiusEnum::Rockets)
				flRadius = 146.f;
			break;
		case TF_WEAPON_PIPEBOMBLAUNCHER:
			if (Vars::Visuals::Simulation::SplashRadius.Value & Vars::Visuals::Simulation::SplashRadiusEnum::Stickies)
				flRadius = 146.f;
			break;
		case TF_WEAPON_GRENADELAUNCHER:
			if (Vars::Visuals::Simulation::SplashRadius.Value & Vars::Visuals::Simulation::SplashRadiusEnum::Pipes)
				flRadius = 146.f;
			break;
		case TF_WEAPON_FLAREGUN:
		case TF_WEAPON_FLAREGUN_REVENGE:
			if (Vars::Visuals::Simulation::SplashRadius.Value & Vars::Visuals::Simulation::SplashRadiusEnum::ScorchShot && pWeapon->As<CTFFlareGun>()->GetFlareGunType() == FLAREGUN_SCORCHSHOT)
				flRadius = 110.f;
		}

		if (flRadius)
		{
			Vec3 vEndPos = trace.endpos;
			flRadius = SDK::AttribHookValue(flRadius, "mult_explosion_radius", pWeapon);
			switch (pWeapon->GetWeaponID())
			{
			case TF_WEAPON_ROCKETLAUNCHER:
			case TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT:
			case TF_WEAPON_PARTICLE_CANNON:
				if (pNormal)
					vEndPos += *pNormal;
				if (pPlayer->InCond(TF_COND_BLASTJUMPING) && SDK::AttribHookValue(1.f, "rocketjump_attackrate_bonus", pWeapon) != 1.f)
					flRadius *= 0.8f;
			}
			vPoints = SplashTrace(vEndPos, flRadius, pNormal ? *pNormal : Vec3(0, 0, 1), Vars::Visuals::Simulation::SplashRadius.Value & Vars::Visuals::Simulation::SplashRadiusEnum::Trace);
		}
	}

	if (bQuick)
	{
		if (Vars::Visuals::Simulation::ProjectileCamera.Value && !I::EngineVGui->IsGameUIVisible() && pPlayer->m_vecOrigin().DistTo(trace.endpos) > 500.f)
		{
			CGameTrace cameraTrace = {};

			auto vAngles = Math::CalcAngle(trace.startpos, trace.endpos);
			Vec3 vForward; Math::AngleVectors(vAngles, &vForward);
			SDK::Trace(trace.endpos, trace.endpos - vForward * 500.f, MASK_SOLID, &filter, &cameraTrace);

			F::CameraWindow.m_bShouldDraw = true;
			F::CameraWindow.m_vCameraOrigin = cameraTrace.endpos;
			F::CameraWindow.m_vCameraAngles = vAngles;
		}

		if (Vars::Visuals::Simulation::TrajectoryPath.Value)
		{
			H::Draw.RenderPath(tProjInfo.m_vPath, Vars::Colors::TrajectoryPath.Value, false, Vars::Visuals::Simulation::TrajectoryPath.Value);
			H::Draw.RenderPath(tProjInfo.m_vPath, Vars::Colors::TrajectoryPathClipped.Value, true, Vars::Visuals::Simulation::TrajectoryPath.Value);

			if (Vars::Visuals::Simulation::Box.Value && pNormal)
			{
				const float flSize = std::max(tProjInfo.m_vHull.Min(), 1.f);
				const Vec3 vSize = { 1.f, flSize, flSize };
				Vec3 vAngles; Math::VectorAngles(*pNormal, vAngles);

				H::Draw.RenderWireframeBox(trace.endpos, vSize * -1, vSize, vAngles, Vars::Colors::TrajectoryPath.Value);
				H::Draw.RenderWireframeBox(trace.endpos, vSize * -1, vSize, vAngles, Vars::Colors::TrajectoryPathClipped.Value, true);
			}

			if (!vPoints.empty())
			{
				H::Draw.RenderPath(vPoints, Vars::Colors::SplashRadius.Value, false, Vars::Visuals::Simulation::StyleEnum::Line);
				H::Draw.RenderPath(vPoints, Vars::Colors::SplashRadiusClipped.Value, true, Vars::Visuals::Simulation::StyleEnum::Line);
			}
		}
	}
	else if (Vars::Visuals::Simulation::ShotPath.Value)
	{
		G::BoxStorage.clear();
		G::PathStorage.clear();

		if (Vars::Colors::ShotPath.Value.a)
			G::PathStorage.emplace_back(tProjInfo.m_vPath, -float(tProjInfo.m_vPath.size()) - TIME_TO_TICKS(F::Backtrack.GetReal()), Vars::Colors::ShotPath.Value, Vars::Visuals::Simulation::ShotPath.Value);
		if (Vars::Colors::ShotPathClipped.Value.a)
			G::PathStorage.emplace_back(tProjInfo.m_vPath, -float(tProjInfo.m_vPath.size()) - TIME_TO_TICKS(F::Backtrack.GetReal()), Vars::Colors::ShotPathClipped.Value, Vars::Visuals::Simulation::ShotPath.Value, true);

		if (Vars::Visuals::Simulation::Box.Value && pNormal)
		{
			const float flSize = std::max(tProjInfo.m_vHull.x, 1.f);
			const Vec3 vSize = { 1.f, flSize, flSize };
			Vec3 vAngles; Math::VectorAngles(*pNormal, vAngles);

			if (Vars::Colors::ShotPath.Value.a)
				G::BoxStorage.emplace_back(trace.endpos, vSize * -1, vSize, vAngles, I::GlobalVars->curtime + TICKS_TO_TIME(tProjInfo.m_vPath.size()) + F::Backtrack.GetReal(), Vars::Colors::ShotPath.Value, Color_t(0, 0, 0, 0));
			if (Vars::Colors::ShotPathClipped.Value.a)
				G::BoxStorage.emplace_back(trace.endpos, vSize * -1, vSize, vAngles, I::GlobalVars->curtime + TICKS_TO_TIME(tProjInfo.m_vPath.size()) + F::Backtrack.GetReal(), Vars::Colors::ShotPathClipped.Value, Color_t(0, 0, 0, 0), true);
		}

		if (!vPoints.empty())
		{
			if (Vars::Colors::SplashRadius.Value.a)
				G::PathStorage.emplace_back(vPoints, I::GlobalVars->curtime + TICKS_TO_TIME(tProjInfo.m_vPath.size()) + F::Backtrack.GetReal(), Vars::Colors::SplashRadius.Value, Vars::Visuals::Simulation::StyleEnum::Line);
			if (Vars::Colors::SplashRadiusClipped.Value.a)
				G::PathStorage.emplace_back(vPoints, I::GlobalVars->curtime + TICKS_TO_TIME(tProjInfo.m_vPath.size()) + F::Backtrack.GetReal(), Vars::Colors::SplashRadiusClipped.Value, Vars::Visuals::Simulation::StyleEnum::Line, true);
		}
	}
}

void CVisuals::SplashRadius(CTFPlayer* pLocal)
{
	if (!Vars::Visuals::Simulation::SplashRadius.Value)
		return;

	for (auto pEntity : H::Entities.GetGroup(EGroupType::WORLD_PROJECTILES))
	{
		bool bShouldDraw = false;
		CTFPlayer* pOwner = nullptr;
		CTFWeaponBase* pWeapon = nullptr;

		switch (pEntity->GetClassID())
		{
		case ETFClassID::CTFWeaponBaseGrenadeProj:
		case ETFClassID::CTFWeaponBaseMerasmusGrenade:
		case ETFClassID::CTFGrenadePipebombProjectile:
			bShouldDraw = Vars::Visuals::Simulation::SplashRadius.Value & (pEntity->As<CTFGrenadePipebombProjectile>()->HasStickyEffects() ? Vars::Visuals::Simulation::SplashRadiusEnum::Stickies : Vars::Visuals::Simulation::SplashRadiusEnum::Pipes);
			break;
		case ETFClassID::CTFProjectile_Rocket:
		case ETFClassID::CTFProjectile_SentryRocket:
		case ETFClassID::CTFProjectile_EnergyBall:
			bShouldDraw = Vars::Visuals::Simulation::SplashRadius.Value & Vars::Visuals::Simulation::SplashRadiusEnum::Rockets;
			break;
		case ETFClassID::CTFProjectile_Flare:
			if (Vars::Visuals::Simulation::SplashRadius.Value & Vars::Visuals::Simulation::SplashRadiusEnum::ScorchShot)
			{
				pWeapon = pEntity->As<CTFProjectile_Flare>()->m_hLauncher().Get()->As<CTFWeaponBase>();
				bShouldDraw = pWeapon && pWeapon->As<CTFFlareGun>()->GetFlareGunType() == FLAREGUN_SCORCHSHOT;
			}
		}
		if (!bShouldDraw)
			continue;

		switch (pEntity->GetClassID())
		{
		case ETFClassID::CTFWeaponBaseGrenadeProj:
		case ETFClassID::CTFWeaponBaseMerasmusGrenade:
		case ETFClassID::CTFGrenadePipebombProjectile:
			pOwner = pEntity->As<CTFGrenadePipebombProjectile>()->m_hThrower().Get()->As<CTFPlayer>();
			break;
		case ETFClassID::CTFProjectile_Rocket:
		case ETFClassID::CTFProjectile_SentryRocket:
		case ETFClassID::CTFProjectile_EnergyBall:
		case ETFClassID::CTFProjectile_Flare:
			pOwner = pEntity->m_hOwnerEntity().Get()->As<CTFPlayer>();
		}
		if (!pOwner || !pOwner->IsPlayer())
			continue;
		else if (pOwner->entindex() != I::EngineClient->GetLocalPlayer())
		{
			if (!(Vars::Visuals::Simulation::SplashRadius.Value & Vars::Visuals::Simulation::SplashRadiusEnum::Priority && F::PlayerUtils.IsPrioritized(pOwner->entindex()))
				&& !(Vars::Visuals::Simulation::SplashRadius.Value & Vars::Visuals::Simulation::SplashRadiusEnum::Friends && H::Entities.IsFriend(pOwner->entindex()))
				&& !(Vars::Visuals::Simulation::SplashRadius.Value & Vars::Visuals::Simulation::SplashRadiusEnum::Party && H::Entities.InParty(pOwner->entindex()))
				&& !(pOwner->m_iTeamNum() != pLocal->m_iTeamNum() ? Vars::Visuals::Simulation::SplashRadius.Value & Vars::Visuals::Simulation::SplashRadiusEnum::Enemy : Vars::Visuals::Simulation::SplashRadius.Value & Vars::Visuals::Simulation::SplashRadiusEnum::Team))
				continue;
		}
		else if (!(Vars::Visuals::Simulation::SplashRadius.Value & Vars::Visuals::Simulation::SplashRadiusEnum::Local))
			continue;

		float flRadius = 146.f;
		switch (pEntity->GetClassID())
		{
		case ETFClassID::CTFWeaponBaseGrenadeProj:
		case ETFClassID::CTFWeaponBaseMerasmusGrenade:
		case ETFClassID::CTFGrenadePipebombProjectile:
			pWeapon = pEntity->As<CTFGrenadePipebombProjectile>()->m_hOriginalLauncher().Get()->As<CTFWeaponBase>();
			break;
		case ETFClassID::CTFProjectile_Rocket:
		case ETFClassID::CTFProjectile_SentryRocket:
		case ETFClassID::CTFProjectile_EnergyBall:
			pWeapon = pEntity->As<CTFBaseRocket>()->m_hLauncher().Get()->As<CTFWeaponBase>();
			break;
		case ETFClassID::CTFProjectile_Flare:
			flRadius = 110.f;
		}
		if (pWeapon)
		{
			flRadius = SDK::AttribHookValue(flRadius, "mult_explosion_radius", pWeapon);
			switch (pWeapon->GetWeaponID())
			{
			case TF_WEAPON_ROCKETLAUNCHER:
			case TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT:
			case TF_WEAPON_PARTICLE_CANNON:
				if (pOwner->InCond(TF_COND_BLASTJUMPING) && SDK::AttribHookValue(1.f, "rocketjump_attackrate_bonus", pWeapon) != 1.f)
					flRadius *= 0.8f;
			}
		}

		auto vPoints = SplashTrace(pEntity->GetAbsOrigin(), flRadius, { 0, 0, 1 }, Vars::Visuals::Simulation::SplashRadius.Value & Vars::Visuals::Simulation::SplashRadiusEnum::Trace);
		H::Draw.RenderPath(vPoints, Vars::Colors::SplashRadius.Value, false, Vars::Visuals::Simulation::StyleEnum::Line);
		H::Draw.RenderPath(vPoints, Vars::Colors::SplashRadiusClipped.Value, true, Vars::Visuals::Simulation::StyleEnum::Line);
	}
}

void CVisuals::DrawAntiAim(CTFPlayer* pLocal)
{
	if (!pLocal->IsAlive() || pLocal->IsAGhost() || !I::Input->CAM_IsThirdPerson())
		return;

	if (F::AntiAim.AntiAimOn() && Vars::Debug::AntiAimLines.Value)
	{
		const auto& vOrigin = pLocal->GetAbsOrigin();

		Vec3 vScreen1, vScreen2;
		if (SDK::W2S(vOrigin, vScreen1))
		{
			if (SDK::W2S(vOrigin + Math::RotatePoint({ 50, 0, 0 }, {}, { 0, F::AntiAim.vRealAngles.y, 0 }), vScreen2))
				H::Draw.Line(vScreen1.x, vScreen1.y, vScreen2.x, vScreen2.y, { 0, 255, 0, 255 });
			if (SDK::W2S(vOrigin + Math::RotatePoint({ 50, 0, 0 }, {}, { 0, F::AntiAim.vFakeAngles.y, 0 }), vScreen2))
				H::Draw.Line(vScreen1.x, vScreen1.y, vScreen2.x, vScreen2.y, { 255, 0, 0, 255 });
		}

		for (auto& vPair : F::AntiAim.vEdgeTrace)
		{
			if (SDK::W2S(vPair.first, vScreen1) && SDK::W2S(vPair.second, vScreen2))
				H::Draw.Line(vScreen1.x, vScreen1.y, vScreen2.x, vScreen2.y, { 255, 255, 255, 255 });
		}
	}
}

void CVisuals::DrawPickupTimers()
{
	if (!Vars::ESP::Other::PickupTimers.Value)
		return;

	for (auto it = m_vPickups.begin(); it != m_vPickups.end();)
	{
		auto& tPickup = *it;

		float flTime = I::EngineClient->Time() - tPickup.Time;
		if (flTime > 10.f)
		{
			it = m_vPickups.erase(it);
			continue;
		}

		Vec3 vScreen; if (SDK::W2S(tPickup.Location, vScreen))
		{
			auto tColor = tPickup.Type ? Vars::Colors::Health.Value : Vars::Colors::Ammo.Value;
			H::Draw.StringOutlined(H::Fonts.GetFont(FONT_ESP), vScreen.x, vScreen.y, tColor, Vars::Menu::Theme::Background.Value, ALIGN_CENTER, std::format("{:.1f}s", 10.f - flTime).c_str());
		}

		it++;
	}
}

#define PAIR(x) { x, #x }
void CVisuals::DrawDebugInfo(CTFPlayer* pLocal)
{
	if (Vars::Debug::Info.Value)
	{
		auto pWeapon = H::Entities.GetWeapon();
		auto pCmd = !I::EngineClient->IsPlayingDemo() ? G::LastUserCmd : I::Input->GetUserCmd(I::ClientState->lastoutgoingcommand);

		int x = 10, y = 10;
		const auto& fFont = H::Fonts.GetFont(FONT_INDICATORS);
		const int nTall = fFont.m_nTall + H::Draw.Scale(1);
		y -= nTall;

		if (pCmd)
		{
			H::Draw.StringOutlined(fFont, x, y += nTall, Vars::Menu::Theme::Active.Value, Vars::Menu::Theme::Background.Value, ALIGN_TOPLEFT, std::format("View: ({:.3f}, {:.3f}, {:.3f})", pCmd->viewangles.x, pCmd->viewangles.y, pCmd->viewangles.z).c_str());
			H::Draw.StringOutlined(fFont, x, y += nTall, Vars::Menu::Theme::Active.Value, Vars::Menu::Theme::Background.Value, ALIGN_TOPLEFT, std::format("Move: ({}, {}, {})", pCmd->forwardmove, pCmd->sidemove, pCmd->upmove).c_str());
			
			std::vector<std::pair<int, const char*>> vFlags = { 
				PAIR(IN_ATTACK),
				PAIR(IN_ATTACK2),
				PAIR(IN_ATTACK3),
				PAIR(IN_FORWARD),
				PAIR(IN_BACK),
				PAIR(IN_MOVELEFT),
				PAIR(IN_MOVERIGHT),
				PAIR(IN_JUMP),
				PAIR(IN_DUCK),
				PAIR(IN_RELOAD),
				PAIR(IN_LEFT),
				PAIR(IN_RIGHT),
				PAIR(IN_SCORE),
				/*
				PAIR(IN_USE),
				PAIR(IN_CANCEL),
				PAIR(IN_RUN),
				PAIR(IN_ALT1),
				PAIR(IN_ALT2),
				PAIR(IN_SPEED),
				PAIR(IN_WALK),
				PAIR(IN_ZOOM),
				PAIR(IN_WEAPON1),
				PAIR(IN_WEAPON2),
				PAIR(IN_BULLRUSH),
				PAIR(IN_GRENADE1),
				PAIR(IN_GRENADE2),
				*/
			};
			std::string sButtons = "NONE";
			int i = 0;
			for (auto& pFlag : vFlags)
			{
				if (pCmd->buttons & pFlag.first)
				{
					if (i)
						sButtons += " | ";
					else
						sButtons = "";
					sButtons += pFlag.second;
					i++;
				}
			}
			H::Draw.StringOutlined(fFont, x, y += nTall, Vars::Menu::Theme::Active.Value, Vars::Menu::Theme::Background.Value, ALIGN_TOPLEFT, std::format("Buttons: {} ({})", pCmd->buttons, sButtons).c_str());
		}
		{
			Vec3 vOrigin = pLocal->m_vecOrigin();
			H::Draw.StringOutlined(fFont, x, y += nTall, Vars::Menu::Theme::Active.Value, Vars::Menu::Theme::Background.Value, ALIGN_TOPLEFT, std::format("Origin: ({:.3f}, {:.3f}, {:.3f})", vOrigin.x, vOrigin.y, vOrigin.z).c_str());
		}
		{
			Vec3 vVelocity = pLocal->m_vecVelocity();
			H::Draw.StringOutlined(fFont, x, y += nTall, Vars::Menu::Theme::Active.Value, Vars::Menu::Theme::Background.Value, ALIGN_TOPLEFT, std::format("Velocity: {:.3f} ({:.3f}, {:.3f}, {:.3f})", vVelocity.Length(), vVelocity.x, vVelocity.y, vVelocity.z).c_str());
		}
		H::Draw.StringOutlined(fFont, x, y += nTall, Vars::Menu::Theme::Active.Value, Vars::Menu::Theme::Background.Value, ALIGN_TOPLEFT, std::format("Choke: {}, {}", G::Choking, I::ClientState->chokedcommands).c_str());
		H::Draw.StringOutlined(fFont, x, y += nTall, Vars::Menu::Theme::Active.Value, Vars::Menu::Theme::Background.Value, ALIGN_TOPLEFT, std::format("Ticks: {}, {}", F::Ticks.m_iShiftedTicks, F::Ticks.m_iShiftedGoal).c_str());
		H::Draw.StringOutlined(fFont, x, y += nTall, Vars::Menu::Theme::Active.Value, Vars::Menu::Theme::Background.Value, ALIGN_TOPLEFT, std::format("Round state: {}, {}, {}", SDK::GetRoundState(), SDK::GetWinningTeam(), I::EngineClient->IsPlayingDemo()).c_str());
		H::Draw.StringOutlined(fFont, x, y += nTall, Vars::Menu::Theme::Active.Value, Vars::Menu::Theme::Background.Value, ALIGN_TOPLEFT, std::format("Tickcount: {}", pLocal->m_nTickBase()).c_str());
		H::Draw.StringOutlined(fFont, x, y += nTall, Vars::Menu::Theme::Active.Value, Vars::Menu::Theme::Background.Value, ALIGN_TOPLEFT, std::format("Entities: {} ({}, {})", I::ClientEntityList->GetMaxEntities(), I::ClientEntityList->GetHighestEntityIndex(), I::ClientEntityList->NumberOfEntities(false)).c_str());
	
		if (pWeapon)
		{
			float flTime = TICKS_TO_TIME(pLocal->m_nTickBase());
			float flPrimaryAttack = pWeapon->m_flNextPrimaryAttack();
			float flSecondaryAttack = pWeapon->m_flNextSecondaryAttack();
			float flAttack = pLocal->m_flNextAttack();

			H::Draw.StringOutlined(fFont, x, y += nTall * 2, Vars::Menu::Theme::Active.Value, Vars::Menu::Theme::Background.Value, ALIGN_TOPLEFT, std::format("Weapon: {}, {}", pWeapon->GetSlot(), pWeapon->GetWeaponID()).c_str());
			H::Draw.StringOutlined(fFont, x, y += nTall, Vars::Menu::Theme::Active.Value, Vars::Menu::Theme::Background.Value, ALIGN_TOPLEFT, std::format("Attacking: {}", G::Attacking).c_str());
			H::Draw.StringOutlined(fFont, x, y += nTall, Vars::Menu::Theme::Active.Value, Vars::Menu::Theme::Background.Value, ALIGN_TOPLEFT, std::format("CanPrimaryAttack: {} ([{:.3f} | {:.3f}] <= {:.3f})", G::CanPrimaryAttack, flPrimaryAttack, flAttack, flTime).c_str());
			H::Draw.StringOutlined(fFont, x, y += nTall, Vars::Menu::Theme::Active.Value, Vars::Menu::Theme::Background.Value, ALIGN_TOPLEFT, std::format("CanSecondaryAttack: {} ([{:.3f} | {:.3f}] <= {:.3f})", G::CanSecondaryAttack, flSecondaryAttack, flAttack, flTime).c_str());
			H::Draw.StringOutlined(fFont, x, y += nTall, Vars::Menu::Theme::Active.Value, Vars::Menu::Theme::Background.Value, ALIGN_TOPLEFT, std::format("Attack: {:.3f}, {:.3f}; {:.3f}", flTime - flPrimaryAttack, flTime - flSecondaryAttack, flTime - flAttack).c_str());
			H::Draw.StringOutlined(fFont, x, y += nTall, Vars::Menu::Theme::Active.Value, Vars::Menu::Theme::Background.Value, ALIGN_TOPLEFT, std::format("Reload: {} ({} || {} != 0)", G::Reloading, pWeapon->m_bInReload(), pWeapon->m_iReloadMode()).c_str());
			H::Draw.StringOutlined(fFont, x, y += nTall, Vars::Menu::Theme::Active.Value, Vars::Menu::Theme::Background.Value, ALIGN_TOPLEFT, std::format("Throw: {}, Smack: {}", G::Throwing, pWeapon->m_flSmackTime()).c_str());
		}
	}
}
#undef PAIR



std::vector<DrawBox_t> CVisuals::GetHitboxes(matrix3x4* aBones, CBaseAnimating* pEntity, std::vector<int> vHitboxes, int iTarget)
{
	if (!Vars::Colors::BoneHitboxEdge.Value.a && !Vars::Colors::BoneHitboxFace.Value.a && !Vars::Colors::BoneHitboxEdgeClipped.Value.a && !Vars::Colors::BoneHitboxFaceClipped.Value.a
		&& !Vars::Colors::TargetHitboxEdge.Value.a && !Vars::Colors::TargetHitboxFace.Value.a && !Vars::Colors::TargetHitboxEdgeClipped.Value.a && !Vars::Colors::TargetHitboxFaceClipped.Value.a)
		return {};

	std::vector<DrawBox_t> vBoxes = {};

	auto pModel = pEntity->GetModel();
	if (!pModel) return vBoxes;
	auto pHDR = I::ModelInfoClient->GetStudiomodel(pModel);
	if (!pHDR) return vBoxes;
	auto pSet = pHDR->pHitboxSet(pEntity->m_nHitboxSet());
	if (!pSet) return vBoxes;

	if (vHitboxes.empty())
	{
		for (int i = 0; i < pSet->numhitboxes; i++)
			vHitboxes.push_back(i);
	}

	for (int i : vHitboxes)
	{
		auto pBox = pSet->pHitbox(i);
		if (!pBox) continue;

		bool bTargeted = i == iTarget;
		Vec3 vAngle; Math::MatrixAngles(aBones[pBox->bone], vAngle);
		Vec3 vOrigin; Math::GetMatrixOrigin(aBones[pBox->bone], vOrigin);
		Vec3 vMins = pBox->bbmin * pEntity->m_flModelScale();
		Vec3 vMaxs = pBox->bbmax * pEntity->m_flModelScale();

		Color_t tEdge = bTargeted ? Vars::Colors::TargetHitboxEdge.Value : Vars::Colors::BoneHitboxEdge.Value;
		Color_t tFace = bTargeted ? Vars::Colors::TargetHitboxFace.Value : Vars::Colors::BoneHitboxFace.Value;
		if (tEdge.a || tFace.a)
			vBoxes.emplace_back(vOrigin, vMins, vMaxs, vAngle, I::GlobalVars->curtime + Vars::Visuals::Hitbox::DrawDuration.Value, tEdge, tFace);

		tEdge = bTargeted ? Vars::Colors::TargetHitboxEdgeClipped.Value : Vars::Colors::BoneHitboxEdgeClipped.Value;
		tFace = bTargeted ? Vars::Colors::TargetHitboxFaceClipped.Value : Vars::Colors::BoneHitboxFaceClipped.Value;
		if (tEdge.a || tFace.a)
			vBoxes.emplace_back(vOrigin, vMins, vMaxs, vAngle, I::GlobalVars->curtime + Vars::Visuals::Hitbox::DrawDuration.Value, tEdge, tFace, true);

		if (Vars::Debug::Info.Value)
		{
			float flBoneScale = std::max(Vars::Aimbot::Hitscan::BoneSizeMinimumScale.Value, Vars::Aimbot::Hitscan::PointScale.Value / 100.f);
			float flBoneSubtract = Vars::Aimbot::Hitscan::BoneSizeSubtract.Value;

			Vec3 vCheckMins = (pBox->bbmin + flBoneSubtract / pEntity->m_flModelScale()) * flBoneScale * pEntity->m_flModelScale();
			Vec3 vCheckMaxs = (pBox->bbmax - flBoneSubtract / pEntity->m_flModelScale()) * flBoneScale * pEntity->m_flModelScale();

			Color_t tEdge = bTargeted ? Vars::Colors::TargetHitboxEdge.Value : Vars::Colors::BoneHitboxEdge.Value;
			Color_t tFace = bTargeted ? Vars::Colors::TargetHitboxFace.Value : Vars::Colors::BoneHitboxFace.Value;
			if (tEdge.a || tFace.a)
				vBoxes.emplace_back(vOrigin, vCheckMins, vCheckMaxs, vAngle, I::GlobalVars->curtime + Vars::Visuals::Hitbox::DrawDuration.Value, tEdge, tFace);

			tEdge = bTargeted ? Vars::Colors::TargetHitboxEdgeClipped.Value : Vars::Colors::BoneHitboxEdgeClipped.Value;
			tFace = bTargeted ? Vars::Colors::TargetHitboxFaceClipped.Value : Vars::Colors::BoneHitboxFaceClipped.Value;
			if (tEdge.a || tFace.a)
				vBoxes.emplace_back(vOrigin, vCheckMins, vCheckMaxs, vAngle, I::GlobalVars->curtime + Vars::Visuals::Hitbox::DrawDuration.Value, tEdge, tFace, true);
		}
	}

	return vBoxes;
}

void CVisuals::DrawEffects()
{
	for (auto& tLine : G::LineStorage)
	{
		if (tLine.m_flTime < I::GlobalVars->curtime)
			continue;

		H::Draw.RenderLine(tLine.m_vOrigin.first, tLine.m_vOrigin.second, tLine.m_tColor, tLine.m_bZBuffer);
	}
	for (auto& tPath : G::PathStorage)
	{
		if (tPath.m_flTime >= 0.f && tPath.m_flTime < I::GlobalVars->curtime)
			continue;

		H::Draw.RenderPath(tPath.m_vPath, tPath.m_tColor, tPath.m_bZBuffer, tPath.m_iStyle, tPath.m_flTime);
	}
	for (auto& tBox : G::BoxStorage)
	{
		if (tBox.m_flTime < I::GlobalVars->curtime)
			continue;

		H::Draw.RenderBox(tBox.m_vOrigin, tBox.m_vMins, tBox.m_vMaxs, tBox.m_vAngles, tBox.m_tColorFace, tBox.m_bZBuffer);
		H::Draw.RenderWireframeBox(tBox.m_vOrigin, tBox.m_vMins, tBox.m_vMaxs, tBox.m_vAngles, tBox.m_tColorEdge, tBox.m_bZBuffer);
	}
	for (auto& tBox : G::SphereStorage)
	{
		if (tBox.m_flTime < I::GlobalVars->curtime)
			continue;

		H::Draw.RenderSphere(tBox.m_vOrigin, tBox.m_flRadius, tBox.m_nTheta, tBox.m_nPhi, tBox.m_tColorFace, tBox.m_bZBuffer);
		H::Draw.RenderWireframeSphere(tBox.m_vOrigin, tBox.m_flRadius, tBox.m_nTheta, tBox.m_nPhi, tBox.m_tColorEdge, tBox.m_bZBuffer);
	}
	for (auto& tBox : G::SweptStorage)
	{
		if (tBox.m_flTime < I::GlobalVars->curtime)
			continue;

		H::Draw.RenderWireframeSweptBox(tBox.m_vOrigin.first, tBox.m_vOrigin.second, tBox.m_vMins, tBox.m_vMaxs, tBox.m_vAngles, tBox.m_tColor, tBox.m_bZBuffer);
	}
	for (auto& tSightline : m_vSightLines)
		H::Draw.RenderLine(tSightline.m_vStart, tSightline.m_vEnd, tSightline.m_Color, tSightline.m_bZBuffer);
}

void CVisuals::DrawServerHitboxes(CTFPlayer* pLocal)
{
	if (!Vars::Debug::DrawServerHitboxes.Value)
		return;

	if (I::Input->CAM_IsThirdPerson() && pLocal->IsAlive())
	{
		if (auto pServerAnimating = S::GetServerAnimating.Call<void*>(pLocal->entindex()))
			S::CBaseAnimating_DrawServerHitboxes.Call<void>(pServerAnimating, 0.f, true);
	}

	for (auto& pEntity : H::Entities.GetGroup(EGroupType::PLAYERS_ALL))
	{
		auto pPlayer = pEntity->As<CTFPlayer>();
		if (pPlayer->entindex() == I::EngineClient->GetLocalPlayer() || !pPlayer->IsAlive())
			continue;

		if (auto pServerAnimating = S::GetServerAnimating.Call<void*>(pPlayer->entindex()))
			S::CBaseAnimating_DrawServerHitboxes.Call<void>(pServerAnimating, 0.f, true);
	}
}



void CVisuals::FOV(CTFPlayer* pLocal, CViewSetup* pView)
{
	bool bZoomed = pLocal->InCond(TF_COND_ZOOMED);
	static auto fov_desired = U::ConVars.FindVar("fov_desired");
	float flDefaultFOV = fov_desired->GetFloat();
	float flRegularOverride = Vars::Visuals::UI::FieldOfView.Value;
	float flZoomOverride = Vars::Visuals::UI::ZoomFieldOfView.Value;
	float flDesiredFOV = !bZoomed ? flRegularOverride : flZoomOverride;

	pView->fov = pLocal->m_iFOV() = flDesiredFOV ? flDesiredFOV : pView->fov;
	pLocal->m_iDefaultFOV() = std::max(flRegularOverride, flDefaultFOV);

	if (!I::Prediction->InPrediction() && (flRegularOverride || flZoomOverride))
	{
		float flDeltaTime = (TICKS_TO_TIME(pLocal->m_nFinalPredictedTick()) - pLocal->m_flFOVTime() + TICKS_TO_TIME(I::GlobalVars->interpolation_amount)) / pLocal->m_flFOVRate();
		if (flDeltaTime < 1.f)
		{
			float flRegular = flRegularOverride ? flRegularOverride : flDefaultFOV;
			float flZoomed = flZoomOverride ? flZoomOverride : 20.f;

			float flFrom = !bZoomed ? flZoomed : flRegular;
			float flTo = !bZoomed ? flRegular : flZoomed;
			pView->fov = pLocal->m_iFOV() = Math::SimpleSplineRemapVal(flDeltaTime, 0.f, 1.f, flFrom, flTo);
		}
	}
}

void CVisuals::ThirdPerson(CTFPlayer* pLocal, CViewSetup* pView)
{
	if (!pLocal->IsAlive() || F::Spectate.m_iTarget != -1)
		return I::Input->CAM_ToFirstPerson();

	const bool bZoom = pLocal->InCond(TF_COND_ZOOMED) && (!Vars::Visuals::Removals::Scope.Value || Vars::Visuals::UI::ZoomFieldOfView.Value < 20);
	const bool bForce = pLocal->IsTaunting() || pLocal->IsAGhost() || pLocal->InCond(TF_COND_HALLOWEEN_KART) || pLocal->InCond(TF_COND_HALLOWEEN_THRILLER);
	//if (bForce)
	//	return;

	if (Vars::Visuals::Thirdperson::Enabled.Value && !bZoom || bForce)
		I::Input->CAM_ToThirdPerson();
	else
		I::Input->CAM_ToFirstPerson();
	pLocal->ThirdPersonSwitch();

	static auto cam_ideallag = U::ConVars.FindVar("cam_ideallag");
	cam_ideallag->SetValue(0.f);

	if (I::Input->CAM_IsThirdPerson())
	{	// thirdperson offset
		Vec3 vForward, vRight, vUp; Math::AngleVectors(pView->angles, &vForward, &vRight, &vUp);

		Vec3 vOffset;
		float flScale = Vars::Visuals::Thirdperson::Scale.Value ? pLocal->m_flModelScale() : 1.f;
		vOffset += vRight * Vars::Visuals::Thirdperson::Right.Value * flScale;
		vOffset += vUp * Vars::Visuals::Thirdperson::Up.Value * flScale;
		vOffset -= vForward * Vars::Visuals::Thirdperson::Distance.Value * flScale;

		Vec3 vOrigin = pLocal->GetEyePosition(); //pView->origin
		Vec3 vStart = vOrigin;
		Vec3 vEnd = vOrigin + vOffset;

		if (Vars::Visuals::Thirdperson::Collide.Value)
		{
			float flHull = 9.f * flScale;
			Vec3 vMins = { -flHull, -flHull, -flHull }, vMaxs = { flHull, flHull, flHull };

			CGameTrace trace = {};
			CTraceFilterWorldAndPropsOnly filter = {};
			SDK::TraceHull(vStart, vEnd, vMins, vMaxs, MASK_SOLID, &filter, &trace);
			vEnd = trace.endpos;
		}

		pView->origin = vEnd;
	}
}

void CVisuals::Event(IGameEvent* pEvent, uint32_t uHash)
{
	switch (uHash)
	{
	case FNV1A::Hash32Const("player_hurt"):
	{
		bool bBones = Vars::Visuals::Hitbox::BonesEnabled.Value & Vars::Visuals::Hitbox::BonesEnabledEnum::OnHit;
		bool bBounds = Vars::Visuals::Hitbox::BoundsEnabled.Value & Vars::Visuals::Hitbox::BoundsEnabledEnum::OnHit;
		if (!bBones && !bBounds)
			return;

		if (I::EngineClient->GetPlayerForUserID(pEvent->GetInt("attacker")) != I::EngineClient->GetLocalPlayer())
			return;

		int iVictim = I::EngineClient->GetPlayerForUserID(pEvent->GetInt("userid"));
		auto pEntity = I::ClientEntityList->GetClientEntity(iVictim)->As<CBaseAnimating>();
		if (!pEntity || iVictim == I::EngineClient->GetLocalPlayer())
			return;

		switch (G::PrimaryWeaponType)
		{
		case EWeaponType::HITSCAN:
		case EWeaponType::MELEE:
		{
			if (!bBones)
				break;

			auto pBones = H::Entities.GetBones(pEntity->entindex());
			if (!pBones)
				return;

			auto vBoxes = GetHitboxes(pBones, pEntity);
			G::BoxStorage.insert(G::BoxStorage.end(), vBoxes.begin(), vBoxes.end());

			return;
		}
		case EWeaponType::PROJECTILE:
		{
			if (!bBounds)
				break;

			if (Vars::Colors::BoundHitboxEdge.Value.a || Vars::Colors::BoundHitboxFace.Value.a)
				G::BoxStorage.emplace_back(pEntity->m_vecOrigin(), pEntity->m_vecMins(), pEntity->m_vecMaxs(), Vec3(), I::GlobalVars->curtime + Vars::Visuals::Hitbox::DrawDuration.Value, Vars::Colors::BoundHitboxEdge.Value, Vars::Colors::BoundHitboxFace.Value, true);
			if (Vars::Colors::BoundHitboxEdgeClipped.Value.a || Vars::Colors::BoundHitboxFaceClipped.Value.a)
				G::BoxStorage.emplace_back(pEntity->m_vecOrigin(), pEntity->m_vecMins(), pEntity->m_vecMaxs(), Vec3(), I::GlobalVars->curtime + Vars::Visuals::Hitbox::DrawDuration.Value, Vars::Colors::BoundHitboxEdgeClipped.Value, Vars::Colors::BoundHitboxFaceClipped.Value, true);
		}
		}

		break;
	}
	case FNV1A::Hash32Const("item_pickup"):
	{
		if (!Vars::ESP::Other::PickupTimers.Value)
			return;

		auto pEntity = I::ClientEntityList->GetClientEntity(I::EngineClient->GetPlayerForUserID(pEvent->GetInt("userid")))->As<CTFPlayer>();
		if (!pEntity || !pEntity->IsPlayer())
			return;

		const char* sItemName = pEvent->GetString("item");
		if (std::strstr(sItemName, "medkit"))
			m_vPickups.emplace_back(1, I::EngineClient->Time(), pEntity->m_vecOrigin());
		else if (std::strstr(sItemName, "ammopack"))
			m_vPickups.emplace_back(0, I::EngineClient->Time(), pEntity->m_vecOrigin());
		
		break;
	}
	case FNV1A::Hash32Const("client_disconnect"):
	case FNV1A::Hash32Const("client_beginconnect"):
	case FNV1A::Hash32Const("game_newmap"):
	case FNV1A::Hash32Const("teamplay_round_start"):
		G::LineStorage.clear();
		G::BoxStorage.clear();
		G::PathStorage.clear();
		G::SphereStorage.clear();
		G::SweptStorage.clear();
	}
}

void CVisuals::Store(CTFPlayer* pLocal)
{
	if (!pLocal)
		return;

	m_vSightLines.clear();
	if (Vars::ESP::Other::SniperSightlines.Value & ~Vars::ESP::Other::SniperSightlinesEnum::DrawThroughWalls)
	{
		std::unordered_map<IClientEntity*, Vec3> mDots = {};
		for (auto pEntity : H::Entities.GetGroup(EGroupType::MISC_DOTS))
		{
			if (auto pOwner = pEntity->m_hOwnerEntity().Get())
				mDots[pOwner] = pEntity->m_vecOrigin();
		}

		bool bDrawThroughWalls = Vars::ESP::Other::SniperSightlines.Value & Vars::ESP::Other::SniperSightlinesEnum::DrawThroughWalls;
		for (auto pEntity : H::Entities.GetGroup(EGroupType::PLAYERS_ALL))
		{
			auto pPlayer = pEntity->As<CTFPlayer>();
			if (pPlayer == pLocal || !(pPlayer->m_iTeamNum() != pLocal->m_iTeamNum() ? Vars::ESP::Other::SniperSightlines.Value & Vars::ESP::Other::SniperSightlinesEnum::Enemy : Vars::ESP::Other::SniperSightlines.Value & Vars::ESP::Other::SniperSightlinesEnum::Team))
				continue;

			auto pWeapon = pPlayer->m_hActiveWeapon().Get()->As<CTFWeaponBase>();
			if (pPlayer->IsDormant() || !pPlayer->IsAlive() || pPlayer->IsAGhost() || !pPlayer->InCond(TF_COND_AIMING) ||
				!pWeapon || pWeapon->GetWeaponID() == TF_WEAPON_COMPOUND_BOW || pWeapon->GetWeaponID() == TF_WEAPON_MINIGUN)
				continue;

			Vec3 vShootPos = pPlayer->m_vecOrigin() + pPlayer->GetViewOffset();
			Vec3 vForward; Math::AngleVectors(pPlayer->GetEyeAngles(), &vForward);
			Vec3 vShootEnd = mDots.contains(pPlayer) ? mDots[pPlayer] : vShootPos + (vForward * 8192.f);

			CGameTrace trace = {};
			CTraceFilterHitscan filter = {}; filter.pSkip = pPlayer;
			SDK::Trace(vShootPos, vShootEnd, MASK_SHOT, &filter, &trace);

			m_vSightLines.emplace_back(vShootPos, trace.endpos, H::Color.GetEntityDrawColor(pLocal, pPlayer, Vars::Colors::Relative.Value), !bDrawThroughWalls);
		}
	}
}



void CVisuals::OverrideWorldTextures()
{
	auto uHash = FNV1A::Hash32(Vars::Visuals::World::WorldTexture.Value.c_str());
	if (uHash == FNV1A::Hash32Const("Default"))
		return;

	KeyValues* kv = new KeyValues("LightmappedGeneric");
	if (!kv)
		return;

	switch (uHash)
	{
	case FNV1A::Hash32Const("Dev"):
		kv->SetString("$basetexture", "dev/dev_measuregeneric01b");
		break;
	case FNV1A::Hash32Const("Camo"):
		kv->SetString("$basetexture", "patterns/paint_strokes");
		break;
	case FNV1A::Hash32Const("Black"):
		kv->SetString("$basetexture", "patterns/combat/black");
		break;
	case FNV1A::Hash32Const("White"):
		kv->SetString("$basetexture", "patterns/combat/white");
		break;
	case FNV1A::Hash32Const("Flat"):
		kv->SetString("$basetexture", "vgui/white_additive");
		kv->SetString("$color2", "[0.12 0.12 0.15]");
		break;
	default:
		kv->SetString("$basetexture", Vars::Visuals::World::WorldTexture.Value.c_str());
	}

	for (auto h = I::MaterialSystem->FirstMaterial(); h != I::MaterialSystem->InvalidMaterial(); h = I::MaterialSystem->NextMaterial(h))
	{
		auto pMaterial = I::MaterialSystem->GetMaterial(h);
		if (!pMaterial || pMaterial->IsErrorMaterial() || !pMaterial->IsPrecached() || pMaterial->IsTranslucent() || pMaterial->IsSpriteCard())
			continue;

		std::string_view sGroup = pMaterial->GetTextureGroupName();
		std::string_view sName = pMaterial->GetName();

		if (!sGroup._Starts_with("World")
			|| sName.find("water") != std::string_view::npos || sName.find("glass") != std::string_view::npos
			|| sName.find("door") != std::string_view::npos || sName.find("tools") != std::string_view::npos
			|| sName.find("player") != std::string_view::npos || sName.find("chicken") != std::string_view::npos
			|| sName.find("wall28") != std::string_view::npos || sName.find("wall26") != std::string_view::npos
			|| sName.find("decal") != std::string_view::npos || sName.find("overlay") != std::string_view::npos
			|| sName.find("hay") != std::string_view::npos)
		{
			continue;
		}

		pMaterial->SetShaderAndParams(kv);
	}
}

static inline void ApplyModulation(const Color_t& tColor, bool bSky = false)
{
	for (auto h = I::MaterialSystem->FirstMaterial(); h != I::MaterialSystem->InvalidMaterial(); h = I::MaterialSystem->NextMaterial(h))
	{
		auto pMaterial = I::MaterialSystem->GetMaterial(h);
		if (!pMaterial || pMaterial->IsErrorMaterial() || !pMaterial->IsPrecached())
			continue;

		auto sGroup = std::string_view(pMaterial->GetTextureGroupName());
		if (!bSky ? !sGroup._Starts_with("World") : !sGroup._Starts_with("SkyBox"))
			continue;

		pMaterial->ColorModulate(tColor.r / 255.f, tColor.g / 255.f, tColor.b / 255.f);
	}
}

void CVisuals::Modulate()
{
	const bool bScreenshot = Vars::Visuals::UI::CleanScreenshots.Value && I::EngineClient->IsTakingScreenshot();
	const bool bWorldModulation = Vars::Visuals::World::Modulations.Value & Vars::Visuals::World::ModulationsEnum::World && !bScreenshot;
	const bool bSkyModulation = Vars::Visuals::World::Modulations.Value & Vars::Visuals::World::ModulationsEnum::Sky && !bScreenshot;

	bool bSetChanged, bColorChanged, bSkyChanged, bConnection;
	{
		static bool bStaticWorld = false, bStaticSky = false;
		const bool bOldWorld = bStaticWorld, bOldSky = bStaticSky;
		const bool bNewWorld = bStaticWorld = bWorldModulation, bNewSky = bStaticSky = bSkyModulation;
		bSetChanged = bNewWorld != bOldWorld || bNewSky != bOldSky;
	}
	{
		static Color_t tStaticWorld = {}, tStaticSky = {};
		const Color_t tOldWorld = tStaticWorld, tOldSky = tStaticSky;
		const Color_t tNewWorld = tStaticWorld = Vars::Colors::WorldModulation.Value, tNewSky = tStaticSky = Vars::Colors::SkyModulation.Value;
		bColorChanged = tNewWorld != tOldWorld || tNewSky != tOldSky;
	}
	{
		static uint32_t uStatic = 0;
		const uint32_t uOld = uStatic;
		const uint32_t uNew = uStatic = FNV1A::Hash32(Vars::Visuals::World::SkyboxChanger.Value.c_str());
		bSkyChanged = uNew != uOld;
	}
	{
		static bool bStaticConnected = false;
		const bool bOldConnected = bStaticConnected;
		const bool bNewConnected = bStaticConnected = I::EngineClient->IsConnected() && I::EngineClient->IsInGame();
		bConnection = bNewConnected == bOldConnected;
	}

	if (bSetChanged || bColorChanged || bSkyChanged || !bConnection)
	{
		bWorldModulation ? ApplyModulation(Vars::Colors::WorldModulation.Value) : ApplyModulation({ 255, 255, 255, 255 });
		bSkyModulation ? ApplyModulation(Vars::Colors::SkyModulation.Value, true) : ApplyModulation({ 255, 255, 255, 255 }, true);
	}
}

void CVisuals::RestoreWorldModulation()
{
	ApplyModulation({ 255, 255, 255, 255 });
	ApplyModulation({ 255, 255, 255, 255 }, true);
}

void CVisuals::CreateMove(CTFPlayer* pLocal, CTFWeaponBase* pWeapon)
{
	if (Vars::Visuals::Simulation::ShotPath.Value && G::Attacking == 1 && !F::Aimbot.m_bRan)
	{
		switch (pWeapon->GetWeaponID())
		{
		case TF_WEAPON_BAT_WOOD:
		case TF_WEAPON_BAT_GIFTWRAP:
			if (!G::Throwing)
				break;
			[[fallthrough]];
		default:
			F::Visuals.ProjectileTrace(pLocal, pWeapon, false);
		}
	}

	if (pLocal && Vars::Visuals::Effects::SpellFootsteps.Value && (F::Ticks.m_bDoubletap || F::Ticks.m_bWarp))
		S::CTFPlayer_FireEvent.Call<void>(pLocal, pLocal->GetAbsOrigin(), QAngle(), 7001, nullptr);
	
	static uint32_t iOldMedigunBeam = 0, iOldMedigunCharge = 0;
	uint32_t iNewMedigunBeam = FNV1A::Hash32(Vars::Visuals::Effects::MedigunBeam.Value.c_str()), iNewMedigunCharge = FNV1A::Hash32(Vars::Visuals::Effects::MedigunCharge.Value.c_str());
	if (iOldMedigunBeam != iNewMedigunBeam || iOldMedigunCharge != iNewMedigunCharge)
	{
		if (pWeapon && pWeapon->GetWeaponID() == TF_WEAPON_MEDIGUN)
		{
			S::CWeaponMedigun_UpdateEffects.Call<void>();
			S::CWeaponMedigun_StopChargeEffect.Call<void>(pWeapon);
			S::CWeaponMedigun_StopChargeEffect.Call<void>(pWeapon, false);
		}

		iOldMedigunBeam = iNewMedigunBeam;
		iOldMedigunCharge = iNewMedigunCharge;
	}

	static float flStaticRatio = 0.f;
	float flOldRatio = flStaticRatio;
	float flNewRatio = flStaticRatio = Vars::Visuals::UI::AspectRatio.Value;

	static auto r_aspectratio = U::ConVars.FindVar("r_aspectratio");
	if (flNewRatio != flOldRatio)
		r_aspectratio->SetValue(flNewRatio);
}