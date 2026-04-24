#include "AimbotMelee.h"

#include "../Aimbot.h"
#include "../../Simulation/MovementSimulation/MovementSimulation.h"
#include "../../EnginePrediction/EnginePrediction.h"
#include "../../Ticks/Ticks.h"
#include "../../Visuals/Visuals.h"

static inline bool AimFriendlyBuilding(CBaseObject* pBuilding)
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

static inline std::vector<Target_t> GetTargets(CTFPlayer* pLocal, CTFWeaponBase* pWeapon)
{
	std::vector<Target_t> vTargets;

	const Vec3 vLocalPos = F::Ticks.GetShootPos();
	const Vec3 vLocalAngles = I::EngineClient->GetViewAngles();

	if (Vars::Aimbot::General::Target.Value & Vars::Aimbot::General::TargetEnum::Players)
	{
		auto eGroup = !F::AimbotGlobal.FriendlyFire() || Vars::Aimbot::General::Ignore.Value & Vars::Aimbot::General::IgnoreEnum::Team ? EntityEnum::PlayerEnemy : EntityEnum::PlayerAll;
		if (Vars::Aimbot::Melee::WhipTeam.Value &&
			!F::AimbotGlobal.FriendlyFire() && SDK::AttribHookValue(0, "speed_buff_ally", pWeapon) > 0)
			eGroup = EntityEnum::PlayerAll;

		for (auto pEntity : H::Entities.GetGroup(eGroup))
		{
			if (F::AimbotGlobal.ShouldIgnore(pEntity, pLocal, pWeapon))
				continue;

			float flFOVTo; Vec3 vPos, vAngleTo;
			if (!F::AimbotGlobal.PlayerBoneInFOV(pEntity->As<CTFPlayer>(), vLocalPos, vLocalAngles, flFOVTo, vPos, vAngleTo))
				continue;

			float flDistTo = vLocalPos.DistToSqr(vPos);
			bool bTeam = pEntity->m_iTeamNum() == pLocal->m_iTeamNum();
			int iPriority = F::AimbotGlobal.GetPriority(pEntity->entindex());
			if (bTeam && !F::AimbotGlobal.FriendlyFire())
				iPriority = 0;
			vTargets.emplace_back(pEntity, TargetEnum::Player, vPos, vAngleTo, flFOVTo, flDistTo, iPriority);
		}
	}

	{
		auto eGroup = EntityEnum::Invalid;
		if (Vars::Aimbot::General::Target.Value & Vars::Aimbot::General::TargetEnum::Building)
			eGroup = EntityEnum::BuildingEnemy;
		bool bWrench = pWeapon->GetWeaponID() == TF_WEAPON_WRENCH, bSapper = SDK::AttribHookValue(0, "set_dmg_apply_to_sapper", pWeapon);
		if (Vars::Aimbot::Healing::AutoRepair.Value && (bWrench || bSapper))
			eGroup = eGroup != EntityEnum::Invalid ? EntityEnum::BuildingAll : EntityEnum::BuildingTeam;
		for (auto pEntity : H::Entities.GetGroup(eGroup))
		{
			if (F::AimbotGlobal.ShouldIgnore(pEntity, pLocal, pWeapon))
				continue;

			bool bTeam = pEntity->m_iTeamNum() == pLocal->m_iTeamNum();
			if (bTeam && (bWrench && !AimFriendlyBuilding(pEntity->As<CBaseObject>()) || bSapper && !pEntity->As<CBaseObject>()->m_bHasSapper()))
				continue;

			Vec3 vPos = pEntity->GetCenter();
			Vec3 vAngleTo = Math::CalcAngle(vLocalPos, vPos);
			float flFOVTo = Math::CalcFov(vLocalAngles, vAngleTo);
			if (flFOVTo > Vars::Aimbot::General::AimFOV.Value)
				continue;

			int iPriority = 0;
			if (bTeam)
			{
				int iOwner = pEntity->As<CBaseObject>()->m_hBuilder().GetEntryIndex();
				switch (Vars::Aimbot::Healing::HealPriority.Value)
				{
				case Vars::Aimbot::Healing::HealPriorityEnum::PrioritizeFriends:
					if (iOwner == I::EngineClient->GetLocalPlayer() || H::Entities.IsFriend(iOwner) || H::Entities.InParty(iOwner))
						iPriority = std::numeric_limits<int>::max();
					break;
				case Vars::Aimbot::Healing::HealPriorityEnum::PrioritizeTeam:
					iPriority = std::numeric_limits<int>::max();
				}
			}

			float flDistTo = vLocalPos.DistToSqr(vPos);
			vTargets.emplace_back(pEntity, pEntity->IsSentrygun() ? TargetEnum::Sentry : pEntity->IsDispenser() ? TargetEnum::Dispenser : TargetEnum::Teleporter, vPos, vAngleTo, flFOVTo, flDistTo, iPriority);
		}
	}

	if (Vars::Aimbot::General::Target.Value & Vars::Aimbot::General::TargetEnum::NPCs)
	{
		for (auto pEntity : H::Entities.GetGroup(EntityEnum::WorldNPC))
		{
			if (F::AimbotGlobal.ShouldIgnore(pEntity, pLocal, pWeapon))
				continue;

			Vec3 vPos = pEntity->GetCenter();
			Vec3 vAngleTo = Math::CalcAngle(vLocalPos, vPos);
			float flFOVTo = Math::CalcFov(vLocalAngles, vAngleTo);
			if (flFOVTo > Vars::Aimbot::General::AimFOV.Value)
				continue;

			float flDistTo = vLocalPos.DistToSqr(vPos);
			vTargets.emplace_back(pEntity, TargetEnum::NPC, vPos, vAngleTo, flFOVTo, flDistTo);
		}
	}

	return vTargets;
}



