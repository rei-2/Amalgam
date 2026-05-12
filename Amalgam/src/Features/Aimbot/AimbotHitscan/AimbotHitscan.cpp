#include "AimbotHitscan.h"

#include "../Aimbot.h"
#include "../../Ticks/Ticks.h"
#include "../../Resolver/Resolver.h"
#include "../../NoSpread/NoSpread.h"
#include "../../Simulation/MovementSimulation/MovementSimulation.h"
#include "../../Visuals/Visuals.h"

static inline std::vector<Target_t> GetTargets(CTFPlayer* pLocal, CTFWeaponBase* pWeapon)
{
	std::vector<Target_t> vTargets;

	Vec3 vLocalPos = F::Ticks.GetShootPos();
	Vec3 vLocalAngles = I::EngineClient->GetViewAngles();

	{
		auto eGroup = EntityEnum::Invalid;
		if (Vars::Aimbot::General::Target.Value & Vars::Aimbot::General::TargetEnum::Players)
		{
			eGroup = !F::AimbotGlobal.FriendlyFire() || Vars::Aimbot::General::Ignore.Value & Vars::Aimbot::General::IgnoreEnum::Team ? EntityEnum::PlayerEnemy : EntityEnum::PlayerAll;
			if (Vars::Aimbot::Hitscan::Modifiers.Value & Vars::Aimbot::Hitscan::ModifiersEnum::ExtinguishTeam &&
				!F::AimbotGlobal.FriendlyFire() && SDK::AttribHookValue(0, "jarate_duration", pWeapon) > 0)
				eGroup = EntityEnum::PlayerAll;
		}
		if (pWeapon->GetWeaponID() == TF_WEAPON_MEDIGUN)
			eGroup = Vars::Aimbot::Healing::AutoHeal.Value ? EntityEnum::PlayerTeam : EntityEnum::Invalid;
		bool bHeal = pWeapon->GetWeaponID() == TF_WEAPON_MEDIGUN;

		for (auto pEntity : H::Entities.GetGroup(eGroup))
		{
			if (F::AimbotGlobal.ShouldIgnore(pEntity, pLocal, pWeapon))
				continue;

			bool bTeam = pEntity->m_iTeamNum() == pLocal->m_iTeamNum();
			if (bTeam)
			{
				if (bHeal)
				{
					if (pEntity->As<CTFPlayer>()->InCond(TF_COND_STEALTHED)
						|| Vars::Aimbot::Healing::HealPriority.Value == Vars::Aimbot::Healing::HealPriorityEnum::FriendsOnly
						&& !H::Entities.IsFriend(pEntity->entindex()) && !H::Entities.InParty(pEntity->entindex()))
						continue;
				}
				if (!F::AimbotGlobal.FriendlyFire() && SDK::AttribHookValue(0, "jarate_duration", pWeapon) > 0)
				{
					if (!pEntity->As<CTFPlayer>()->InCond(TF_COND_BURNING))
						continue;
				}
			}

			float flFOVTo; Vec3 vPos, vAngleTo;
			if (!F::AimbotGlobal.PlayerBoneInFOV(pEntity->As<CTFPlayer>(), vLocalPos, vLocalAngles, flFOVTo, vPos, vAngleTo, Vars::Aimbot::Hitscan::Hitboxes.Value))
				continue;

			float flDistTo = vLocalPos.DistToSqr(vPos);
			int iPriority = F::AimbotGlobal.GetPriority(pEntity->entindex());
			if (bTeam && bHeal)
			{
				iPriority = 0;
				switch (Vars::Aimbot::Healing::HealPriority.Value)
				{
				case Vars::Aimbot::Healing::HealPriorityEnum::PrioritizeFriends:
					if (H::Entities.IsFriend(pEntity->entindex()) || H::Entities.InParty(pEntity->entindex()))
						iPriority = std::numeric_limits<int>::max();
					break;
				case Vars::Aimbot::Healing::HealPriorityEnum::PrioritizeTeam:
					iPriority = std::numeric_limits<int>::max();
				}
			}
			vTargets.emplace_back(pEntity, TargetEnum::Player, vPos, vAngleTo, flFOVTo, flDistTo, iPriority);
		}

		if (pWeapon->GetWeaponID() == TF_WEAPON_MEDIGUN)
			return vTargets;
	}

	if (Vars::Aimbot::General::Target.Value & Vars::Aimbot::General::TargetEnum::Building)
	{
		for (auto pEntity : H::Entities.GetGroup(EntityEnum::BuildingEnemy))
		{
			if (F::AimbotGlobal.ShouldIgnore(pEntity, pLocal, pWeapon))
				continue;

			float flFOVTo; Vec3 vPos, vAngleTo;
			if (!F::AimbotGlobal.EntityCenterInFOV(pEntity, vLocalPos, vLocalAngles, flFOVTo, vPos, vAngleTo))
				continue;

			float flDistTo = vLocalPos.DistToSqr(vPos);
			vTargets.emplace_back(pEntity, pEntity->IsSentrygun() ? TargetEnum::Sentry : pEntity->IsDispenser() ? TargetEnum::Dispenser : TargetEnum::Teleporter, vPos, vAngleTo, flFOVTo, flDistTo);
		}
	}

	if (Vars::Aimbot::General::Target.Value & Vars::Aimbot::General::TargetEnum::Stickies)
	{
		for (auto pEntity : H::Entities.GetGroup(EntityEnum::WorldProjectile))
		{
			if (F::AimbotGlobal.ShouldIgnore(pEntity, pLocal, pWeapon))
				continue;

			float flFOVTo; Vec3 vPos, vAngleTo;
			if (!F::AimbotGlobal.EntityCenterInFOV(pEntity, vLocalPos, vLocalAngles, flFOVTo, vPos, vAngleTo))
				continue;

			float flDistTo = vLocalPos.DistToSqr(vPos);
			vTargets.emplace_back(pEntity, TargetEnum::Sticky, vPos, vAngleTo, flFOVTo, flDistTo);
		}
	}

	if (Vars::Aimbot::General::Target.Value & Vars::Aimbot::General::TargetEnum::NPCs)
	{
		for (auto pEntity : H::Entities.GetGroup(EntityEnum::WorldNPC))
		{
			if (F::AimbotGlobal.ShouldIgnore(pEntity, pLocal, pWeapon))
				continue;

			float flFOVTo; Vec3 vPos, vAngleTo;
			if (!F::AimbotGlobal.EntityCenterInFOV(pEntity, vLocalPos, vLocalAngles, flFOVTo, vPos, vAngleTo))
				continue;

			float flDistTo = vLocalPos.DistToSqr(vPos);
			vTargets.emplace_back(pEntity, TargetEnum::NPC, vPos, vAngleTo, flFOVTo, flDistTo);
		}
	}

	if (Vars::Aimbot::General::Target.Value & Vars::Aimbot::General::TargetEnum::Bombs)
	{
		for (auto pEntity : H::Entities.GetGroup(EntityEnum::WorldBomb))
		{
			if (F::AimbotGlobal.ShouldIgnore(pEntity, pLocal, pWeapon))
				continue;

			float flFOVTo; Vec3 vPos, vAngleTo;
			if (!F::AimbotGlobal.EntityCenterInFOV(pEntity, vLocalPos, vLocalAngles, flFOVTo, vPos, vAngleTo))
				continue;

			float flDistTo = vLocalPos.DistToSqr(vPos);
			vTargets.emplace_back(pEntity, TargetEnum::Bomb, vPos, vAngleTo, flFOVTo, flDistTo);
		}
	}

	return vTargets;
}



