#include "AimbotMelee.h"

#include "../Aimbot.h"
#include "../../Simulation/MovementSimulation/MovementSimulation.h"
#include "../../EnginePrediction/EnginePrediction.h"
#include "../../Ticks/Ticks.h"
#include "../../Visuals/Visuals.h"

std::vector<Target_t> CAimbotMelee::GetTargets(CTFPlayer* pLocal, CTFWeaponBase* pWeapon)
{
	std::vector<Target_t> vTargets;

	const Vec3 vLocalPos = F::Ticks.GetShootPos();
	const Vec3 vLocalAngles = I::EngineClient->GetViewAngles();

	if (Vars::Aimbot::General::Target.Value & Vars::Aimbot::General::TargetEnum::Players)
	{
		bool bDisciplinary = Vars::Aimbot::Melee::WhipTeam.Value && SDK::AttribHookValue(0, "speed_buff_ally", pWeapon) > 0;
		for (auto pEntity : H::Entities.GetGroup(bDisciplinary ? EGroupType::PLAYERS_ALL : EGroupType::PLAYERS_ENEMIES))
		{
			bool bTeammate = pEntity->m_iTeamNum() == pLocal->m_iTeamNum();
			if (F::AimbotGlobal.ShouldIgnore(pEntity, pLocal, pWeapon))
				continue;

			float flFOVTo; Vec3 vPos, vAngleTo;
			if (!F::AimbotGlobal.PlayerBoneInFOV(pEntity->As<CTFPlayer>(), vLocalPos, vLocalAngles, flFOVTo, vPos, vAngleTo))
				continue;

			float flDistTo = vLocalPos.DistTo(vPos);
			vTargets.emplace_back(pEntity, TargetEnum::Player, vPos, vAngleTo, flFOVTo, flDistTo, bTeammate ? 0 : F::AimbotGlobal.GetPriority(pEntity->entindex()));
		}
	}

	if (Vars::Aimbot::General::Target.Value)
	{
		bool bWrench = pWeapon->GetWeaponID() == TF_WEAPON_WRENCH;
		bool bDestroySapper = pWeapon->GetWeaponID() == TF_WEAPON_FIREAXE && SDK::AttribHookValue(0, "set_dmg_apply_to_sapper", pWeapon);

		for (auto pEntity : H::Entities.GetGroup(bWrench || bDestroySapper ? EGroupType::BUILDINGS_ALL : EGroupType::BUILDINGS_ENEMIES))
		{
			if (F::AimbotGlobal.ShouldIgnore(pEntity, pLocal, pWeapon))
				continue;

			if (pEntity->m_iTeamNum() == pLocal->m_iTeamNum() && (bWrench && !AimFriendlyBuilding(pEntity->As<CBaseObject>()) || bDestroySapper && !pEntity->As<CBaseObject>()->m_bHasSapper()))
				continue;

			Vec3 vPos = pEntity->GetCenter();
			Vec3 vAngleTo = Math::CalcAngle(vLocalPos, vPos);
			float flFOVTo = Math::CalcFov(vLocalAngles, vAngleTo);
			if (flFOVTo > Vars::Aimbot::General::AimFOV.Value)
				continue;

			float flDistTo = vLocalPos.DistTo(vPos);
			vTargets.emplace_back(pEntity, pEntity->IsSentrygun() ? TargetEnum::Sentry : pEntity->IsDispenser() ? TargetEnum::Dispenser : TargetEnum::Teleporter, vPos, vAngleTo, flFOVTo, flDistTo);
		}
	}

	if (Vars::Aimbot::General::Target.Value & Vars::Aimbot::General::TargetEnum::NPCs)
	{
		for (auto pEntity : H::Entities.GetGroup(EGroupType::WORLD_NPC))
		{
			if (F::AimbotGlobal.ShouldIgnore(pEntity, pLocal, pWeapon))
				continue;

			Vec3 vPos = pEntity->GetCenter();
			Vec3 vAngleTo = Math::CalcAngle(vLocalPos, vPos);
			float flFOVTo = Math::CalcFov(vLocalAngles, vAngleTo);
			if (flFOVTo > Vars::Aimbot::General::AimFOV.Value)
				continue;

			float flDistTo = vLocalPos.DistTo(vPos);
			vTargets.emplace_back(pEntity, TargetEnum::NPC, vPos, vAngleTo, flFOVTo, flDistTo);
		}
	}

	return vTargets;
}

