#include "AimbotMelee.h"

#include "../../Simulation/MovementSimulation/MovementSimulation.h"
#include "../../TickHandler/TickHandler.h"
#include "../../Backtrack/Backtrack.h"
#include "../../Visuals/Visuals.h"
#include "../../EnginePrediction/EnginePrediction.h"

std::vector<Target_t> CAimbotMelee::GetTargets(CTFPlayer* pLocal, CTFWeaponBase* pWeapon)
{
	std::vector<Target_t> validTargets;

	const Vec3 vLocalPos = pLocal->GetShootPos();
	const Vec3 vLocalAngles = I::EngineClient->GetViewAngles();
	if (lockedTarget.m_pEntity)
	{
		auto pPlayer = lockedTarget.m_pEntity->As<CTFPlayer>();
		if (!pPlayer->IsAlive() || pPlayer->IsAGhost())
			lockedTarget.m_pEntity = nullptr;
		else
		{
			Vec3 vPos = pPlayer->GetCenter();
			Vec3 vAngleTo = Math::CalcAngle(vLocalPos, vPos);

			const float flDistTo = vLocalPos.DistTo(vPos);
			validTargets.push_back({ lockedTarget.m_pEntity, lockedTarget.m_TargetType, vPos, vAngleTo, lockedTarget.m_flFOVTo, flDistTo, lockedTarget.m_nPriority });

			return validTargets;
		}
	}

	if (Vars::Aimbot::General::Target.Value & PLAYER)
	{
		const bool bDisciplinary = Vars::Aimbot::Melee::WhipTeam.Value && pWeapon->m_iItemDefinitionIndex() == Soldier_t_TheDisciplinaryAction;
		for (auto pEntity : H::Entities.GetGroup(bDisciplinary ? EGroupType::PLAYERS_ALL : EGroupType::PLAYERS_ENEMIES))
		{
			auto pPlayer = pEntity->As<CTFPlayer>();
			if (pPlayer == pLocal || !pPlayer->IsAlive() || pPlayer->IsAGhost())
				continue;

			if (F::AimbotGlobal.ShouldIgnore(pPlayer, pLocal, pWeapon))
				continue;

			Vec3 vPos = pPlayer->GetCenter();
			Vec3 vAngleTo = Math::CalcAngle(vLocalPos, vPos);
			const float flFOVTo = Math::CalcFov(vLocalAngles, vAngleTo);

			if (flFOVTo > Vars::Aimbot::General::AimFOV.Value)
				continue;

			const float flDistTo = vLocalPos.DistTo(vPos);
			const int priority = F::AimbotGlobal.GetPriority(pPlayer->entindex());
			validTargets.push_back({ pPlayer, ETargetType::PLAYER, vPos, vAngleTo, flFOVTo, flDistTo, priority });
		}
	}

	if (Vars::Aimbot::General::Target.Value)
	{
		const bool hasWrench = (pWeapon->m_iWeaponID() == TF_WEAPON_WRENCH);
		const bool canDestroySapper = (G::WeaponDefIndex == Pyro_t_Homewrecker ||
									   G::WeaponDefIndex == Pyro_t_TheMaul ||
									   G::WeaponDefIndex == Pyro_t_NeonAnnihilator ||
									   G::WeaponDefIndex == Pyro_t_NeonAnnihilatorG);

		for (auto pEntity : H::Entities.GetGroup(hasWrench || canDestroySapper ? EGroupType::BUILDINGS_ALL : EGroupType::BUILDINGS_ENEMIES))
		{
			auto pBuilding = pEntity->As<CBaseObject>();

			bool isSentry = pBuilding->IsSentrygun(), isDispenser = pBuilding->IsDispenser(), isTeleporter = pBuilding->IsTeleporter();

			if (!(Vars::Aimbot::General::Target.Value & SENTRY) && isSentry)
				continue;
			if (!(Vars::Aimbot::General::Target.Value & DISPENSER) && isDispenser)
				continue;
			if (!(Vars::Aimbot::General::Target.Value & TELEPORTER) && isTeleporter)
				continue;

			if (pBuilding->m_iTeamNum() == pLocal->m_iTeamNum())
			{
				if (hasWrench)
				{
					if (!AimFriendlyBuilding(pBuilding))
						continue;
				}

				if (canDestroySapper)
				{
					if (!pBuilding->m_bHasSapper())
						continue;
				}
			}

			Vec3 vPos = pBuilding->GetCenter();
			Vec3 vAngleTo = Math::CalcAngle(vLocalPos, vPos);
			const float flFOVTo = Math::CalcFov(vLocalAngles, vAngleTo);
			const float flDistTo = vLocalPos.DistTo(vPos);

			if (flFOVTo > Vars::Aimbot::General::AimFOV.Value)
				continue;

			validTargets.push_back({ pBuilding, isSentry ? ETargetType::SENTRY : (isDispenser ? ETargetType::DISPENSER : ETargetType::TELEPORTER), vPos, vAngleTo, flFOVTo, flDistTo });
		}
	}

	if (Vars::Aimbot::General::Target.Value & NPC)
	{
		for (auto pNPC : H::Entities.GetGroup(EGroupType::WORLD_NPC))
		{
			Vec3 vPos = pNPC->GetCenter();
			Vec3 vAngleTo = Math::CalcAngle(vLocalPos, vPos);

			const float flFOVTo = Math::CalcFov(vLocalAngles, vAngleTo);
			const float flDistTo = vLocalPos.DistTo(vPos);

			if (flFOVTo > Vars::Aimbot::General::AimFOV.Value)
				continue;

			validTargets.push_back({ pNPC, ETargetType::NPC, vPos, vAngleTo, flFOVTo, flDistTo });
		}
	}

	return validTargets;
}