std::vector<HitscanHitbox_t> CAimbotHitscan::GetHitboxes(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CBaseEntity* pTarget)
{
	std::vector<HitscanHitbox_t> vHitboxes = {};

	if (pWeapon->GetWeaponID() == TF_WEAPON_LASER_POINTER)
	{
		CObjectSentrygun* pSentry = nullptr;
		for (auto pEntity : H::Entities.GetGroup(EntityEnum::BuildingTeam))
		{
			if (pEntity->IsSentrygun() && pEntity->As<CBaseObject>()->m_hBuilder().GetEntryIndex() == I::EngineClient->GetLocalPlayer())
			{
				pSentry = pEntity->As<CObjectSentrygun>();
				break;
			}
		}
		if (!pSentry)
			return vHitboxes;

		Vec3 vOrigin = pTarget->m_vecOrigin();
		{
			MoveStorage tMoveStorage; // run movesim based on ping
			F::MoveSim.Initialize(pTarget, tMoveStorage);
			if (!tMoveStorage.m_bFailed)
			{
				for (int i = 1 - TIME_TO_TICKS(F::Backtrack.GetReal()); i <= 0; i++)
				{
					F::MoveSim.RunTick(tMoveStorage);
					vOrigin = tMoveStorage.m_vPredictedOrigin;
				}
			}
			F::MoveSim.Restore(tMoveStorage);
		}
		if (!SDK::VisPosWorld(nullptr, pTarget, pSentry->m_vecOrigin() + pSentry->GetOffset() / 2 * pSentry->m_flModelScale(), vOrigin + pTarget->GetOffset() / 2))
			return vHitboxes;

		vHitboxes.emplace_back(nullptr, vOrigin);
		return vHitboxes;
	}

	if (!pTarget->IsPlayer())
	{
		vHitboxes.emplace_back();
		return vHitboxes;
	}

	if (auto pSet = pTarget->As<CBaseAnimating>()->GetHitboxSet())
	{
		std::vector<std::tuple<const mstudiobbox_t*, int, int>> vBones;
		for (int nHitbox = 0; nHitbox < pSet->numhitboxes; nHitbox++)
		{
			int iPriority = GetHitboxPriority(nHitbox, pLocal, pWeapon, pTarget);
			if (iPriority == -1)
				continue;

			auto pBox = pSet->pHitbox(nHitbox);
			if (!pBox)
				continue;

			vBones.emplace_back(pBox, nHitbox, iPriority);
		}
		std::sort(vBones.begin(), vBones.end(), [&](const auto& a, const auto& b) -> bool
		{
			return std::get<2>(a) < std::get<2>(b);
		});
		vHitboxes.reserve(vBones.size());
		for (auto& [pBox, nHitbox, _] : vBones)
			vHitboxes.emplace_back(pBox, std::nullopt, nHitbox);
	}

	return vHitboxes;
}