int CAimbotMelee::GetSwingTime(CTFWeaponBase* pWeapon, bool bVar)
{
	if (pWeapon->GetWeaponID() == TF_WEAPON_KNIFE)
		return 0;
	int iSmackTicks = ceilf(pWeapon->GetSmackDelay() / TICK_INTERVAL);
	if (bVar)
		iSmackTicks = std::max(iSmackTicks + Vars::Aimbot::Melee::SwingOffset.Value, 0);
	return iSmackTicks;
}

void CAimbotMelee::UpdateInfo(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd, std::vector<Target_t> vTargets)
{
	m_mRecordMap.clear(); m_mPaths.clear();
	m_iDoubletapTicks = F::Ticks.GetTicks(pWeapon);
	m_vEyePos = pLocal->GetShootPos();
	m_flRange = pWeapon->GetSwingRange();

	int iSimTicks = GetSwingTime(pWeapon), iSwingTicks = GetSwingTime(pWeapon, false);

	if ((Vars::Aimbot::Melee::SwingPrediction.Value && iSimTicks || m_iDoubletapTicks) && G::CanPrimaryAttack && pWeapon->m_flSmackTime() < 0.f)
	{
		std::unordered_map<int, MoveStorage> mMoveStorage;

		F::MoveSim.Initialize(pLocal, mMoveStorage[I::EngineClient->GetLocalPlayer()], false, !m_iDoubletapTicks);
		for (auto& tTarget : vTargets)
			F::MoveSim.Initialize(tTarget.m_pEntity, mMoveStorage[tTarget.m_pEntity->entindex()], false);

		int iMax = std::max(iSimTicks, m_iDoubletapTicks);
		int iTicks = iMax; bool bSwung = false;
		for (int i = 0; i < iTicks; i++) // intended for plocal to collide with targets
		{
			{
				auto& tMoveStorage = mMoveStorage[I::EngineClient->GetLocalPlayer()];

				if (!bSwung && (!m_iDoubletapTicks || Vars::Doubletap::AntiWarp.Value && pLocal->m_hGroundEntity() || iMax - i <= iSwingTicks))
				{
					iTicks = std::min(i + iSwingTicks, iMax), bSwung = true;
					if (!iSwingTicks)
						break;

					if (pLocal->InCond(TF_COND_SHIELD_CHARGE))
					{	// demo charge fix for swing pred
						tMoveStorage.m_MoveData.m_flMaxSpeed = tMoveStorage.m_MoveData.m_flClientMaxSpeed = SDK::MaxSpeed(pLocal, false, true);
						pLocal->m_flMaxspeed() = tMoveStorage.m_MoveData.m_flMaxSpeed;
						pLocal->RemoveCond(TF_COND_SHIELD_CHARGE);
					}
				}
				if (m_iDoubletapTicks && Vars::Doubletap::AntiWarp.Value && pLocal->m_hGroundEntity())
					F::Ticks.AntiWarp(pLocal, pCmd->viewangles.y, tMoveStorage.m_MoveData.m_flForwardMove, tMoveStorage.m_MoveData.m_flSideMove, iMax - i - 1);

				F::MoveSim.RunTick(tMoveStorage);
				m_mRecordMap[I::EngineClient->GetLocalPlayer()].emplace_front(
					pLocal->m_flSimulationTime() + TICKS_TO_TIME(i + 1),
					tMoveStorage.m_MoveData.m_vecAbsOrigin,
					pLocal->m_vecMins(), pLocal->m_vecMaxs()
				);
			}

			if (i < iSimTicks - m_iDoubletapTicks)
			{
				for (auto& tTarget : vTargets)
				{
					auto& tMoveStorage = mMoveStorage[tTarget.m_pEntity->entindex()];
					if (tMoveStorage.m_bFailed)
						continue;

					F::MoveSim.RunTick(tMoveStorage);
					m_mRecordMap[tTarget.m_pEntity->entindex()].emplace_front(
						!Vars::Aimbot::Melee::SwingPredictLag.Value || tMoveStorage.m_bPredictNetworked ? tTarget.m_pEntity->m_flSimulationTime() + TICKS_TO_TIME(i + 1) : 0.f,
						Vars::Aimbot::Melee::SwingPredictLag.Value ? tMoveStorage.m_vPredictedOrigin : tMoveStorage.m_MoveData.m_vecAbsOrigin,
						tTarget.m_pEntity->m_vecMins(), tTarget.m_pEntity->m_vecMaxs()
					);
				}
			}
		}
		m_vEyePos = mMoveStorage[I::EngineClient->GetLocalPlayer()].m_MoveData.m_vecAbsOrigin + pLocal->m_vecViewOffset();
		m_flRange = pWeapon->GetSwingRange();

		if (Vars::Visuals::Prediction::SwingLines.Value && Vars::Visuals::Prediction::PlayerPath.Value)
		{
			for (auto& [iIndex, tMoveStorage] : mMoveStorage)
				m_mPaths[iIndex] = tMoveStorage.m_vPath;

			const bool bAlwaysDraw = !Vars::Aimbot::General::AutoShoot.Value || Vars::Debug::Info.Value;
			if (bAlwaysDraw)
			{
				G::LineStorage.clear();
				G::BoxStorage.clear();
				G::PathStorage.clear();

				for (auto& vPath : m_mPaths | std::views::values)
				{
					float flDuration = Vars::Visuals::Prediction::PlayerDrawDuration.Value;
					if (Vars::Colors::PlayerPathIgnoreZ.Value.a)
						G::PathStorage.emplace_back(vPath, !flDuration ? -int(vPath.size()) : I::GlobalVars->curtime + flDuration, Vars::Colors::PlayerPathIgnoreZ.Value, Vars::Visuals::Prediction::PlayerPath.Value);
					if (Vars::Colors::PlayerPath.Value.a)
						G::PathStorage.emplace_back(vPath, !flDuration ? -int(vPath.size()) : I::GlobalVars->curtime + flDuration, Vars::Colors::PlayerPath.Value, Vars::Visuals::Prediction::PlayerPath.Value, true);
				}
			}
		}

		for (auto& tMoveStorage : mMoveStorage | std::views::values)
			F::MoveSim.Restore(tMoveStorage);
	}

	m_bShouldSwing = m_iDoubletapTicks <= iSwingTicks || Vars::Doubletap::AntiWarp.Value && pLocal->m_hGroundEntity();
}

