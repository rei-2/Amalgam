#include "AimbotMelee.h"

#include "../../Simulation/MovementSimulation/MovementSimulation.h"
#include "../../TickHandler/TickHandler.h"
#include "../../Visuals/Visuals.h"

std::vector<Target_t> CAimbotMelee::GetTargets(CTFPlayer* pLocal, CTFWeaponBase* pWeapon)
{
	std::vector<Target_t> vTargets;

	const Vec3 vLocalPos = F::Ticks.GetShootPos();
	const Vec3 vLocalAngles = I::EngineClient->GetViewAngles();

	if (Vars::Aimbot::General::Target.Value & Vars::Aimbot::General::TargetEnum::Players)
	{
		bool bDisciplinary = Vars::Aimbot::Melee::WhipTeam.Value && pWeapon->m_iItemDefinitionIndex() == Soldier_t_TheDisciplinaryAction;
		for (auto pEntity : H::Entities.GetGroup(bDisciplinary ? EGroupType::PLAYERS_ALL : EGroupType::PLAYERS_ENEMIES))
		{
			bool bTeammate = pEntity->m_iTeamNum() == pLocal->m_iTeamNum();
			if (F::AimbotGlobal.ShouldIgnore(pEntity, pLocal, pWeapon))
				continue;

			float flFOVTo; Vec3 vPos, vAngleTo;
			if (!F::AimbotGlobal.PlayerBoneInFOV(pEntity->As<CTFPlayer>(), vLocalPos, vLocalAngles, flFOVTo, vPos, vAngleTo))
				continue;

			float flDistTo = vLocalPos.DistTo(vPos);
			vTargets.push_back({ pEntity, TargetEnum::Player, vPos, vAngleTo, flFOVTo, flDistTo, bTeammate ? 0 : F::AimbotGlobal.GetPriority(pEntity->entindex()) });
		}
	}

	if (Vars::Aimbot::General::Target.Value)
	{
		bool bWrench = pWeapon->GetWeaponID() == TF_WEAPON_WRENCH, bDestroySapper = false;
		switch (pWeapon->m_iItemDefinitionIndex())
		{
		case Pyro_t_Homewrecker:
		case Pyro_t_TheMaul:
		case Pyro_t_NeonAnnihilator:
		case Pyro_t_NeonAnnihilatorG:
			bDestroySapper = true;
		}

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
			vTargets.push_back({ pEntity, pEntity->IsSentrygun() ? TargetEnum::Sentry : pEntity->IsDispenser() ? TargetEnum::Dispenser : TargetEnum::Teleporter, vPos, vAngleTo, flFOVTo, flDistTo });
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
			vTargets.push_back({ pEntity, TargetEnum::NPC, vPos, vAngleTo, flFOVTo, flDistTo });
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

	F::AimbotGlobal.SortTargets(&vTargets, Vars::Aimbot::General::TargetSelectionEnum::Distance);
	vTargets.resize(std::min(size_t(Vars::Aimbot::General::MaxTargets.Value), vTargets.size()));
	F::AimbotGlobal.SortPriority(&vTargets);
	return vTargets;
}



int CAimbotMelee::GetSwingTime(CTFWeaponBase* pWeapon)
{
	if (pWeapon->GetWeaponID() == TF_WEAPON_KNIFE)
		return 0;
	return Vars::Aimbot::Melee::SwingTicks.Value;
}

void CAimbotMelee::SimulatePlayers(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, std::vector<Target_t> targets,
								   Vec3& vEyePos, std::unordered_map<CBaseEntity*, std::deque<TickRecord>>& pRecordMap,
								   std::unordered_map<CBaseEntity*, std::deque<Vec3>>& mPaths)
{
	// swing prediction / auto warp
	const int iSwingTicks = GetSwingTime(pWeapon);
	int iMax = (iDoubletapTicks && Vars::CL_Move::Doubletap::AntiWarp.Value && pLocal->m_hGroundEntity())
		? std::max(iSwingTicks - Vars::CL_Move::Doubletap::TickLimit.Value - 1, 0)
		: std::max(iSwingTicks, iDoubletapTicks);

	if ((Vars::Aimbot::Melee::SwingPrediction.Value || iDoubletapTicks) && pWeapon->m_flSmackTime() < 0.f && iMax)
	{
		PlayerStorage localStorage;
		std::unordered_map<CBaseEntity*, PlayerStorage> targetStorage;

		F::MoveSim.Initialize(pLocal, localStorage, false, iDoubletapTicks);
		for (auto& target : targets)
			F::MoveSim.Initialize(target.m_pEntity, targetStorage[target.m_pEntity], false);

		for (int i = 0; i < iMax; i++) // intended for plocal to collide with targets
		{
			if (i < iMax)
			{
				if (pLocal->InCond(TF_COND_SHIELD_CHARGE) && iMax - i <= GetSwingTime(pWeapon)) // demo charge fix for swing pred
					localStorage.m_MoveData.m_flMaxSpeed = localStorage.m_MoveData.m_flClientMaxSpeed = SDK::MaxSpeed(pLocal, false, true);
				F::MoveSim.RunTick(localStorage);
			}
			if (i < iSwingTicks - iDoubletapTicks)
			{
				for (auto& target : targets)
				{
					F::MoveSim.RunTick(targetStorage[target.m_pEntity]);
					if (!targetStorage[target.m_pEntity].m_bFailed)
						pRecordMap[target.m_pEntity].push_front({
							!Vars::Aimbot::Melee::SwingPredictLag.Value || targetStorage[target.m_pEntity].m_bPredictNetworked ? target.m_pEntity->m_flSimulationTime() + TICKS_TO_TIME(i + 1) : 0.f,
							{},
							Vars::Aimbot::Melee::SwingPredictLag.Value ? targetStorage[target.m_pEntity].m_vPredictedOrigin : targetStorage[target.m_pEntity].m_MoveData.m_vecAbsOrigin
						});
				}
			}
		}
		vEyePos = localStorage.m_MoveData.m_vecAbsOrigin + pLocal->m_vecViewOffset();

		if (Vars::Visuals::Simulation::SwingLines.Value && Vars::Visuals::Simulation::PlayerPath.Value)
		{
			const bool bAlwaysDraw = !Vars::Aimbot::General::AutoShoot.Value || Vars::Debug::Info.Value;
			if (!bAlwaysDraw)
			{
				mPaths[pLocal] = localStorage.m_vPath;
				for (auto& target : targets)
					mPaths[target.m_pEntity] = targetStorage[target.m_pEntity].m_vPath;
			}
			else
			{
				G::LineStorage.clear();
				G::BoxStorage.clear();
				G::PathStorage.clear();

				if (Vars::Colors::PlayerPath.Value.a)
				{
					G::PathStorage.push_back({ localStorage.m_vPath, I::GlobalVars->curtime + 5.f, Vars::Colors::PlayerPath.Value, Vars::Visuals::Simulation::PlayerPath.Value });
					for (auto& target : targets)
						G::PathStorage.push_back({ targetStorage[target.m_pEntity].m_vPath, I::GlobalVars->curtime + 5.f, Vars::Colors::PlayerPath.Value, Vars::Visuals::Simulation::PlayerPath.Value });
				}
				if (Vars::Colors::PlayerPathClipped.Value.a)
				{
					G::PathStorage.push_back({ localStorage.m_vPath, I::GlobalVars->curtime + 5.f, Vars::Colors::PlayerPathClipped.Value, Vars::Visuals::Simulation::PlayerPath.Value, true });
					for (auto& target : targets)
						G::PathStorage.push_back({ targetStorage[target.m_pEntity].m_vPath, I::GlobalVars->curtime + 5.f, Vars::Colors::PlayerPathClipped.Value, Vars::Visuals::Simulation::PlayerPath.Value, true });
				}
			}
		}

		F::MoveSim.Restore(localStorage);
		for (auto& target : targets)
			F::MoveSim.Restore(targetStorage[target.m_pEntity]);
	}
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

	Vec3 vToTarget = pTarget->GetAbsOrigin() - pLocal->m_vecOrigin();
	vToTarget.z = 0.f;
	const float flDist = vToTarget.Length();
	if (!flDist)
		return false;

	vToTarget.Normalize();
	float flTolerance = 0.0625f;
	float flExtra = 2.f * flTolerance / flDist; // account for origin tolerance

	float flPosVsTargetViewMinDot = 0.f + 0.0031f + flExtra;
	float flPosVsOwnerViewMinDot = 0.5f + flExtra;
	float flViewAnglesMinDot = -0.3f + 0.0031f; // 0.00306795676297 ?

	auto TestDots = [&](Vec3 vTargetAngles)
		{
			Vec3 vOwnerForward; Math::AngleVectors(vEyeAngles, &vOwnerForward);
			vOwnerForward.z = 0.f;
			vOwnerForward.Normalize();

			Vec3 vTargetForward; Math::AngleVectors(vTargetAngles, &vTargetForward);
			vTargetForward.z = 0.f;
			vTargetForward.Normalize();

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

int CAimbotMelee::CanHit(Target_t& target, CTFPlayer* pLocal, CTFWeaponBase* pWeapon, Vec3 vEyePos, std::deque<TickRecord>& vSimRecords)
{
	if (Vars::Aimbot::General::Ignore.Value & Vars::Aimbot::General::IgnoreEnum::Unsimulated && H::Entities.GetChoke(target.m_pEntity->entindex()) > Vars::Aimbot::General::TickTolerance.Value)
		return false;

	float flHull = SDK::AttribHookValue(18, "melee_bounds_multiplier", pWeapon);
	float flRange = SDK::AttribHookValue(pWeapon->GetSwingRange(pLocal), "melee_range_multiplier", pWeapon);
	if (pLocal->m_flModelScale() > 1.0f)
	{
		flRange *= pLocal->m_flModelScale();
		flHull *= pLocal->m_flModelScale();
	}
	Vec3 vSwingMins = { -flHull, -flHull, -flHull };
	Vec3 vSwingMaxs = { flHull, flHull, flHull };

	std::deque<TickRecord> vRecords;
	{
		auto pRecords = F::Backtrack.GetRecords(target.m_pEntity);
		if (pRecords && target.m_iTargetType == TargetEnum::Player)
		{
			if (Vars::Backtrack::Enabled.Value)
				vRecords = *pRecords;
			else
			{
				vRecords = F::Backtrack.GetValidRecords(pRecords, pLocal);
				if (!vRecords.empty())
					vRecords = { vRecords.front() };
			}
		}
		if (!pRecords || vRecords.empty())
		{
			if (auto pBones = H::Entities.GetBones(target.m_pEntity->entindex()))
				vRecords.push_front({ target.m_pEntity->m_flSimulationTime(), *reinterpret_cast<BoneMatrix*>(pBones), target.m_pEntity->m_vecOrigin(),
					target.m_pEntity->m_vecMins(), target.m_pEntity->m_vecMaxs() });
			else
			{
				matrix3x4 aBones[MAXSTUDIOBONES];
				if (!target.m_pEntity->SetupBones(aBones, MAXSTUDIOBONES, BONE_USED_BY_ANYTHING, target.m_pEntity->m_flSimulationTime()))
					return false;

				vRecords.push_front({ target.m_pEntity->m_flSimulationTime(), *reinterpret_cast<BoneMatrix*>(&aBones), target.m_pEntity->m_vecOrigin(),
					target.m_pEntity->m_vecMins(), target.m_pEntity->m_vecMaxs() });
			}
		}
	}
	if (!vSimRecords.empty())
	{
		for (TickRecord& tRecord : vSimRecords)
		{
			vRecords.pop_back();
			vRecords.push_front({ tRecord.m_flSimTime, {}, tRecord.m_vOrigin, target.m_pEntity->m_vecMins(), target.m_pEntity->m_vecMaxs() });
		}
		for (TickRecord& tRecord : vRecords)
			tRecord.m_flSimTime -= TICKS_TO_TIME(vSimRecords.size());
	}
	vRecords = target.m_iTargetType == TargetEnum::Player ? F::Backtrack.GetValidRecords(&vRecords, pLocal, true) : vRecords;
	if (!Vars::Backtrack::Enabled.Value && !vRecords.empty())
		vRecords = { vRecords.front() };

	CGameTrace trace = {};
	CTraceFilterHitscan filter = {}; filter.pSkip = pLocal;
	for (auto& tRecord : vRecords)
	{
		Vec3 vRestoreOrigin = target.m_pEntity->GetAbsOrigin();
		Vec3 vRestoreMins = target.m_pEntity->m_vecMins();
		Vec3 vRestoreMaxs = target.m_pEntity->m_vecMaxs();

		target.m_pEntity->SetAbsOrigin(tRecord.m_vOrigin);
		target.m_pEntity->m_vecMins() = tRecord.m_vMins + 0.125f; // account for origin tolerance
		target.m_pEntity->m_vecMaxs() = tRecord.m_vMaxs - 0.125f;

		Vec3 vDiff = { 0, 0, std::clamp(vEyePos.z - tRecord.m_vOrigin.z, target.m_pEntity->m_vecMins().z, target.m_pEntity->m_vecMaxs().z) };
		target.m_vPos = tRecord.m_vOrigin + vDiff;
		target.m_vAngleTo = Aim(G::CurrentUserCmd->viewangles, Math::CalcAngle(vEyePos, target.m_vPos));

		Vec3 vForward; Math::AngleVectors(target.m_vAngleTo, &vForward);
		Vec3 vTraceEnd = vEyePos + (vForward * flRange);

		SDK::TraceHull(vEyePos, vTraceEnd, {}, {}, MASK_SOLID, &filter, &trace);
		bool bReturn = trace.m_pEnt && trace.m_pEnt == target.m_pEntity;
		if (!bReturn)
		{
			SDK::TraceHull(vEyePos, vTraceEnd, vSwingMins, vSwingMaxs, MASK_SOLID, &filter, &trace);
			bReturn = trace.m_pEnt && trace.m_pEnt == target.m_pEntity;
		}

		if (bReturn && Vars::Aimbot::Melee::AutoBackstab.Value && pWeapon->GetWeaponID() == TF_WEAPON_KNIFE)
		{
			if (target.m_iTargetType == TargetEnum::Player)
				bReturn = CanBackstab(target.m_pEntity, pLocal, target.m_vAngleTo);
			else
				bReturn = false;
		}

		target.m_pEntity->SetAbsOrigin(vRestoreOrigin);
		target.m_pEntity->m_vecMins() = vRestoreMins;
		target.m_pEntity->m_vecMaxs() = vRestoreMaxs;

		if (bReturn)
		{
			target.m_tRecord = tRecord;
			target.m_bBacktrack = target.m_iTargetType == TargetEnum::Player /*&& Vars::Backtrack::Enabled.Value*/;

			return true;
		}
		else if (Vars::Aimbot::General::AimType.Value == Vars::Aimbot::General::AimTypeEnum::Smooth)
		{
			auto vAngle = Math::CalcAngle(vEyePos, target.m_vPos);

			Vec3 vForward = Vec3(); Math::AngleVectors(vAngle, &vForward);
			Vec3 vTraceEnd = vEyePos + (vForward * flRange);

			SDK::Trace(vEyePos, vTraceEnd, MASK_SHOT | CONTENTS_GRATE, &filter, &trace);
			if (trace.m_pEnt && trace.m_pEnt == target.m_pEntity)
				return 2;
		}
	}

	return false;
}



Vec3 CAimbotMelee::Aim(Vec3 vCurAngle, Vec3 vToAngle, int iMethod)
{
	Vec3 vReturn = {};

	Math::ClampAngles(vToAngle);

	switch (iMethod)
	{
	case Vars::Aimbot::General::AimTypeEnum::Plain:
	case Vars::Aimbot::General::AimTypeEnum::Silent:
	case Vars::Aimbot::General::AimTypeEnum::Locking:
		vReturn = vToAngle;
		break;
	case Vars::Aimbot::General::AimTypeEnum::Smooth:
	{
		auto shortDist = [](const float flAngleA, const float flAngleB)
			{
				const float flDelta = fmodf((flAngleA - flAngleB), 360.f);
				return fmodf(2 * flDelta, 360.f) - flDelta;
			};
		const float t = 1.f - Vars::Aimbot::General::Smoothing.Value / 100.f;
		vReturn.x = vCurAngle.x - shortDist(vCurAngle.x, vToAngle.x) * t;
		vReturn.y = vCurAngle.y - shortDist(vCurAngle.y, vToAngle.y) * t;
		break;
	}
	}

	return vReturn;
}

// assume angle calculated outside with other overload
void CAimbotMelee::Aim(CUserCmd* pCmd, Vec3& vAngle)
{
	switch (Vars::Aimbot::General::AimType.Value)
	{
	case Vars::Aimbot::General::AimTypeEnum::Plain:
	case Vars::Aimbot::General::AimTypeEnum::Smooth:
		pCmd->viewangles = vAngle;
		I::EngineClient->SetViewAngles(vAngle);
		break;
	case Vars::Aimbot::General::AimTypeEnum::Silent:
	{
		bool bDoubleTap = F::Ticks.m_bDoubletap || F::Ticks.GetTicks(H::Entities.GetWeapon()) || F::Ticks.m_bSpeedhack;
		if (G::Attacking == 1 || bDoubleTap)
		{
			SDK::FixMovement(pCmd, vAngle);
			pCmd->viewangles = vAngle;
			G::PSilentAngles = true;
		}
		break;
	}
	case Vars::Aimbot::General::AimTypeEnum::Locking:
	{
		SDK::FixMovement(pCmd, vAngle);
		pCmd->viewangles = vAngle;
	}
	}
}

void CAimbotMelee::Run(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd)
{
	static int iAimType = 0;
	if (pWeapon->m_flSmackTime() < 0.f)
		iAimType = Vars::Aimbot::General::AimType.Value;
	else if (iAimType)
		Vars::Aimbot::General::AimType.Value = iAimType;

	if (Vars::Aimbot::General::AimHoldsFire.Value == Vars::Aimbot::General::AimHoldsFireEnum::Always && !G::CanPrimaryAttack && G::LastUserCmd->buttons & IN_ATTACK && Vars::Aimbot::General::AimType.Value)
		pCmd->buttons |= IN_ATTACK;
	if (!Vars::Aimbot::General::AimType.Value || Vars::Aimbot::General::AimType.Value == Vars::Aimbot::General::AimTypeEnum::Silent && !G::CanPrimaryAttack && pWeapon->m_flSmackTime() < 0.f)
		return;

	if (RunSapper(pLocal, pWeapon, pCmd))
		return;

	auto targets = SortTargets(pLocal, pWeapon);
	if (targets.empty())
		return;

	iDoubletapTicks = F::Ticks.GetTicks(pWeapon);
	const bool bShouldSwing = iDoubletapTicks <= (GetSwingTime(pWeapon) ? 14 : 0) || Vars::CL_Move::Doubletap::AntiWarp.Value && pLocal->m_hGroundEntity();

	Vec3 vEyePos = pLocal->GetShootPos();
	std::unordered_map<CBaseEntity*, std::deque<TickRecord>> pRecordMap;
	std::unordered_map<CBaseEntity*, std::deque<Vec3>> mPaths;
	SimulatePlayers(pLocal, pWeapon, targets, vEyePos, pRecordMap, mPaths);

	for (auto& target : targets)
	{
		const auto iResult = CanHit(target, pLocal, pWeapon, vEyePos, pRecordMap[target.m_pEntity]);
		if (!iResult) continue;
		if (iResult == 2)
		{
			Aim(pCmd, target.m_vAngleTo);
			break;
		}

		G::Target = { target.m_pEntity->entindex(), I::GlobalVars->tickcount };
		G::AimPosition = { target.m_vPos, I::GlobalVars->tickcount };

		if (Vars::Aimbot::General::AutoShoot.Value && pWeapon->m_flSmackTime() < 0.f)
		{
			if (bShouldSwing)
				pCmd->buttons |= IN_ATTACK;
			if (iDoubletapTicks)
				F::Ticks.m_bDoubletap = true;
		}

		G::Attacking = SDK::IsAttacking(pLocal, pWeapon, pCmd, true);

		if (G::Attacking == 1)
		{
			if (target.m_bBacktrack)
				pCmd->tick_count = TIME_TO_TICKS(target.m_tRecord.m_flSimTime) + TIME_TO_TICKS(F::Backtrack.m_flFakeInterp);
			// bug: fast old records seem to be progressively more unreliable ?
		}
		else
		{
			Vec3 vEyePos = pLocal->GetShootPos();
			target.m_vAngleTo = Aim(G::CurrentUserCmd->viewangles, Math::CalcAngle(vEyePos, target.m_vPos));
		}

		bool bPath = Vars::Visuals::Simulation::SwingLines.Value && Vars::Visuals::Simulation::PlayerPath.Value && Vars::Aimbot::General::AutoShoot.Value && !Vars::Debug::Info.Value;
		bool bLine = Vars::Visuals::Line::Enabled.Value;
		bool bBoxes = Vars::Visuals::Hitbox::BonesEnabled.Value & Vars::Visuals::Hitbox::BonesEnabledEnum::OnShot;
		if (pCmd->buttons & IN_ATTACK && pWeapon->m_flSmackTime() < 0.f && bPath)
		{
			G::LineStorage.clear();
			G::BoxStorage.clear();
			G::PathStorage.clear();

			if (bPath)
			{
				if (Vars::Colors::PlayerPath.Value.a)
				{
					G::PathStorage.push_back({ mPaths[pLocal], I::GlobalVars->curtime + Vars::Visuals::Simulation::DrawDuration.Value, Vars::Colors::PlayerPath.Value, Vars::Visuals::Simulation::PlayerPath.Value });
					G::PathStorage.push_back({ mPaths[target.m_pEntity], I::GlobalVars->curtime + Vars::Visuals::Simulation::DrawDuration.Value, Vars::Colors::PlayerPath.Value, Vars::Visuals::Simulation::PlayerPath.Value });
				}
				if (Vars::Colors::PlayerPathClipped.Value.a)
				{
					G::PathStorage.push_back({ mPaths[pLocal], I::GlobalVars->curtime + Vars::Visuals::Simulation::DrawDuration.Value, Vars::Colors::PlayerPathClipped.Value, Vars::Visuals::Simulation::PlayerPath.Value, true });
					G::PathStorage.push_back({ mPaths[target.m_pEntity], I::GlobalVars->curtime + Vars::Visuals::Simulation::DrawDuration.Value, Vars::Colors::PlayerPathClipped.Value, Vars::Visuals::Simulation::PlayerPath.Value, true });
				}
			}
		}
		if (G::Attacking == 1 && (bLine || bBoxes))
		{
			G::LineStorage.clear();
			G::BoxStorage.clear();

			if (bLine)
			{
				Vec3 vEyePos = pLocal->GetShootPos();
				float flDist = vEyePos.DistTo(target.m_vPos);
				Vec3 vForward; Math::AngleVectors(target.m_vAngleTo + pLocal->m_vecPunchAngle(), &vForward);

				if (Vars::Colors::Line.Value.a)
					G::LineStorage.push_back({ { vEyePos, vEyePos + vForward * flDist }, I::GlobalVars->curtime + Vars::Visuals::Simulation::DrawDuration.Value, Vars::Colors::Line.Value });
				if (Vars::Colors::LineClipped.Value.a)
					G::LineStorage.push_back({ { vEyePos, vEyePos + vForward * flDist }, I::GlobalVars->curtime + Vars::Visuals::Simulation::DrawDuration.Value, Vars::Colors::LineClipped.Value, true });
			}
			if (bBoxes)
			{
				auto vBoxes = F::Visuals.GetHitboxes(target.m_tRecord.m_BoneMatrix.m_aBones, target.m_pEntity->As<CBaseAnimating>());
				G::BoxStorage.insert(G::BoxStorage.end(), vBoxes.begin(), vBoxes.end());
			}
		}

		Aim(pCmd, target.m_vAngleTo);
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

	static auto tf_obj_max_attach_dist = U::ConVars.FindVar("tf_obj_max_attach_dist");
	float flNearestPoint = tf_obj_max_attach_dist ? tf_obj_max_attach_dist->GetFloat() : 160.f;
	for (int i = 0; i < pBuilding->GetNumBuildPoints(); i++)
	{
		int v = GetAttachment(pBuilding, i);

		Vec3 vOrigin;
		if (pBuilding->GetAttachment(v, vOrigin)) // issues using pBuilding->GetBuildPoint i on sentries above level 1 for some reason
		{
			if (!SDK::VisPos(pLocal, pBuilding, pLocal->GetShootPos(), vOrigin))
				continue;

			float flDist = (vOrigin - pLocal->GetAbsOrigin()).Length();
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

	std::vector<Target_t> validTargets;

	const Vec3 vLocalPos = pLocal->GetShootPos();
	const Vec3 vLocalAngles = I::EngineClient->GetViewAngles();
	for (auto pEntity : H::Entities.GetGroup(EGroupType::BUILDINGS_ENEMIES))
	{
		auto pBuilding = pEntity->As<CBaseObject>();
		if (pBuilding->m_bHasSapper() || pBuilding->m_iTeamNum() != 2 && pBuilding->m_iTeamNum() != 3)
			continue;

		Vec3 vPoint;
		if (!FindNearestBuildPoint(pBuilding, pLocal, vPoint))
			continue;

		Vec3 vAngleTo = Math::CalcAngle(vLocalPos, vPoint);
		const float flFOVTo = Math::CalcFov(vLocalAngles, vAngleTo);
		const float flDistTo = vLocalPos.DistTo(vPoint);

		if (flFOVTo > Vars::Aimbot::General::AimFOV.Value)
			continue;

		validTargets.push_back({ pBuilding, TargetEnum::Unknown, vPoint, vAngleTo, flFOVTo, flDistTo });
	}

	F::AimbotGlobal.SortTargets(&validTargets, Vars::Aimbot::General::TargetSelectionEnum::Distance);
	for (auto& target : validTargets)
	{
		static int iLastRun = 0;

		bool bShouldAim = (Vars::Aimbot::General::AimType.Value == Vars::Aimbot::General::AimTypeEnum::Silent ? iLastRun != I::GlobalVars->tickcount - 1 || G::PSilentAngles && !F::Ticks.CanChoke() : true) && Vars::Aimbot::General::AutoShoot.Value;
		pCmd->buttons |= IN_ATTACK;

		if (bShouldAim)
		{
			G::Attacking = true;
			target.m_vAngleTo = Aim(pCmd->viewangles, Math::CalcAngle(vLocalPos, target.m_vPos));
			target.m_vAngleTo.x = pCmd->viewangles.x; // we don't need to care about pitch
			Aim(pCmd, target.m_vAngleTo);

			iLastRun = I::GlobalVars->tickcount;
		}

		break;
	}

	return true;
}