int CAimbotHitscan::GetHitboxPriority(int nHitbox, CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CBaseEntity* pTarget)
{
	if (!F::AimbotGlobal.IsHitboxValid(pTarget, nHitbox, Vars::Aimbot::Hitscan::Hitboxes.Value))
		return -1;

	bool bHeadshot = false;
	switch (pWeapon->GetWeaponID())
	{
	case TF_WEAPON_SNIPERRIFLE:
	case TF_WEAPON_SNIPERRIFLE_DECAP:
	case TF_WEAPON_SNIPERRIFLE_CLASSIC:
	{
		auto pSniperRifle = pWeapon->As<CTFSniperRifle>();

		if (G::CanHeadshot
			|| pLocal->InCond(TF_COND_AIMING) && (pSniperRifle->GetRifleType() == RIFLE_JARATE && SDK::AttribHookValue(0, "jarate_duration", pWeapon) > 0 || Vars::Aimbot::Hitscan::Modifiers.Value & Vars::Aimbot::Hitscan::ModifiersEnum::WaitForHeadshot))
			bHeadshot = true;
		break;
	}
	case TF_WEAPON_REVOLVER:
	{
		if (SDK::AttribHookValue(0, "set_weapon_mode", pWeapon) == 1 && (pWeapon->AmbassadorCanHeadshot() || Vars::Aimbot::Hitscan::Modifiers.Value & Vars::Aimbot::Hitscan::ModifiersEnum::WaitForHeadshot))
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

			int iDamage = ceilf(std::max(pSniperRifle->m_flChargedDamage(), 50.f) * pSniperRifle->GetBodyshotMult(pPlayer));
			if (pPlayer->m_iHealth() <= iDamage)
				bHeadshot = false;
			break;
		}
		case TF_WEAPON_REVOLVER:
		{
			if (SDK::AttribHookValue(0, "set_weapon_mode", pWeapon) == 1)
			{
				float flDistTo = pTarget->m_vecOrigin().DistTo(pLocal->m_vecOrigin());

				float flMult = SDK::AttribHookValue(1, "mult_dmg", pWeapon);
				int iDamage = ceilf(Math::RemapVal(flDistTo, 90.f, 900.f, 60.f, 21.f) * flMult);
				if (pPlayer->m_iHealth() <= iDamage)
					bHeadshot = false;
			}
		}
		}
	}
	
	bool bHeadOnly = bHeadshot && Vars::Aimbot::Hitscan::Hitboxes.Value & Vars::Aimbot::Hitscan::HitboxesEnum::HeadshotOnly;

	int iHeadPriority = bHeadshot ? 0 : 1;
	int iBodyPriority = bHeadOnly ? -1 : bHeadshot ? 1 : 0;
	int iMiscPriority = bHeadOnly ? -1 : 2;
	int iLimbPriority = bHeadOnly ? -1 : 3;

	switch (pTarget->GetHitboxToBase(nHitbox))
	{
	case HITBOX_HEAD: return iHeadPriority;
	case HITBOX_SPINE0:
	case HITBOX_SPINE1:
	case HITBOX_SPINE2:
	case HITBOX_SPINE3: return iBodyPriority;
	case HITBOX_PELVIS: return iMiscPriority;
	}

	return iLimbPriority;
};