bool CAimbotMelee::AimFriendlyBuilding(CBaseObject* pBuilding)
{
	if (!pBuilding->m_bMiniBuilding() && pBuilding->m_iUpgradeLevel() != 3 || pBuilding->m_iHealth() < pBuilding->m_iMaxHealth() || pBuilding->m_bHasSapper())
		return true;

	if (pBuilding->IsSentrygun())
	{
		int iShells, iMaxShells, iRockets, iMaxRockets; pBuilding->As<CObjectSentrygun>()->GetAmmoCount(iShells, iMaxShells, iRockets, iMaxRockets);
		if (iShells < iMaxShells || iRockets < iMaxRockets)
			return true;
	}

	return false;
}

std::vector<Target_t> CAimbotMelee::SortTargets(CTFPlayer* pLocal, CTFWeaponBase* pWeapon)
{
	auto vTargets = GetTargets(pLocal, pWeapon);

	F::AimbotGlobal.SortTargets(vTargets, Vars::Aimbot::General::TargetSelectionEnum::Distance);
	vTargets.resize(std::min(size_t(Vars::Aimbot::General::MaxTargets.Value), vTargets.size()));
	F::AimbotGlobal.SortPriority(vTargets);
	return vTargets;
}



int CAimbotMelee::GetSwingTime(CTFWeaponBase* pWeapon)
{
	if (pWeapon->GetWeaponID() == TF_WEAPON_KNIFE)
		return 0;
	return Vars::Aimbot::Melee::SwingTicks.Value;
}

void CAimbotMelee::SimulatePlayers(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, std::vector<Target_t> vTargets, Vec3& vEyePos)
{
	// swing prediction / auto warp
	const int iSwingTicks = GetSwingTime(pWeapon);
	int iMax = (m_iDoubletapTicks && Vars::Doubletap::AntiWarp.Value && pLocal->m_hGroundEntity())
		? std::max(iSwingTicks - Vars::Doubletap::TickLimit.Value - 1, 0)
		: std::max(iSwingTicks, m_iDoubletapTicks);

	int iSimTicks = GetSwingTime(pWeapon), iSwingTicks = GetSwingTime(pWeapon, false);

	if ((Vars::Aimbot::Melee::SwingPrediction.Value && iSimTicks || m_iDoubletapTicks) && G::CanPrimaryAttack && pWeapon->m_flSmackTime() < 0.f)
	{
		MoveStorage tStorage;
		std::unordered_map<int, MoveStorage> mStorage;

		bool bSwung = false;
		int iLocal = 0;

		F::MoveSim.Initialize(pLocal, tStorage, false, !m_iDoubletapTicks);
		for (auto& tTarget : vTargets)
			F::MoveSim.Initialize(tTarget.m_pEntity, mStorage[tTarget.m_pEntity->entindex()], false);

		int iMax = std::max(iSimTicks, m_iDoubletapTicks);
		int iTicks = iMax; bool bSwung = false;
		for (int i = 0; i < iTicks; i++) // intended for plocal to collide with targets
		{
			if (i < iMax)
			{

				if (!bSwung && (!m_iDoubletapTicks || Vars::Doubletap::AntiWarp.Value && pLocal->m_hGroundEntity() || iMax - i <= iSwingTicks))
				{
					iTicks = std::min(i + iSwingTicks, iMax), bSwung = true;
					if (!iSwingTicks)
						break;

					if (pLocal->InCond(TF_COND_SHIELD_CHARGE))
					{	// demo charge fix for swing pred
						tStorage.m_MoveData.m_flMaxSpeed = tStorage.m_MoveData.m_flClientMaxSpeed = SDK::MaxSpeed(pLocal, false, true);
						pLocal->m_flMaxspeed() = tStorage.m_MoveData.m_flMaxSpeed;
						pLocal->m_nPlayerCond() &= ~TF_COND_SHIELD_CHARGE;
					}
				}
				if (m_iDoubletapTicks && Vars::Doubletap::AntiWarp.Value && pLocal->m_hGroundEntity())
					F::Ticks.AntiWarp(pLocal, G::CurrentUserCmd->viewangles.y, tStorage.m_MoveData.m_flForwardMove, tStorage.m_MoveData.m_flSideMove, iMax - i - 1);

				F::MoveSim.RunTick(tStorage);
				m_mRecordMap[I::EngineClient->GetLocalPlayer()].emplace_front(
					pLocal->m_flSimulationTime() + TICKS_TO_TIME(i + 1),
					tStorage.m_MoveData.m_vecAbsOrigin,
					pLocal->m_vecMins(), pLocal->m_vecMaxs()
				);
			}
			if (i < iSwingTicks - m_iDoubletapTicks)
			{
				for (auto& tTarget : vTargets)
				{
					auto& tStorage = mStorage[tTarget.m_pEntity->entindex()];

					F::MoveSim.RunTick(tStorage);
					if (!tStorage.m_bFailed)
						m_mRecordMap[tTarget.m_pEntity->entindex()].emplace_front(
							!Vars::Aimbot::Melee::SwingPredictLag.Value || tStorage.m_bPredictNetworked ? tTarget.m_pEntity->m_flSimulationTime() + TICKS_TO_TIME(i + 1) : 0.f,
							Vars::Aimbot::Melee::SwingPredictLag.Value ? tStorage.m_vPredictedOrigin : tStorage.m_MoveData.m_vecAbsOrigin,
							tTarget.m_pEntity->m_vecMins(), tTarget.m_pEntity->m_vecMaxs()
						);
				}
			}
		}
		vEyePos = tStorage.m_MoveData.m_vecAbsOrigin + pLocal->m_vecViewOffset();

		if (Vars::Visuals::Simulation::SwingLines.Value && Vars::Visuals::Simulation::PlayerPath.Value)
		{
			const bool bAlwaysDraw = !Vars::Aimbot::General::AutoShoot.Value || Vars::Debug::Info.Value;
			if (!bAlwaysDraw)
			{
				m_mPaths[pLocal->entindex()] = tStorage.m_vPath;
				for (auto& tTarget : vTargets)
					m_mPaths[tTarget.m_pEntity->entindex()] = mStorage[tTarget.m_pEntity->entindex()].m_vPath;
			}
			else
			{
				G::LineStorage.clear();
				G::BoxStorage.clear();
				G::PathStorage.clear();

				if (Vars::Colors::PlayerPathIgnoreZ.Value.a)
				{
					G::PathStorage.emplace_back(tStorage.m_vPath, I::GlobalVars->curtime + Vars::Visuals::Simulation::DrawDuration.Value, Vars::Colors::PlayerPathIgnoreZ.Value, Vars::Visuals::Simulation::PlayerPath.Value);
					for (auto& tTarget : vTargets)
						G::PathStorage.emplace_back(mStorage[tTarget.m_pEntity->entindex()].m_vPath, I::GlobalVars->curtime + Vars::Visuals::Simulation::DrawDuration.Value, Vars::Colors::PlayerPathIgnoreZ.Value, Vars::Visuals::Simulation::PlayerPath.Value);
				}
				if (Vars::Colors::PlayerPath.Value.a)
				{
					G::PathStorage.emplace_back(tStorage.m_vPath, I::GlobalVars->curtime + Vars::Visuals::Simulation::DrawDuration.Value, Vars::Colors::PlayerPath.Value, Vars::Visuals::Simulation::PlayerPath.Value, true);
					for (auto& tTarget : vTargets)
						G::PathStorage.emplace_back(mStorage[tTarget.m_pEntity->entindex()].m_vPath, I::GlobalVars->curtime + Vars::Visuals::Simulation::DrawDuration.Value, Vars::Colors::PlayerPath.Value, Vars::Visuals::Simulation::PlayerPath.Value, true);
				}
			}
		}

		F::MoveSim.Restore(tStorage);
		for (auto& tTarget : vTargets)
			F::MoveSim.Restore(mStorage[tTarget.m_pEntity->entindex()]);
	}

	m_bShouldSwing = m_iDoubletapTicks <= iSwingTicks || Vars::Doubletap::AntiWarp.Value && pLocal->m_hGroundEntity();
}