bool CAimbotMelee::AimFriendlyBuilding(CBaseObject* pBuilding)
{
	if (!pBuilding->m_bMiniBuilding() && pBuilding->m_iUpgradeLevel() != 3 || pBuilding->m_bHasSapper() || pBuilding->m_iHealth() < pBuilding->m_iMaxHealth())
		return true;

	if (pBuilding->IsSentrygun())
	{
		auto pSentry = pBuilding->As<CObjectSentrygun>();
		int iShells, iMaxShells, iRockets, iMaxRockets;

		pSentry->GetAmmoCount(iShells, iMaxShells, iRockets, iMaxRockets);

		if (iShells < iMaxShells || iRockets < iMaxRockets)
			return true;
	}

	return false;
}

std::vector<Target_t> CAimbotMelee::SortTargets(CTFPlayer* pLocal, CTFWeaponBase* pWeapon)
{
	if (lockedTarget.m_pEntity)
		return { lockedTarget };

	auto validTargets = GetTargets(pLocal, pWeapon);

	const auto& sortMethod = ESortMethod::DISTANCE; //static_cast<ESortMethod>(Vars::Aimbot::Melee::SortMethod.Value);
	F::AimbotGlobal.SortTargets(&validTargets, sortMethod);

	std::vector<Target_t> sortedTargets = {};
	int i = 0; for (auto& target : validTargets)
	{
		i++; if (i > Vars::Aimbot::General::MaxTargets.Value) break;

		sortedTargets.push_back(target);
	}

	F::AimbotGlobal.SortPriority(&sortedTargets);

	return sortedTargets;
}



int CAimbotMelee::GetSwingTime(CTFWeaponBase* pWeapon)
{
	if (pWeapon->m_iWeaponID() == TF_WEAPON_KNIFE)
		return 0;
	return Vars::Aimbot::Melee::SwingTicks.Value;
}