void CAimbotHitscan::GetHitboxInfo(HitscanHitbox_t& tHitbox, CBaseEntity* pTarget, matrix3x4* aBones, const matrix3x4** pTransform, Vec3* pMins, Vec3* pMaxs)
{
	if (pMins && pMaxs)
	{
		if (tHitbox.m_pBox)
		{
			*pMins = tHitbox.m_pBox->bbmin;
			*pMaxs = tHitbox.m_pBox->bbmax;
		}
		else if (pTarget)
		{
			*pMins = pTarget->m_vecMins();
			*pMaxs = pTarget->m_vecMaxs();
		}
	}

	if (pTransform)
	{
		if (aBones && tHitbox.m_pBox)
			*pTransform = &aBones[tHitbox.m_pBox->bone];
		else if (tHitbox.m_vPos)
			*pTransform = &m_mMatrix, Math::MatrixInitialize(m_mMatrix, *tHitbox.m_vPos, false);
		else if (pTarget)
			*pTransform = &pTarget->m_Collision()->CollisionToWorldTransform();
	}
}

void CAimbotHitscan::GetHullInfo(CBaseEntity* pTarget, TickRecord* pRecord, const matrix3x4** pTransform, Vec3* pMins, Vec3* pMaxs, float* pModelScale)
{
	if (!pTarget->IsPlayer())
		return; // only really important for player targets to check that our trace will be in the bbox

	if (pModelScale)
		*pModelScale = pTarget->As<CBaseAnimating>()->m_flModelScale();

	if (pMins && pMaxs)
	{
		float flModelScale = pModelScale ? *pModelScale : 1.f;
		auto pGameRules = I::TFGameRules();
		auto pViewVectors = pGameRules ? pGameRules->GetViewVectors() : nullptr;
		*pMins = (pViewVectors ? pViewVectors->m_vHullMin : Vec3(-24, -24, 0)) * flModelScale;
		*pMaxs = (pViewVectors ? pViewVectors->m_vHullMax : Vec3(24, 24, 82)) * flModelScale;
	}

	if (pTransform && pRecord)
		*pTransform = &m_mHullMatrix, Math::MatrixInitialize(m_mHullMatrix, pRecord->m_vOrigin, false);
}

std::vector<Vec3> CAimbotHitscan::GetHitboxPoints(CBaseEntity* pTarget, CTFWeaponBase* pWeapon, const Vec3& vMins, const Vec3& vMaxs, int nHitbox)
{
	std::vector<Vec3> vPoints = { Vec3() };

	bool bPlayer = pTarget->IsPlayer();
	if (bPlayer && !F::AimbotGlobal.ShouldMultipoint(pTarget, nHitbox, Vars::Aimbot::Hitscan::MultipointHitboxes.Value)
		|| pWeapon->GetWeaponID() == TF_WEAPON_LASER_POINTER)
		return vPoints;

	switch (Vars::Aimbot::General::AimType.Value)
	{
	case Vars::Aimbot::General::AimTypeEnum::Smooth:
	case Vars::Aimbot::General::AimTypeEnum::Assistive:
		if (!Vars::Aimbot::General::AssistStrength.Value)
			return vPoints; // triggerbot
	}

	float flScale = !bPlayer ? 0.5f : Vars::Aimbot::Hitscan::MultipointScale.Value / 100;
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

	return vPoints;
}