bool CAimbotMelee::CanBackstab(CBaseEntity* pTarget, CTFPlayer* pLocal, Vec3 vEyeAngles)
{
	if (!pLocal || !pTarget)
		return false;

	if (Vars::Aimbot::Melee::IgnoreRazorback.Value)
	{
		CUtlVector<CBaseEntity*> itemList;
		int iBackstabShield = SDK::AttribHookValue(0, "set_blockbackstab_once", pTarget, &itemList);
		if (iBackstabShield && itemList.Count())
		{
			CBaseEntity* pEntity = itemList.Element(0);
			if (pEntity && pEntity->ShouldDraw())
				return false;
		}
	}

	Vec3 vEyePos = pLocal->GetShootPos();
	const float flCompDist = 0.0625f;
	const float flSqCompDist = 0.0884f;

	if (auto pCmd = G::CurrentUserCmd;
		m_mRecordMap[pLocal->entindex()].empty() && pCmd->viewangles != vEyeAngles && G::CanPrimaryAttack)
	{	// repredict, prevent prediction error causing miss
		CUserCmd tOldCmd = *pCmd;
		Vec3 vOldAngles = I::EngineClient->GetViewAngles();
		int iOldAttacking = G::Attacking;
		bool bOldSilent = G::PSilentAngles;
		F::EnginePrediction.End(pLocal, pCmd);

		G::Attacking = true;
		Aim(pCmd, vEyeAngles);
		F::Ticks.Start(pLocal, pCmd);
		vEyePos = pLocal->GetShootPos();
		F::EnginePrediction.End(pLocal, pCmd);

		*pCmd = tOldCmd;
		I::EngineClient->SetViewAngles(vOldAngles);
		G::Attacking = iOldAttacking;
		G::PSilentAngles = bOldSilent;
		F::Ticks.Start(pLocal, pCmd);
	}

	Vec3 vToTarget = (pTarget->GetAbsOrigin() - vEyePos).To2D();
	const float flDist = vToTarget.Normalize();
	if (flDist < flSqCompDist)
		return false;

	const float flExtra = 2.f * flCompDist / flDist; // account for origin compression

	float flPosVsTargetViewMinDot = 0.f + 0.0031f + flExtra;
	float flPosVsOwnerViewMinDot = 0.5f + flExtra;
	float flViewAnglesMinDot = -0.3f + 0.0031f; // 0.00306795676297 ?

	auto TestDots = [&](Vec3 vTargetAngles)
		{
			Vec3 vOwnerForward; Math::AngleVectors(vEyeAngles, &vOwnerForward);
			vOwnerForward.Normalize2D();

			Vec3 vTargetForward; Math::AngleVectors(vTargetAngles, &vTargetForward);
			vTargetForward.Normalize2D();

			const float flPosVsTargetViewDot = vToTarget.Dot(vTargetForward); // Behind?
			const float flPosVsOwnerViewDot = vToTarget.Dot(vOwnerForward); // Facing?
			const float flViewAnglesDot = vTargetForward.Dot(vOwnerForward); // Facestab?

			return flPosVsTargetViewDot > flPosVsTargetViewMinDot && flPosVsOwnerViewDot > flPosVsOwnerViewMinDot && flViewAnglesDot > flViewAnglesMinDot;
		};

	Vec3 vTargetAngles = { 0.f, H::Entities.GetEyeAngles(pTarget->entindex()).y, 0.f };
	if (!Vars::Aimbot::Melee::BackstabAccountPing.Value)
	{
		if (!TestDots(vTargetAngles))
			return false;
	}
	else
	{
		if (Vars::Aimbot::Melee::BackstabDoubleTest.Value && !TestDots(vTargetAngles))
			return false;

		vTargetAngles.y += H::Entities.GetPingAngles(pTarget->entindex()).y;
		if (!TestDots(vTargetAngles))
			return false;
	}

	return true;
}