void CAimbotMelee::SimulatePlayers(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, std::vector<Target_t> targets,
								   Vec3& vEyePos, std::unordered_map<CBaseEntity*, std::deque<TickRecord>>& pRecordMap,
								   std::unordered_map<CBaseEntity*, std::deque<std::pair<Vec3, Vec3>>>& simLines)
{
	if (lockedTarget.m_pEntity)
		return;

	// swing prediction / auto warp
	const int iSwingTicks = GetSwingTime(pWeapon);
	int iMax = (iDoubletapTicks && Vars::CL_Move::Doubletap::AntiWarp.Value && pLocal->OnSolid())
		? std::max(iSwingTicks - Vars::CL_Move::Doubletap::TickLimit.Value - 1, 0)
		: std::max(iSwingTicks, iDoubletapTicks);

	if ((Vars::Aimbot::Melee::SwingPrediction.Value || iDoubletapTicks) && pWeapon->m_flSmackTime() < 0.f && iMax)
	{
		PlayerStorage localStorage;
		std::unordered_map<CBaseEntity*, PlayerStorage> targetStorage;

		F::MoveSim.Initialize(pLocal, localStorage, false, iDoubletapTicks);
		if (pLocal->OnSolid()) // lol
		{
			localStorage.m_MoveData.m_flForwardMove = G::CurrentUserCmd->forwardmove;
			localStorage.m_MoveData.m_flSideMove = G::CurrentUserCmd->sidemove;
			localStorage.m_MoveData.m_flUpMove = G::CurrentUserCmd->upmove;

			localStorage.m_MoveData.m_vecAngles.y = localStorage.m_MoveData.m_vecOldAngles.y = localStorage.m_MoveData.m_vecViewAngles.y = G::CurrentUserCmd->viewangles.y;
		}
		for (auto& target : targets)
			F::MoveSim.Initialize(target.m_pEntity, targetStorage[target.m_pEntity], false);

		for (int i = 0; i < iMax; i++) // intended for plocal to collide with targets
		{
			if (i < iMax)
			{
				if (pLocal->IsCharging() && iMax - i <= GetSwingTime(pWeapon)) // demo charge fix for swing pred
				{
					localStorage.m_MoveData.m_flMaxSpeed = pLocal->TeamFortress_CalculateMaxSpeed(true);
					localStorage.m_MoveData.m_flClientMaxSpeed = localStorage.m_MoveData.m_flMaxSpeed;
				}
				F::MoveSim.RunTick(localStorage);
			}
			if (i < iSwingTicks - iDoubletapTicks)
			{
				for (auto& target : targets)
				{
					F::MoveSim.RunTick(targetStorage[target.m_pEntity]);
					if (!targetStorage[target.m_pEntity].m_bFailed)
						pRecordMap[target.m_pEntity].push_front({
							target.m_pEntity->m_flSimulationTime() + TICKS_TO_TIME(i + 1),
							I::GlobalVars->curtime + TICKS_TO_TIME(i + 1),
							I::GlobalVars->tickcount + i + 1,
							false,
							BoneMatrixes{},
							targetStorage[target.m_pEntity].m_MoveData.m_vecAbsOrigin
						});
				}
			}
		}
		vEyePos = localStorage.m_MoveData.m_vecAbsOrigin + pLocal->m_vecViewOffset();

		if (Vars::Visuals::Simulation::SwingLines.Value)
		{
			const bool bAlwaysDraw = !Vars::Aimbot::General::AutoShoot.Value || Vars::Debug::Info.Value;
			if (!bAlwaysDraw)
			{
				simLines[pLocal] = localStorage.PredictionLines;
				for (auto& target : targets)
					simLines[target.m_pEntity] = targetStorage[target.m_pEntity].PredictionLines;
			}
			else
			{
				G::LinesStorage.clear();
				G::LinesStorage.push_back({ localStorage.PredictionLines, I::GlobalVars->curtime + 5.f, Vars::Colors::ProjectileColor.Value });
				for (auto& target : targets)
					G::LinesStorage.push_back({ targetStorage[target.m_pEntity].PredictionLines, I::GlobalVars->curtime + 5.f, Vars::Colors::PredictionColor.Value });
			}
		}

		F::MoveSim.Restore(localStorage);
		for (auto& target : targets)
			F::MoveSim.Restore(targetStorage[target.m_pEntity]);
	}
}

bool CAimbotMelee::CanBackstab(CBaseEntity* pTarget, CTFPlayer* pLocal, Vec3 eyeAngles)
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

	Vector vToTarget;
	vToTarget = pTarget->GetAbsOrigin() - pLocal->m_vecOrigin();
	vToTarget.z = 0.f;
	const float flDist = vToTarget.Length();
	vToTarget.Normalize();

	Vector vOwnerForward;
	Math::AngleVectors(eyeAngles, &vOwnerForward);
	vOwnerForward.z = 0.f;
	vOwnerForward.Normalize();

	Vector vTargetForward;
	Math::AngleVectors(F::Backtrack.mEyeAngles[pTarget], &vTargetForward);
	vTargetForward.z = 0.f;
	vTargetForward.Normalize();

	const float flPosVsTargetViewDot = vToTarget.Dot(vTargetForward); // Behind?
	const float flPosVsOwnerViewDot = vToTarget.Dot(vOwnerForward); // Facing?
	const float flViewAnglesDot = vTargetForward.Dot(vOwnerForward); // Facestab?

	return flDist > 0.1f && flPosVsTargetViewDot > (0.f + 0.0031f) && flPosVsOwnerViewDot > 0.5f && flViewAnglesDot > (-0.3f + 0.0031f); // 0.00306795676297 ?
}

