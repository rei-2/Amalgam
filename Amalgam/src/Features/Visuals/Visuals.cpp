#include "Visuals.h"

#include "../Visuals/PlayerConditions/PlayerConditions.h"
#include "../Backtrack/Backtrack.h"
#include "../PacketManip/AntiAim/AntiAim.h"
#include "../Simulation/ProjectileSimulation/ProjectileSimulation.h"
#include "../CameraWindow/CameraWindow.h"
#include "../NoSpread/NoSpreadHitscan/NoSpreadHitscan.h"
#include "../Players/PlayerUtils.h"
#include "Materials/Materials.h"

MAKE_SIGNATURE(RenderLine, "engine.dll", "48 89 5C 24 ? 48 89 74 24 ? 44 89 44 24", 0x0);
MAKE_SIGNATURE(RenderBox, "engine.dll", "48 83 EC ? 8B 84 24 ? ? ? ? 4D 8B D8", 0x0);
MAKE_SIGNATURE(RenderWireframeBox, "engine.dll", "48 89 5C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 55 41 54 41 55 41 56 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 49 8B F9", 0x0);
MAKE_SIGNATURE(DrawServerHitboxes, "server.dll", "44 88 44 24 ? 53 48 81 EC", 0x0);
MAKE_SIGNATURE(GetServerAnimating, "server.dll", "48 83 EC ? 8B D1 85 C9 7E ? 48 8B 05", 0x0);
MAKE_SIGNATURE(CTFPlayer_FireEvent, "client.dll", "48 8B C4 48 89 58 ? 48 89 70 ? 48 89 78 ? 55 41 54 41 57", 0x0);
MAKE_SIGNATURE(CWeaponMedigun_UpdateEffects, "client.dll", "40 57 48 81 EC ? ? ? ? 8B 91 ? ? ? ? 48 8B F9 85 D2 0F 84 ? ? ? ? 48 89 B4 24", 0x0);
MAKE_SIGNATURE(CWeaponMedigun_StopChargeEffect, "client.dll", "40 53 48 83 EC ? 44 0F B6 C2", 0x0);
MAKE_SIGNATURE(CWeaponMedigun_ManageChargeEffect, "client.dll", "48 89 5C 24 ? 48 89 74 24 ? 57 48 81 EC ? ? ? ? 48 8B F1 E8 ? ? ? ? 48 8B D8", 0x0);

void CVisuals::DrawAimbotFOV(CTFPlayer* pLocal)
{
	if (!Vars::Aimbot::General::FOVCircle.Value || !Vars::Colors::FOVCircle.Value.a || !pLocal->IsAlive() || pLocal->IsAGhost() || pLocal->IsTaunting() || pLocal->IsStunned() || pLocal->IsInBumperKart())
		return;

	const float flR = tanf(DEG2RAD(Vars::Aimbot::General::AimFOV.Value) / 2.0f) / tanf(DEG2RAD(pLocal->m_iFOV()) / 2.0f) * H::Draw.m_nScreenW;
	const Color_t clr = Vars::Colors::FOVCircle.Value;
	H::Draw.LineCircle(H::Draw.m_nScreenW / 2, H::Draw.m_nScreenH / 2, flR, 68, clr);
}

void CVisuals::DrawTickbaseText(CTFPlayer* pLocal)
{
	if (!(Vars::Menu::Indicators.Value & (1 << 0)))
		return;

	if (!pLocal->IsAlive())
		return;

	const int iTicks = std::clamp(G::ShiftedTicks + G::ChokeAmount, 0, G::MaxShift);

	const DragBox_t dtPos = Vars::Menu::TicksDisplay.Value;
	const auto& fFont = H::Fonts.GetFont(FONT_INDICATORS);

	const int offset = 7 + 12 * Vars::Menu::DPI.Value;
	H::Draw.String(fFont, dtPos.x, dtPos.y + 2, Vars::Menu::Theme::Active.Value, ALIGN_TOP, "Ticks %d / %d", iTicks, G::MaxShift);

	const float ratioCurrent = (float)iTicks / (float)G::MaxShift;
	float sizeX = 100 * Vars::Menu::DPI.Value, sizeY = 12 * Vars::Menu::DPI.Value, posX = dtPos.x - sizeX / 2, posY = dtPos.y + 5 + fFont.m_nTall;
	H::Draw.LineRect(posX, dtPos.y + 5 + fFont.m_nTall, sizeX, sizeY, Vars::Menu::Theme::Accent.Value);
	if (iTicks)
	{
		sizeX -= 4, sizeY -= 4, posX = dtPos.x - sizeX / 2;
		H::Draw.StartClipping(posX, posY, sizeX * ratioCurrent, sizeY + 2 /*?*/);
		H::Draw.FillRect(posX, posY + 2, sizeX, sizeY, Vars::Menu::Theme::Accent.Value);
		H::Draw.EndClipping();
	}
}

