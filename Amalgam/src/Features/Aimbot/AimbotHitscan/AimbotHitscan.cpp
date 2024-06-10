#include "AimbotHitscan.h"

#include "../../Backtrack/Backtrack.h"
#include "../../Resolver/Resolver.h"
#include "../../Visuals/Visuals.h"

bool CAimbotHitscan::PlayerBoneInFOV(CTFPlayer* pTarget, Vec3 vLocalPos, Vec3 vLocalAngles, float& flFOVTo, Vec3& vPos, Vec3& vAngleTo) // this won't prevent shooting bones outside of fov
{
	bool bReturn = false;

	float flMinFOV = 180.f;
	for (int nHitbox = 0; nHitbox < pTarget->GetNumOfHitboxes(); nHitbox++)
	{
		if (!IsHitboxValid(nHitbox))
			continue;

		Vec3 vCurPos = pTarget->GetHitboxPos(nHitbox);
		Vec3 vCurAngleTo = Math::CalcAngle(vLocalPos, vCurPos);
		float flCurFOVTo = Math::CalcFov(vLocalAngles, vCurAngleTo);

		if (flCurFOVTo < flMinFOV && flCurFOVTo < Vars::Aimbot::General::AimFOV.Value)
		{
			bReturn = true;
			vPos = vCurPos;
			vAngleTo = vCurAngleTo;
			flFOVTo = flMinFOV = flCurFOVTo;
		}
	}

	return bReturn;
}