bool CAimbotMelee::CanBackstab(CBaseEntity* pTarget, CTFPlayer* pLocal, Vec3 vEyeAngles)
{
	if (!pTarget->IsPlayer() || pTarget->m_iTeamNum() == pLocal->m_iTeamNum())
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

	Vec3 vEyePos = m_vEyePos;
	const float flCompDist = PLAYER_ORIGIN_COMPRESSION / 2;
	const float flSqCompDist = 0.0884f;

	if (auto pCmd = G::CurrentUserCmd;
		m_mRecordMap[pLocal->entindex()].empty() && pCmd->viewangles != vEyeAngles && G::CanPrimaryAttack)
	{	// repredict, prevent prediction error potentially causing miss
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

	auto fTestDots = [&](Vec3 vTargetAngles)
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
		if (!fTestDots(vTargetAngles))
			return false;
	}
	else
	{
		if (Vars::Aimbot::Melee::BackstabDoubleTest.Value && !fTestDots(vTargetAngles))
			return false;

		vTargetAngles.y += H::Entities.GetDeltaAngles(pTarget->entindex()).y;
		if (!fTestDots(vTargetAngles))
			return false;
	}

	return true;
}

int CAimbotMelee::CanHit(Target_t& tTarget, CTFPlayer* pLocal, CTFWeaponBase* pWeapon)
{
	if (Vars::Aimbot::General::Ignore.Value & Vars::Aimbot::General::IgnoreEnum::Unsimulated && H::Entities.GetChoke(tTarget.m_pEntity->entindex()) > Vars::Aimbot::General::TickTolerance.Value)
		return false;

	float flRange = SDK::AttribHookValue(m_flRange, "melee_range_multiplier", pWeapon);
	float flHull = SDK::AttribHookValue(18, "melee_bounds_multiplier", pWeapon);
	if (pLocal->m_flModelScale() > 1.0f)
	{
		flRange *= pLocal->m_flModelScale();
		flHull *= pLocal->m_flModelScale();
	}
	if (pWeapon->GetWeaponID() == TF_WEAPON_WRENCH && tTarget.m_pEntity->m_iTeamNum() == pLocal->m_iTeamNum())
	{
		flRange = 70;
		flHull = 18;
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
		F::Backtrack.m_tRecord = { tTarget.m_pEntity->m_flSimulationTime(), tTarget.m_pEntity->m_vecOrigin(), tTarget.m_pEntity->m_vecMins(), tTarget.m_pEntity->m_vecMaxs() };
		if (!tTarget.m_pEntity->SetupBones(F::Backtrack.m_tRecord.m_aBones, MAXSTUDIOBONES, BONE_USED_BY_ANYTHING, tTarget.m_pEntity->m_flSimulationTime()))
			return false;

		vRecords = { &F::Backtrack.m_tRecord };
	}

	CGameTrace trace = {};
	CTraceFilterHitscan filter = {};
	filter.pSkip = pLocal;

	for (auto pRecord : vRecords)
	{
		Vec3 vRestoreOrigin = tTarget.m_pEntity->GetAbsOrigin();
		Vec3 vRestoreMins = tTarget.m_pEntity->m_vecMins();
		Vec3 vRestoreMaxs = tTarget.m_pEntity->m_vecMaxs();

		tTarget.m_pEntity->SetAbsOrigin(pRecord->m_vOrigin);
		tTarget.m_pEntity->m_vecMins() = pRecord->m_vMins + PLAYER_ORIGIN_COMPRESSION;
		tTarget.m_pEntity->m_vecMaxs() = pRecord->m_vMaxs - PLAYER_ORIGIN_COMPRESSION;

		// possibly account melee bounds as well?
		tTarget.m_vPos = m_vEyePos.Clamp(pRecord->m_vOrigin + tTarget.m_pEntity->m_vecMins(), pRecord->m_vOrigin + tTarget.m_pEntity->m_vecMaxs());
		if (Vars::Aimbot::Melee::AutoBackstab.Value && pWeapon->GetWeaponID() == TF_WEAPON_KNIFE)
			tTarget.m_vPos.x = pRecord->m_vOrigin.x, tTarget.m_vPos.y = pRecord->m_vOrigin.y;
		Aim(G::CurrentUserCmd->viewangles, Math::CalcAngle(m_vEyePos, tTarget.m_vPos), tTarget.m_vAngleTo);

		Vec3 vForward; Math::AngleVectors(tTarget.m_vAngleTo, &vForward);
		Vec3 vTraceEnd = m_vEyePos + vForward * flRange;

		SDK::TraceHull(m_vEyePos, vTraceEnd, {}, {}, MASK_SOLID, &filter, &trace);
		bool bReturn = trace.m_pEnt == tTarget.m_pEntity;
		if (!bReturn)
		{
			SDK::TraceHull(m_vEyePos, vTraceEnd, vSwingMins, vSwingMaxs, MASK_SOLID, &filter, &trace);
			bReturn = trace.m_pEnt == tTarget.m_pEntity;
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
			auto vAngle = Math::CalcAngle(m_vEyePos, tTarget.m_vPos);

			Math::AngleVectors(vAngle, &vForward);
			vTraceEnd = m_vEyePos + vForward * flRange;

			SDK::TraceHull(m_vEyePos, vTraceEnd, vSwingMins, vSwingMaxs, MASK_SOLID, &filter, &trace);
			if (trace.m_pEnt == tTarget.m_pEntity)
				return 2;
		}
		}
	}

	return false;
}