void CVisuals::DrawOnScreenPing(CTFPlayer* pLocal)
{
	if (!(Vars::Menu::Indicators.Value & (1 << 3)) || !pLocal || !pLocal->IsAlive())
		return;

	auto pResource = H::Entities.GetPR();
	auto pNetChan = I::EngineClient->GetNetChannelInfo();
	if (!pResource || !pNetChan)
		return;

	const float flFake = std::min(F::Backtrack.flFakeLatency + (F::Backtrack.flFakeInterp > G::Lerp ? F::Backtrack.flFakeInterp : 0.f), F::Backtrack.flMaxUnlag) * 1000.f;
	const float flLatency = F::Backtrack.GetReal() * 1000.f;
	const int iLatencyScoreboard = pResource->GetPing(pLocal->entindex());

	int x = Vars::Menu::PingDisplay.Value.x;
	int y = Vars::Menu::PingDisplay.Value.y + 8;
	const auto& fFont = H::Fonts.GetFont(FONT_INDICATORS);

	EAlign align = ALIGN_TOP;
	if (x <= (100 + 50 * Vars::Menu::DPI.Value))
	{
		x -= 42 * Vars::Menu::DPI.Value;
		align = ALIGN_TOPLEFT;
	}
	else if (x >= H::Draw.m_nScreenW - (100 + 50 * Vars::Menu::DPI.Value))
	{
		x += 42 * Vars::Menu::DPI.Value;
		align = ALIGN_TOPRIGHT;
	}

	if (flFake || Vars::Backtrack::Interp.Value && Vars::Backtrack::Enabled.Value)
	{
		if (flLatency > 0.f)
			H::Draw.String(fFont, x, y, Vars::Menu::Theme::Active.Value, align, "Real %.0f (+ %.0f) ms", flLatency, flFake);
		else
			H::Draw.String(fFont, x, y, Vars::Menu::Theme::Active.Value, align, "Syncing");
	}
	else
		H::Draw.String(fFont, x, y, Vars::Menu::Theme::Active.Value, align, "Real %.0f ms", flLatency);
	H::Draw.String(fFont, x, y += fFont.m_nTall + 1, Vars::Menu::Theme::Active.Value, align, "Scoreboard %d ms", iLatencyScoreboard);
}

void CVisuals::DrawOnScreenConditions(CTFPlayer* pLocal)
{
	if (!(Vars::Menu::Indicators.Value & (1 << 4)) || !pLocal->IsAlive())
		return;

	int x = Vars::Menu::ConditionsDisplay.Value.x;
	int y = Vars::Menu::ConditionsDisplay.Value.y + 8;
	const auto& fFont = H::Fonts.GetFont(FONT_INDICATORS);

	EAlign align = ALIGN_TOP;
	if (x <= (100 + 50 * Vars::Menu::DPI.Value))
	{
		x -= 42 * Vars::Menu::DPI.Value;
		align = ALIGN_TOPLEFT;
	}
	else if (x >= H::Draw.m_nScreenW - (100 + 50 * Vars::Menu::DPI.Value))
	{
		x += 42 * Vars::Menu::DPI.Value;
		align = ALIGN_TOPRIGHT;
	}
	
	std::vector<std::wstring> conditionsVec = F::PlayerConditions.GetPlayerConditions(pLocal);

	int offset = 0;
	for (const std::wstring& cond : conditionsVec)
	{
		H::Draw.String(fFont, x, y + offset, Vars::Menu::Theme::Active.Value, align, cond.data());
		offset += fFont.m_nTall + 1;
	}
}

void CVisuals::DrawSeedPrediction(CTFPlayer* pLocal)
{
	if (!(Vars::Menu::Indicators.Value & (1 << 5)) || !pLocal || !pLocal->IsAlive() || !Vars::Aimbot::General::NoSpread.Value)
		return;

	{
		auto pWeapon = H::Entities.GetWeapon();
		if (!pWeapon || !F::NoSpreadHitscan.ShouldRun(pLocal, pWeapon))
			return;
	}

	int x = Vars::Menu::SeedPredictionDisplay.Value.x;
	int y = Vars::Menu::SeedPredictionDisplay.Value.y + 8;
	const auto& fFont = H::Fonts.GetFont(FONT_INDICATORS);

	EAlign align = ALIGN_TOP;
	if (x <= (100 + 50 * Vars::Menu::DPI.Value))
	{
		x -= 42 * Vars::Menu::DPI.Value;
		align = ALIGN_TOPLEFT;
	}
	else if (x >= H::Draw.m_nScreenW - (100 + 50 * Vars::Menu::DPI.Value))
	{
		x += 42 * Vars::Menu::DPI.Value;
		align = ALIGN_TOPRIGHT;
	}

	const auto& cColor = F::NoSpreadHitscan.bSynced ? Vars::Menu::Theme::Active.Value : Vars::Menu::Theme::Inactive.Value;

	H::Draw.String(fFont, x, y, cColor, align, std::format("Uptime {}", F::NoSpreadHitscan.GetFormat(F::NoSpreadHitscan.flServerTime)).c_str());
	H::Draw.String(fFont, x, y += fFont.m_nTall + 1, cColor, align, std::format("Mantissa step {}", F::NoSpreadHitscan.flMantissaStep).c_str());
}

std::deque<Vec3> SplashTrace(Vec3 vOrigin, float flRadius, Vec3 vNormal = { 0, 0, 1 }, bool bTrace = true, int iSegments = 32)
{
	if (!flRadius)
		return {};

	Vec3 vAngles; Math::VectorAngles(vNormal, vAngles);
	Vec3 vRight, vUp; Math::AngleVectors(vAngles, nullptr, &vRight, &vUp);

	std::deque<Vec3> vPoints = {};
	for (float i = 0.f; i < iSegments; i++)
	{
		Vec3 vPoint = vOrigin + (vRight * cos(2 * PI * i / iSegments) + vUp * sin(2 * PI * i / iSegments)) * flRadius;
		if (bTrace)
		{
			CGameTrace trace = {};
			CTraceFilterWorldAndPropsOnly filter = {};
			SDK::Trace(vOrigin, vPoint, MASK_SOLID, &filter, &trace);
			vPoint = trace.endpos;
		}
		vPoints.push_back(vPoint);
	}
	vPoints.push_back(vPoints.front());

	return vPoints;
}