int CAimbotHitscan::CanHit(Target_t& tTarget, CTFPlayer* pLocal, CTFWeaponBase* pWeapon)
{
	if (Vars::Aimbot::General::Ignore.Value & Vars::Aimbot::General::IgnoreEnum::Unsimulated && H::Entities.GetChoke(tTarget.m_pEntity->entindex()) > Vars::Aimbot::General::TickTolerance.Value)
		return false;

	m_vEyePos = pLocal->GetShootPos();
	float flMaxRangeSqr = powf(pWeapon->GetRange(), 2.f);
	bool bPlayer = tTarget.m_pEntity->IsPlayer();
	bool bWrangler = pWeapon->GetWeaponID() == TF_WEAPON_LASER_POINTER;

	std::vector<TickRecord*> vRecords = {};
	if (F::Backtrack.GetRecords(tTarget.m_pEntity, vRecords) && !bWrangler)
	{
		vRecords = F::Backtrack.GetValidRecords(vRecords, pLocal);
		if (vRecords.empty())
			return false;
	}
	else
	{
		F::Backtrack.m_tRecord = { tTarget.m_pEntity->m_flSimulationTime(), tTarget.m_pEntity->m_vecOrigin(), Vec3(), Vec3() };
		if (!tTarget.m_pEntity->SetupBones(F::Backtrack.m_tRecord.m_aBones, MAXSTUDIOBONES, BONE_USED_BY_ANYTHING, tTarget.m_pEntity->m_flSimulationTime()))
			return false;

		vRecords = { &F::Backtrack.m_tRecord };
	}

	auto vHitboxes = GetHitboxes(pLocal, pWeapon, tTarget.m_pEntity);
	if (vHitboxes.empty())
		return false;

	int iReturn = false;

	std::optional<Vec3> vPeekPos = std::nullopt;
	if (Vars::Aimbot::Hitscan::PeekAmount.Value && pWeapon->GetWeaponSpread())
	{
		switch (Vars::Aimbot::Hitscan::PeekCheck.Value)
		{
		case Vars::Aimbot::Hitscan::PeekCheckEnum::DoubletapOnly:
			if (!F::Ticks.GetTicks(pWeapon))
				break;
			[[fallthrough]];
		case Vars::Aimbot::Hitscan::PeekCheckEnum::Always:
			vPeekPos = m_vEyePos - pLocal->m_vecVelocity() * TICKS_TO_TIME(Vars::Aimbot::Hitscan::PeekAmount.Value);
		}
	}

	// if we're doubletapping, we can't change viewangles so work around that
	static int iTargetBone = 0;
	Vec3* pHoldAngle = F::Ticks.GetShootAngle(); if (pHoldAngle && bPlayer && vRecords.size() > 1)
	{
		Vec3 vHoldAngle = *pHoldAngle - F::NoSpread.GetOffset();
		std::sort(vRecords.begin(), vRecords.end(), [&](const TickRecord* a, const TickRecord* b) -> bool
		{
			Vec3 vPosA = { a->m_aBones[iTargetBone][0][3], a->m_aBones[iTargetBone][1][3], a->m_aBones[iTargetBone][2][3] };
			Vec3 vPosB = { b->m_aBones[iTargetBone][0][3], b->m_aBones[iTargetBone][1][3], b->m_aBones[iTargetBone][2][3] };
			Vec3 vAnglesA = Math::CalcAngle(m_vEyePos, vPosA);
			Vec3 vAnglesB = Math::CalcAngle(m_vEyePos, vPosB);
			return vHoldAngle.DeltaAngle(vAnglesA).Length2DSqr() < vHoldAngle.DeltaAngle(vAnglesB).Length2DSqr();
		});
	}

	float flBoneScale = std::max(Vars::Aimbot::Hitscan::BoneSizeMinimumScale.Value, Vars::Aimbot::Hitscan::MultipointScale.Value / 100.f);
	float flBoneSubtract = Vars::Aimbot::Hitscan::BoneSizeSubtract.Value;
	float flModelScale = 1.f;
	Vec3 vMins, vMaxs, vHullMins, vHullMaxs;
	const matrix3x4* pTransform = &m_mMatrix, *pHullTransform = nullptr;

	GetHullInfo(tTarget.m_pEntity, nullptr, nullptr, &vHullMins, &vHullMaxs, &flModelScale);
	for (auto pRecord : vRecords)
	{
		bool bRunPeekCheck = vPeekPos.has_value();
		auto aBones = pRecord->m_aBones;

		GetHullInfo(tTarget.m_pEntity, bPlayer && !bWrangler ? pRecord : nullptr, &pHullTransform);
		for (auto& tHitbox : vHitboxes)
		{
			GetHitboxInfo(tHitbox, tTarget.m_pEntity, aBones, &pTransform, &vMins, &vMaxs);
			Vec3 vCheckMins = (vMins + flBoneSubtract / flModelScale) * flBoneScale;
			Vec3 vCheckMaxs = (vMaxs - flBoneSubtract / flModelScale) * flBoneScale;
			Vec3 vOrigin; Math::VectorTransform({}, *pTransform, vOrigin);
			Vec3 vCenter; Math::VectorTransform((vMins + vMaxs) / 2, *pTransform, vCenter);
			Vec3 vOffset = vCenter - vOrigin;

			std::vector<Vec3> vPoints = GetHitboxPoints(tTarget.m_pEntity, pWeapon, vMins, vMaxs, tHitbox.m_iHitbox);
			for (auto& vPoint : vPoints)
			{
				Math::VectorTransform(vPoint, *pTransform, vOrigin); vOrigin += vOffset;
				if (m_vEyePos.DistToSqr(vOrigin) > flMaxRangeSqr)
					continue;

				if (bRunPeekCheck)
				{
					bRunPeekCheck = false;
					if (!SDK::VisPos(pLocal, tTarget.m_pEntity, *vPeekPos, vOrigin))
						goto skip; // if we can't hit our primary hitbox, don't bother
				}

				Vec3 vAngles; bool bChanged = Aim(G::CurrentUserCmd->viewangles, Math::CalcAngle(m_vEyePos, vOrigin), vAngles);
				if (pHoldAngle) vAngles -= F::NoSpread.GetOffset(); // hold angle recorded with nospread, correct it
				if (!F::AimbotGlobal.ShouldAimAtAngle(vAngles) || !bChanged && !SDK::VisPos(pLocal, tTarget.m_pEntity, m_vEyePos, vOrigin))
					continue;

				// for the time being, no vischecks against other hitboxes
				Vec3 vForward; Math::AngleVectors(vAngles, &vForward);
				if ((!bChanged || Math::RayToOBB(m_vEyePos, vForward, vCheckMins, vCheckMaxs, *pTransform, flModelScale) && SDK::VisPos(pLocal, tTarget.m_pEntity, m_vEyePos, m_vEyePos + vForward * m_vEyePos.DistTo(vOrigin)))
					&& (!pHullTransform || Math::RayToOBB(m_vEyePos, vForward, vHullMins, vHullMaxs, *pHullTransform)))
				{
					tTarget.m_vAngleTo = vAngles;
					tTarget.m_pRecord = pRecord;
					tTarget.m_vPos = vOrigin;
					if (tHitbox.m_pBox)
					{
						tTarget.m_bBacktrack = true;
						tTarget.m_nAimedHitbox = tHitbox.m_iHitbox;
						iTargetBone = tHitbox.m_pBox->bone;
					}
					return true;
				}
				else if (bChanged && SDK::VisPos(pLocal, tTarget.m_pEntity, m_vEyePos, vOrigin))
				{
					if (iReturn != 2 || vAngles.DeltaAngle(G::CurrentUserCmd->viewangles).Length2DSqr() < tTarget.m_vAngleTo.DeltaAngle(G::CurrentUserCmd->viewangles).Length2DSqr())
						tTarget.m_vAngleTo = vAngles;
					iReturn = 2;
				}
			}
		}

		skip: continue;
	}

	return iReturn;
}