int CAimbotMelee::CanHit(Target_t& tTarget, CTFPlayer* pLocal, CTFWeaponBase* pWeapon, Vec3 vEyePos)
{
	if (Vars::Aimbot::General::Ignore.Value & Vars::Aimbot::General::IgnoreEnum::Unsimulated && H::Entities.GetChoke(tTarget.m_pEntity->entindex()) > Vars::Aimbot::General::TickTolerance.Value)
		return false;

	float flRange = SDK::AttribHookValue(pWeapon->GetSwingRange(), "melee_range_multiplier", pWeapon);
	float flHull = SDK::AttribHookValue(18, "melee_bounds_multiplier", pWeapon);
	if (pLocal->m_flModelScale() > 1.0f)
	{
		flRange *= pLocal->m_flModelScale();
		flHull *= pLocal->m_flModelScale();
	}
	Vec3 vSwingMins = { -flHull, -flHull, -flHull };
	Vec3 vSwingMaxs = { flHull, flHull, flHull };
	auto& vSimRecords = m_mRecordMap[tTarget.m_pEntity->entindex()];

	std::vector<TickRecord*> vRecords = {};
	if (F::Backtrack.GetRecords(tTarget.m_pEntity, vRecords))
	{
		if (!vRecords.empty())
		{
			for (auto& tRecord : vSimRecords)
				vRecords.push_back(&tRecord);
			vRecords = F::Backtrack.GetValidRecords(vRecords, pLocal, true, -TICKS_TO_TIME(vSimRecords.size()));
		}
		if (vRecords.empty())
			return false;
	}
	else
	{
		matrix3x4 aBones[MAXSTUDIOBONES];
		if (!tTarget.m_pEntity->SetupBones(aBones, MAXSTUDIOBONES, BONE_USED_BY_ANYTHING, tTarget.m_pEntity->m_flSimulationTime()))
			return false;

		F::Backtrack.m_tRecord = { tTarget.m_pEntity->m_flSimulationTime(), tTarget.m_pEntity->m_vecOrigin(), tTarget.m_pEntity->m_vecMins(), tTarget.m_pEntity->m_vecMaxs(), *reinterpret_cast<BoneMatrix*>(&aBones) };
		vRecords = { &F::Backtrack.m_tRecord };
	}

	CGameTrace trace = {};
	CTraceFilterHitscan filter = {}; filter.pSkip = pLocal;
	for (auto pRecord : vRecords)
	{
		Vec3 vRestoreOrigin = tTarget.m_pEntity->GetAbsOrigin();
		Vec3 vRestoreMins = tTarget.m_pEntity->m_vecMins();
		Vec3 vRestoreMaxs = tTarget.m_pEntity->m_vecMaxs();

		tTarget.m_pEntity->SetAbsOrigin(pRecord->m_vOrigin);
		tTarget.m_pEntity->m_vecMins() = pRecord->m_vMins + 0.125f; // account for origin compression
		tTarget.m_pEntity->m_vecMaxs() = pRecord->m_vMaxs - 0.125f;

		Vec3 vDiff = { 0, 0, std::clamp(vEyePos.z - pRecord->m_vOrigin.z, (float)pRecord->m_vMins.z, (float)pRecord->m_vMaxs.z) };
		tTarget.m_vPos = pRecord->m_vOrigin + vDiff;
		Aim(G::CurrentUserCmd->viewangles, Math::CalcAngle(vEyePos, tTarget.m_vPos), tTarget.m_vAngleTo);

		Vec3 vForward; Math::AngleVectors(tTarget.m_vAngleTo, &vForward);
		Vec3 vTraceEnd = vEyePos + (vForward * flRange);

		SDK::TraceHull(vEyePos, vTraceEnd, {}, {}, MASK_SOLID, &filter, &trace);
		bool bReturn = trace.m_pEnt && trace.m_pEnt == tTarget.m_pEntity;
		if (!bReturn)
		{
			SDK::TraceHull(vEyePos, vTraceEnd, vSwingMins, vSwingMaxs, MASK_SOLID, &filter, &trace);
			bReturn = trace.m_pEnt && trace.m_pEnt == tTarget.m_pEntity;
		}

		if (bReturn && Vars::Aimbot::Melee::AutoBackstab.Value && pWeapon->GetWeaponID() == TF_WEAPON_KNIFE)
			bReturn = CanBackstab(tTarget.m_pEntity, pLocal, tTarget.m_vAngleTo);

		tTarget.m_pEntity->SetAbsOrigin(vRestoreOrigin);
		tTarget.m_pEntity->m_vecMins() = vRestoreMins;
		tTarget.m_pEntity->m_vecMaxs() = vRestoreMaxs;

		if (bReturn)
		{
			tTarget.m_pRecord = pRecord;
			tTarget.m_bBacktrack = tTarget.m_iTargetType == TargetEnum::Player;

			return true;
		}
		else switch (Vars::Aimbot::General::AimType.Value)
		{
		case Vars::Aimbot::General::AimTypeEnum::Smooth:
		case Vars::Aimbot::General::AimTypeEnum::Assistive:
		{
			auto vAngle = Math::CalcAngle(vEyePos, tTarget.m_vPos);

			Math::AngleVectors(vAngle, &vForward);
			vTraceEnd = vEyePos + (vForward * flRange);

			SDK::TraceHull(vEyePos, vTraceEnd, vSwingMins, vSwingMaxs, MASK_SOLID, &filter, &trace);
			if (trace.m_pEnt && trace.m_pEnt == tTarget.m_pEntity)
				return 2;
		}
		}
	}

	return false;
}