bool CAimbotMelee::Aim(Vec3 vCurAngle, Vec3 vToAngle, Vec3& vOut, int iMethod)
{
	/*
	if (Vec3* pDoubletapAngle = F::Ticks.GetShootAngle())
	{
		vOut = *pDoubletapAngle;
		return true;
	}
	*/

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

	if (iMethod != Vars::Aimbot::General::AimTypeEnum::Silent || Vars::Misc::Game::AntiCheatCompatibility.Value)
		Math::ClampAngles(vOut);
	return bReturn;
}

// assume angle calculated outside with other overload
void CAimbotMelee::Aim(CUserCmd* pCmd, Vec3& vAngles, int iMethod)
{
	bool bUnsure = F::Ticks.IsTimingUnsure();
	switch (iMethod)
	{
	case Vars::Aimbot::General::AimTypeEnum::Plain:
		if (G::Attacking != 1 && !bUnsure)
			break;
		[[fallthrough]];
	case Vars::Aimbot::General::AimTypeEnum::Smooth:
	case Vars::Aimbot::General::AimTypeEnum::Assistive:
		pCmd->viewangles = vAngles;
		I::EngineClient->SetViewAngles(vAngles);
		break;
	case Vars::Aimbot::General::AimTypeEnum::Silent:
		if (G::Attacking == 1 || bUnsure)
		{
			SDK::FixMovement(pCmd, vAngles);
			pCmd->viewangles = vAngles;
			G::PSilentAngles = true;
		}
		break;
	case Vars::Aimbot::General::AimTypeEnum::Locking:
		SDK::FixMovement(pCmd, vAngles);
		pCmd->viewangles = vAngles;
		G::SilentAngles = true;
	}
}