void CVisuals::ProjectileTrace(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, const bool bQuick)
{
	if (bQuick)
		F::CameraWindow.m_bShouldDraw = false;
	if ((bQuick ? !Vars::Visuals::Simulation::ProjectileTrajectory.Value && !Vars::Visuals::Simulation::ProjectileCamera.Value : !Vars::Visuals::Simulation::TrajectoryOnShot.Value)
		|| !pLocal || !pWeapon || pWeapon->GetWeaponID() == TF_WEAPON_FLAMETHROWER)
		return;

	ProjectileInfo projInfo = {};
	if (!F::ProjSim.GetInfo(pLocal, pWeapon, bQuick ? I::EngineClient->GetViewAngles() : G::CurrentUserCmd->viewangles, projInfo, true, bQuick, (bQuick && Vars::Aimbot::Projectile::AutoRelease.Value) ? Vars::Aimbot::Projectile::AutoRelease.Value / 100 : -1.f)
		|| !F::ProjSim.Initialize(projInfo))
		return;

	CGameTrace trace = {};
	CTraceFilterProjectile filter = {}; filter.pSkip = pLocal;
	Vec3* pNormal = nullptr;

	for (int n = 1; n <= TIME_TO_TICKS(projInfo.m_flLifetime); n++)
	{
		Vec3 Old = F::ProjSim.GetOrigin();
		F::ProjSim.RunTick(projInfo);
		Vec3 New = F::ProjSim.GetOrigin();

		SDK::TraceHull(Old, New, projInfo.m_vHull * -1, projInfo.m_vHull, MASK_SOLID, &filter, &trace);
		if (trace.DidHit())
		{
			pNormal = &trace.plane.normal;
			break;
		}
	}

	if (projInfo.PredictionLines.empty())
		return;

	projInfo.PredictionLines.push_back(trace.endpos);

	std::deque<Vec3> vPoints = {};
	if ((bQuick ? Vars::Visuals::Simulation::ProjectileTrajectory.Value : Vars::Visuals::Simulation::TrajectoryOnShot.Value) && Vars::Visuals::Simulation::SplashRadius.Value & (1 << 0))
	{
		float flRadius = 0.f;
		switch (pWeapon->GetWeaponID())
		{
		case TF_WEAPON_ROCKETLAUNCHER:
		case TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT:
		case TF_WEAPON_PARTICLE_CANNON:
			if (Vars::Visuals::Simulation::SplashRadius.Value & (1 << 6))
				flRadius = 146.f;
			break;
		case TF_WEAPON_PIPEBOMBLAUNCHER:
			if (Vars::Visuals::Simulation::SplashRadius.Value & (1 << 7))
				flRadius = 146.f;
			break;
		case TF_WEAPON_GRENADELAUNCHER:
			if (Vars::Visuals::Simulation::SplashRadius.Value & (1 << 8))
				flRadius = 146.f;
		}
		if (!flRadius && pWeapon->m_iItemDefinitionIndex() == Pyro_s_TheScorchShot && Vars::Visuals::Simulation::SplashRadius.Value & (1 << 9))
			flRadius = 110.f;

		if (flRadius)
		{
			flRadius = SDK::AttribHookValue(flRadius, "mult_explosion_radius", pWeapon);
			switch (pWeapon->GetWeaponID())
			{
			case TF_WEAPON_ROCKETLAUNCHER:
			case TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT:
			case TF_WEAPON_PARTICLE_CANNON:
				if (pLocal->InCond(TF_COND_BLASTJUMPING) && SDK::AttribHookValue(1.f, "rocketjump_attackrate_bonus", pWeapon) != 1.f)
					flRadius *= 0.8f;
			}
			vPoints = SplashTrace(trace.endpos, flRadius, pNormal ? *pNormal : Vec3(0, 0, 1), Vars::Visuals::Simulation::SplashRadius.Value & (1 << 10));
		}
	}

	if (bQuick)
	{
		if (Vars::Visuals::Simulation::ProjectileCamera.Value && !I::EngineVGui->IsGameUIVisible() && pLocal->m_vecOrigin().DistTo(trace.endpos) > 500.f)
		{
			CGameTrace cameraTrace = {};

			auto vAngles = Math::CalcAngle(trace.startpos, trace.endpos);
			Vec3 vForward; Math::AngleVectors(vAngles, &vForward);
			SDK::Trace(trace.endpos, trace.endpos - vForward * 500.f, MASK_SOLID, &filter, &cameraTrace);

			F::CameraWindow.m_bShouldDraw = true;
			F::CameraWindow.m_vCameraOrigin = cameraTrace.endpos;
			F::CameraWindow.m_vCameraAngles = vAngles;
		}

		if (Vars::Visuals::Simulation::ProjectileTrajectory.Value)
		{
			DrawSimLine(projInfo.PredictionLines, Vars::Colors::ProjectileColor.Value);
			if (Vars::Colors::ClippedColor.Value.a)
				DrawSimLine(projInfo.PredictionLines, Vars::Colors::ClippedColor.Value, 0, true);

			if (pNormal)
			{
				const float flSize = std::max(projInfo.m_vHull.x, 1.f);
				const Vec3 vSize = { 1.f, flSize, flSize };
				Vec3 vAngles; Math::VectorAngles(*pNormal, vAngles);

				RenderBox(trace.endpos, vSize * -1, vSize, vAngles, Vars::Colors::ProjectileColor.Value, { 0, 0, 0, 0 });
				if (Vars::Colors::ClippedColor.Value.a)
					RenderBox(trace.endpos, vSize * -1, vSize, vAngles, Vars::Colors::ClippedColor.Value, { 0, 0, 0, 0 }, true);
			}

			if (!vPoints.empty())
			{
				F::Visuals.DrawSimLine(vPoints, Vars::Colors::ProjectileColor.Value);
				if (Vars::Colors::ClippedColor.Value.a)
					F::Visuals.DrawSimLine(vPoints, Vars::Colors::ClippedColor.Value, 0, true);
			}
		}
	}
	else if (Vars::Visuals::Simulation::TrajectoryOnShot.Value)
	{
		G::LinesStorage.clear();
		G::LinesStorage.push_back({ projInfo.PredictionLines, -float(projInfo.PredictionLines.size()) - TIME_TO_TICKS(F::Backtrack.GetReal()), Vars::Colors::ProjectileColor.Value });

		if (pNormal)
		{
			const float flSize = std::max(projInfo.m_vHull.x, 1.f);
			const Vec3 vSize = { 1.f, flSize, flSize };
			Vec3 vAngles; Math::VectorAngles(*pNormal, vAngles);

			G::BoxesStorage.clear();
			G::BoxesStorage.push_back({ trace.endpos, vSize * -1, vSize, vAngles, I::GlobalVars->curtime + TICKS_TO_TIME(projInfo.PredictionLines.size()) + F::Backtrack.GetReal(), Vars::Colors::ProjectileColor.Value, { 0, 0, 0, 0 } });
		}

		if (!vPoints.empty())
			G::LinesStorage.push_back({ vPoints, I::GlobalVars->curtime + TICKS_TO_TIME(projInfo.PredictionLines.size()) + F::Backtrack.GetReal(), Vars::Colors::ProjectileColor.Value });
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
		case ETFClassID::CBaseGrenade:
		case ETFClassID::CTFWeaponBaseGrenadeProj:
		case ETFClassID::CTFWeaponBaseMerasmusGrenade:
		case ETFClassID::CTFGrenadePipebombProjectile:
			bShouldDraw = Vars::Visuals::Simulation::SplashRadius.Value & (pEntity->As<CTFGrenadePipebombProjectile>()->HasStickyEffects() ? (1 << 7) : (1 << 8));
			break;
		case ETFClassID::CTFBaseRocket:
		case ETFClassID::CTFProjectile_Rocket:
		case ETFClassID::CTFProjectile_SentryRocket:
		case ETFClassID::CTFProjectile_EnergyBall:
			bShouldDraw = Vars::Visuals::Simulation::SplashRadius.Value & (1 << 6);
			break;
		case ETFClassID::CTFProjectile_Flare:
			if (Vars::Visuals::Simulation::SplashRadius.Value & (1 << 9))
			{
				pWeapon = pEntity->As<CTFProjectile_Flare>()->m_hLauncher().Get()->As<CTFWeaponBase>();
				bShouldDraw = pWeapon && pWeapon->m_iItemDefinitionIndex() == ETFWeapons::Pyro_s_TheScorchShot;
			}
		}
		if (!bShouldDraw)
			continue;

		switch (pEntity->GetClassID())
		{
		case ETFClassID::CBaseGrenade:
		case ETFClassID::CTFWeaponBaseGrenadeProj:
		case ETFClassID::CTFWeaponBaseMerasmusGrenade:
		case ETFClassID::CTFGrenadePipebombProjectile:
			pOwner = pEntity->As<CTFGrenadePipebombProjectile>()->m_hThrower().Get()->As<CTFPlayer>();
			break;
		case ETFClassID::CTFBaseRocket:
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
			if (!(Vars::Visuals::Simulation::SplashRadius.Value & (1 << 5) && H::Entities.IsFriend(pOwner->entindex()))
				&& !(Vars::Visuals::Simulation::SplashRadius.Value & (1 << 1) && F::PlayerUtils.GetPriority(pOwner->entindex()) > F::PlayerUtils.m_vTags[DEFAULT_TAG].Priority)
				&& pOwner->m_iTeamNum() == pLocal->m_iTeamNum() ? !(Vars::Visuals::Simulation::SplashRadius.Value & (1 << 3)) : !(Vars::Visuals::Simulation::SplashRadius.Value & (1 << 2)))
				continue;
		}
		else if (!(Vars::Visuals::Simulation::SplashRadius.Value & (1 << 4)))
			continue;

		float flRadius = 146.f;
		switch (pEntity->GetClassID())
		{
		case ETFClassID::CBaseGrenade:
		case ETFClassID::CTFWeaponBaseGrenadeProj:
		case ETFClassID::CTFWeaponBaseMerasmusGrenade:
		case ETFClassID::CTFGrenadePipebombProjectile:
			pWeapon = pEntity->As<CTFGrenadePipebombProjectile>()->m_hOriginalLauncher().Get()->As<CTFWeaponBase>();
			break;
		case ETFClassID::CTFBaseRocket:
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

		auto vPoints = SplashTrace(pEntity->GetAbsOrigin(), flRadius, { 0, 0, 1 }, Vars::Visuals::Simulation::SplashRadius.Value & (1 << 10));
		F::Visuals.DrawSimLine(vPoints, Vars::Colors::ProjectileColor.Value);
		if (Vars::Colors::ClippedColor.Value.a)
			F::Visuals.DrawSimLine(vPoints, Vars::Colors::ClippedColor.Value, 0, true);
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
			constexpr auto distance = 50.f;
			if (SDK::W2S(Math::GetRotatedPosition(vOrigin, F::AntiAim.vRealAngles.y, distance), vScreen2))
				H::Draw.Line(vScreen1.x, vScreen1.y, vScreen2.x, vScreen2.y, { 0, 255, 0, 255 });

			if (SDK::W2S(Math::GetRotatedPosition(vOrigin, F::AntiAim.vFakeAngles.y, distance), vScreen2))
				H::Draw.Line(vScreen1.x, vScreen1.y, vScreen2.x, vScreen2.y, { 255, 0, 0, 255 });
		}

		for (auto& vPair : F::AntiAim.vEdgeTrace)
		{
			if (SDK::W2S(vPair.first, vScreen1) && SDK::W2S(vPair.second, vScreen2))
				H::Draw.Line(vScreen1.x, vScreen1.y, vScreen2.x, vScreen2.y, { 255, 255, 255, 255 });
		}
	}
}