bool CAimbotMelee::Aim(Vec3 vCurAngle, Vec3 vToAngle, Vec3& vOut, int iMethod)
{
	if (Vec3* pDoubletapAngle = F::Ticks.GetShootAngle())
	{
		vOut = *pDoubletapAngle;
		return true;
	}

	bool bReturn = false;
	switch (iMethod)
	{
	case Vars::Aimbot::General::AimTypeEnum::Plain:
	case Vars::Aimbot::General::AimTypeEnum::Silent:
	case Vars::Aimbot::General::AimTypeEnum::Locking:
		vOut = vToAngle;
		break;
	case Vars::Aimbot::General::AimTypeEnum::Smooth:
		vOut = vCurAngle.LerpAngle(vToAngle, Vars::Aimbot::General::AssistStrength.Value / 100.f);
		bReturn = true;
		break;
	case Vars::Aimbot::General::AimTypeEnum::Assistive:
		Vec3 vMouseDelta = G::CurrentUserCmd->viewangles.DeltaAngle(G::LastUserCmd->viewangles);
		Vec3 vTargetDelta = vToAngle.DeltaAngle(G::LastUserCmd->viewangles);
		float flMouseDelta = vMouseDelta.Length2D(), flTargetDelta = vTargetDelta.Length2D();
		vTargetDelta = vTargetDelta.Normalized() * std::min(flMouseDelta, flTargetDelta);
		vOut = vCurAngle - vMouseDelta + vMouseDelta.LerpAngle(vTargetDelta, Vars::Aimbot::General::AssistStrength.Value / 100.f);
		bReturn = true;
		break;
	}

	Math::ClampAngles(vOut);
	return bReturn;
}

