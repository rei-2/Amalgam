#include "AimbotHitscan.h"

#include "../Aimbot.h"
#include "../../Resolver/Resolver.h"
#include "../../TickHandler/TickHandler.h"
#include "../../Visuals/Visuals.h"
#include "../../Simulation/MovementSimulation/MovementSimulation.h"

std::vector<Target_t> CAimbotHitscan::GetTargets(CTFPlayer* pLocal, CTFWeaponBase* pWeapon)
{
	std::vector<Target_t> vTargets;
	const auto iSort = Vars::Aimbot::General::TargetSelection.Value;

	Vec3 vLocalPos = F::Ticks.GetShootPos();
	Vec3 vLocalAngles = I::EngineClient->GetViewAngles();

	{
		auto eGroupType = EGroupType::GROUP_INVALID;
		if (Vars::Aimbot::General::Target.Value & Vars::Aimbot::General::TargetEnum::Players)
			eGroupType = EGroupType::PLAYERS_ENEMIES;
		if (SDK::AttribHookValue(0, "jarate_duration", pWeapon) > 0 && Vars::Aimbot::Hitscan::Modifiers.Value & Vars::Aimbot::Hitscan::ModifiersEnum::ExtinguishTeam)
			eGroupType = EGroupType::PLAYERS_ALL;
		if (pWeapon->GetWeaponID() == TF_WEAPON_MEDIGUN)
			eGroupType = Vars::Aimbot::Healing::AutoHeal.Value ? EGroupType::PLAYERS_TEAMMATES : EGroupType::GROUP_INVALID;

		for (auto pEntity : H::Entities.GetGroup(eGroupType))
		{
			bool bTeammate = pEntity->m_iTeamNum() == pLocal->m_iTeamNum();
			if (F::AimbotGlobal.ShouldIgnore(pEntity, pLocal, pWeapon))
				continue;

			if (bTeammate)
			{
				if (SDK::AttribHookValue(0, "jarate_duration", pWeapon) > 0)
				{
					if (!pEntity->As<CTFPlayer>()->InCond(TF_COND_BURNING))
						continue;
				}
				else if (pWeapon->GetWeaponID() == TF_WEAPON_MEDIGUN)
				{
					if (pEntity->As<CTFPlayer>()->InCond(TF_COND_STEALTHED)
						|| Vars::Aimbot::Healing::FriendsOnly.Value && !H::Entities.IsFriend(pEntity->entindex()) && !H::Entities.InParty(pEntity->entindex()))
						continue;
				}
			}

			float flFOVTo; Vec3 vPos, vAngleTo;
			if (!F::AimbotGlobal.PlayerBoneInFOV(pEntity->As<CTFPlayer>(), vLocalPos, vLocalAngles, flFOVTo, vPos, vAngleTo, Vars::Aimbot::Hitscan::Hitboxes.Value))
				continue;

			float flDistTo = iSort == Vars::Aimbot::General::TargetSelectionEnum::Distance ? vLocalPos.DistTo(vPos) : 0.f;
			vTargets.emplace_back(pEntity, TargetEnum::Player, vPos, vAngleTo, flFOVTo, flDistTo, bTeammate ? 0 : F::AimbotGlobal.GetPriority(pEntity->entindex()));
		}

		if (pWeapon->GetWeaponID() == TF_WEAPON_MEDIGUN)
			return vTargets;
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
			vTargets.emplace_back(pEntity, pEntity->IsSentrygun() ? TargetEnum::Sentry : pEntity->IsDispenser() ? TargetEnum::Dispenser : TargetEnum::Teleporter, vPos, vAngleTo, flFOVTo, flDistTo);
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
			vTargets.emplace_back(pEntity, TargetEnum::Sticky, vPos, vAngleTo, flFOVTo, flDistTo);
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
			vTargets.emplace_back(pEntity, TargetEnum::NPC, vPos, vAngleTo, flFOVTo, flDistTo);
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
			vTargets.emplace_back(pEntity, TargetEnum::Bomb, vPos, vAngleTo, flFOVTo, flDistTo);
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

			if (G::CanHeadshot || pLocal->InCond(TF_COND_AIMING) && (
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

		if (Vars::Aimbot::Hitscan::Hitboxes.Value & Vars::Aimbot::Hitscan::HitboxesEnum::BodyaimIfLethal && bHeadshot)
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
					int iDamage = std::ceil(Math::RemapVal(flDistTo, 90.f, 900.f, 60.f, 21.f) * flMult);
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

int CAimbotHitscan::CanHit(Target_t& tTarget, CTFPlayer* pLocal, CTFWeaponBase* pWeapon)
{
	if (Vars::Aimbot::General::Ignore.Value & Vars::Aimbot::General::IgnoreEnum::Unsimulated && H::Entities.GetChoke(tTarget.m_pEntity->entindex()) > Vars::Aimbot::General::TickTolerance.Value)
		return false;

	Vec3 vEyePos = pLocal->GetShootPos(), vPeekPos = {};
	const float flMaxRange = powf(pWeapon->GetRange(), 2.f);

	auto pModel = tTarget.m_pEntity->GetModel();
	if (!pModel) return false;
	auto pHDR = I::ModelInfoClient->GetStudiomodel(pModel);
	if (!pHDR) return false;
	auto pSet = pHDR->pHitboxSet(tTarget.m_pEntity->As<CBaseAnimating>()->m_nHitboxSet());
	if (!pSet) return false;

	std::deque<TickRecord> vRecords;
	{
		auto pRecords = F::Backtrack.GetRecords(tTarget.m_pEntity);
		if (pRecords)
		{
			vRecords = F::Backtrack.GetValidRecords(pRecords, pLocal);
			if (!Vars::Backtrack::Enabled.Value && !vRecords.empty())
				vRecords = { vRecords.front() };
		}
		if (!pRecords || vRecords.empty())
		{
			if (auto pBones = H::Entities.GetBones(tTarget.m_pEntity->entindex()))
				vRecords.emplace_front(tTarget.m_pEntity->m_flSimulationTime(), *reinterpret_cast<BoneMatrix*>(pBones), tTarget.m_pEntity->m_vecOrigin());
			else
			{
				matrix3x4 aBones[MAXSTUDIOBONES];
				if (!tTarget.m_pEntity->SetupBones(aBones, MAXSTUDIOBONES, BONE_USED_BY_ANYTHING, tTarget.m_pEntity->m_flSimulationTime()))
					return false;

				vRecords.emplace_front(tTarget.m_pEntity->m_flSimulationTime(), *reinterpret_cast<BoneMatrix*>(&aBones), tTarget.m_pEntity->m_vecOrigin());
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

	auto RayToOBB = [&](const Vec3& vOrigin, const Vec3& vDirection, const Vec3& vMins, const Vec3& vMaxs, const matrix3x4& mMatrix, float flScale = 1.f) -> bool
		{
			switch (Vars::Aimbot::General::AimType.Value)
			{
			case Vars::Aimbot::General::AimTypeEnum::Smooth:
			case Vars::Aimbot::General::AimTypeEnum::Assistive:
				return Math::RayToOBB(vOrigin, vDirection, vMins, vMaxs, mMatrix, flScale);
			}

			return true;
		};

	int iReturn = false;
	for (auto& tRecord : vRecords)
	{
		bool bRunPeekCheck = flSpread && (Vars::Aimbot::General::PeekDTOnly.Value ? F::Ticks.GetTicks(pWeapon) : true) && Vars::Aimbot::General::HitscanPeek.Value;

		if (pWeapon->GetWeaponID() == TF_WEAPON_LASER_POINTER)
		{
			tTarget.m_vPos = tTarget.m_pEntity->m_vecOrigin();

			// not lag compensated (i assume) so run movesim based on ping
			PlayerStorage tStorage;
			F::MoveSim.Initialize(tTarget.m_pEntity, tStorage);
			if (!tStorage.m_bFailed)
			{
				for (int i = 1 - TIME_TO_TICKS(F::Backtrack.GetReal()); i <= 0; i++)
					tTarget.m_vPos = tStorage.m_vPredictedOrigin;
			}
			F::MoveSim.Restore(tStorage);

			float flBoneScale = std::max(Vars::Aimbot::Hitscan::BoneSizeMinimumScale.Value, Vars::Aimbot::Hitscan::PointScale.Value / 100.f);
			float flBoneSubtract = Vars::Aimbot::Hitscan::BoneSizeSubtract.Value;

			Vec3 vMins = tTarget.m_pEntity->m_vecMins();
			Vec3 vMaxs = tTarget.m_pEntity->m_vecMaxs();
			Vec3 vCheckMins = (vMins + flBoneSubtract) * flBoneScale;
			Vec3 vCheckMaxs = (vMaxs - flBoneSubtract) * flBoneScale;

			const matrix3x4 mTransform = { { 1, 0, 0, tTarget.m_vPos.x }, { 0, 1, 0, tTarget.m_vPos.y }, { 0, 0, 1, tTarget.m_vPos.z } };

			tTarget.m_vPos += (tTarget.m_pEntity->m_vecMins() + tTarget.m_pEntity->m_vecMaxs()) / 2;
			if (vEyePos.DistToSqr(tTarget.m_vPos) > flMaxRange)
				break;

			if (SDK::VisPos(pLocal, tTarget.m_pEntity, vEyePos, tTarget.m_vPos))
			{
				auto vAngles = Aim(G::CurrentUserCmd->viewangles, Math::CalcAngle(pLocal->GetShootPos(), tTarget.m_vPos));
				Vec3 vForward; Math::AngleVectors(vAngles, &vForward);

				if (RayToOBB(vEyePos, vForward, vCheckMins, vCheckMaxs, mTransform))
				{
					flPreferredRecord = tRecord.m_flSimTime;

					tTarget.m_vAngleTo = vAngles;
					tTarget.m_tRecord = tRecord;
					return true;
				}
				else if (iReturn == 2 ? vAngles.DeltaAngle(G::CurrentUserCmd->viewangles).Length2D() < tTarget.m_vAngleTo.DeltaAngle(G::CurrentUserCmd->viewangles).Length2D() : true)
					tTarget.m_vAngleTo = vAngles;
				iReturn = 2;
			}

			break;
		}

		if (tTarget.m_iTargetType == TargetEnum::Player)
		{
			auto aBones = tRecord.m_BoneMatrix.m_aBones;
			if (!aBones)
				continue;

			std::vector<std::tuple<const mstudiobbox_t*, int, int>> vHitboxes;
			for (int i = 0; i < pSet->numhitboxes; i++)
			{
				if (!F::AimbotGlobal.IsHitboxValid(H::Entities.GetModel(tTarget.m_pEntity->entindex()), i, Vars::Aimbot::Hitscan::Hitboxes.Value))
					continue;

				auto pBox = pSet->pHitbox(i);
				if (!pBox) continue;

				int iPriority = GetHitboxPriority(i, pLocal, pWeapon, tTarget.m_pEntity);
				vHitboxes.emplace_back(pBox, i, iPriority);
			}
			std::sort(vHitboxes.begin(), vHitboxes.end(), [&](const auto& a, const auto& b) -> bool
				{
					return std::get<2>(a) < std::get<2>(b);
				});

			float flModelScale = tTarget.m_pEntity->As<CBaseAnimating>()->m_flModelScale();
			float flBoneScale = std::max(Vars::Aimbot::Hitscan::BoneSizeMinimumScale.Value, Vars::Aimbot::Hitscan::PointScale.Value / 100.f);
			float flBoneSubtract = Vars::Aimbot::Hitscan::BoneSizeSubtract.Value;

			auto pGameRules = I::TFGameRules();
			auto pViewVectors = pGameRules ? pGameRules->GetViewVectors() : nullptr;
			Vec3 vHullMins = (pViewVectors ? pViewVectors->m_vHullMin : Vec3(-24, -24, 0)) * flModelScale;
			Vec3 vHullMaxs = (pViewVectors ? pViewVectors->m_vHullMax : Vec3(24, 24, 82)) * flModelScale;

			const matrix3x4 mTransform = { { 1, 0, 0, tRecord.m_vOrigin.x }, { 0, 1, 0, tRecord.m_vOrigin.y }, { 0, 0, 1, tRecord.m_vOrigin.z } };

			for (auto& [pBox, iHitbox, _] : vHitboxes)
			{
				Vec3 vAngle; Math::MatrixAngles(aBones[pBox->bone], vAngle);
				Vec3 vMins = pBox->bbmin;
				Vec3 vMaxs = pBox->bbmax;

				Vec3 vOffset;
				{
					Vec3 vOrigin, vCenter;
					Math::VectorTransform({}, aBones[pBox->bone], vOrigin);
					Math::VectorTransform((vMins + vMaxs) / 2, aBones[pBox->bone], vCenter);
					vOffset = vCenter - vOrigin;
				}

				std::vector<Vec3> vPoints = { Vec3() };
				if (Vars::Aimbot::Hitscan::PointScale.Value > 0.f)
				{
					bool bTriggerbot = (Vars::Aimbot::General::AimType.Value == Vars::Aimbot::General::AimTypeEnum::Smooth
						|| Vars::Aimbot::General::AimType.Value == Vars::Aimbot::General::AimTypeEnum::Assistive)
						&& !Vars::Aimbot::General::AssistStrength.Value;

					if (!bTriggerbot)
					{
						float flScale = Vars::Aimbot::Hitscan::PointScale.Value / 100;
						Vec3 vMinsS = (vMins - vMaxs) / 2 * flScale;
						Vec3 vMaxsS = (vMaxs - vMins) / 2 * flScale;

						vPoints = {
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
				}

				for (auto& vPoint : vPoints)
				{
					Vec3 vOrigin = {}; Math::VectorTransform(vPoint, aBones[pBox->bone], vOrigin); vOrigin += vOffset;

					if (vEyePos.DistToSqr(vOrigin) > flMaxRange)
						continue;

					if (bRunPeekCheck)
					{
						bRunPeekCheck = false;
						if (!SDK::VisPos(pLocal, tTarget.m_pEntity, vPeekPos, vOrigin))
							goto nextTick; // if we can't hit our primary hitbox, don't bother
					}

					if (SDK::VisPos(pLocal, tTarget.m_pEntity, vEyePos, vOrigin))
					{
						auto vAngles = Aim(G::CurrentUserCmd->viewangles, Math::CalcAngle(pLocal->GetShootPos(), vOrigin));
						Vec3 vForward; Math::AngleVectors(vAngles, &vForward);

						// not perfect but it will do for now
						if (Math::RayToOBB(vEyePos, vForward, vHullMins, vHullMaxs, mTransform))
						{
							// for the time being, no vischecks against other hitboxes
							Vec3 vCheckMins = (vMins + flBoneSubtract / flModelScale) * flBoneScale, vCheckMaxs = (vMaxs - flBoneSubtract / flModelScale) * flBoneScale;
							if (RayToOBB(vEyePos, vForward, vCheckMins, vCheckMaxs, aBones[pBox->bone], flModelScale))
							{
								flPreferredRecord = tRecord.m_flSimTime;

								tTarget.m_vAngleTo = vAngles;
								tTarget.m_tRecord = tRecord;
								tTarget.m_vPos = vOrigin;
								tTarget.m_nAimedHitbox = iHitbox;
								tTarget.m_bBacktrack = true;
								return true;
							}
							else if (iReturn == 2 ? vAngles.DeltaAngle(G::CurrentUserCmd->viewangles).Length2D() < tTarget.m_vAngleTo.DeltaAngle(G::CurrentUserCmd->viewangles).Length2D() : true)
								tTarget.m_vAngleTo = vAngles;
							iReturn = 2;
						}
					}
				}
			}
		}
		else
		{
			float flBoneScale = std::max(Vars::Aimbot::Hitscan::BoneSizeMinimumScale.Value, Vars::Aimbot::Hitscan::PointScale.Value / 100.f);
			float flBoneSubtract = Vars::Aimbot::Hitscan::BoneSizeSubtract.Value;

			Vec3 vMins = tTarget.m_pEntity->m_vecMins();
			Vec3 vMaxs = tTarget.m_pEntity->m_vecMaxs();
			Vec3 vCheckMins = (vMins + flBoneSubtract) * flBoneScale;
			Vec3 vCheckMaxs = (vMaxs - flBoneSubtract) * flBoneScale;

			auto pCollideable = tTarget.m_pEntity->GetCollideable();
			const matrix3x4& mTransform = pCollideable ? pCollideable->CollisionToWorldTransform() : tTarget.m_pEntity->RenderableToWorldTransform();

			std::vector<Vec3> vPoints = { Vec3() };
			//if (Vars::Aimbot::Hitscan::PointScale.Value > 0.f)
			{
				bool bTriggerbot = (Vars::Aimbot::General::AimType.Value == Vars::Aimbot::General::AimTypeEnum::Smooth
					|| Vars::Aimbot::General::AimType.Value == Vars::Aimbot::General::AimTypeEnum::Assistive)
					&& !Vars::Aimbot::General::AssistStrength.Value;

				if (!bTriggerbot)
				{
					float flScale = 0.5f; //Vars::Aimbot::Hitscan::PointScale.Value / 100;
					Vec3 vMinsS = (vMins - vMaxs) / 2 * flScale;
					Vec3 vMaxsS = (vMaxs - vMins) / 2 * flScale;

					vPoints = {
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
			}

			for (auto& vPoint : vPoints)
			{
				Vec3 vOrigin = tTarget.m_pEntity->GetCenter() + vPoint;

				if (vEyePos.DistToSqr(vOrigin) > flMaxRange)
					continue;

				if (bRunPeekCheck)
				{
					bRunPeekCheck = false;
					if (!SDK::VisPos(pLocal, tTarget.m_pEntity, vPeekPos, vOrigin))
						goto nextTick; // if we can't hit our primary hitbox, don't bother
				}

				if (SDK::VisPos(pLocal, tTarget.m_pEntity, vEyePos, vOrigin))
				{
					auto vAngles = Aim(G::CurrentUserCmd->viewangles, Math::CalcAngle(pLocal->GetShootPos(), vOrigin));
					Vec3 vForward; Math::AngleVectors(vAngles, &vForward);

					if (RayToOBB(vEyePos, vForward, vCheckMins, vCheckMaxs, mTransform))
					{
						tTarget.m_vAngleTo = vAngles;
						tTarget.m_tRecord = tRecord;
						tTarget.m_vPos = vOrigin;
						return true;
					}
					else if (iReturn == 2 ? vAngles.DeltaAngle(G::CurrentUserCmd->viewangles).Length2D() < tTarget.m_vAngleTo.DeltaAngle(G::CurrentUserCmd->viewangles).Length2D() : true)
						tTarget.m_vAngleTo = vAngles;
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
bool CAimbotHitscan::ShouldFire(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd, const Target_t& tTarget)
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
			auto pPlayer = tTarget.m_pEntity->As<CTFPlayer>();
			auto pSniperRifle = pWeapon->As<CTFSniperRifle>();

			if (!pLocal->InCond(TF_COND_AIMING) || pSniperRifle->m_flChargedDamage() == 150.f)
				return true;

			if (tTarget.m_nAimedHitbox == HITBOX_HEAD && (pWeapon->GetWeaponID() != TF_WEAPON_SNIPERRIFLE_CLASSIC ? true : pSniperRifle->m_flChargedDamage() == 150.f))
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
		vReturn = vCurAngle.LerpAngle(vToAngle, Vars::Aimbot::General::AssistStrength.Value / 100.f);
		break;
	case Vars::Aimbot::General::AimTypeEnum::Assistive:
		Vec3 vMouseDelta = G::CurrentUserCmd->viewangles.DeltaAngle(G::LastUserCmd->viewangles);
		Vec3 vTargetDelta = vToAngle.DeltaAngle(G::LastUserCmd->viewangles);
		float flMouseDelta = vMouseDelta.Length2D(), flTargetDelta = vTargetDelta.Length2D();
		vTargetDelta = vTargetDelta.Normalized() * std::min(flMouseDelta, flTargetDelta);
		vReturn = vCurAngle - vMouseDelta + vMouseDelta.LerpAngle(vTargetDelta, Vars::Aimbot::General::AssistStrength.Value / 100.f);
		break;
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
	case Vars::Aimbot::General::AimTypeEnum::Assistive:
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

	if (!F::Aimbot.m_bRunningSecondary && (Vars::Aimbot::General::AimHoldsFire.Value == Vars::Aimbot::General::AimHoldsFireEnum::Always || Vars::Aimbot::General::AimHoldsFire.Value == 1 && nWeaponID == TF_WEAPON_MINIGUN) && !G::CanPrimaryAttack && G::LastUserCmd->buttons & IN_ATTACK && Vars::Aimbot::General::AimType.Value && !pWeapon->IsInReload())
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

	auto vTargets = SortTargets(pLocal, pWeapon);
	if (vTargets.empty())
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

		if (!bScoped && SDK::AttribHookValue(0, "sniper_only_fire_zoomed", pWeapon))
			return;

		break;
	}
	case TF_WEAPON_SNIPERRIFLE_CLASSIC:
		if (iRealAimType)
			pCmd->buttons |= IN_ATTACK;
	}

	for (auto& tTarget : vTargets)
	{
		if (nWeaponID == TF_WEAPON_MEDIGUN && pWeapon->As<CWeaponMedigun>()->m_hHealingTarget().Get() == tTarget.m_pEntity)
		{
			if (G::LastUserCmd->buttons & IN_ATTACK)
				pCmd->buttons |= IN_ATTACK;
			return;
		}

		const auto iResult = CanHit(tTarget, pLocal, pWeapon);
		if (!iResult) continue;
		if (iResult == 2)
		{
			G::Target = { tTarget.m_pEntity->entindex(), I::GlobalVars->tickcount };
			Aim(pCmd, tTarget.m_vAngleTo);
			break;
		}

		G::Target = { tTarget.m_pEntity->entindex(), I::GlobalVars->tickcount };
		G::AimPosition = { tTarget.m_vPos, I::GlobalVars->tickcount };

		bool bShouldFire = ShouldFire(pLocal, pWeapon, pCmd, tTarget);

		if (bShouldFire)
		{
			if (nWeaponID != TF_WEAPON_MEDIGUN || !(G::LastUserCmd->buttons & IN_ATTACK))
				pCmd->buttons |= IN_ATTACK;

			if (nWeaponID == TF_WEAPON_SNIPERRIFLE_CLASSIC && pWeapon->As<CTFSniperRifle>()->m_flChargedDamage() && pLocal->m_hGroundEntity())
				pCmd->buttons &= ~IN_ATTACK;

			if (nWeaponID == TF_WEAPON_LASER_POINTER)
				pCmd->buttons |= IN_ATTACK2;

			if (Vars::Aimbot::Hitscan::Modifiers.Value & Vars::Aimbot::Hitscan::ModifiersEnum::Tapfire && pWeapon->GetWeaponSpread() != 0.f && !pLocal->InCond(TF_COND_RUNE_PRECISION)
				&& pLocal->GetShootPos().DistTo(tTarget.m_vPos) > Vars::Aimbot::Hitscan::TapFireDist.Value)
			{
				const float flTimeSinceLastShot = (pLocal->m_nTickBase() * TICK_INTERVAL) - pWeapon->m_flLastFireTime();
				if (flTimeSinceLastShot <= (pWeapon->GetBulletsPerShot() > 1 ? 0.25f : 1.25f))
					pCmd->buttons &= ~IN_ATTACK;
			}
		}

		G::Attacking = SDK::IsAttacking(pLocal, pWeapon, pCmd, true);

		if (G::Attacking == 1 && nWeaponID != TF_WEAPON_LASER_POINTER)
		{
			if (tTarget.m_pEntity->IsPlayer())
				F::Resolver.HitscanRan(pLocal, tTarget.m_pEntity->As<CTFPlayer>(), pWeapon, tTarget.m_nAimedHitbox);

			if (tTarget.m_bBacktrack)
				pCmd->tick_count = TIME_TO_TICKS(tTarget.m_tRecord.m_flSimTime) + TIME_TO_TICKS(F::Backtrack.GetFakeInterp());

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
					float flDist = vEyePos.DistTo(tTarget.m_vPos);
					Vec3 vForward; Math::AngleVectors(tTarget.m_vAngleTo + pLocal->m_vecPunchAngle(), &vForward);

					if (Vars::Colors::Line.Value.a)
						G::LineStorage.emplace_back(std::pair<Vec3, Vec3>(vEyePos, vEyePos + vForward * flDist), I::GlobalVars->curtime + Vars::Visuals::Line::DrawDuration.Value, Vars::Colors::Line.Value);
					if (Vars::Colors::LineClipped.Value.a)
						G::LineStorage.emplace_back(std::pair<Vec3, Vec3>(vEyePos, vEyePos + vForward * flDist), I::GlobalVars->curtime + Vars::Visuals::Line::DrawDuration.Value, Vars::Colors::LineClipped.Value, true);
				}
				if (bBoxes)
				{
					auto vBoxes = F::Visuals.GetHitboxes(tTarget.m_tRecord.m_BoneMatrix.m_aBones, tTarget.m_pEntity->As<CBaseAnimating>(), {}, tTarget.m_nAimedHitbox);
					G::BoxStorage.insert(G::BoxStorage.end(), vBoxes.begin(), vBoxes.end());
				}
			}
		}

		Aim(pCmd, tTarget.m_vAngleTo);
		break;
	}
}