void CVisuals::DrawDebugInfo(CTFPlayer* pLocal)
{
	// Debug info
	if (Vars::Debug::Info.Value)
	{
		int x = 10, y = 10;
		const auto& fFont = H::Fonts.GetFont(FONT_INDICATORS);
		y -= fFont.m_nTall + 1;

		if (auto pCmd = G::LastUserCmd)
		{
			H::Draw.String(fFont, x, y += fFont.m_nTall + 1, {}, ALIGN_TOPLEFT, "View: (%.3f, %.3f, %.3f)", pCmd->viewangles.x, pCmd->viewangles.y, pCmd->viewangles.z);
			H::Draw.String(fFont, x, y += fFont.m_nTall + 1, {}, ALIGN_TOPLEFT, "Move: (%.0f, %.0f, %.0f)", pCmd->forwardmove, pCmd->sidemove, pCmd->upmove);
			H::Draw.String(fFont, x, y += fFont.m_nTall + 1, {}, ALIGN_TOPLEFT, "Buttons: %i", pCmd->buttons);
		}
		H::Draw.String(fFont, x, y += fFont.m_nTall + 1, {}, ALIGN_TOPLEFT, std::format("Choke: {}, {}", G::Choking, I::ClientState->chokedcommands).c_str());
		H::Draw.String(fFont, x, y += fFont.m_nTall + 1, {}, ALIGN_TOPLEFT, std::format("Ticks: {}, {}", G::ShiftedTicks, G::ShiftedGoal).c_str());
		H::Draw.String(fFont, x, y += fFont.m_nTall + 1, {}, ALIGN_TOPLEFT, std::format("Attacking: {} ({}, {})", G::IsAttacking, G::CanPrimaryAttack, G::CanSecondaryAttack).c_str());
		{
			Vec3 vOrigin = pLocal->m_vecOrigin();
			H::Draw.String(fFont, x, y += fFont.m_nTall + 1, {}, ALIGN_TOPLEFT, "Origin: (%.3f, %.3f, %.3f)", vOrigin.x, vOrigin.y, vOrigin.z);
		}
		{
			Vec3 vVelocity = pLocal->m_vecVelocity();
			H::Draw.String(fFont, x, y += fFont.m_nTall + 1, {}, ALIGN_TOPLEFT, "Velocity: %.3f (%.3f, %.3f, %.3f)", vVelocity.Length(), vVelocity.x, vVelocity.y, vVelocity.z);
		}
		H::Draw.String(fFont, x, y += fFont.m_nTall + 1, {}, ALIGN_TOPLEFT, "Round state: %i", SDK::GetRoundState());
		H::Draw.String(fFont, x, y += fFont.m_nTall + 1, {}, ALIGN_TOPLEFT, "Tickcount: %i", pLocal->m_nTickBase());
	}
}