// assume angle calculated outside with other overload
void CAimbotMelee::Aim(CUserCmd* pCmd, Vec3& vAngle, int iMethod)
{
	bool bUnsure = F::Ticks.IsTimingUnsure() || F::Ticks.GetTicks(H::Entities.GetWeapon());
	switch (iMethod)
	{
	case Vars::Aimbot::General::AimTypeEnum::Plain:
		if (G::Attacking != 1 && !bUnsure)
			break;
		[[fallthrough]];
	case Vars::Aimbot::General::AimTypeEnum::Smooth:
	case Vars::Aimbot::General::AimTypeEnum::Assistive:
		pCmd->viewangles = vAngle;
		I::EngineClient->SetViewAngles(vAngle);
		break;
	case Vars::Aimbot::General::AimTypeEnum::Silent:
		if (G::Attacking == 1 || bUnsure)
		{
			SDK::FixMovement(pCmd, vAngle);
			pCmd->viewangles = vAngle;
			G::PSilentAngles = true;
		}
		break;
	case Vars::Aimbot::General::AimTypeEnum::Locking:
		SDK::FixMovement(pCmd, vAngle);
		pCmd->viewangles = vAngle;
		G::SilentAngles = true;
	}
}

static inline void DrawVisuals(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd, Target_t& tTarget, std::unordered_map<int, std::vector<Vec3>>& mPaths)
{
	bool bPath = Vars::Visuals::Simulation::SwingLines.Value && Vars::Visuals::Simulation::PlayerPath.Value && Vars::Aimbot::General::AutoShoot.Value && !Vars::Debug::Info.Value;
	bool bLine = Vars::Visuals::Line::Enabled.Value;
	bool bBoxes = Vars::Visuals::Hitbox::BonesEnabled.Value & Vars::Visuals::Hitbox::BonesEnabledEnum::OnShot;
	if (pCmd->buttons & IN_ATTACK && G::CanPrimaryAttack && pWeapon->m_flSmackTime() < 0.f)
	{
		G::LineStorage.clear();
		G::BoxStorage.clear();
		G::PathStorage.clear();

		if (bPath)
		{
			if (Vars::Colors::PlayerPathIgnoreZ.Value.a)
			{
				G::PathStorage.emplace_back(mPaths[pLocal->entindex()], I::GlobalVars->curtime + Vars::Visuals::Simulation::DrawDuration.Value, Vars::Colors::PlayerPathIgnoreZ.Value, Vars::Visuals::Simulation::PlayerPath.Value);
				G::PathStorage.emplace_back(mPaths[tTarget.m_pEntity->entindex()], I::GlobalVars->curtime + Vars::Visuals::Simulation::DrawDuration.Value, Vars::Colors::PlayerPathIgnoreZ.Value, Vars::Visuals::Simulation::PlayerPath.Value);
			}
			if (Vars::Colors::PlayerPath.Value.a)
			{
				G::PathStorage.emplace_back(mPaths[pLocal->entindex()], I::GlobalVars->curtime + Vars::Visuals::Simulation::DrawDuration.Value, Vars::Colors::PlayerPath.Value, Vars::Visuals::Simulation::PlayerPath.Value, true);
				G::PathStorage.emplace_back(mPaths[tTarget.m_pEntity->entindex()], I::GlobalVars->curtime + Vars::Visuals::Simulation::DrawDuration.Value, Vars::Colors::PlayerPath.Value, Vars::Visuals::Simulation::PlayerPath.Value, true);
			}
		}
	}
	if (G::Attacking == 1)
	{
		if (bLine)
		{
			Vec3 vEyePos = pLocal->GetShootPos();
			float flDist = vEyePos.DistTo(tTarget.m_vPos);
			Vec3 vForward; Math::AngleVectors(tTarget.m_vAngleTo, &vForward);

			if (Vars::Colors::LineIgnoreZ.Value.a)
				G::LineStorage.emplace_back(std::pair<Vec3, Vec3>(vEyePos, vEyePos + vForward * flDist), I::GlobalVars->curtime + Vars::Visuals::Line::DrawDuration.Value, Vars::Colors::LineIgnoreZ.Value);
			if (Vars::Colors::Line.Value.a)
				G::LineStorage.emplace_back(std::pair<Vec3, Vec3>(vEyePos, vEyePos + vForward * flDist), I::GlobalVars->curtime + Vars::Visuals::Line::DrawDuration.Value, Vars::Colors::Line.Value, true);
		}
		if (bBoxes)
		{
			auto vBoxes = F::Visuals.GetHitboxes(tTarget.m_pRecord->m_BoneMatrix.m_aBones, tTarget.m_pEntity->As<CBaseAnimating>());
			G::BoxStorage.insert(G::BoxStorage.end(), vBoxes.begin(), vBoxes.end());

			//if (Vars::Colors::BoneHitboxEdgeIgnoreZ.Value.a || Vars::Colors::BoneHitboxFaceIgnoreZ.Value.a)
			//	G::BoxStorage.emplace_back(tTarget.m_pRecord->m_vOrigin, tTarget.m_pRecord->m_vMins, tTarget.m_pRecord->m_vMaxs, Vec3(), I::GlobalVars->curtime + Vars::Visuals::Hitbox::DrawDuration.Value, Vars::Colors::BoneHitboxEdgeIgnoreZ.Value, Vars::Colors::BoneHitboxFaceIgnoreZ.Value);
			//if (Vars::Colors::BoneHitboxEdge.Value.a || Vars::Colors::BoneHitboxFace.Value.a)
			//	G::BoxStorage.emplace_back(tTarget.m_pRecord->m_vOrigin, tTarget.m_pRecord->m_vMins, tTarget.m_pRecord->m_vMaxs, Vec3(), I::GlobalVars->curtime + Vars::Visuals::Hitbox::DrawDuration.Value, Vars::Colors::BoneHitboxEdge.Value, Vars::Colors::BoneHitboxFace.Value, true);
		}
	}
}