bool CAimbotHitscan::ShouldFire(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd, const Target_t& tTarget)
{
	if (!Vars::Aimbot::General::AutoShoot.Value)
		return false;

	if (Vars::Aimbot::Hitscan::Modifiers.Value & Vars::Aimbot::Hitscan::ModifiersEnum::WaitForHeadshot
		&& tTarget.m_pEntity->IsPlayer())
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
			auto pSniperRifle = pWeapon->As<CTFSniperRifle>();
			if (!pLocal->InCond(TF_COND_AIMING) || pSniperRifle->m_flChargedDamage() == 150.f)
				break;

			if (tTarget.m_pEntity->IsPlayer())
			{
				auto pPlayer = tTarget.m_pEntity->As<CTFPlayer>();
				if (tTarget.m_nAimedHitbox == HITBOX_HEAD && (pWeapon->GetWeaponID() != TF_WEAPON_SNIPERRIFLE_CLASSIC || pSniperRifle->m_flChargedDamage() == 150.f))
				{
					int iDamage = ceilf(std::max(pSniperRifle->m_flChargedDamage(), 50.f) * pSniperRifle->GetHeadshotMult(pPlayer));
					if (pPlayer->m_iHealth() <= iDamage && (G::CanHeadshot || pLocal->IsCritBoosted()))
						break;
				}
				else
				{
					int iDamage = ceilf(std::max(pSniperRifle->m_flChargedDamage(), 50.f) * pSniperRifle->GetBodyshotMult(pPlayer));
					if (pPlayer->m_iHealth() <= iDamage)
						break;
				}
			}
			else if (tTarget.m_pEntity->IsBuilding())
			{
				auto pBuilding = tTarget.m_pEntity->As<CBaseObject>();
				int iDamage = ceilf(std::max(pSniperRifle->m_flChargedDamage(), 50.f));
				if (pBuilding->m_iHealth() <= iDamage)
					break;
			}
			else
				break;
			
			return false;
		}
		}
	}

	return true;
}