std::vector<DrawBox> CVisuals::GetHitboxes(matrix3x4 aBones[MAXSTUDIOBONES], CBaseAnimating* pEntity, const int iHitbox)
{
	if (!Vars::Colors::BoneHitboxEdge.Value.a && !Vars::Colors::BoneHitboxFace.Value.a)
		return {};

	std::vector<DrawBox> vBoxes = {};

	auto pModel = pEntity->GetModel();
	if (!pModel) return vBoxes;
	auto pHDR = I::ModelInfoClient->GetStudiomodel(pModel);
	if (!pHDR) return vBoxes;
	auto pSet = pHDR->pHitboxSet(pEntity->m_nHitboxSet());
	if (!pSet) return vBoxes;

	for (int i = iHitbox != -1 ? iHitbox : 0; i < pSet->numhitboxes; ++i)
	{
		if (iHitbox != -1 && iHitbox != i)
			break;

		auto pBox = pSet->pHitbox(i);
		if (!pBox) continue;

		matrix3x4 rotation; Math::AngleMatrix(pBox->angle, rotation);
		matrix3x4 matrix; Math::ConcatTransforms(aBones[pBox->bone], rotation, matrix);
		Vec3 vAngle; Math::MatrixAngles(matrix, vAngle);
		Vec3 vOrigin; Math::GetMatrixOrigin(matrix, vOrigin);

		vBoxes.push_back({ vOrigin, pBox->bbmin, pBox->bbmax, vAngle, I::GlobalVars->curtime + 5.f, Vars::Colors::BoneHitboxEdge.Value, Vars::Colors::BoneHitboxFace.Value, true });
	}

	return vBoxes;
}

void CVisuals::DrawBulletLines()
{
	for (auto& Line : G::BulletsStorage)
	{
		if (Line.m_flTime < I::GlobalVars->curtime) continue;

		RenderLine(Line.m_line.first, Line.m_line.second, Line.m_color, Line.m_bZBuffer);
	}
}