void CAimbotMelee::Run(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd)
{
	static int iStaticAimType = Vars::Aimbot::General::AimType.Value;
	const int iLastAimType = iStaticAimType;
	const int iRealAimType = Vars::Aimbot::General::AimType.Value;

	if (pWeapon->m_flSmackTime() > 0.f && !iRealAimType && iLastAimType)
		Vars::Aimbot::General::AimType.Value = iLastAimType;
	iStaticAimType = Vars::Aimbot::General::AimType.Value;

	if (F::AimbotGlobal.ShouldHoldAttack(pWeapon))
		pCmd->buttons |= IN_ATTACK;
	if (!Vars::Aimbot::General::AimType.Value
		|| !F::AimbotGlobal.ShouldAim() && pWeapon->m_flSmackTime() < 0.f)
		return;

	if (RunSapper(pLocal, pWeapon, pCmd))
		return;

	auto vTargets = SortTargets(pLocal, pWeapon);
	if (vTargets.empty())
		return;

	m_mRecordMap.clear(); m_mPaths.clear();
	m_iDoubletapTicks = F::Ticks.GetTicks(pWeapon);
	const bool bShouldSwing = m_iDoubletapTicks <= (GetSwingTime(pWeapon) ? 14 : 0) || Vars::Doubletap::AntiWarp.Value && pLocal->m_hGroundEntity();
	Vec3 vEyePos = pLocal->GetShootPos();

	SimulatePlayers(pLocal, pWeapon, vTargets, vEyePos);

	//if (!G::AimTarget.m_iEntIndex)
	//	G::AimTarget = { vTargets.front().m_pEntity->entindex(), I::GlobalVars->tickcount, 0 };

	for (auto& tTarget : vTargets)
	{
		const auto iResult = CanHit(tTarget, pLocal, pWeapon, vEyePos);
		if (!iResult) continue;
		if (iResult == 2)
		{
			G::AimTarget = { tTarget.m_pEntity->entindex(), I::GlobalVars->tickcount, 0 };
			Aim(pCmd, tTarget.m_vAngleTo);
			break;
		}

		G::AimTarget = { tTarget.m_pEntity->entindex(), I::GlobalVars->tickcount };
		G::AimPoint = { tTarget.m_vPos, I::GlobalVars->tickcount };

		if (Vars::Aimbot::General::AutoShoot.Value && pWeapon->m_flSmackTime() < 0.f)
		{
			if (bShouldSwing)
				pCmd->buttons |= IN_ATTACK;
			if (m_iDoubletapTicks)
				F::Ticks.m_bDoubletap = true;
		}

		G::Attacking = SDK::IsAttacking(pLocal, pWeapon, pCmd, true);
		if (G::Attacking == 1)
		{
			if (tTarget.m_bBacktrack)
				pCmd->tick_count = TIME_TO_TICKS(tTarget.m_pRecord->m_flSimTime) + TIME_TO_TICKS(F::Backtrack.GetFakeInterp());
			// bug: fast old records seem to be progressively more unreliable ?
		}
		else
		{
			vEyePos = pLocal->GetShootPos();
			Aim(G::CurrentUserCmd->viewangles, Math::CalcAngle(vEyePos, tTarget.m_vPos), tTarget.m_vAngleTo);
		}
		DrawVisuals(pLocal, pWeapon, pCmd, tTarget, m_mPaths);

		Aim(pCmd, tTarget.m_vAngleTo);
		break;
	}
}