bool CAimbotHitscan::Aim(const Vec3& vCurAngle, Vec3 vToAngle, Vec3& vOut, int iMethod)
{
	if (Vec3* pHoldAngle = F::Ticks.GetShootAngle())
	{
		vOut = *pHoldAngle;
		return true;
	}

	bool bReturn = false;
	if (auto pLocal = H::Entities.GetLocal())
		vToAngle -= pLocal->m_vecPunchAngle();
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
		float flMouseDelta = vMouseDelta.Length2DSqr(), flTargetDelta = vTargetDelta.Length2DSqr();
		vTargetDelta = vTargetDelta.Normalized() * sqrtf(std::min(flMouseDelta, flTargetDelta));
		vOut = vCurAngle - vMouseDelta + vMouseDelta.LerpAngle(vTargetDelta, Vars::Aimbot::General::AssistStrength.Value / 100.f);
		bReturn = true;
		break;
	}

	//if (iMethod != Vars::Aimbot::General::AimTypeEnum::Silent || Vars::Misc::Game::AntiCheatCompatibility.Value)
		Math::ClampAngles(vOut);
	return bReturn;
}

// assume angle calculated outside with other overload
void CAimbotHitscan::Aim(CUserCmd* pCmd, Vec3& vAngles, int iMethod)
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
			G::SilentAngles = true;
		}
		break;
	case Vars::Aimbot::General::AimTypeEnum::Locking:
		SDK::FixMovement(pCmd, vAngles);
		pCmd->viewangles = vAngles;
		G::SilentAngles = true;
	}
}

static inline void DrawVisuals(CTFPlayer* pLocal, Target_t& tTarget, int nWeaponID)
{
	if (G::Attacking == 1)
	{
		G::AimTarget = { tTarget.m_pEntity->entindex(), I::GlobalVars->tickcount };
		G::AimPoint = { tTarget.m_vPos, I::GlobalVars->tickcount };
	}
	else
	{
		G::AimTarget = { tTarget.m_pEntity->entindex(), I::GlobalVars->tickcount, 0 };
		G::AimPoint = { tTarget.m_vPos, I::GlobalVars->tickcount, 0 };
	}

	if (G::Attacking == 1 && nWeaponID != TF_WEAPON_LASER_POINTER)
	{
		bool bLine = Vars::Visuals::Line::TracersEnabled.Value;
		bool bBoxes = Vars::Visuals::Hitbox::BonesEnabled.Value & Vars::Visuals::Hitbox::BonesEnabledEnum::OnShot;
		if (bLine || bBoxes)
		{
			G::LineStorage.clear();
			G::BoxStorage.clear();
			G::PathStorage.clear();

			if (bLine)
			{
				Vec3 vEyePos = pLocal->GetShootPos();
				float flDist = vEyePos.DistTo(tTarget.m_vPos);
				Vec3 vForward; Math::AngleVectors(tTarget.m_vAngleTo + pLocal->m_vecPunchAngle(), &vForward);

				if (Vars::Colors::LineIgnoreZ.Value.a)
					G::LineStorage.emplace_back(std::pair<Vec3, Vec3>(vEyePos, vEyePos + vForward * flDist), I::GlobalVars->curtime + Vars::Visuals::Line::DrawDuration.Value, Vars::Colors::LineIgnoreZ.Value);
				if (Vars::Colors::Line.Value.a)
					G::LineStorage.emplace_back(std::pair<Vec3, Vec3>(vEyePos, vEyePos + vForward * flDist), I::GlobalVars->curtime + Vars::Visuals::Line::DrawDuration.Value, Vars::Colors::Line.Value, true);
			}
			if (bBoxes)
			{
				auto vBoxes = F::Visuals.GetHitboxes(tTarget.m_pRecord->m_aBones, tTarget.m_pEntity->As<CBaseAnimating>(), {}, tTarget.m_nAimedHitbox);
				G::BoxStorage.insert(G::BoxStorage.end(), vBoxes.begin(), vBoxes.end());
			}
		}
	}
}