void CVisuals::DrawSimLine(std::deque<Vec3>& Line, Color_t Color, int iStyle, bool bZBuffer, float flTime)
{
	for (size_t i = 1; i < Line.size(); i++)
	{
		if (flTime < 0.f && Line.size() - i > -flTime)
			continue;

		switch (iStyle)
		{
		case 0:
		{
			RenderLine(Line[i - 1], Line[i], Color, bZBuffer);
			break;
		}
		case 1:
		{
			RenderLine(Line[i - 1], Line[i], Color, bZBuffer);
			if (!(i % 4))
			{
				Vec3& vStart = Line[i - 1];
				Vec3& vEnd = Line[i];

				Vec3 vSeparator = vEnd - vStart;
				vSeparator.z = 0;
				vSeparator.Normalize();
				vSeparator = Math::RotatePoint(vSeparator * 12, {}, { 0, 90, 0 });
				RenderLine(vEnd, vEnd + vSeparator, Color, bZBuffer);
			}
			break;
		}
		case 2:
		{
			if (!(i % 2))
				RenderLine(Line[i - 1], Line[i], Color, bZBuffer);
			break;
		}
		}
	}
}

void CVisuals::DrawSimLines()
{
	for (auto& Line : G::LinesStorage)
	{
		if (Line.m_flTime >= 0.f && Line.m_flTime < I::GlobalVars->curtime)
			continue;

		DrawSimLine(Line.m_line, Line.m_color, Vars::Visuals::Simulation::Style.Value, Line.m_bZBuffer, Line.m_flTime);
	}
}

void CVisuals::DrawBoxes()
{
	for (auto& Box : G::BoxesStorage)
	{
		if (Box.m_flTime < I::GlobalVars->curtime) continue;

		RenderBox(Box.m_vecPos, Box.m_vecMins, Box.m_vecMaxs, Box.m_vecOrientation, Box.m_colorEdge, Box.m_colorFace, Box.m_bZBuffer);
	}
}

void CVisuals::RevealBulletLines()
{
	for (auto& Line : G::BulletsStorage)
		Line.m_flTime = I::GlobalVars->curtime + 60.f;
}

void CVisuals::RevealSimLines()
{
	for (auto& PredictionLine : G::LinesStorage)
		PredictionLine.m_flTime = I::GlobalVars->curtime + 60.f;
}

void CVisuals::RevealBoxes()
{
	for (auto& Box : G::BoxesStorage)
		Box.m_flTime = I::GlobalVars->curtime + 60.f;
}

void CVisuals::DrawServerHitboxes(CTFPlayer* pLocal)
{
	static int iOldTick = I::GlobalVars->tickcount;
	if (iOldTick == I::GlobalVars->tickcount)
		return;
	iOldTick = I::GlobalVars->tickcount;

	if (I::Input->CAM_IsThirdPerson() && Vars::Debug::ServerHitbox.Value && pLocal->IsAlive())
	{
		using GetServerAnimating_t = void* (*)(int);
		static auto GetServerAnimating = S::GetServerAnimating.As<GetServerAnimating_t>();

		using DrawServerHitboxes_t = void(__fastcall*)(void*, float, bool); // CBaseAnimating, Duration, MonoColour
		static auto DrawServerHitboxes = S::DrawServerHitboxes.As<DrawServerHitboxes_t>();

		void* server_animating = GetServerAnimating(pLocal->entindex());
		if (server_animating)
			DrawServerHitboxes(server_animating, TICK_INTERVAL, true);
	}
}

void CVisuals::RenderLine(const Vec3& vStart, const Vec3& vEnd, Color_t cLine, bool bZBuffer)
{
	if (cLine.a)
	{
		static auto fnRenderLine = S::RenderLine.As<void(__cdecl*)(const Vector&, const Vector&, Color_t, bool)>();
		fnRenderLine(vStart, vEnd, cLine, bZBuffer);
	}
}

void CVisuals::RenderBox(const Vec3& vPos, const Vec3& vMins, const Vec3& vMaxs, const Vec3& vOrientation, Color_t cEdge, Color_t cFace, bool bZBuffer)
{
	if (cFace.a)
	{
		static auto fnRenderBox = S::RenderBox.As<void(__cdecl*)(const Vec3&, const Vec3&, const Vec3&, const Vec3&, Color_t, bool, bool)>();
		fnRenderBox(vPos, vOrientation, vMins, vMaxs, cFace, bZBuffer, false);
	}

	if (cEdge.a)
	{
		static auto fnRenderWireframeBox = S::RenderWireframeBox.As<void(__cdecl*)(const Vec3&, const Vec3&, const Vec3&, const Vec3&, Color_t, bool)>();
		fnRenderWireframeBox(vPos, vOrientation, vMins, vMaxs, cEdge, bZBuffer);
	}
}



void CVisuals::FOV(CTFPlayer* pLocal, CViewSetup* pView)
{
	pLocal->m_iFOV() = pView->fov;

	const int fov = pLocal->IsScoped() ? Vars::Visuals::UI::ZoomFieldOfView.Value : Vars::Visuals::UI::FieldOfView.Value;
	if (!fov)
		return;

	pView->fov = fov;
	pLocal->m_iFOV() = fov;
}

MAKE_SIGNATURE(Test, "client.dll", "40 53 48 83 EC ? 48 8B D9 E8 ? ? ? ? 84 C0 0F 84 ? ? ? ? 48 8B 05 ? ? ? ? 48 85 C0 74 ? 80 B8 ? ? ? ? ? 0F 85 ? ? ? ? F3 0F 10 0D", 0x0);

