#include "AimbotHitscan.h"

#include "../../Resolver/Resolver.h"
#include "../../TickHandler/TickHandler.h"
#include "../../Visuals/Visuals.h"

std::vector<Target_t> CAimbotHitscan::GetTargetsMedigun(CTFPlayer* pLocal, CWeaponMedigun* pWeapon)
{
	if (!Vars::Aimbot::Healing::AutoHeal.Value)
		return {};

	std::vector<Target_t> vTargets;
	const auto iSort = Vars::Aimbot::General::TargetSelection.Value;

	Vec3 vLocalPos = F::Ticks.GetShootPos();
	Vec3 vLocalAngles = I::EngineClient->GetViewAngles();

	for (auto pEntity : H::Entities.GetGroup(EGroupType::PLAYERS_TEAMMATES))
	{
		auto pPlayer = pEntity->As<CTFPlayer>();
		if (pPlayer == pLocal || pPlayer->IsDormant() || !pPlayer->IsAlive() || pPlayer->IsAGhost() || pPlayer->InCond(TF_COND_STEALTHED))
			continue;

		if (Vars::Aimbot::Healing::FriendsOnly.Value && !H::Entities.IsFriend(pEntity->entindex()) && !H::Entities.InParty(pEntity->entindex()))
			continue;

		float flFOVTo; Vec3 vPos, vAngleTo;
		if (!F::AimbotGlobal.PlayerBoneInFOV(pPlayer, vLocalPos, vLocalAngles, flFOVTo, vPos, vAngleTo, Vars::Aimbot::Hitscan::Hitboxes.Value) && pWeapon->m_hHealingTarget().Get() != pPlayer)
			continue;

		float flDistTo = iSort == Vars::Aimbot::General::TargetSelectionEnum::Distance ? vLocalPos.DistTo(vPos) : 0.f;
		vTargets.push_back({ pPlayer, TargetEnum::Player, vPos, vAngleTo, flFOVTo, flDistTo });
	}

	return vTargets;
}