static inline void DrawVisuals(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd, Target_t& tTarget, std::unordered_map<int, std::vector<Vec3>>& mPaths)
{
	G::AimTarget = { tTarget.m_pEntity->entindex(), I::GlobalVars->tickcount };
	G::AimPoint = { tTarget.m_vPos, I::GlobalVars->tickcount };

	bool bPath = Vars::Visuals::Prediction::SwingLines.Value && Vars::Visuals::Prediction::PlayerPath.Value;
	bool bLine = Vars::Visuals::Line::Enabled.Value;
	bool bBoxes = Vars::Visuals::Hitbox::BonesEnabled.Value & Vars::Visuals::Hitbox::BonesEnabledEnum::OnShot;
	if (bPath || bLine || bBoxes)
	{
		if (pCmd->buttons & IN_ATTACK && G::CanPrimaryAttack && pWeapon->m_flSmackTime() < 0.f)
		{
			G::LineStorage.clear();
			G::BoxStorage.clear();
			G::PathStorage.clear();

			if (bPath)
			{
				float flDuration = Vars::Visuals::Prediction::PlayerDrawDuration.Value;
				if (Vars::Colors::PlayerPathIgnoreZ.Value.a)
				{
					G::PathStorage.emplace_back(mPaths[I::EngineClient->GetLocalPlayer()], !flDuration ? -int(mPaths[I::EngineClient->GetLocalPlayer()].size()) : I::GlobalVars->curtime + flDuration, Vars::Colors::PlayerPathIgnoreZ.Value, Vars::Visuals::Prediction::PlayerPath.Value);
					G::PathStorage.emplace_back(mPaths[tTarget.m_pEntity->entindex()], !flDuration ? -int(mPaths[tTarget.m_pEntity->entindex()].size()) : I::GlobalVars->curtime + flDuration, Vars::Colors::PlayerPathIgnoreZ.Value, Vars::Visuals::Prediction::PlayerPath.Value);
				}
				if (Vars::Colors::PlayerPath.Value.a)
				{
					G::PathStorage.emplace_back(mPaths[I::EngineClient->GetLocalPlayer()], !flDuration ? -int(mPaths[I::EngineClient->GetLocalPlayer()].size()) : I::GlobalVars->curtime + flDuration, Vars::Colors::PlayerPath.Value, Vars::Visuals::Prediction::PlayerPath.Value, true);
					G::PathStorage.emplace_back(mPaths[tTarget.m_pEntity->entindex()], !flDuration ? -int(mPaths[tTarget.m_pEntity->entindex()].size()) : I::GlobalVars->curtime + flDuration, Vars::Colors::PlayerPath.Value, Vars::Visuals::Prediction::PlayerPath.Value, true);
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
				auto vBoxes = F::Visuals.GetHitboxes(tTarget.m_pRecord->m_aBones, tTarget.m_pEntity->As<CBaseAnimating>());
				G::BoxStorage.insert(G::BoxStorage.end(), vBoxes.begin(), vBoxes.end());

				//if (Vars::Colors::BoneHitboxEdgeIgnoreZ.Value.a || Vars::Colors::BoneHitboxFaceIgnoreZ.Value.a)
				//	G::BoxStorage.emplace_back(tTarget.m_pRecord->m_vOrigin, tTarget.m_pRecord->m_vMins, tTarget.m_pRecord->m_vMaxs, Vec3(), I::GlobalVars->curtime + Vars::Visuals::Hitbox::DrawDuration.Value, Vars::Colors::BoneHitboxEdgeIgnoreZ.Value, Vars::Colors::BoneHitboxFaceIgnoreZ.Value);
				//if (Vars::Colors::BoneHitboxEdge.Value.a || Vars::Colors::BoneHitboxFace.Value.a)
				//	G::BoxStorage.emplace_back(tTarget.m_pRecord->m_vOrigin, tTarget.m_pRecord->m_vMins, tTarget.m_pRecord->m_vMaxs, Vec3(), I::GlobalVars->curtime + Vars::Visuals::Hitbox::DrawDuration.Value, Vars::Colors::BoneHitboxEdge.Value, Vars::Colors::BoneHitboxFace.Value, true);
			}
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

	auto vTargets = F::AimbotGlobal.ManageTargets(GetTargets, pLocal, pWeapon, Vars::Aimbot::General::TargetSelectionEnum::Distance);
	if (vTargets.empty())
		return;

	//if (!G::AimTarget.m_iEntIndex)
	//	G::AimTarget = { vTargets.front().m_pEntity->entindex(), I::GlobalVars->tickcount, 0 };

	UpdateInfo(pLocal, pWeapon, pCmd, vTargets);
	for (auto& tTarget : vTargets)
	{
		const auto iResult = CanHit(tTarget, pLocal, pWeapon);
		if (!iResult) continue;
		if (iResult == 2)
		{
			G::AimTarget = { tTarget.m_pEntity->entindex(), I::GlobalVars->tickcount, 0 };
			Aim(pCmd, tTarget.m_vAngleTo);
			break;
		}

		if (Vars::Aimbot::General::AutoShoot.Value && pWeapon->m_flSmackTime() < 0.f)
		{
			if (m_bShouldSwing)
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
			m_vEyePos = pLocal->GetShootPos();
			Aim(G::CurrentUserCmd->viewangles, Math::CalcAngle(m_vEyePos, tTarget.m_vPos), tTarget.m_vAngleTo);
		}
		DrawVisuals(pLocal, pWeapon, pCmd, tTarget, m_mPaths);

		Aim(pCmd, tTarget.m_vAngleTo);
		break;
	}
}

static inline int GetAttachment(CBaseObject* pBuilding, int i)
{
	int iAttachment = pBuilding->GetBuildPointAttachmentIndex(i);
	if (pBuilding->IsSentrygun() && pBuilding->m_iUpgradeLevel() > 1)
		iAttachment = 3; // idk why this is needed
	return iAttachment;
}
bool CAimbotMelee::FindNearestBuildPoint(CBaseObject* pBuilding, CTFPlayer* pLocal, Vec3& vPoint)
{
	bool bFoundPoint = false;

	m_vEyePos = pLocal->GetShootPos();
	static auto tf_obj_max_attach_dist = H::ConVars.FindVar("tf_obj_max_attach_dist");
	float flNearestPoint = tf_obj_max_attach_dist->GetFloat();

	for (int i = 0; i < pBuilding->GetNumBuildPoints(); i++)
	{
		Vector vOrigin;
		if (pBuilding->GetAttachment(GetAttachment(pBuilding, i), vOrigin))
		{
			if (!SDK::VisPos(pLocal, pBuilding, m_vEyePos, vOrigin))
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

	const Vec3 vLocalPos = F::Ticks.GetShootPos();
	const Vec3 vLocalAngles = I::EngineClient->GetViewAngles();

	std::vector<Target_t> vTargets;
	for (auto pEntity : H::Entities.GetGroup(EntityEnum::BuildingEnemy))
	{
		if (F::AimbotGlobal.ShouldIgnore(pEntity, pLocal, pWeapon))
			continue;

		auto pBuilding = pEntity->As<CBaseObject>();
		if (pBuilding->m_bHasSapper() || !pBuilding->IsInValidTeam())
			continue;

		Vec3 vPoint;
		if (!FindNearestBuildPoint(pBuilding, pLocal, vPoint))
			continue;

		Vec3 vAngleTo = Math::CalcAngle(vLocalPos, vPoint);
		const float flFOVTo = Math::CalcFov(vLocalAngles, vAngleTo);
		const float flDistTo = vLocalPos.DistToSqr(vPoint);

		if (flFOVTo > Vars::Aimbot::General::AimFOV.Value)
			continue;

		vTargets.emplace_back(pBuilding, TargetEnum::Unknown, vPoint, vAngleTo, flFOVTo, flDistTo);
	}
	F::AimbotGlobal.SortTargetsPre(vTargets, Vars::Aimbot::General::TargetSelectionEnum::Distance);
	if (vTargets.empty())
		return true;

	auto& tTarget = vTargets.front();

	bool bShouldAim = true;
	if (Vars::Aimbot::General::AutoShoot.Value)
		pCmd->buttons |= IN_ATTACK;
	else
		bShouldAim = pCmd->buttons & IN_ATTACK;
	if (Vars::Aimbot::General::AimType.Value == Vars::Aimbot::General::AimTypeEnum::Silent)
		bShouldAim &= !I::ClientState->chokedcommands && F::Ticks.CanChoke(true);
		
	if (bShouldAim)
	{
		G::AimTarget = { tTarget.m_pEntity->entindex(), I::GlobalVars->tickcount };
		G::AimPoint = { tTarget.m_vPos, I::GlobalVars->tickcount };

		G::Attacking = true;

		Aim(pCmd->viewangles, Math::CalcAngle(m_vEyePos, tTarget.m_vPos), tTarget.m_vAngleTo);
		tTarget.m_vAngleTo.x = pCmd->viewangles.x; // we don't need to care about pitch
		Aim(pCmd, tTarget.m_vAngleTo);
	}

	return true;
}