void CVisuals::ThirdPerson(CTFPlayer* pLocal, CViewSetup* pView)
{
	if (!pLocal->IsAlive())
		return I::Input->CAM_ToFirstPerson();

	const bool bNoZoom = (!Vars::Visuals::Removals::Scope.Value || Vars::Visuals::UI::ZoomFieldOfView.Value < 70) && pLocal->IsScoped();
	const bool bForce = pLocal->IsTaunting() || pLocal->IsAGhost() || pLocal->IsInBumperKart() || pLocal->InCond(TF_COND_HALLOWEEN_THRILLER);
	//if (bForce)
	//	return;

	if (Vars::Visuals::ThirdPerson::Enabled.Value && !bNoZoom || bForce)
		I::Input->CAM_ToThirdPerson();
	else
		I::Input->CAM_ToFirstPerson();
	pLocal->ThirdPersonSwitch();

	static auto cam_ideallag = U::ConVars.FindVar("cam_ideallag");
	if (cam_ideallag)
		cam_ideallag->SetValue(0.f);

	if (I::Input->CAM_IsThirdPerson())
	{	// Thirdperson offset
		Vec3 vForward, vRight, vUp; Math::AngleVectors(pView->angles, &vForward, &vRight, &vUp);

		Vec3 offset;
		offset += vRight * Vars::Visuals::ThirdPerson::Right.Value;
		offset += vUp * Vars::Visuals::ThirdPerson::Up.Value;
		offset -= vForward * Vars::Visuals::ThirdPerson::Distance.Value;

		const Vec3 viewDiff = pView->origin - pLocal->GetEyePosition();
		CGameTrace trace = {};
		CTraceFilterWorldAndPropsOnly filter = {};
		SDK::TraceHull(pView->origin - viewDiff, pView->origin + offset - viewDiff, { -9.f, -9.f, -9.f }, { 9.f, 9.f, 9.f }, MASK_SOLID, &filter, &trace);

		pView->origin += offset * trace.fraction - viewDiff;
	}
}

void CVisuals::DrawSightlines()
{
	if (Vars::Visuals::UI::SniperSightlines.Value)
	{
		for (auto& sightline : m_vSightLines)
			RenderLine(sightline.m_vStart, sightline.m_vEnd, sightline.m_Color);
	}
}

void CVisuals::Store()
{
	auto pLocal = H::Entities.GetLocal();
	if (Vars::Visuals::UI::SniperSightlines.Value && pLocal)
	{
		m_vSightLines.clear();

		std::unordered_map<IClientEntity*, Vec3> mDots = {};
		for (auto pEntity : H::Entities.GetGroup(EGroupType::MISC_DOTS))
		{
			if (auto pOwner = pEntity->m_hOwnerEntity().Get())
				mDots[pOwner] = pEntity->m_vecOrigin();
		}

		for (auto pEntity : H::Entities.GetGroup(EGroupType::PLAYERS_ENEMIES))
		{
			auto pPlayer = pEntity->As<CTFPlayer>();

			auto pWeapon = pPlayer->m_hActiveWeapon().Get()->As<CTFWeaponBase>();
			if (!pPlayer->IsAlive() || pPlayer->IsAGhost() || pPlayer->IsDormant() || !pPlayer->InCond(TF_COND_AIMING) ||
				!pWeapon || pWeapon->GetWeaponID() == TF_WEAPON_COMPOUND_BOW || pWeapon->GetWeaponID() == TF_WEAPON_MINIGUN)
			{
				continue;
			}

			Vec3 vShootPos = pPlayer->m_vecOrigin() + pPlayer->GetViewOffset();
			Vec3 vForward; Math::AngleVectors(pPlayer->GetEyeAngles(), &vForward);
			Vec3 vShootEnd = mDots.contains(pPlayer) ? mDots[pPlayer] : vShootPos + (vForward * 8192.f);

			CGameTrace trace = {};
			CTraceFilterHitscan filter = {}; filter.pSkip = pPlayer;
			SDK::Trace(vShootPos, vShootEnd, MASK_SHOT, &filter, &trace);

			m_vSightLines.push_back({ vShootPos, trace.endpos, H::Color.GetEntityDrawColor(pLocal, pPlayer, Vars::Colors::Relative.Value) });
		}
	}
}

void CVisuals::PickupTimers()
{
	if (!Vars::Visuals::UI::PickupTimers.Value)
		return;

	for (auto pickupData = m_vPickupDatas.begin(); pickupData != m_vPickupDatas.end();)
	{
		const float timeDiff = I::EngineClient->Time() - pickupData->Time;
		if (timeDiff > 10.f)
		{
			pickupData = m_vPickupDatas.erase(pickupData);
			continue;
		}

		auto timerText = std::format("{:.1f}s", 10.f - timeDiff);
		auto color = pickupData->Type ? Vars::Colors::Health.Value : Vars::Colors::Ammo.Value;

		Vec3 vScreen;
		if (SDK::W2S(pickupData->Location, vScreen))
			H::Draw.String(H::Fonts.GetFont(FONT_ESP), vScreen.x, vScreen.y, color, ALIGN_CENTER, timerText.c_str());

		++pickupData;
	}
}