std::vector<Target_t> CAimbotHitscan::GetTargets(CTFPlayer* pLocal, CTFWeaponBase* pWeapon)
{
	if (pWeapon->GetWeaponID() == TF_WEAPON_MEDIGUN)
		return GetTargetsMedigun(pLocal, pWeapon->As<CWeaponMedigun>());

	std::vector<Target_t> vTargets;
	const auto iSort = Vars::Aimbot::General::TargetSelection.Value;

	Vec3 vLocalPos = F::Ticks.GetShootPos();
	Vec3 vLocalAngles = I::EngineClient->GetViewAngles();

	if (Vars::Aimbot::General::Target.Value & Vars::Aimbot::General::TargetEnum::Players)
	{
		bool bPissRifle = pWeapon->m_iItemDefinitionIndex() == Sniper_m_TheSydneySleeper;
		for (auto pEntity : H::Entities.GetGroup(bPissRifle ? EGroupType::PLAYERS_ALL : EGroupType::PLAYERS_ENEMIES))
		{
			bool bTeammate = pEntity->m_iTeamNum() == pLocal->m_iTeamNum();
			if (F::AimbotGlobal.ShouldIgnore(pEntity, pLocal, pWeapon))
				continue;

			if (pWeapon->m_iItemDefinitionIndex() == Sniper_m_TheSydneySleeper && bTeammate
				&& (!(Vars::Aimbot::Hitscan::Modifiers.Value & Vars::Aimbot::Hitscan::ModifiersEnum::ExtinguishTeam) || !pEntity->As<CTFPlayer>()->InCond(TF_COND_BURNING)))
				continue;

			float flFOVTo; Vec3 vPos, vAngleTo;
			if (!F::AimbotGlobal.PlayerBoneInFOV(pEntity->As<CTFPlayer>(), vLocalPos, vLocalAngles, flFOVTo, vPos, vAngleTo, Vars::Aimbot::Hitscan::Hitboxes.Value))
				continue;

			float flDistTo = iSort == Vars::Aimbot::General::TargetSelectionEnum::Distance ? vLocalPos.DistTo(vPos) : 0.f;
			vTargets.push_back({ pEntity, TargetEnum::Player, vPos, vAngleTo, flFOVTo, flDistTo, bTeammate ? 0 : F::AimbotGlobal.GetPriority(pEntity->entindex()) });
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

			float flDistTo = iSort == Vars::Aimbot::General::TargetSelectionEnum::Distance ? vLocalPos.DistTo(vPos) : 0.f;
			vTargets.push_back({ pEntity, pEntity->IsSentrygun() ? TargetEnum::Sentry : pEntity->IsDispenser() ? TargetEnum::Dispenser : TargetEnum::Teleporter, vPos, vAngleTo, flFOVTo, flDistTo });
		}
	}

	if (Vars::Aimbot::General::Target.Value & Vars::Aimbot::General::TargetEnum::Stickies)
	{
		for (auto pEntity : H::Entities.GetGroup(EGroupType::WORLD_PROJECTILES))
		{
			if (F::AimbotGlobal.ShouldIgnore(pEntity, pLocal, pWeapon))
				continue;

			Vec3 vPos = pEntity->m_vecOrigin();
			Vec3 vAngleTo = Math::CalcAngle(vLocalPos, vPos);
			float flFOVTo = Math::CalcFov(vLocalAngles, vAngleTo);
			if (flFOVTo > Vars::Aimbot::General::AimFOV.Value)
				continue;

			float flDistTo = iSort == Vars::Aimbot::General::TargetSelectionEnum::Distance ? vLocalPos.DistTo(vPos) : 0.f;
			vTargets.push_back({ pEntity, TargetEnum::Sticky, vPos, vAngleTo, flFOVTo, flDistTo });
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

			float flDistTo = iSort == Vars::Aimbot::General::TargetSelectionEnum::Distance ? vLocalPos.DistTo(vPos) : 0.f;
			vTargets.push_back({ pEntity, TargetEnum::NPC, vPos, vAngleTo, flFOVTo, flDistTo });
		}
	}

	if (Vars::Aimbot::General::Target.Value & Vars::Aimbot::General::TargetEnum::Bombs)
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

			float flDistTo = iSort == Vars::Aimbot::General::TargetSelectionEnum::Distance ? vLocalPos.DistTo(vPos) : 0.f;
			vTargets.push_back({ pEntity, TargetEnum::Bomb, vPos, vAngleTo, flFOVTo, flDistTo });
		}
	}

	return vTargets;
}

std::vector<Target_t> CAimbotHitscan::SortTargets(CTFPlayer* pLocal, CTFWeaponBase* pWeapon)
{
	auto vTargets = GetTargets(pLocal, pWeapon);

	F::AimbotGlobal.SortTargets(&vTargets, Vars::Aimbot::General::TargetSelection.Value);
	vTargets.resize(std::min(size_t(Vars::Aimbot::General::MaxTargets.Value), vTargets.size()));
	F::AimbotGlobal.SortPriority(&vTargets);
	return vTargets;
}