static inline int GetAttachment(CBaseObject* pBuilding, int i)
{
	int iAttachment = pBuilding->GetBuildPointAttachmentIndex(i);
	if (pBuilding->IsSentrygun() && pBuilding->m_iUpgradeLevel() > 1) // idk why i need this
		iAttachment = 3;
	return iAttachment;
}
bool CAimbotMelee::FindNearestBuildPoint(CBaseObject* pBuilding, CTFPlayer* pLocal, Vec3& vPoint)
{
	bool bFoundPoint = false;

	Vec3 vEyePos = pLocal->GetShootPos();
	static auto tf_obj_max_attach_dist = U::ConVars.FindVar("tf_obj_max_attach_dist");
	float flNearestPoint = tf_obj_max_attach_dist->GetFloat();

	for (int i = 0; i < pBuilding->GetNumBuildPoints(); i++)
	{
		int v = GetAttachment(pBuilding, i);

		Vec3 vOrigin;
		if (pBuilding->GetAttachment(v, vOrigin)) // issues using pBuilding->GetBuildPoint i on sentries above level 1 for some reason
		{
			if (!SDK::VisPos(pLocal, pBuilding, vEyePos, vOrigin))
				continue;

			float flDist = (vOrigin - pLocal->m_vecOrigin()).Length();
			if (flDist < flNearestPoint)
			{
				flNearestPoint = flDist;
				vPoint = vOrigin;
				bFoundPoint = true;
			}
		}
	}

	return bFoundPoint;
}

bool CAimbotMelee::RunSapper(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd)
{
	if (pWeapon->GetWeaponID() != TF_WEAPON_BUILDER)
		return false;

	const Vec3 vLocalPos = pLocal->GetShootPos();
	const Vec3 vLocalAngles = I::EngineClient->GetViewAngles();

	std::vector<Target_t> vTargets;
	for (auto pEntity : H::Entities.GetGroup(EGroupType::BUILDINGS_ENEMIES))
	{
		auto pBuilding = pEntity->As<CBaseObject>();
		if (pBuilding->m_bHasSapper() || !pBuilding->IsInValidTeam())
			continue;

		Vec3 vPoint;
		if (!FindNearestBuildPoint(pBuilding, pLocal, vPoint))
			continue;

		Vec3 vAngleTo = Math::CalcAngle(vLocalPos, vPoint);
		const float flFOVTo = Math::CalcFov(vLocalAngles, vAngleTo);
		const float flDistTo = vLocalPos.DistTo(vPoint);

		if (flFOVTo > Vars::Aimbot::General::AimFOV.Value)
			continue;

		vTargets.emplace_back(pBuilding, TargetEnum::Unknown, vPoint, vAngleTo, flFOVTo, flDistTo);
	}
	F::AimbotGlobal.SortTargets(vTargets, Vars::Aimbot::General::TargetSelectionEnum::Distance);
	if (vTargets.empty())
		return true;

	//if (!G::AimTarget.m_iEntIndex)
	//	G::AimTarget = { vTargets.front().m_pEntity->entindex(), I::GlobalVars->tickcount, 0 };

	for (auto& tTarget : vTargets)
	{
		static int iLastRun = 0;

		bool bShouldAim = true;
		if (Vars::Aimbot::General::AutoShoot.Value)
			pCmd->buttons |= IN_ATTACK;
		else
			bShouldAim = pCmd->buttons & IN_ATTACK;
		if (Vars::Aimbot::General::AimType.Value == Vars::Aimbot::General::AimTypeEnum::Silent)
			bShouldAim = bShouldAim && (iLastRun != I::GlobalVars->tickcount - 1 || G::PSilentAngles && !F::Ticks.CanChoke());
		
		if (bShouldAim)
		{
			G::AimTarget = { tTarget.m_pEntity->entindex(), I::GlobalVars->tickcount };
			G::AimPoint = { tTarget.m_vPos, I::GlobalVars->tickcount };

			G::Attacking = true;

			Aim(pCmd->viewangles, Math::CalcAngle(vLocalPos, tTarget.m_vPos), tTarget.m_vAngleTo);
			tTarget.m_vAngleTo.x = pCmd->viewangles.x; // we don't need to care about pitch
			Aim(pCmd, tTarget.m_vAngleTo);

			iLastRun = I::GlobalVars->tickcount;
		}

		break;
	}

	return true;
}