#include "AimbotHitscan.h"

#include "../../Resolver/Resolver.h"
#include "../../TickHandler/TickHandler.h"
#include "../../Visuals/Visuals.h"

std::vector<Target_t> CAimbotHitscan::GetTargetsMedigun(CTFPlayer* pLocal, CWeaponMedigun* pWeapon)
{
	if (!Vars::Aimbot::Healing::AutoHeal.Value)
		return {};

	std::vector<Target_t> vTargets;
	const auto sortMethod = ESortMethod(Vars::Aimbot::General::TargetSelection.Value);

	Vec3 vLocalPos = pLocal->GetShootPos();
	Vec3 vLocalAngles = I::EngineClient->GetViewAngles();

	for (auto pEntity : H::Entities.GetGroup(EGroupType::PLAYERS_TEAMMATES))
	{
		auto pPlayer = pEntity->As<CTFPlayer>();
		if (pPlayer == pLocal || !pPlayer->IsAlive() || pPlayer->IsAGhost() || pPlayer->InCond(TF_COND_STEALTHED))
			continue;

		if (Vars::Aimbot::Healing::FriendsOnly.Value && !H::Entities.IsFriend(pEntity->entindex()))
			continue;

		float flFOVTo; Vec3 vPos, vAngleTo;
		if (!F::AimbotGlobal.PlayerBoneInFOV(pPlayer, vLocalPos, vLocalAngles, flFOVTo, vPos, vAngleTo) && pWeapon->m_hHealingTarget().Get() != pPlayer)
			continue;

		float flDistTo = sortMethod == ESortMethod::DISTANCE ? vLocalPos.DistTo(vPos) : 0.f;
		vTargets.push_back({ pPlayer, ETargetType::PLAYER, vPos, vAngleTo, flFOVTo, flDistTo });
	}

	return vTargets;
}