int CAimbotMelee::CanHit(Target_t& target, CTFPlayer* pLocal, CTFWeaponBase* pWeapon, Vec3 vEyePos, std::deque<TickRecord> newRecords)
{
	float flHull = SDK::AttribHookValue(18, "melee_bounds_multiplier", pWeapon);
	float flRange = pWeapon->GetSwingRange(pLocal);
	if (pLocal->m_flModelScale() > 1.0f)
	{
		flRange *= pLocal->m_flModelScale();
		flRange *= pLocal->m_flModelScale();
		flRange *= pLocal->m_flModelScale();
	}
	flRange = SDK::AttribHookValue(flRange, "melee_range_multiplier", pWeapon);
	if (flHull <= 0.f || flRange <= 0.f)
		return false;

	static Vec3 vecSwingMins = { -flHull, -flHull, -flHull };
	static Vec3 vecSwingMaxs = { flHull, flHull, flHull };

	CGameTrace trace;
	CTraceFilterHitscan filter;
	filter.pSkip = pLocal;

	matrix3x4 bones[128];
	target.m_pEntity->SetupBones(bones, 128, BONE_USED_BY_ANYTHING, target.m_pEntity->m_flSimulationTime());

	std::deque<TickRecord> vRecords;
	{
		auto pRecords = F::Backtrack.GetRecords(target.m_pEntity);
		if (pRecords && target.m_TargetType == ETargetType::PLAYER)
			vRecords = *pRecords;
		else
		{
			matrix3x4 bones[128];
			if (!target.m_pEntity->SetupBones(bones, 128, BONE_USED_BY_ANYTHING, target.m_pEntity->m_flSimulationTime()))
				return false;

			vRecords.push_front({
				target.m_pEntity->m_flSimulationTime(),
				I::GlobalVars->curtime,
				I::GlobalVars->tickcount,
				false,
				*reinterpret_cast<BoneMatrixes*>(&bones),
				target.m_pEntity->m_vecOrigin()
			});
		}
	}
	if (!newRecords.empty())
	{
		for (TickRecord& pTick : newRecords)
		{
			vRecords.pop_back(); vRecords.push_front({ pTick.flSimTime, pTick.flCreateTime, pTick.iTickCount, false, *reinterpret_cast<BoneMatrixes*>(&bones), pTick.vOrigin });
		}
		for (TickRecord& pTick : vRecords)
		{
			pTick.flSimTime -= TICKS_TO_TIME(newRecords.size());
			pTick.flCreateTime -= TICKS_TO_TIME(newRecords.size());
			pTick.iTickCount -= int(newRecords.size());
		}
	}
	std::deque<TickRecord> validRecords = target.m_TargetType == ETargetType::PLAYER ? F::Backtrack.GetValidRecords(&vRecords, pLocal, true) : vRecords;
	if (!Vars::Backtrack::Enabled.Value && !validRecords.empty())
		validRecords = { validRecords.front() };

	// this might be retarded
	const float flTargetPos = (target.m_pEntity->m_vecMaxs().z - target.m_pEntity->m_vecMins().z) * 65.f / 82.f;
	const float flLocalPos = (pLocal->m_vecMaxs().z - pLocal->m_vecMins().z) * 65.f / 82.f;
	const Vec3 vecDiff = { 0, 0, std::min(flTargetPos, flLocalPos) };

	for (auto& pTick : validRecords)
	{
		const Vec3 vRestore = target.m_pEntity->GetAbsOrigin();
		target.m_pEntity->SetAbsOrigin(pTick.vOrigin);

		target.m_vPos = pTick.vOrigin + vecDiff;
		target.m_vAngleTo = Aim(G::CurrentUserCmd->viewangles, Math::CalcAngle(vEyePos, target.m_vPos), iAimType);

		Vec3 vecForward = Vec3();
		Math::AngleVectors(target.m_vAngleTo, &vecForward);
		Vec3 vecTraceEnd = vEyePos + (vecForward * flRange);

		SDK::Trace(vEyePos, vecTraceEnd, MASK_SHOT | CONTENTS_GRATE, &filter, &trace);
		bool bReturn = trace.m_pEnt && trace.m_pEnt == target.m_pEntity;
		if (!bReturn)
		{
			SDK::TraceHull(vEyePos, vecTraceEnd, vecSwingMins, vecSwingMaxs, MASK_SHOT | CONTENTS_GRATE, &filter, &trace);
			bReturn = trace.m_pEnt && trace.m_pEnt == target.m_pEntity;
		}

		if (bReturn && Vars::Aimbot::Melee::AutoBackstab.Value && pWeapon->m_iWeaponID() == TF_WEAPON_KNIFE)
		{
			if (target.m_TargetType == ETargetType::PLAYER)
				bReturn = CanBackstab(target.m_pEntity, pLocal, target.m_vAngleTo);
			else
				bReturn = false;
		}

		target.m_pEntity->SetAbsOrigin(vRestore);

		if (bReturn)
		{
			target.m_Tick = pTick;
			if (target.m_TargetType == ETargetType::PLAYER) // && Vars::Backtrack::Enabled.Value
				target.m_bBacktrack = true;

			return true;
		}
		else if (iAimType == 2)
		{
			auto vAngle = Math::CalcAngle(vEyePos, target.m_vPos);

			Vec3 vecForward = Vec3();
			Math::AngleVectors(vAngle, &vecForward);
			Vec3 vecTraceEnd = vEyePos + (vecForward * flRange);

			SDK::Trace(vEyePos, vecTraceEnd, MASK_SHOT | CONTENTS_GRATE, &filter, &trace);
			if (trace.m_pEnt && trace.m_pEnt == target.m_pEntity)
				return 2;
		}
	}

	return false;
}