int CAimbotHitscan::GetHitboxPriority(int nHitbox, CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CBaseEntity* pTarget)
{
	bool bHeadshot = false;
	if (pTarget->IsPlayer())
	{
		switch (pWeapon->GetWeaponID())
		{
		case TF_WEAPON_SNIPERRIFLE:
		case TF_WEAPON_SNIPERRIFLE_DECAP:
		case TF_WEAPON_SNIPERRIFLE_CLASSIC:
		{
			auto pSniperRifle = pWeapon->As<CTFSniperRifle>();

			if (G::CanHeadshot
				|| pLocal->InCond(TF_COND_AIMING) && (
					pSniperRifle->GetRifleType() == RIFLE_JARATE && SDK::AttribHookValue(0, "jarate_duration", pWeapon) > 0
					|| Vars::Aimbot::Hitscan::Modifiers.Value & Vars::Aimbot::Hitscan::ModifiersEnum::WaitForHeadshot
				))
				bHeadshot = true;
			break;
		}
		case TF_WEAPON_REVOLVER:
		{
			if (SDK::AttribHookValue(0, "set_weapon_mode", pWeapon) == 1
				&& (pWeapon->AmbassadorCanHeadshot() || Vars::Aimbot::Hitscan::Modifiers.Value & Vars::Aimbot::Hitscan::ModifiersEnum::WaitForHeadshot))
				bHeadshot = true;
		}
		}

		if (Vars::Aimbot::Hitscan::Modifiers.Value & Vars::Aimbot::Hitscan::ModifiersEnum::BodyaimIfLethal && bHeadshot)
		{
			auto pPlayer = pTarget->As<CTFPlayer>();

			switch (pWeapon->GetWeaponID())
			{
			case TF_WEAPON_SNIPERRIFLE:
			case TF_WEAPON_SNIPERRIFLE_DECAP:
			case TF_WEAPON_SNIPERRIFLE_CLASSIC:
			{
				auto pSniperRifle = pWeapon->As<CTFSniperRifle>();

				int iDamage = std::ceil(std::max(pSniperRifle->m_flChargedDamage(), 50.f) * pSniperRifle->GetBodyshotMult(pPlayer));
				if (pPlayer->m_iHealth() <= iDamage)
					bHeadshot = false;
				break;
			}
			case TF_WEAPON_REVOLVER:
			{
				if (SDK::AttribHookValue(0, "set_weapon_mode", pWeapon) == 1)
				{
					float flDistTo = pTarget->m_vecOrigin().DistTo(pLocal->m_vecOrigin());

					float flMult = SDK::AttribHookValue(1.f, "mult_dmg", pWeapon);
					int iDamage = std::ceil(Math::RemapValClamped(flDistTo, 90.f, 900.f, 60.f, 21.f) * flMult);
					if (pPlayer->m_iHealth() <= iDamage)
						bHeadshot = false;
				}
			}
			}
		}
	}

	switch (H::Entities.GetModel(pTarget->entindex()))
	{
	case FNV1A::Hash32Const("models/vsh/player/saxton_hale.mdl"):
	{
		switch (nHitbox)
		{
		case HITBOX_SAXTON_HEAD: return bHeadshot ? 0 : 2;
		//case HITBOX_SAXTON_NECK:
		//case HITBOX_SAXTON_PELVIS: return 2;
		case HITBOX_SAXTON_BODY:
		case HITBOX_SAXTON_THORAX:
		case HITBOX_SAXTON_CHEST:
		case HITBOX_SAXTON_UPPER_CHEST: return bHeadshot ? 1 : 0;
		/*
		case HITBOX_SAXTON_LEFT_UPPER_ARM:
		case HITBOX_SAXTON_LEFT_FOREARM:
		case HITBOX_SAXTON_LEFT_HAND:
		case HITBOX_SAXTON_RIGHT_UPPER_ARM:
		case HITBOX_SAXTON_RIGHT_FOREARM:
		case HITBOX_SAXTON_RIGHT_HAND:
		case HITBOX_SAXTON_LEFT_THIGH:
		case HITBOX_SAXTON_LEFT_CALF:
		case HITBOX_SAXTON_LEFT_FOOT:
		case HITBOX_SAXTON_RIGHT_THIGH:
		case HITBOX_SAXTON_RIGHT_CALF:
		case HITBOX_SAXTON_RIGHT_FOOT:
		*/
		}
		break;
	}
	default:
	{
		switch (nHitbox)
		{
		case HITBOX_HEAD: return bHeadshot ? 0 : 2;
		//case HITBOX_PELVIS: return 2;
		case HITBOX_BODY:
		case HITBOX_THORAX:
		case HITBOX_CHEST:
		case HITBOX_UPPER_CHEST: return bHeadshot ? 1 : 0;
		/*
		case HITBOX_LEFT_UPPER_ARM:
		case HITBOX_LEFT_FOREARM:
		case HITBOX_LEFT_HAND:
		case HITBOX_RIGHT_UPPER_ARM:
		case HITBOX_RIGHT_FOREARM:
		case HITBOX_RIGHT_HAND:
		case HITBOX_LEFT_THIGH:
		case HITBOX_LEFT_CALF:
		case HITBOX_LEFT_FOOT:
		case HITBOX_RIGHT_THIGH:
		case HITBOX_RIGHT_CALF:
		case HITBOX_RIGHT_FOOT:
		*/
		}
	}
	}

	return 2;
};