std::vector<Target_t> CAimbotHitscan::GetTargets(CTFPlayer* pLocal, CTFWeaponBase* pWeapon)
{
	std::vector<Target_t> validTargets;
	const auto sortMethod = static_cast<ESortMethod>(Vars::Aimbot::General::TargetSelection.Value);

	const Vec3 vLocalPos = pLocal->GetShootPos();
	const Vec3 vLocalAngles = I::EngineClient->GetViewAngles();

	const bool bIsMedigun = pWeapon->m_iWeaponID() == TF_WEAPON_MEDIGUN;

	if (Vars::Aimbot::General::Target.Value & PLAYER)
	{
		const bool bPissRifle = G::WeaponDefIndex == Sniper_m_TheSydneySleeper;

		EGroupType groupType = EGroupType::PLAYERS_ENEMIES;
		if (bIsMedigun)
			groupType = EGroupType::PLAYERS_TEAMMATES;
		else if (bPissRifle)
			groupType = EGroupType::PLAYERS_ALL;

		for (auto pEntity : H::Entities.GetGroup(groupType))
		{
			auto pPlayer = pEntity->As<CTFPlayer>();
			if (pPlayer == pLocal || !pPlayer->IsAlive() || pPlayer->IsAGhost())
				continue;

			// Can we extinguish a teammate using the piss rifle?
			if (bPissRifle && (pPlayer->m_iTeamNum() == pLocal->m_iTeamNum()))
			{
				if (!(Vars::Aimbot::Hitscan::Modifiers.Value & (1 << 6)) || !pPlayer->IsOnFire())
					continue;
			}

			if (F::AimbotGlobal.ShouldIgnore(pPlayer, pLocal, pWeapon, bIsMedigun))
				continue;

			float flFOVTo; Vec3 vPos, vAngleTo;
			if (!PlayerBoneInFOV(pPlayer, vLocalPos, vLocalAngles, flFOVTo, vPos, vAngleTo))
				continue;

			const float flDistTo = vLocalPos.DistTo(vPos);
			const int priority = F::AimbotGlobal.GetPriority(pPlayer->entindex());
			validTargets.push_back({ pPlayer, ETargetType::PLAYER, vPos, vAngleTo, flFOVTo, flDistTo, priority });
		}
	}

	if (bIsMedigun) // do not attempt to heal buildings or other
		return validTargets;

	if (Vars::Aimbot::General::Target.Value)
	{
		for (auto pEntity : H::Entities.GetGroup(EGroupType::BUILDINGS_ENEMIES))
		{
			auto pBuilding = pEntity->As<CBaseObject>();

			bool isSentry = pBuilding->IsSentrygun(), isDispenser = pBuilding->IsDispenser(), isTeleporter = pBuilding->IsTeleporter();

			if (!(Vars::Aimbot::General::Target.Value & SENTRY) && isSentry)
				continue;
			if (!(Vars::Aimbot::General::Target.Value & DISPENSER) && isDispenser)
				continue;
			if (!(Vars::Aimbot::General::Target.Value & TELEPORTER) && isTeleporter)
				continue;

			Vec3 vPos = pBuilding->GetCenter();
			Vec3 vAngleTo = Math::CalcAngle(vLocalPos, vPos);
			const float flFOVTo = Math::CalcFov(vLocalAngles, vAngleTo);

			if (flFOVTo > Vars::Aimbot::General::AimFOV.Value)
				continue;

			const float flDistTo = vLocalPos.DistTo(vPos);
			validTargets.push_back({ pBuilding, isSentry ? ETargetType::SENTRY : (isDispenser ? ETargetType::DISPENSER : ETargetType::TELEPORTER), vPos, vAngleTo, flFOVTo, flDistTo });
		}
	}

	if (Vars::Aimbot::General::Target.Value & STICKY)
	{
		for (auto pEntity : H::Entities.GetGroup(EGroupType::WORLD_PROJECTILES))
		{
			if (pEntity->GetClassID() != ETFClassID::CTFGrenadePipebombProjectile)
				continue;

			auto pProjectile = pEntity->As<CTFGrenadePipebombProjectile>();
			if (pProjectile->m_iType() != TF_GL_MODE_REMOTE_DETONATE || !pProjectile->m_bTouched())
				continue;

			auto pOwner = pProjectile->m_hThrower().Get();
			if (!pOwner || pOwner->m_iTeamNum() == pLocal->m_iTeamNum())
				continue;

			Vec3 vPos = pProjectile->GetCenter();
			Vec3 vAngleTo = Math::CalcAngle(vLocalPos, vPos);
			const float flFOVTo = Math::CalcFov(vLocalAngles, vAngleTo);

			if (flFOVTo > Vars::Aimbot::General::AimFOV.Value)
				continue;

			const float flDistTo = vLocalPos.DistTo(vPos);
			validTargets.push_back({ pProjectile, ETargetType::STICKY, vPos, vAngleTo, flFOVTo, flDistTo });
		}
	}

	if (Vars::Aimbot::General::Target.Value & NPC)
	{
		for (auto pNPC : H::Entities.GetGroup(EGroupType::WORLD_NPC))
		{
			Vec3 vPos = pNPC->GetCenter();
			Vec3 vAngleTo = Math::CalcAngle(vLocalPos, vPos);
			const float flFOVTo = Math::CalcFov(vLocalAngles, vAngleTo);

			if (flFOVTo > Vars::Aimbot::General::AimFOV.Value)
				continue;

			const float flDistTo = vLocalPos.DistTo(vPos);
			validTargets.push_back({ pNPC, ETargetType::NPC, vPos, vAngleTo, flFOVTo, flDistTo });
		}
	}

	if (Vars::Aimbot::General::Target.Value & BOMB)
	{
		for (auto pBomb : H::Entities.GetGroup(EGroupType::WORLD_BOMBS))
		{
			Vec3 vPos = pBomb->GetCenter();
			Vec3 vAngleTo = Math::CalcAngle(vLocalPos, vPos);

			const float flFOVTo = Math::CalcFov(vLocalAngles, vAngleTo);
			const float flDistTo = sortMethod == ESortMethod::DISTANCE ? vLocalPos.DistTo(vPos) : 0.0f;

			if (flFOVTo > Vars::Aimbot::General::AimFOV.Value)
				continue;

			if (!F::AimbotGlobal.ValidBomb(pLocal, pWeapon, pBomb))
				continue;

			validTargets.push_back({ pBomb, ETargetType::BOMBS, vPos, vAngleTo, flFOVTo, flDistTo });
		}
	}

	return validTargets;
}