std::vector<Target_t> CAimbotHitscan::GetTargets(CTFPlayer* pLocal, CTFWeaponBase* pWeapon)
{
	if (pWeapon->GetWeaponID() == TF_WEAPON_MEDIGUN)
		return GetTargetsMedigun(pLocal, pWeapon->As<CWeaponMedigun>());

	std::vector<Target_t> vTargets;
	const auto sortMethod = ESortMethod(Vars::Aimbot::General::TargetSelection.Value);

	Vec3 vLocalPos = pLocal->GetShootPos();
	Vec3 vLocalAngles = I::EngineClient->GetViewAngles();

	if (Vars::Aimbot::General::Target.Value & PLAYER)
	{
		bool bPissRifle = pWeapon->m_iItemDefinitionIndex() == Sniper_m_TheSydneySleeper;
		for (auto pEntity : H::Entities.GetGroup(bPissRifle ? EGroupType::PLAYERS_ALL : EGroupType::PLAYERS_ENEMIES))
		{
			if (F::AimbotGlobal.ShouldIgnore(pEntity, pLocal, pWeapon))
				continue;

			if (pWeapon->m_iItemDefinitionIndex() == Sniper_m_TheSydneySleeper && pEntity->m_iTeamNum() == pLocal->m_iTeamNum()
				&& (!(Vars::Aimbot::Hitscan::Modifiers.Value & (1 << 6)) || !pEntity->As<CTFPlayer>()->IsOnFire()))
				continue;

			float flFOVTo; Vec3 vPos, vAngleTo;
			if (!F::AimbotGlobal.PlayerBoneInFOV(pEntity->As<CTFPlayer>(), vLocalPos, vLocalAngles, flFOVTo, vPos, vAngleTo))
				continue;

			float flDistTo = sortMethod == ESortMethod::DISTANCE ? vLocalPos.DistTo(vPos) : 0.f;
			vTargets.push_back({ pEntity, ETargetType::PLAYER, vPos, vAngleTo, flFOVTo, flDistTo, F::AimbotGlobal.GetPriority(pEntity->entindex()) });
		}
	}

	if (Vars::Aimbot::General::Target.Value)
	{
		for (auto pEntity : H::Entities.GetGroup(EGroupType::BUILDINGS_ENEMIES))
		{
			if (F::AimbotGlobal.ShouldIgnore(pEntity, pLocal, pWeapon))
				continue;

			Vec3 vPos = pEntity->GetCenter();
			Vec3 vAngleTo = Math::CalcAngle(vLocalPos, vPos);
			float flFOVTo = Math::CalcFov(vLocalAngles, vAngleTo);
			if (flFOVTo > Vars::Aimbot::General::AimFOV.Value)
				continue;

			float flDistTo = sortMethod == ESortMethod::DISTANCE ? vLocalPos.DistTo(vPos) : 0.f;
			vTargets.push_back({ pEntity, pEntity->IsSentrygun() ? ETargetType::SENTRY : pEntity->IsDispenser() ? ETargetType::DISPENSER : ETargetType::TELEPORTER, vPos, vAngleTo, flFOVTo, flDistTo });
		}
	}

	if (Vars::Aimbot::General::Target.Value & STICKY)
	{
		for (auto pEntity : H::Entities.GetGroup(EGroupType::WORLD_PROJECTILES))
		{
			if (F::AimbotGlobal.ShouldIgnore(pEntity, pLocal, pWeapon))
				continue;

			Vec3 vPos = pEntity->GetCenter();
			Vec3 vAngleTo = Math::CalcAngle(vLocalPos, vPos);
			float flFOVTo = Math::CalcFov(vLocalAngles, vAngleTo);
			if (flFOVTo > Vars::Aimbot::General::AimFOV.Value)
				continue;

			float flDistTo = sortMethod == ESortMethod::DISTANCE ? vLocalPos.DistTo(vPos) : 0.f;
			vTargets.push_back({ pEntity, ETargetType::STICKY, vPos, vAngleTo, flFOVTo, flDistTo });
		}
	}

	if (Vars::Aimbot::General::Target.Value & NPC)
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

			float flDistTo = sortMethod == ESortMethod::DISTANCE ? vLocalPos.DistTo(vPos) : 0.f;
			vTargets.push_back({ pEntity, ETargetType::NPC, vPos, vAngleTo, flFOVTo, flDistTo });
		}
	}

	if (Vars::Aimbot::General::Target.Value & BOMB)
	{
		for (auto pEntity : H::Entities.GetGroup(EGroupType::WORLD_BOMBS))
		{
			Vec3 vPos = pEntity->GetCenter();
			Vec3 vAngleTo = Math::CalcAngle(vLocalPos, vPos);
			float flFOVTo = Math::CalcFov(vLocalAngles, vAngleTo);
			if (flFOVTo > Vars::Aimbot::General::AimFOV.Value)
				continue;

			if (!F::AimbotGlobal.ValidBomb(pLocal, pWeapon, pEntity))
				continue;

			float flDistTo = sortMethod == ESortMethod::DISTANCE ? vLocalPos.DistTo(vPos) : 0.f;
			vTargets.push_back({ pEntity, ETargetType::BOMBS, vPos, vAngleTo, flFOVTo, flDistTo });
		}
	}

	return vTargets;
}

std::vector<Target_t> CAimbotHitscan::SortTargets(CTFPlayer* pLocal, CTFWeaponBase* pWeapon)
{
	auto vTargets = GetTargets(pLocal, pWeapon);

	F::AimbotGlobal.SortTargets(&vTargets, ESortMethod(Vars::Aimbot::General::TargetSelection.Value));
	vTargets.resize(std::min(size_t(Vars::Aimbot::General::MaxTargets.Value), vTargets.size()));
	F::AimbotGlobal.SortPriority(&vTargets);
	return vTargets;
}