int CAimbotHitscan::CanHit(Target_t& target, CTFPlayer* pLocal, CTFWeaponBase* pWeapon)
{
	if (Vars::Aimbot::General::Ignore.Value & Vars::Aimbot::General::IgnoreEnum::Unsimulated && H::Entities.GetChoke(target.m_pEntity->entindex()) > Vars::Aimbot::General::TickTolerance.Value)
		return false;

	Vec3 vEyePos = pLocal->GetShootPos(), vPeekPos = {};
	const float flMaxRange = powf(pWeapon->GetRange(), 2.f);

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
			if (auto pBones = H::Entities.GetBones(target.m_pEntity->entindex()))
				vRecords.push_front({ target.m_pEntity->m_flSimulationTime(), *reinterpret_cast<BoneMatrix*>(pBones), target.m_pEntity->m_vecOrigin() });
			else
			{
				matrix3x4 aBones[MAXSTUDIOBONES];
				if (!target.m_pEntity->SetupBones(aBones, MAXSTUDIOBONES, BONE_USED_BY_ANYTHING, target.m_pEntity->m_flSimulationTime()))
					return false;

				vRecords.push_front({ target.m_pEntity->m_flSimulationTime(), *reinterpret_cast<BoneMatrix*>(&aBones), target.m_pEntity->m_vecOrigin() });
			}
		}
	}

	float flSpread = pWeapon->GetWeaponSpread();
	if (flSpread && Vars::Aimbot::General::HitscanPeek.Value)
		vPeekPos = pLocal->GetShootPos() + pLocal->m_vecVelocity() * TICKS_TO_TIME(-Vars::Aimbot::General::HitscanPeek.Value);

	static float flPreferredRecord = 0; // if we're doubletapping, we can't change viewangles so have a preferred tick to use
	if (!I::ClientState->chokedcommands)
		flPreferredRecord = 0;
	if (flPreferredRecord)
	{
		auto pivot = std::find_if(vRecords.begin(), vRecords.end(), [](auto& s) -> bool { return s.m_flSimTime == flPreferredRecord; });
		if (pivot != vRecords.end())
			std::rotate(vRecords.begin(), pivot, pivot + 1);
	}

	auto RayToOBB = [](const Vec3& origin, const Vec3& direction, const Vec3& min, const Vec3& max, const matrix3x4& matrix) -> bool
		{
			if (Vars::Aimbot::General::AimType.Value != Vars::Aimbot::General::AimTypeEnum::Smooth)
				return true;

			return Math::RayToOBB(origin, direction, min, max, matrix);
		};

	int iReturn = false;
	for (auto& tRecord : vRecords)
	{
		bool bRunPeekCheck = flSpread && (Vars::Aimbot::General::PeekDTOnly.Value ? F::Ticks.GetTicks(pWeapon) : true) && Vars::Aimbot::General::HitscanPeek.Value;

		if (target.m_iTargetType == TargetEnum::Player || target.m_iTargetType == TargetEnum::Sentry)
		{
			auto aBones = tRecord.m_BoneMatrix.m_aBones;
			if (!aBones)
				continue;

			std::vector<std::pair<const mstudiobbox_t*, int>> vHitboxes;
			{
				if (target.m_iTargetType != TargetEnum::Sentry)
				{
					std::vector<std::pair<const mstudiobbox_t*, int>> primary, secondary, tertiary; // dumb
					for (int nHitbox = 0; nHitbox < target.m_pEntity->As<CTFPlayer>()->GetNumOfHitboxes(); nHitbox++)
					{
						if (!F::AimbotGlobal.IsHitboxValid(H::Entities.GetModel(target.m_pEntity->entindex()), nHitbox, Vars::Aimbot::Hitscan::Hitboxes.Value))
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
							if (target.m_iTargetType == TargetEnum::Sentry) // point of hit for sentries needs to be within bbox
							{
								const matrix3x4& transform = target.m_pEntity->RenderableToWorldTransform();
								const Vec3 vMin = target.m_pEntity->m_vecMins(), vMax = target.m_pEntity->m_vecMaxs();
								bWillHit = RayToOBB(vEyePos, vForward, vMin, vMax, transform);
							}
							if (bWillHit)
							{
								flPreferredRecord = tRecord.m_flSimTime;

								target.m_tRecord = tRecord;
								target.m_vPos = vTransformed;
								target.m_nAimedHitbox = pair.second;
								if (target.m_iTargetType == TargetEnum::Player)
									target.m_bBacktrack = true; //target.m_iTargetType == TargetEnum::PLAYER /*&& Vars::Backtrack::Enabled.Value*/;
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
						target.m_tRecord = tRecord;
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

	if (Vars::Aimbot::Hitscan::Modifiers.Value & Vars::Aimbot::Hitscan::ModifiersEnum::WaitForHeadshot)
	{
		switch (pWeapon->GetWeaponID())
		{
		case TF_WEAPON_SNIPERRIFLE:
		case TF_WEAPON_SNIPERRIFLE_DECAP:
			if (!G::CanHeadshot && pLocal->InCond(TF_COND_AIMING) && pWeapon->As<CTFSniperRifle>()->GetRifleType() != RIFLE_JARATE)
				return false;
			break;
		case TF_WEAPON_SNIPERRIFLE_CLASSIC:
			if (!G::CanHeadshot)
				return false;
			break;
		case TF_WEAPON_REVOLVER:
			if (SDK::AttribHookValue(0, "set_weapon_mode", pWeapon) == 1 && !pWeapon->AmbassadorCanHeadshot())
				return false;
		}
	}

	if (Vars::Aimbot::Hitscan::Modifiers.Value & Vars::Aimbot::Hitscan::ModifiersEnum::WaitForCharge)
	{
		switch (pWeapon->GetWeaponID())
		{
		case TF_WEAPON_SNIPERRIFLE:
		case TF_WEAPON_SNIPERRIFLE_DECAP:
		case TF_WEAPON_SNIPERRIFLE_CLASSIC:
		{
			auto pPlayer = target.m_pEntity->As<CTFPlayer>();
			auto pSniperRifle = pWeapon->As<CTFSniperRifle>();

			if (!pLocal->InCond(TF_COND_AIMING) || pSniperRifle->m_flChargedDamage() == 150.f)
				return true;

			if (target.m_nAimedHitbox == HITBOX_HEAD && (pWeapon->m_iItemDefinitionIndex() != Sniper_m_TheClassic ? true : pSniperRifle->m_flChargedDamage() == 150.f))
			{
				if (pSniperRifle->GetRifleType() == RIFLE_JARATE)
					return true;

				int iHeadDamage = std::ceil(std::max(pSniperRifle->m_flChargedDamage(), 50.f) * pSniperRifle->GetHeadshotMult(pPlayer));
				if (pPlayer->m_iHealth() <= iHeadDamage && (G::CanHeadshot || pLocal->IsCritBoosted()))
					return true;
			}
			else
			{
				int iBodyDamage = std::ceil(std::max(pSniperRifle->m_flChargedDamage(), 50.f) * pSniperRifle->GetBodyshotMult(pPlayer));
				if (pPlayer->m_iHealth() <= iBodyDamage)
					return true;
			}
			return false;
		}
		}
	}

	return true;
}

Vec3 CAimbotHitscan::Aim(Vec3 vCurAngle, Vec3 vToAngle, int iMethod)
{
	Vec3 vReturn = {};

	if (auto pLocal = H::Entities.GetLocal())
		vToAngle -= pLocal->m_vecPunchAngle();
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
void CAimbotHitscan::Aim(CUserCmd* pCmd, Vec3& vAngle)
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
			G::SilentAngles = true;
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
		if (!iRealAimType && iLastAimType && G::Attacking)
			Vars::Aimbot::General::AimType.Value = iLastAimType;
	}

	if ((Vars::Aimbot::General::AimHoldsFire.Value == Vars::Aimbot::General::AimHoldsFireEnum::Always || Vars::Aimbot::General::AimHoldsFire.Value == 1 && nWeaponID == TF_WEAPON_MINIGUN) && !G::CanPrimaryAttack && G::LastUserCmd->buttons & IN_ATTACK && Vars::Aimbot::General::AimType.Value && !pWeapon->IsInReload())
		pCmd->buttons |= IN_ATTACK;
	if (!Vars::Aimbot::General::AimType.Value || !G::CanPrimaryAttack && !G::Reloading && !F::Ticks.m_bDoubletap && !F::Ticks.m_bSpeedhack && Vars::Aimbot::General::AimType.Value == 3 && (nWeaponID == TF_WEAPON_MINIGUN ? pWeapon->As<CTFMinigun>()->m_iWeaponState() == AC_STATE_FIRING || pWeapon->As<CTFMinigun>()->m_iWeaponState() == AC_STATE_SPINNING : true))
		return;

	switch (nWeaponID)
	{
	case TF_WEAPON_MINIGUN:
		if (Vars::Aimbot::Hitscan::Modifiers.Value & Vars::Aimbot::Hitscan::ModifiersEnum::AutoRev)
			pCmd->buttons |= IN_ATTACK2;
		if (pWeapon->As<CTFMinigun>()->m_iWeaponState() != AC_STATE_FIRING && pWeapon->As<CTFMinigun>()->m_iWeaponState() != AC_STATE_SPINNING)
			return;
		break;
	}

	auto targets = SortTargets(pLocal, pWeapon);
	if (targets.empty())
		return;

	switch (nWeaponID)
	{
	case TF_WEAPON_SNIPERRIFLE:
	case TF_WEAPON_SNIPERRIFLE_DECAP:
	{
		const bool bScoped = pLocal->InCond(TF_COND_ZOOMED);

		if (Vars::Aimbot::Hitscan::Modifiers.Value & Vars::Aimbot::Hitscan::ModifiersEnum::AutoScope && !bScoped)
		{
			pCmd->buttons |= IN_ATTACK2;
			return;
		}

		if (Vars::Aimbot::Hitscan::Modifiers.Value & Vars::Aimbot::Hitscan::ModifiersEnum::ScopedOnly && !bScoped)
			return;

		switch (pWeapon->m_iItemDefinitionIndex())
		{
		case Sniper_m_TheMachina:
		case Sniper_m_ShootingStar:
			if (!bScoped)
				return;
		}

		break;
	}
	case TF_WEAPON_SNIPERRIFLE_CLASSIC:
		if (iRealAimType)
			pCmd->buttons |= IN_ATTACK;
	}

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
		G::AimPosition = { target.m_vPos, I::GlobalVars->tickcount };

		bool bShouldFire = ShouldFire(pLocal, pWeapon, pCmd, target);

		if (bShouldFire)
		{
			if (pWeapon->GetWeaponID() != TF_WEAPON_MEDIGUN || !(G::LastUserCmd->buttons & IN_ATTACK))
				pCmd->buttons |= IN_ATTACK;

			if (nWeaponID == TF_WEAPON_SNIPERRIFLE_CLASSIC && pWeapon->As<CTFSniperRifle>()->m_flChargedDamage() && pLocal->m_hGroundEntity())
				pCmd->buttons &= ~IN_ATTACK;

			switch (pWeapon->m_iItemDefinitionIndex())
			{
			case Engi_s_TheWrangler:
			case Engi_s_FestiveWrangler:
				pCmd->buttons |= IN_ATTACK2;
			}

			if (Vars::Aimbot::Hitscan::Modifiers.Value & Vars::Aimbot::Hitscan::ModifiersEnum::Tapfire && pWeapon->GetWeaponSpread() != 0.f && !pLocal->InCond(TF_COND_RUNE_PRECISION)
				&& pLocal->GetShootPos().DistTo(target.m_vPos) > Vars::Aimbot::Hitscan::TapFireDist.Value)
			{
				const float flTimeSinceLastShot = (pLocal->m_nTickBase() * TICK_INTERVAL) - pWeapon->m_flLastFireTime();
				if (flTimeSinceLastShot <= (pWeapon->GetBulletsPerShot() > 1 ? 0.25f : 1.25f))
					pCmd->buttons &= ~IN_ATTACK;
			}
		}

		G::Attacking = SDK::IsAttacking(pLocal, pWeapon, pCmd, true);

		if (G::Attacking == 1)
		{
			if (target.m_pEntity->IsPlayer())
				F::Resolver.HitscanRan(pLocal, target.m_pEntity->As<CTFPlayer>(), pWeapon, target.m_nAimedHitbox);

			if (target.m_bBacktrack)
				pCmd->tick_count = TIME_TO_TICKS(target.m_tRecord.m_flSimTime) + TIME_TO_TICKS(F::Backtrack.m_flFakeInterp);

			bool bLine = Vars::Visuals::Line::Enabled.Value;
			bool bBoxes = Vars::Visuals::Hitbox::BonesEnabled.Value & Vars::Visuals::Hitbox::BonesEnabledEnum::OnShot;
			if (G::CanPrimaryAttack && (bLine || bBoxes))
			{
				G::LineStorage.clear();
				G::BoxStorage.clear();
				G::PathStorage.clear();

				if (bLine)
				{
					Vec3 vEyePos = pLocal->GetShootPos();
					float flDist = vEyePos.DistTo(target.m_vPos);
					Vec3 vForward; Math::AngleVectors(target.m_vAngleTo + pLocal->m_vecPunchAngle(), &vForward);

					if (Vars::Colors::Line.Value.a)
						G::LineStorage.push_back({ { vEyePos, vEyePos + vForward * flDist }, I::GlobalVars->curtime + Vars::Visuals::Line::DrawDuration.Value, Vars::Colors::Line.Value });
					if (Vars::Colors::LineClipped.Value.a)
						G::LineStorage.push_back({ { vEyePos, vEyePos + vForward * flDist }, I::GlobalVars->curtime + Vars::Visuals::Line::DrawDuration.Value, Vars::Colors::LineClipped.Value, true });
				}
				if (bBoxes)
				{
					auto vBoxes = F::Visuals.GetHitboxes(target.m_tRecord.m_BoneMatrix.m_aBones, target.m_pEntity->As<CBaseAnimating>(), {}, target.m_nAimedHitbox);
					G::BoxStorage.insert(G::BoxStorage.end(), vBoxes.begin(), vBoxes.end());
				}
			}
		}

		Aim(pCmd, target.m_vAngleTo);
		break;
	}
}