bool CAimbotMelee::IsAttacking(const CUserCmd* pCmd, CTFWeaponBase* pWeapon)
{
	if (pWeapon->m_iWeaponID() == TF_WEAPON_KNIFE)
		return pCmd->buttons & IN_ATTACK;

	return TIME_TO_TICKS(pWeapon->m_flSmackTime()) == I::GlobalVars->tickcount - 1; // seems to work most (?) of the time
}

// assume angle calculated outside with other overload
void CAimbotMelee::Aim(CUserCmd* pCmd, Vec3& vAngle)
{
	if (iAimType != 3)
	{
		pCmd->viewangles = vAngle;
		I::EngineClient->SetViewAngles(pCmd->viewangles);
	}
	else if (G::IsAttacking)
	{
		SDK::FixMovement(pCmd, vAngle);
		pCmd->viewangles = vAngle;
		G::PSilentAngles = true;
	}
}

Vec3 CAimbotMelee::Aim(Vec3 vCurAngle, Vec3 vToAngle, int iMethod)
{
	Vec3 vReturn = {};

	Math::ClampAngles(vToAngle);

	switch (iMethod)
	{
	case 1: // Plain
	case 3: // Silent
		vReturn = vToAngle;
		break;
	case 2: // Smooth
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

void CAimbotMelee::Run(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd)
{
	if (lockedTarget.m_pEntity && pWeapon->m_flSmackTime() < 0.f)
		lockedTarget.m_pEntity = nullptr;

	if (!Vars::Aimbot::General::AimType.Value && !lockedTarget.m_pEntity || (!G::CanPrimaryAttack || !Vars::Aimbot::General::AutoShoot.Value) && pWeapon->m_flSmackTime() < 0.f)
		return;
	else if (!lockedTarget.m_pEntity)
		iAimType = Vars::Aimbot::General::AimType.Value;

	if (RunSapper(pLocal, pWeapon, pCmd))
		return;

	auto targets = SortTargets(pLocal, pWeapon);
	if (targets.empty())
		return;

	iDoubletapTicks = F::Ticks.GetTicks(pLocal);
	const bool bShouldSwing = iDoubletapTicks <= (GetSwingTime(pWeapon) ? 14 : 0) || Vars::CL_Move::Doubletap::AntiWarp.Value && pLocal->OnSolid();

	Vec3 vEyePos = pLocal->GetShootPos();
	std::unordered_map<CBaseEntity*, std::deque<TickRecord>> pRecordMap;
	std::unordered_map<CBaseEntity*, std::deque<std::pair<Vec3, Vec3>>> simLines;
	SimulatePlayers(pLocal, pWeapon, targets, vEyePos, pRecordMap, simLines);

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
		if (!pRecordMap[target.m_pEntity].empty() && !lockedTarget.m_pEntity)
			lockedTarget = target;

		if (iAimType == 3)
			G::AimPosition = target.m_vPos;

		if (Vars::Aimbot::General::AutoShoot.Value && pWeapon->m_flSmackTime() < 0.f)
		{
			if (bShouldSwing)
				pCmd->buttons |= IN_ATTACK;
			if (iDoubletapTicks)
				G::DoubleTap = true;
		}

		/*
		// game will not manage this while shifting w/o prediction, force its hand
		if (!Vars::Misc::Game::NetworkFix.Value && !Vars::Misc::Game::PredictionFix.Value
			&& G::DoubleTap && pWeapon->m_iWeaponID() != TF_WEAPON_KNIFE)
		{
			if (pCmd->buttons & IN_ATTACK && pWeapon->m_flSmackTime() < 0.f)
				pWeapon->m_flSmackTime() = TICKS_TO_TIME(I::GlobalVars->tickcount + 13);
			I::Prediction->Update(I::ClientState->m_nDeltaTick, I::ClientState->m_nDeltaTick > 0, I::ClientState->last_command_ack, I::ClientState->lastoutgoingcommand + I::ClientState->chokedcommands);
		}
		*/

		const bool bAttacking = IsAttacking(pCmd, pWeapon);
		G::IsAttacking = bAttacking || bShouldSwing && G::DoubleTap; // dumb but works

		if (G::IsAttacking)
		{
			if (target.m_bBacktrack)
				pCmd->tick_count = TIME_TO_TICKS(target.m_Tick.flSimTime) + TIME_TO_TICKS(F::Backtrack.flFakeInterp);
			// bug: fast old records seem to be progressively more unreliable ?

			if (Vars::Visuals::Bullet::BulletTracer.Value)
			{
				G::BulletsStorage.clear();
				G::BulletsStorage.push_back({ {vEyePos, target.m_vPos}, I::GlobalVars->curtime + 5.f, Vars::Colors::BulletTracer.Value, true });
			}
			if (Vars::Visuals::Simulation::SwingLines.Value)
			{
				const bool bAlwaysDraw = !Vars::Aimbot::General::AutoShoot.Value || Vars::Debug::Info.Value;
				if (!bAlwaysDraw)
				{
					G::LinesStorage.clear();
					G::LinesStorage.push_back({ simLines[pLocal], I::GlobalVars->curtime + 5.f, Vars::Colors::ProjectileColor.Value });
					G::LinesStorage.push_back({ simLines[target.m_pEntity], I::GlobalVars->curtime + 5.f, Vars::Colors::PredictionColor.Value });
				}
			}
			if (Vars::Visuals::Hitbox::ShowHitboxes.Value)
			{
				G::BoxesStorage.clear();
				auto vBoxes = F::Visuals.GetHitboxes((matrix3x4*)(&target.m_Tick.BoneMatrix.BoneMatrix), target.m_pEntity->As<CBaseAnimating>());
				G::BoxesStorage.insert(G::BoxesStorage.end(), vBoxes.begin(), vBoxes.end());
			}
		}

		Aim(pCmd, target.m_vAngleTo);
		G::IsAttacking = bAttacking;
		break;
	}
}

int GetAttachment(CBaseObject* pBuilding, int i)
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
	if (pWeapon->m_iWeaponID() != TF_WEAPON_BUILDER)
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

		validTargets.push_back({ pBuilding, ETargetType::UNKNOWN, vPoint, vAngleTo, flFOVTo, flDistTo });
	}

	F::AimbotGlobal.SortTargets(&validTargets, ESortMethod::DISTANCE);
	for (auto& target : validTargets)
	{
		static int iLastRun = 0;

		if ((Vars::Aimbot::General::AimType.Value == 3 ? iLastRun != I::GlobalVars->tickcount - 1 || G::PSilentAngles && G::ShiftedTicks == G::MaxShift : true) && Vars::Aimbot::General::AutoShoot.Value)
			pCmd->buttons |= IN_ATTACK;

		if (pCmd->buttons & IN_ATTACK)
		{
			G::IsAttacking = true;
			target.m_vAngleTo = Aim(pCmd->viewangles, Math::CalcAngle(vLocalPos, target.m_vPos));
			target.m_vAngleTo.x = pCmd->viewangles.x; // we don't need to care about pitch
			Aim(pCmd, target.m_vAngleTo);

			iLastRun = I::GlobalVars->tickcount;
		}

		break;
	}

	return true;
}