void CVisuals::OverrideWorldTextures()
{
	auto uHash = FNV1A::Hash32(Vars::Visuals::World::WorldTexture.Value.c_str());
	if (uHash == FNV1A::Hash32Const("Default"))
		return;

	KeyValues* kv = new KeyValues("LightmappedGeneric");
	if (uHash == FNV1A::Hash32Const("Dev"))
		kv->SetString("$basetexture", "dev/dev_measuregeneric01b");
	else if (uHash == FNV1A::Hash32Const("Camo"))
		kv->SetString("$basetexture", "patterns/paint_strokes");
	else if (uHash == FNV1A::Hash32Const("Black"))
		kv->SetString("$basetexture", "patterns/combat/black");
	else if (uHash == FNV1A::Hash32Const("White"))
		kv->SetString("$basetexture", "patterns/combat/white");
	else if (uHash == FNV1A::Hash32Const("Flat"))
	{
		kv->SetString("$basetexture", "vgui/white_additive");
		kv->SetString("$color2", "[0.12 0.12 0.15]");
	}
	else
		kv->SetString("$basetexture", Vars::Visuals::World::WorldTexture.Value.c_str());

	if (!kv)
		return;

	for (auto h = I::MaterialSystem->FirstMaterial(); h != I::MaterialSystem->InvalidMaterial(); h = I::MaterialSystem->NextMaterial(h))
	{
		auto pMaterial = I::MaterialSystem->GetMaterial(h);
		if (!pMaterial || pMaterial->IsErrorMaterial() || !pMaterial->IsPrecached() || pMaterial->IsTranslucent() || pMaterial->IsSpriteCard())
			continue;

		auto sGroup = std::string_view(pMaterial->GetTextureGroupName());
		auto sName = std::string_view(pMaterial->GetName());

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

void ApplyModulation(const Color_t& clr, bool bSky = false)
{
	for (auto h = I::MaterialSystem->FirstMaterial(); h != I::MaterialSystem->InvalidMaterial(); h = I::MaterialSystem->NextMaterial(h))
	{
		auto pMaterial = I::MaterialSystem->GetMaterial(h);
		if (!pMaterial || pMaterial->IsErrorMaterial() || !pMaterial->IsPrecached())
			continue;

		auto sGroup = std::string_view(pMaterial->GetTextureGroupName());
		if (!bSky ? !sGroup._Starts_with("World") : !sGroup._Starts_with("SkyBox"))
			continue;

		pMaterial->ColorModulate(float(clr.r) / 255.f, float(clr.g) / 255.f, float(clr.b) / 255.f);
	}
}

void CVisuals::Modulate()
{
	const bool bScreenshot = Vars::Visuals::UI::CleanScreenshots.Value && I::EngineClient->IsTakingScreenshot();
	const bool bWorldModulation = Vars::Visuals::World::Modulations.Value & (1 << 0) && !bScreenshot;
	const bool bSkyModulation = Vars::Visuals::World::Modulations.Value & (1 << 1) && !bScreenshot;

	bool bSetChanged, bColorChanged, bSkyChanged, bConnection;
	{
		static bool bStaticWorld = false, bStaticSky = false;
		bool bOldWorld = bStaticWorld, bOldSky = bStaticSky;
		bool bNewWorld = bStaticWorld = bWorldModulation, bNewSky = bStaticSky = bSkyModulation;
		bSetChanged = bNewWorld != bOldWorld || bNewSky != bOldSky;
	}
	{
		static Color_t tStaticWorld = {}, tStaticSky = {};
		Color_t tOldWorld = tStaticWorld, tOldSky = tStaticSky;
		Color_t tNewWorld = tStaticWorld = Vars::Colors::WorldModulation.Value, tNewSky = tStaticSky = Vars::Colors::SkyModulation.Value;
		bColorChanged = tNewWorld != tOldWorld || tNewSky != tOldSky;
	}
	{
		static uint32_t uStatic = 0;
		uint32_t uOld = uStatic;
		uint32_t uNew = uStatic = FNV1A::Hash32(Vars::Visuals::World::SkyboxChanger.Value.c_str());
		bSkyChanged = uNew != uOld;
	}
	{
		static bool bStaticConnected = false;
		bool bOldConnected = bStaticConnected;
		bool bNewConnected = bStaticConnected = I::EngineClient->IsConnected() && I::EngineClient->IsInGame();
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
	if (pLocal && Vars::Visuals::Particles::SpellFootsteps.Value && (G::DoubleTap || G::Warp))
	{
		static auto fnFireEvent = S::CTFPlayer_FireEvent.As<void(__fastcall*)(void*, const Vector, const QAngle, int, const char*)>();
		fnFireEvent(pLocal, pLocal->GetAbsOrigin(), {}, 7001, nullptr);
	}
	
	static uint32_t iOldMedigunBeam = 0, iOldMedigunCharge = 0;
	uint32_t iNewMedigunBeam = FNV1A::Hash32(Vars::Visuals::Particles::MedigunBeam.Value.c_str()), iNewMedigunCharge = FNV1A::Hash32(Vars::Visuals::Particles::MedigunCharge.Value.c_str());
	if (iOldMedigunBeam != iNewMedigunBeam || iOldMedigunCharge != iNewMedigunCharge)
	{
		if (pWeapon && pWeapon->GetWeaponID() == TF_WEAPON_MEDIGUN)
		{
			static auto fnUpdateEffects = S::CWeaponMedigun_UpdateEffects.As<void(__fastcall*)(void*)>();
			fnUpdateEffects(pWeapon);

			static auto fnStopChargeEffect = S::CWeaponMedigun_StopChargeEffect.As<void(__fastcall*)(void*)>();
			fnStopChargeEffect(pWeapon);

			static auto fnManageChargeEffect = S::CWeaponMedigun_StopChargeEffect.As<void(__fastcall*)(void*, bool)>();
			fnManageChargeEffect(pWeapon, false);
		}

		iOldMedigunBeam = iNewMedigunBeam;
		iOldMedigunCharge = iNewMedigunCharge;
	}
}