std::vector<Target_t> CAimbotHitscan::SortTargets(CTFPlayer* pLocal, CTFWeaponBase* pWeapon)
{
	auto validTargets = GetTargets(pLocal, pWeapon);

	const auto& sortMethod = static_cast<ESortMethod>(Vars::Aimbot::General::TargetSelection.Value);
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



bool CAimbotHitscan::IsHitboxValid(int nHitbox) // check that this & all other uses are right
{
	const int iHitboxes = Vars::Aimbot::Hitscan::Hitboxes.Value;
	switch (nHitbox)
	{
	case -1: return true;
	case HITBOX_HEAD: return iHitboxes & (1 << 0);
	case HITBOX_NECK: return iHitboxes & (1 << 1);
	case HITBOX_LOWER_NECK:
	case HITBOX_PELVIS:
	case HITBOX_BODY:
	case HITBOX_THORAX: return iHitboxes & (1 << 2);
	case HITBOX_CHEST:
	case HITBOX_UPPER_CHEST:
	case HITBOX_RIGHT_THIGH:
	case HITBOX_LEFT_THIGH:
	case HITBOX_RIGHT_CALF:
	case HITBOX_LEFT_CALF: return iHitboxes & (1 << 3);
	case HITBOX_RIGHT_FOOT:
	case HITBOX_LEFT_FOOT:
	case HITBOX_RIGHT_HAND:
	case HITBOX_LEFT_HAND:
	case HITBOX_RIGHT_UPPER_ARM:
	case HITBOX_RIGHT_FOREARM:
	case HITBOX_LEFT_UPPER_ARM:
	case HITBOX_LEFT_FOREARM: return iHitboxes & (1 << 4);
	}

	return false;
}

int CAimbotHitscan::GetHitboxPriority(int nHitbox, CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CBaseEntity* pTarget)
{
	bool bHeadshot = false;
	{
		const int nClassNum = pLocal->m_iClass();

		if (nClassNum == TF_CLASS_SNIPER)
		{
			if (G::WeaponDefIndex != Sniper_m_TheClassic && pLocal->IsScoped() ||
				G::WeaponDefIndex == Sniper_m_TheClassic && pWeapon->As<CTFSniperRifle>()->m_flChargedDamage() > 149.9f)
				bHeadshot = true;
		}
		if (nClassNum == TF_CLASS_SPY)
		{
			if ((G::WeaponDefIndex == Spy_m_TheAmbassador || G::WeaponDefIndex == Spy_m_FestiveAmbassador) && pWeapon->AmbassadorCanHeadshot())
				bHeadshot = true;
		}

		if (Vars::Aimbot::Hitscan::Modifiers.Value & (1 << 5) && bHeadshot && pTarget->IsPlayer())
		{
			{
				float flBodyMult = 1.f;
				switch (G::WeaponDefIndex)
				{
				case Sniper_m_TheClassic: flBodyMult = 0.9f; break;
				case Sniper_m_TheHitmansHeatmaker: flBodyMult = 0.8f; break;
				case Sniper_m_TheMachina:
				case Sniper_m_ShootingStar: if (pWeapon->As<CTFSniperRifle>()->m_flChargedDamage() > 149.9f) flBodyMult = 1.15f;
				}
				if (pWeapon->As<CTFSniperRifle>()->m_flChargedDamage() * flBodyMult >= pTarget->As<CTFPlayer>()->m_iHealth())
					bHeadshot = false;
			}

			if (G::WeaponDefIndex == Spy_m_TheAmbassador || G::WeaponDefIndex == Spy_m_FestiveAmbassador)
			{
				const float flDistTo = pTarget->m_vecOrigin().DistTo(pLocal->m_vecOrigin());
				const int nAmbassadorBodyshotDamage = Math::RemapValClamped(flDistTo, 90, 900, 51, 18);

				if (pTarget->As<CTFPlayer>()->m_iHealth() < (nAmbassadorBodyshotDamage + 2)) // whatever
					bHeadshot = false;
			}
		}
	}

	switch (nHitbox)
	{
	case -1: return 2;
	case HITBOX_HEAD: return bHeadshot ? 0 : 2;
	case HITBOX_NECK: return 2;
	case HITBOX_LOWER_NECK:
	case HITBOX_PELVIS:
	case HITBOX_BODY:
	case HITBOX_THORAX: return bHeadshot ? 1 : 0;
	case HITBOX_CHEST:
	case HITBOX_UPPER_CHEST:
	case HITBOX_RIGHT_THIGH:
	case HITBOX_LEFT_THIGH:
	case HITBOX_RIGHT_CALF:
	case HITBOX_LEFT_CALF: return 2;
	case HITBOX_RIGHT_FOOT:
	case HITBOX_LEFT_FOOT:
	case HITBOX_RIGHT_HAND:
	case HITBOX_LEFT_HAND:
	case HITBOX_RIGHT_UPPER_ARM:
	case HITBOX_RIGHT_FOREARM:
	case HITBOX_LEFT_UPPER_ARM:
	case HITBOX_LEFT_FOREARM: return 2;
	}

	return 2;
};

float CAimbotHitscan::GetMaxRange(CTFWeaponBase* pWeapon)
{
	switch (pWeapon->m_iWeaponID())
	{
	case TF_WEAPON_MEDIGUN: return 450.f;
	case TF_WEAPON_MECHANICAL_ARM: return 256.f;
	}

	auto tfWeaponInfo = pWeapon->GetWeaponInfo();
	return tfWeaponInfo ? tfWeaponInfo->GetWeaponData(0).m_flRange : 8192.f;
}

int CAimbotHitscan::CanHit(Target_t& target, CTFPlayer* pLocal, CTFWeaponBase* pWeapon)
{
	if (Vars::Aimbot::General::Ignore.Value & UNSIMULATED && G::ChokeMap[target.m_pEntity->entindex()] > Vars::Aimbot::General::TickTolerance.Value)
		return false;

	Vec3 vEyePos = pLocal->GetShootPos(), vPeekPos = {};
	float flSpread = pWeapon->GetWeaponSpread();
	if (flSpread)
		vPeekPos = pLocal->GetShootPos() + pLocal->GetAbsVelocity() * TICKS_TO_TIME(-Vars::Aimbot::General::HitscanPeek.Value);
	const float flMaxRange = powf(GetMaxRange(pWeapon), 2.f);

	auto pModel = target.m_pEntity->GetModel();
	if (!pModel) return false;
	auto pHDR = I::ModelInfoClient->GetStudiomodel(pModel);
	if (!pHDR) return false;
	auto pSet = pHDR->pHitboxSet(target.m_pEntity->As<CBaseAnimating>()->m_nHitboxSet());
	if (!pSet) return false;

	std::deque<TickRecord> vRecords;
	{
		auto pRecords = F::Backtrack.GetRecords(target.m_pEntity);
		if (pRecords && target.m_TargetType == ETargetType::PLAYER)
		{
			vRecords = F::Backtrack.GetValidRecords(pRecords, pLocal);
			if (!Vars::Backtrack::Enabled.Value && !vRecords.empty())
				vRecords = { vRecords.front() };
		}
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

	static int iPreferredTick = 0; // if we're doubletapping, we can't change viewangles so have a preferred tick to use
	if (!I::ClientState->chokedcommands)
		iPreferredTick = 0;
	if (iPreferredTick)
	{
		auto pivot = std::find_if(vRecords.begin(), vRecords.end(), [](auto& s) -> bool { return s.iTickCount == iPreferredTick; });
		if (pivot != vRecords.end())
			std::rotate(vRecords.begin(), pivot, pivot + 1);
	}

	auto RayToOBB = [](const Vec3& origin, const Vec3& direction, const Vec3& position, const Vec3& min, const Vec3& max, const matrix3x4 orientation) -> bool
		{
			if (Vars::Aimbot::General::AimType.Value != 2)
				return true;

			return Math::RayToOBB(origin, direction, position, min, max, orientation);
		};

	int iReturn = false;
	for (auto& pTick : vRecords)
	{
		if (target.m_TargetType == ETargetType::PLAYER || target.m_TargetType == ETargetType::SENTRY)
		{
			auto boneMatrix = (matrix3x4*)(&pTick.BoneMatrix.BoneMatrix);
			if (!boneMatrix)
				continue;

			std::vector<std::pair<const mstudiobbox_t*, int>> hitboxes;
			{
				if (target.m_TargetType != ETargetType::SENTRY)
				{
					std::vector<std::pair<const mstudiobbox_t*, int>> primary, secondary, tertiary; // dumb
					for (int nHitbox = 0; nHitbox < target.m_pEntity->As<CTFPlayer>()->GetNumOfHitboxes(); nHitbox++)
					{
						if (!IsHitboxValid(nHitbox))
							continue;

						auto pBox = pSet->pHitbox(nHitbox);
						if (!pBox) continue;

						switch (GetHitboxPriority(nHitbox, pLocal, pWeapon, target.m_pEntity))
						{
						case 0: primary.push_back({ pBox, nHitbox }); break;
						case 1: secondary.push_back({ pBox, nHitbox }); break;
						case 2: tertiary.push_back({ pBox, nHitbox }); break;
						}
					}
					for (auto& pair : primary) hitboxes.push_back(pair);
					for (auto& pair : secondary) hitboxes.push_back(pair);
					for (auto& pair : tertiary) hitboxes.push_back(pair);
				}
				else
				{
					for (int nHitbox = 0; nHitbox < target.m_pEntity->As<CObjectSentrygun>()->GetNumOfHitboxes(); nHitbox++)
					{
						const mstudiobbox_t* pBox = pSet->pHitbox(nHitbox);
						if (!pBox) continue;

						hitboxes.push_back({ pBox, nHitbox });
					}
					std::sort(hitboxes.begin(), hitboxes.end(), [&](const auto& a, const auto& b) -> bool
						{
							Vec3 aCenter = {}, bCenter = {}, vCenter = target.m_pEntity->GetCenter();
							Math::VectorTransform({}, boneMatrix[a.first->bone], aCenter);
							Math::VectorTransform({}, boneMatrix[b.first->bone], bCenter);

							return vCenter.DistTo(aCenter) < vCenter.DistTo(bCenter);
						});
				}
			}

			for (auto& pair : hitboxes)
			{
				const float flScale = Vars::Aimbot::Hitscan::PointScale.Value / 100;
				const Vec3 vMins = pair.first->bbmin, vMinsS = vMins * flScale;
				const Vec3 vMaxs = pair.first->bbmax, vMaxsS = vMaxs * flScale;

				std::vector<Vec3> vecPoints = { Vec3() };
				if (flScale > 0.f)
				{
					vecPoints = {
						Vec3(),
						Vec3(vMinsS.x, vMinsS.y, vMaxsS.z),
						Vec3(vMaxsS.x, vMinsS.y, vMaxsS.z),
						Vec3(vMinsS.x, vMaxsS.y, vMaxsS.z),
						Vec3(vMaxsS.x, vMaxsS.y, vMaxsS.z),
						Vec3(vMinsS.x, vMinsS.y, vMinsS.z),
						Vec3(vMaxsS.x, vMinsS.y, vMinsS.z),
						Vec3(vMinsS.x, vMaxsS.y, vMinsS.z),
						Vec3(vMaxsS.x, vMaxsS.y, vMinsS.z)
					};
				}

				for (const auto& point : vecPoints)
				{
					Vec3 vCenter = {}, vTransformed = {};
					Math::VectorTransform({}, boneMatrix[pair.first->bone], vCenter);
					Math::VectorTransform(point, boneMatrix[pair.first->bone], vTransformed);

					if (vEyePos.DistToSqr(vTransformed) > flMaxRange)
						continue;

					auto vAngles = Aim(G::CurrentUserCmd->viewangles, Math::CalcAngle(pLocal->GetShootPos(), vTransformed));
					Vec3 vForward = {};
					Math::AngleVectors(vAngles, &vForward);

					bool bPeekCheck = flSpread ? SDK::VisPos(pLocal, target.m_pEntity, vPeekPos, vTransformed) : true;
					if (bPeekCheck)
						flSpread = 0.f; // only use with a single hitbox

					if (SDK::VisPos(pLocal, target.m_pEntity, vEyePos, vTransformed) && bPeekCheck)
					{
						target.m_vAngleTo = vAngles;
						if (RayToOBB(vEyePos, vForward, vCenter, vMins, vMaxs, boneMatrix[pair.first->bone])) // for the time being, no vischecks against other hitboxes
						{
							bool bWillHit = true;
							if (target.m_TargetType == ETargetType::SENTRY) // point of hit for sentries needs to be within bbox
							{
								const matrix3x4& transform = target.m_pEntity->RenderableToWorldTransform();
								const Vec3 vMin = target.m_pEntity->m_vecMins(), vMax = target.m_pEntity->m_vecMaxs();
								bWillHit = RayToOBB(vEyePos, vForward, target.m_pEntity->m_vecOrigin(), vMin, vMax, transform);
							}
							if (bWillHit)
							{
								iPreferredTick = pTick.iTickCount;

								target.m_Tick = pTick;
								target.m_vPos = vTransformed;
								if (target.m_TargetType == ETargetType::PLAYER)
								{
									//if (Vars::Backtrack::Enabled.Value)
										target.m_bBacktrack = target.m_TargetType == ETargetType::PLAYER;
									target.m_nAimedHitbox = pair.second;
								}
								return true;
							}
						}
						iReturn = 2;
					}
				}
			}
		}
		else
		{
			const float flScale = Vars::Aimbot::Hitscan::PointScale.Value / 100;
			const Vec3 vMins = target.m_pEntity->m_vecMins(), vMinsS = vMins * flScale;
			const Vec3 vMaxs = target.m_pEntity->m_vecMaxs(), vMaxsS = vMaxs * flScale;

			std::vector<Vec3> vecPoints = { Vec3() };
			if (flScale > 0.f)
			{
				vecPoints = {
					Vec3(),
					Vec3(vMinsS.x, vMinsS.y, vMaxsS.z),
					Vec3(vMaxsS.x, vMinsS.y, vMaxsS.z),
					Vec3(vMinsS.x, vMaxsS.y, vMaxsS.z),
					Vec3(vMaxsS.x, vMaxsS.y, vMaxsS.z),
					Vec3(vMinsS.x, vMinsS.y, vMinsS.z),
					Vec3(vMaxsS.x, vMinsS.y, vMinsS.z),
					Vec3(vMinsS.x, vMaxsS.y, vMinsS.z),
					Vec3(vMaxsS.x, vMaxsS.y, vMinsS.z)
				};
			}

			const matrix3x4& transform = target.m_pEntity->RenderableToWorldTransform();
			for (const auto& point : vecPoints)
			{
				Vec3 vTransformed = target.m_pEntity->GetCenter() + point;

				if (vEyePos.DistToSqr(vTransformed) > flMaxRange)
					continue;

				auto vAngles = Aim(G::CurrentUserCmd->viewangles, Math::CalcAngle(pLocal->GetShootPos(), vTransformed));
				Vec3 vForward = {};
				Math::AngleVectors(vAngles, &vForward);

				bool bPeekCheck = flSpread ? SDK::VisPos(pLocal, target.m_pEntity, vPeekPos, vTransformed) : true;
				if (bPeekCheck)
					flSpread = 0.f; // only use with a single hitbox

				if (SDK::VisPos(pLocal, target.m_pEntity, vEyePos, vTransformed) && bPeekCheck)
				{
					target.m_vAngleTo = vAngles;
					if (RayToOBB(vEyePos, vForward, target.m_pEntity->m_vecOrigin(), vMins, vMaxs, transform)) // for the time being, no vischecks against other hitboxes
					{
						target.m_Tick = pTick;
						target.m_vPos = vTransformed;
						return true;
					}
					iReturn = 2;
				}
			}
		}
	}

	return iReturn;
}



/* Returns whether AutoShoot should fire */
bool CAimbotHitscan::ShouldFire(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd, const Target_t& target)
{
	if (!Vars::Aimbot::General::AutoShoot.Value) return false;

	switch (pLocal->m_iClass())
	{
	case TF_CLASS_SNIPER:
	{
		const bool bIsScoped = pLocal->IsScoped();

		if (Vars::Aimbot::Hitscan::Modifiers.Value & (1 << 1))
		{
			if (G::WeaponDefIndex != Sniper_m_TheClassic
				&& G::WeaponDefIndex != Sniper_m_TheSydneySleeper
				&& !G::CanHeadshot && bIsScoped)
			{
				return false;
			}
		}

		if (Vars::Aimbot::Hitscan::Modifiers.Value & (1 << 2) && (bIsScoped || G::WeaponDefIndex == Sniper_m_TheClassic))
		{
			auto pPlayer = target.m_pEntity->As<CTFPlayer>();
			auto pSniperRifle = pWeapon->As<CTFSniperRifle>();
			const int nHealth = pPlayer->m_iHealth();
			const bool bIsCritBoosted = pLocal->IsCritBoosted();

			if (target.m_nAimedHitbox == HITBOX_HEAD && G::WeaponDefIndex != Sniper_m_TheSydneySleeper && (G::WeaponDefIndex != Sniper_m_TheClassic || G::WeaponDefIndex == Sniper_m_TheClassic && pSniperRifle->m_flChargedDamage() > 149.9f))
			{
				if (nHealth > 150)
				{
					const float flDamage = Math::RemapValClamped(pSniperRifle->m_flChargedDamage(), 0.0f, 150.0f, 0.0f, 450.0f);
					const int nDamage = static_cast<int>(flDamage);

					if (nDamage < nHealth && nDamage != 450)
						return false;
				}
				else if (!bIsCritBoosted && !G::CanHeadshot)
					return false;
			}
			else
			{
				if (nHealth > (bIsCritBoosted ? 150 : 50))
				{
					float flCritMult = pPlayer->IsInJarate() ? 1.36f : 1.0f;

					if (bIsCritBoosted)
						flCritMult = 3.0f;

					float flBodyMult = 1.f;
					switch (G::WeaponDefIndex)
					{
					case Sniper_m_TheClassic: flBodyMult = 0.9f; break;
					case Sniper_m_TheHitmansHeatmaker: flBodyMult = 0.8f; break;
					case Sniper_m_TheMachina:
					case Sniper_m_ShootingStar: if (pSniperRifle->m_flChargedDamage() > 149.9f) flBodyMult = 1.15f;
					}

					const float flMax = 150.0f * flCritMult * flBodyMult;
					const int nDamage = static_cast<int>(pSniperRifle->m_flChargedDamage() * flCritMult * flBodyMult);

					if (nDamage < pPlayer->m_iHealth() && nDamage != static_cast<int>(flMax))
						return false;
				}
			}
		}

		break;
	}
	case TF_CLASS_SPY:
		if (Vars::Aimbot::Hitscan::Modifiers.Value & (1 << 1) && !pWeapon->AmbassadorCanHeadshot())
			return false;
		break;
	}

	return true;
}

// assume angle calculated outside with other overload
void CAimbotHitscan::Aim(CUserCmd* pCmd, Vec3& vAngle)
{
	if (Vars::Aimbot::General::AimType.Value != 3)
	{
		pCmd->viewangles = vAngle;
		I::EngineClient->SetViewAngles(pCmd->viewangles); // remove these if uncommenting l124 of createmove
	}
	else if (G::IsAttacking)
	{
		SDK::FixMovement(pCmd, vAngle);
		pCmd->viewangles = vAngle;
		G::SilentAngles = true;
	}
}

Vec3 CAimbotHitscan::Aim(Vec3 vCurAngle, Vec3 vToAngle, int iMethod)
{
	Vec3 vReturn = {};

	vToAngle -= G::PunchAngles;
	Math::ClampAngles(vToAngle);

	switch (iMethod)
	{
	case 1: // Plain
	case 3: // Silent
		vReturn = vToAngle;
		break;
	case 2: //Smooth
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

void CAimbotHitscan::Run(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd)
{
	static int iStaticAimType = Vars::Aimbot::General::AimType.Value;
	const int iRealAimType = Vars::Aimbot::General::AimType.Value;
	const int iLastAimType = iStaticAimType;
	iStaticAimType = iRealAimType;

	const int nWeaponID = pWeapon->m_iWeaponID();

	const bool bAutomatic = pWeapon->IsStreamingWeapon(), bKeepFiring = bAutomatic && G::LastUserCmd->buttons & IN_ATTACK;
	if (bKeepFiring && !G::CanPrimaryAttack && Vars::Aimbot::General::AimType.Value)
		pCmd->buttons |= IN_ATTACK;

	switch (nWeaponID)
	{
	case TF_WEAPON_SNIPERRIFLE_CLASSIC:
		if (!iRealAimType && iLastAimType && G::IsAttacking)
			Vars::Aimbot::General::AimType.Value = iLastAimType;
	}

	if (!Vars::Aimbot::General::AimType.Value || !G::CanPrimaryAttack && Vars::Aimbot::General::AimType.Value == 3 && (nWeaponID == TF_WEAPON_MINIGUN ? pWeapon->As<CTFMinigun>()->m_iWeaponState() == AC_STATE_FIRING || pWeapon->As<CTFMinigun>()->m_iWeaponState() == AC_STATE_SPINNING : true))
		return;

	switch (nWeaponID)
	{
	case TF_WEAPON_MINIGUN:
		pCmd->buttons |= IN_ATTACK2;
		if (pWeapon->As<CTFMinigun>()->m_iWeaponState() != AC_STATE_FIRING && pWeapon->As<CTFMinigun>()->m_iWeaponState() != AC_STATE_SPINNING)
			return;
		break;
	case TF_WEAPON_SNIPERRIFLE:
	case TF_WEAPON_SNIPERRIFLE_DECAP:
	{
		const bool bScoped = pLocal->IsScoped();

		if (Vars::Aimbot::Hitscan::Modifiers.Value & (1 << 4) && !bScoped)
		{
			pCmd->buttons |= IN_ATTACK2;
			return;
		}

		if ((Vars::Aimbot::Hitscan::Modifiers.Value & (1 << 3) || G::WeaponDefIndex == Sniper_m_TheMachina || G::WeaponDefIndex == Sniper_m_ShootingStar) && !bScoped)
			return;

		break;
	}
	case TF_WEAPON_SNIPERRIFLE_CLASSIC:
		if (iRealAimType)
			pCmd->buttons |= IN_ATTACK;
	}

	auto targets = SortTargets(pLocal, pWeapon);
	if (targets.empty())
		return;

	for (auto& target : targets)
	{
		const auto iResult = CanHit(target, pLocal, pWeapon);
		if (!iResult) continue;
		if (iResult == 2)
		{
			Aim(pCmd, target.m_vAngleTo);
			break;
		}

		G::Target = { target.m_pEntity->entindex(), I::GlobalVars->tickcount };
		if (Vars::Aimbot::General::AimType.Value == 3)
			G::AimPosition = target.m_vPos;

		bool bShouldFire = ShouldFire(pLocal, pWeapon, pCmd, target);

		if (bShouldFire)
		{
			pCmd->buttons |= IN_ATTACK;

			if (nWeaponID == TF_WEAPON_SNIPERRIFLE_CLASSIC && pWeapon->As<CTFSniperRifle>()->m_flChargedDamage())
				pCmd->buttons &= ~IN_ATTACK;

			if (G::WeaponDefIndex == Engi_s_TheWrangler || G::WeaponDefIndex == Engi_s_FestiveWrangler)
				pCmd->buttons |= IN_ATTACK2;

			if (Vars::Aimbot::Hitscan::Modifiers.Value & (1 << 0) && nWeaponID == TF_WEAPON_MINIGUN && !pLocal->IsPrecisionRune())
			{
				if (pLocal->GetShootPos().DistTo(target.m_vPos) > Vars::Aimbot::Hitscan::TapFireDist.Value && pWeapon->GetWeaponSpread() != 0.f)
				{
					const float flTimeSinceLastShot = (pLocal->m_nTickBase() * TICK_INTERVAL) - pWeapon->m_flLastFireTime();

					auto tfWeaponInfo = pWeapon->GetWeaponInfo();
					if (tfWeaponInfo && tfWeaponInfo->GetWeaponData(0).m_nBulletsPerShot > 1)
					{
						if (flTimeSinceLastShot <= 0.25f)
							pCmd->buttons &= ~IN_ATTACK;
					}
					else if (flTimeSinceLastShot <= 1.25f)
						pCmd->buttons &= ~IN_ATTACK;
				}
			}
		}

		G::IsAttacking = SDK::IsAttacking(pLocal, pWeapon, pCmd);

		if (G::IsAttacking)
		{
			if (target.m_pEntity->IsPlayer())
			F::Resolver.Aimbot(target.m_pEntity->As<CTFPlayer>(), target.m_nAimedHitbox == 0);

			if (target.m_bBacktrack)
				pCmd->tick_count = TIME_TO_TICKS(target.m_Tick.flSimTime) + TIME_TO_TICKS(F::Backtrack.flFakeInterp);

			if (!pWeapon->IsInReload())
			{
				if (Vars::Visuals::Bullet::BulletTracer.Value)
				{
					G::BulletsStorage.clear();
					G::BulletsStorage.push_back({ {pLocal->GetShootPos(), target.m_vPos}, I::GlobalVars->curtime + 5.f, Vars::Colors::BulletTracer.Value, true });
				}
				if (Vars::Visuals::Hitbox::ShowHitboxes.Value)
				{
					G::BoxesStorage.clear();
					auto vBoxes = F::Visuals.GetHitboxes((matrix3x4*)(&target.m_Tick.BoneMatrix.BoneMatrix), target.m_pEntity->As<CBaseAnimating>());
					G::BoxesStorage.insert(G::BoxesStorage.end(), vBoxes.begin(), vBoxes.end());
				}
			}
		}

		Aim(pCmd, target.m_vAngleTo);
		break;
	}
}