int CAimbotHitscan::GetHitboxPriority(int nHitbox, CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CBaseEntity* pTarget)
{
	bool bHeadshot = false;
	{
		const int nClassNum = pLocal->m_iClass();

		if (nClassNum == TF_CLASS_SNIPER)
		{
			if (pWeapon->m_iItemDefinitionIndex() != Sniper_m_TheClassic ? pLocal->IsScoped() : pWeapon->As<CTFSniperRifle>()->m_flChargedDamage() == 150.f)
				bHeadshot = true;
		}
		if (nClassNum == TF_CLASS_SPY)
		{
			switch (pWeapon->m_iItemDefinitionIndex())
			{
			case Spy_m_TheAmbassador:
			case Spy_m_FestiveAmbassador:
				if (pWeapon->AmbassadorCanHeadshot())
					bHeadshot = true;
			}
		}

		if (Vars::Aimbot::Hitscan::Modifiers.Value & (1 << 5) && bHeadshot && pTarget->IsPlayer())
		{
			{
				float flBodyMult = 1.f;
				switch (pWeapon->m_iItemDefinitionIndex())
				{
				case Sniper_m_TheClassic: flBodyMult = 0.9f; break;
				case Sniper_m_TheHitmansHeatmaker: flBodyMult = 0.8f; break;
				case Sniper_m_TheMachina:
				case Sniper_m_ShootingStar: if (pWeapon->As<CTFSniperRifle>()->m_flChargedDamage() == 150.f) flBodyMult = 1.15f;
				}
				if (pWeapon->As<CTFSniperRifle>()->m_flChargedDamage() * flBodyMult >= pTarget->As<CTFPlayer>()->m_iHealth())
					bHeadshot = false;
			}

			switch (pWeapon->m_iItemDefinitionIndex())
			{
			case Spy_m_TheAmbassador:
			case Spy_m_FestiveAmbassador:
			{
				const float flDistTo = pTarget->m_vecOrigin().DistTo(pLocal->m_vecOrigin());
				const int nAmbassadorBodyshotDamage = Math::RemapValClamped(flDistTo, 90, 900, 51, 18);

				if (pTarget->As<CTFPlayer>()->m_iHealth() < (nAmbassadorBodyshotDamage + 2)) // whatever
					bHeadshot = false;
			}
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
	switch (pWeapon->GetWeaponID())
	{
	case TF_WEAPON_MEDIGUN: return 450.f;
	case TF_WEAPON_MECHANICAL_ARM: return 256.f;
	}

	auto pWeaponInfo = pWeapon->GetWeaponInfo();
	return pWeaponInfo ? pWeaponInfo->GetWeaponData(0).m_flRange : 8192.f;
}

int CAimbotHitscan::CanHit(Target_t& target, CTFPlayer* pLocal, CTFWeaponBase* pWeapon)
{
	if (Vars::Aimbot::General::Ignore.Value & UNSIMULATED && H::Entities.GetChoke(target.m_pEntity) > Vars::Aimbot::General::TickTolerance.Value)
		return false;

	Vec3 vEyePos = pLocal->GetShootPos(), vPeekPos = {};
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
		if (pRecords)
		{
			vRecords = F::Backtrack.GetValidRecords(pRecords, pLocal);
			if (!Vars::Backtrack::Enabled.Value && !vRecords.empty())
				vRecords = { vRecords.front() };
		}
		if (!pRecords || vRecords.empty())
		{
			matrix3x4 aBones[MAXSTUDIOBONES];
			if (!target.m_pEntity->SetupBones(aBones, MAXSTUDIOBONES, BONE_USED_BY_ANYTHING, target.m_pEntity->m_flSimulationTime()))
				return false;

			vRecords.push_front({
				target.m_pEntity->m_flSimulationTime(),
				*reinterpret_cast<BoneMatrix*>(&aBones),
				target.m_pEntity->m_vecOrigin()
			});
		}
	}

	float flSpread = pWeapon->GetWeaponSpread();
	if (flSpread && Vars::Aimbot::General::HitscanPeek.Value)
		vPeekPos = pLocal->GetShootPos() + pLocal->GetAbsVelocity() * TICKS_TO_TIME(-Vars::Aimbot::General::HitscanPeek.Value);

	static float flPreferredRecord = 0; // if we're doubletapping, we can't change viewangles so have a preferred tick to use
	if (!I::ClientState->chokedcommands)
		flPreferredRecord = 0;
	if (flPreferredRecord)
	{
		auto pivot = std::find_if(vRecords.begin(), vRecords.end(), [](auto& s) -> bool { return s.flSimTime == flPreferredRecord; });
		if (pivot != vRecords.end())
			std::rotate(vRecords.begin(), pivot, pivot + 1);
	}

	auto RayToOBB = [](const Vec3& origin, const Vec3& direction, const Vec3& min, const Vec3& max, const matrix3x4& matrix) -> bool
		{
			if (Vars::Aimbot::General::AimType.Value != 2)
				return true;

			return Math::RayToOBB(origin, direction, min, max, matrix);
		};

	int iReturn = false;
	for (auto& pTick : vRecords)
	{
		bool bRunPeekCheck = flSpread && (Vars::Aimbot::General::PeekDTOnly.Value ? F::Ticks.GetTicks(pLocal) : true) && Vars::Aimbot::General::HitscanPeek.Value;

		if (target.m_TargetType == ETargetType::PLAYER || target.m_TargetType == ETargetType::SENTRY)
		{
			auto aBones = pTick.BoneMatrix.aBones;
			if (!aBones)
				continue;

			std::vector<std::pair<const mstudiobbox_t*, int>> vHitboxes;
			{
				if (target.m_TargetType != ETargetType::SENTRY)
				{
					std::vector<std::pair<const mstudiobbox_t*, int>> primary, secondary, tertiary; // dumb
					for (int nHitbox = 0; nHitbox < target.m_pEntity->As<CTFPlayer>()->GetNumOfHitboxes(); nHitbox++)
					{
						if (!F::AimbotGlobal.IsHitboxValid(nHitbox))
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
					for (auto& pair : primary) vHitboxes.push_back(pair);
					for (auto& pair : secondary) vHitboxes.push_back(pair);
					for (auto& pair : tertiary) vHitboxes.push_back(pair);
				}
				else
				{
					Vec3 vCenter = target.m_pEntity->GetCenter();
					std::vector<std::pair<float, std::pair<const mstudiobbox_t*, int>>> vCenters = {};
					for (int nHitbox = 0; nHitbox < target.m_pEntity->As<CObjectSentrygun>()->GetNumOfHitboxes(); nHitbox++)
					{
						const mstudiobbox_t* pBox = pSet->pHitbox(nHitbox);
						if (pBox)
							vCenters.push_back({ target.m_pEntity->As<CBaseAnimating>()->GetHitboxCenter(nHitbox).DistTo(vCenter), {pBox, nHitbox}});
					}
					std::sort(vCenters.begin(), vCenters.end(), [&](const auto& a, const auto& b) -> bool
						{
							return a.first < b.first;
						});
					for (auto& pair : vCenters)
						vHitboxes.push_back(pair.second);
				}
			}

			for (auto& pair : vHitboxes)
			{
				Vec3 vMins = pair.first->bbmin;
				Vec3 vMaxs = pair.first->bbmax;

				float flScale = Vars::Aimbot::Hitscan::PointScale.Value / 100;
				Vec3 vMinsU = (vMins - vMaxs) / 2, vMinsS = vMinsU * flScale;
				Vec3 vMaxsU = (vMaxs - vMins) / 2, vMaxsS = vMaxsU * flScale;

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

				Vec3 vOffset;
				{
					Vec3 vOrigin, vCenter;
					Math::VectorTransform({}, aBones[pair.first->bone], vOrigin);
					Math::VectorTransform((vMins + vMaxs) / 2, aBones[pair.first->bone], vCenter);
					vOffset = vCenter - vOrigin;
				}

				for (auto& point : vecPoints)
				{
					Vec3 vTransformed = {}; Math::VectorTransform(point, aBones[pair.first->bone], vTransformed); vTransformed += vOffset;

					if (vEyePos.DistToSqr(vTransformed) > flMaxRange)
						continue;

					auto vAngles = Aim(G::CurrentUserCmd->viewangles, Math::CalcAngle(pLocal->GetShootPos(), vTransformed));
					Vec3 vForward; Math::AngleVectors(vAngles, &vForward);

					if (bRunPeekCheck)
					{
						bRunPeekCheck = false;
						if (!SDK::VisPos(pLocal, target.m_pEntity, vPeekPos, vTransformed))
							goto nextTick; // if we can't hit our primary hitbox, don't bother
					}

					if (SDK::VisPos(pLocal, target.m_pEntity, vEyePos, vTransformed))
					{
						target.m_vAngleTo = vAngles;
						if (RayToOBB(vEyePos, vForward, vMins, vMaxs, aBones[pair.first->bone])) // for the time being, no vischecks against other hitboxes
						{
							bool bWillHit = true;
							if (target.m_TargetType == ETargetType::SENTRY) // point of hit for sentries needs to be within bbox
							{
								const matrix3x4& transform = target.m_pEntity->RenderableToWorldTransform();
								const Vec3 vMin = target.m_pEntity->m_vecMins(), vMax = target.m_pEntity->m_vecMaxs();
								bWillHit = RayToOBB(vEyePos, vForward, vMin, vMax, transform);
							}
							if (bWillHit)
							{
								flPreferredRecord = pTick.flSimTime;

								target.m_Tick = pTick;
								target.m_vPos = vTransformed;
								if (target.m_TargetType == ETargetType::PLAYER)
								{
									target.m_bBacktrack = true; //target.m_TargetType == ETargetType::PLAYER /*&& Vars::Backtrack::Enabled.Value*/;
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
			for (auto& point : vecPoints)
			{
				Vec3 vTransformed = target.m_pEntity->GetCenter() + point;

				if (vEyePos.DistToSqr(vTransformed) > flMaxRange)
					continue;

				auto vAngles = Aim(G::CurrentUserCmd->viewangles, Math::CalcAngle(pLocal->GetShootPos(), vTransformed));
				Vec3 vForward; Math::AngleVectors(vAngles, &vForward);

				if (bRunPeekCheck)
				{
					bRunPeekCheck = false;
					if (!SDK::VisPos(pLocal, target.m_pEntity, vPeekPos, vTransformed))
						goto nextTick; // if we can't hit our primary hitbox, don't bother
				}

				if (SDK::VisPos(pLocal, target.m_pEntity, vEyePos, vTransformed))
				{
					target.m_vAngleTo = vAngles;
					if (RayToOBB(vEyePos, vForward, vMins, vMaxs, transform)) // for the time being, no vischecks against other hitboxes
					{
						target.m_Tick = pTick;
						target.m_vPos = vTransformed;
						return true;
					}
					iReturn = 2;
				}
			}
		}

	nextTick:
		continue;
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
			if (pWeapon->m_iItemDefinitionIndex() != Sniper_m_TheClassic
				&& pWeapon->m_iItemDefinitionIndex() != Sniper_m_TheSydneySleeper
				&& !G::CanHeadshot && bIsScoped)
			{
				return false;
			}
		}

		if (Vars::Aimbot::Hitscan::Modifiers.Value & (1 << 2) && (bIsScoped || pWeapon->m_iItemDefinitionIndex() == Sniper_m_TheClassic))
		{
			auto pPlayer = target.m_pEntity->As<CTFPlayer>();
			auto pSniperRifle = pWeapon->As<CTFSniperRifle>();
			const int nHealth = pPlayer->m_iHealth();
			const bool bIsCritBoosted = pLocal->IsCritBoosted();

			if (target.m_nAimedHitbox == HITBOX_HEAD && pWeapon->m_iItemDefinitionIndex() != Sniper_m_TheSydneySleeper && (pWeapon->m_iItemDefinitionIndex() != Sniper_m_TheClassic ?  true : pSniperRifle->m_flChargedDamage() == 150.f))
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
					switch (pWeapon->m_iItemDefinitionIndex())
					{
					case Sniper_m_TheClassic: flBodyMult = 0.9f; break;
					case Sniper_m_TheHitmansHeatmaker: flBodyMult = 0.8f; break;
					case Sniper_m_TheMachina:
					case Sniper_m_ShootingStar: if (pSniperRifle->m_flChargedDamage() == 150.f) flBodyMult = 1.15f;
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

	if (auto pLocal = H::Entities.GetLocal())
		vToAngle -= pLocal->m_vecPunchAngle();
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
	const int nWeaponID = pWeapon->GetWeaponID();

	static int iStaticAimType = Vars::Aimbot::General::AimType.Value;
	const int iRealAimType = Vars::Aimbot::General::AimType.Value;
	const int iLastAimType = iStaticAimType;
	iStaticAimType = iRealAimType;

	switch (nWeaponID)
	{
	case TF_WEAPON_SNIPERRIFLE_CLASSIC:
		if (!iRealAimType && iLastAimType && G::IsAttacking)
			Vars::Aimbot::General::AimType.Value = iLastAimType;
	}

	if ((Vars::Aimbot::General::AimHoldsFire.Value == 2 || Vars::Aimbot::General::AimHoldsFire.Value == 1 && nWeaponID == TF_WEAPON_MINIGUN) && !G::CanPrimaryAttack && G::LastUserCmd->buttons & IN_ATTACK && Vars::Aimbot::General::AimType.Value && !pWeapon->IsInReload())
		pCmd->buttons |= IN_ATTACK;
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

		if (Vars::Aimbot::Hitscan::Modifiers.Value & (1 << 3))
		{
			switch (pWeapon->m_iItemDefinitionIndex())
			{
			case Sniper_m_TheMachina:
			case Sniper_m_ShootingStar:
				if (!bScoped)
					return;
			}
		}

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
		if (pWeapon->GetWeaponID() == TF_WEAPON_MEDIGUN && pWeapon->As<CWeaponMedigun>()->m_hHealingTarget().Get() == target.m_pEntity)
		{
			if (G::LastUserCmd->buttons & IN_ATTACK)
				pCmd->buttons |= IN_ATTACK;
			return;
		}

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
			if (pWeapon->GetWeaponID() != TF_WEAPON_MEDIGUN || !(G::LastUserCmd->buttons & IN_ATTACK))
				pCmd->buttons |= IN_ATTACK;

			if (nWeaponID == TF_WEAPON_SNIPERRIFLE_CLASSIC && pWeapon->As<CTFSniperRifle>()->m_flChargedDamage())
				pCmd->buttons &= ~IN_ATTACK;

			switch (pWeapon->m_iItemDefinitionIndex())
			{
			case Engi_s_TheWrangler:
			case Engi_s_FestiveWrangler:
				pCmd->buttons |= IN_ATTACK2;
			}

			if (Vars::Aimbot::Hitscan::Modifiers.Value & (1 << 0) && nWeaponID == TF_WEAPON_MINIGUN && !pLocal->IsPrecisionRune())
			{
				if (pLocal->GetShootPos().DistTo(target.m_vPos) > Vars::Aimbot::Hitscan::TapFireDist.Value && pWeapon->GetWeaponSpread() != 0.f)
				{
					const float flTimeSinceLastShot = (pLocal->m_nTickBase() * TICK_INTERVAL) - pWeapon->m_flLastFireTime();

					auto pWeaponInfo = pWeapon->GetWeaponInfo();
					if (pWeaponInfo && pWeaponInfo->GetWeaponData(0).m_nBulletsPerShot > 1)
					{
						if (flTimeSinceLastShot <= 0.25f)
							pCmd->buttons &= ~IN_ATTACK;
					}
					else if (flTimeSinceLastShot <= 1.25f)
						pCmd->buttons &= ~IN_ATTACK;
				}
			}
		}

		G::IsAttacking = SDK::IsAttacking(pLocal, pWeapon, pCmd, true);

		if (G::IsAttacking)
		{
			if (target.m_pEntity->IsPlayer())
				F::Resolver.Aimbot(target.m_pEntity->As<CTFPlayer>(), target.m_nAimedHitbox == 0);

			if (target.m_bBacktrack)
				pCmd->tick_count = TIME_TO_TICKS(target.m_Tick.flSimTime) + TIME_TO_TICKS(F::Backtrack.flFakeInterp);

			if (!pWeapon->IsInReload())
			{
				if (Vars::Visuals::Bullet::Enabled.Value)
				{
					G::LineStorage.clear();
					G::LineStorage.push_back({ {pLocal->GetShootPos(), target.m_vPos}, I::GlobalVars->curtime + 5.f, Vars::Colors::Bullet.Value, true });
				}
				if (Vars::Visuals::Hitbox::Enabled.Value & (1 << 0))
				{
					G::BoxStorage.clear();
					auto vBoxes = F::Visuals.GetHitboxes(target.m_Tick.BoneMatrix.aBones, target.m_pEntity->As<CBaseAnimating>());
					G::BoxStorage.insert(G::BoxStorage.end(), vBoxes.begin(), vBoxes.end());
				}
			}
		}

		if (!pWeapon->IsInReload())
			Aim(pCmd, target.m_vAngleTo);
		break;
	}
}