void CAimbotHitscan::Run(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd)
{
	const int nWeaponID = pWeapon->GetWeaponID();

	static int iStaticAimType = Vars::Aimbot::General::AimType.Value;
	const int iLastAimType = iStaticAimType;
	const int iRealAimType = Vars::Aimbot::General::AimType.Value;

	switch (nWeaponID)
	{
	case TF_WEAPON_SNIPERRIFLE_CLASSIC:
		if (G::Attacking && !iRealAimType && iLastAimType)
			Vars::Aimbot::General::AimType.Value = iLastAimType;
	}
	iStaticAimType = Vars::Aimbot::General::AimType.Value;

	if (F::AimbotGlobal.ShouldHoldAttack(pWeapon))
		pCmd->buttons |= IN_ATTACK;
	if (!Vars::Aimbot::General::AimType.Value
		|| !F::AimbotGlobal.ShouldAim() && (nWeaponID != TF_WEAPON_MINIGUN || pWeapon->As<CTFMinigun>()->m_iWeaponState() == AC_STATE_FIRING || pWeapon->As<CTFMinigun>()->m_iWeaponState() == AC_STATE_SPINNING))
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

	auto vTargets = F::AimbotGlobal.ManageTargets(GetTargets, pLocal, pWeapon);
	if (vTargets.empty())
		return;

	switch (nWeaponID)
	{
	case TF_WEAPON_SNIPERRIFLE:
	case TF_WEAPON_SNIPERRIFLE_DECAP:
	{
		bool bScoped = pLocal->InCond(TF_COND_ZOOMED);
		if (Vars::Aimbot::Hitscan::Modifiers.Value & Vars::Aimbot::Hitscan::ModifiersEnum::AutoScope && !bScoped)
		{
			pCmd->buttons |= IN_ATTACK2;
			return;
		}
		else if (Vars::Aimbot::Hitscan::Modifiers.Value & Vars::Aimbot::Hitscan::ModifiersEnum::ScopedOnly && !bScoped)
			return;
		else if (!bScoped && SDK::AttribHookValue(0, "sniper_only_fire_zoomed", pWeapon))
			return;
		break;
	}
	case TF_WEAPON_SNIPERRIFLE_CLASSIC:
		if (iRealAimType)
			pCmd->buttons |= IN_ATTACK;
	}

	if (!G::AimTarget.m_iEntIndex)
		G::AimTarget = { vTargets.front().m_pEntity->entindex(), I::GlobalVars->tickcount, 0 };

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
			G::AimTarget = { tTarget.m_pEntity->entindex(), I::GlobalVars->tickcount, 0 };
			Aim(pCmd, tTarget.m_vAngleTo);
			break;
		}

		if (ShouldFire(pLocal, pWeapon, pCmd, tTarget))
		{
			switch (nWeaponID)
			{
			case TF_WEAPON_MEDIGUN:
				if (!(G::LastUserCmd->buttons & IN_ATTACK))
					pCmd->buttons |= IN_ATTACK;
				break;
			case TF_WEAPON_SNIPERRIFLE_CLASSIC:
				if (pWeapon->As<CTFSniperRifle>()->m_flChargedDamage() && pLocal->m_hGroundEntity())
					pCmd->buttons &= ~IN_ATTACK;
				break;
			case TF_WEAPON_LASER_POINTER:
				pCmd->buttons |= IN_ATTACK | IN_ATTACK2;
				break;
			default:
				pCmd->buttons |= IN_ATTACK;
			}

			if (Vars::Aimbot::Hitscan::Modifiers.Value & Vars::Aimbot::Hitscan::ModifiersEnum::Tapfire && pWeapon->GetWeaponSpread() != 0.f && !pLocal->InCond(TF_COND_RUNE_PRECISION)
				&& m_vEyePos.DistToSqr(tTarget.m_vPos) > powf(Vars::Aimbot::Hitscan::TapfireDistance.Value, 2))
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
				pCmd->tick_count = TIME_TO_TICKS(tTarget.m_pRecord->m_flSimTime) + TIME_TO_TICKS(F::Backtrack.GetFakeInterp());
		}
		DrawVisuals(pLocal, tTarget, nWeaponID);

		Aim(pCmd, tTarget.m_vAngleTo);
		if (G::SilentAngles)
		{
			switch (nWeaponID)
			{
			case TF_WEAPON_MEDIGUN:
			//case TF_WEAPON_LASER_POINTER: // we can psilent with the wrangler though probably with some hacks
				G::SilentAngles = false, G::PSilentAngles = true;
			}
		}
		break;
	}
}