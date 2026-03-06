#include "AimbotProjectile.h"

#include "../Aimbot.h"
#include "../../EnginePrediction/EnginePrediction.h"
#include "../../Ticks/Ticks.h"
#include "../AutoAirblast/AutoAirblast.h"

//#define SPLASH_DEBUG1 // normal splash visualization
//#define SPLASH_DEBUG2 // obstructed splash visualization
//#define SPLASH_DEBUG3 // points visualization
//#define SPLASH_DEBUG4 // trace visualization
//#define SPLASH_DEBUG5 // trace count

#ifdef SPLASH_DEBUG5
static std::map<std::string, int> s_mTraceCount = {};
#endif

static inline std::vector<Target_t> GetTargets(CTFPlayer* pLocal, CTFWeaponBase* pWeapon)
{
	std::vector<Target_t> vTargets;

	const Vec3 vLocalPos = F::Ticks.GetShootPos();
	const Vec3 vLocalAngles = I::EngineClient->GetViewAngles();

	{
		auto eGroupType = EntityEnum::Invalid;
		if (Vars::Aimbot::General::Target.Value & Vars::Aimbot::General::TargetEnum::Players)
			eGroupType = !F::AimbotGlobal.FriendlyFire() || Vars::Aimbot::General::Ignore.Value & Vars::Aimbot::General::IgnoreEnum::Team ? EntityEnum::PlayerEnemy : EntityEnum::PlayerAll;
		switch (pWeapon->GetWeaponID())
		{
		case TF_WEAPON_CROSSBOW:
			if (Vars::Aimbot::Healing::AutoArrow.Value)
				eGroupType = eGroupType != EntityEnum::Invalid ? EntityEnum::PlayerAll : EntityEnum::PlayerTeam;
			break;
		case TF_WEAPON_LUNCHBOX:
			if (Vars::Aimbot::Healing::AutoSandvich.Value)
				eGroupType = EntityEnum::PlayerTeam;
			break;
		}
		bool bHeal = pWeapon->GetWeaponID() == TF_WEAPON_CROSSBOW || pWeapon->GetWeaponID() == TF_WEAPON_LUNCHBOX;

		for (auto pEntity : H::Entities.GetGroup(eGroupType))
		{
			if (F::AimbotGlobal.ShouldIgnore(pEntity, pLocal, pWeapon))
				continue;

			bool bTeam = pEntity->m_iTeamNum() == pLocal->m_iTeamNum();
			if (bTeam && bHeal)
			{
				if (pEntity->As<CTFPlayer>()->m_iHealth() >= pEntity->As<CTFPlayer>()->GetMaxHealth()
					|| Vars::Aimbot::Healing::HealPriority.Value == Vars::Aimbot::Healing::HealPriorityEnum::FriendsOnly && !H::Entities.IsFriend(pEntity->entindex()) && !H::Entities.InParty(pEntity->entindex()))
					continue;
			}

			float flFOVTo; Vec3 vPos, vAngleTo;
			if (!F::AimbotGlobal.PlayerBoneInFOV(pEntity->As<CTFPlayer>(), vLocalPos, vLocalAngles, flFOVTo, vPos, vAngleTo))
				continue;

			float flDistTo = vLocalPos.DistTo(vPos);
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

		if (pWeapon->GetWeaponID() == TF_WEAPON_LUNCHBOX)
			return vTargets;
	}

	{
		auto eGroupType = EntityEnum::Invalid;
		if (Vars::Aimbot::General::Target.Value & Vars::Aimbot::General::TargetEnum::Building)
			eGroupType = EntityEnum::BuildingEnemy;
		if (Vars::Aimbot::Healing::AutoRepair.Value && pWeapon->GetWeaponID() == TF_WEAPON_SHOTGUN_BUILDING_RESCUE)
			eGroupType = eGroupType != EntityEnum::Invalid ? EntityEnum::BuildingAll : EntityEnum::BuildingTeam;
		for (auto pEntity : H::Entities.GetGroup(eGroupType))
		{
			if (F::AimbotGlobal.ShouldIgnore(pEntity, pLocal, pWeapon))
				continue;

			bool bTeam = pEntity->m_iTeamNum() == pLocal->m_iTeamNum();
			if (bTeam && (pEntity->As<CBaseObject>()->m_iHealth() >= pEntity->As<CBaseObject>()->m_iMaxHealth() || pEntity->As<CBaseObject>()->m_bBuilding()))
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

			float flDistTo = vLocalPos.DistTo(vPos);
			vTargets.emplace_back(pEntity, pEntity->IsSentrygun() ? TargetEnum::Sentry : pEntity->IsDispenser() ? TargetEnum::Dispenser : TargetEnum::Teleporter, vPos, vAngleTo, flFOVTo, flDistTo, iPriority);
		}
	}

	if (Vars::Aimbot::General::Target.Value & Vars::Aimbot::General::TargetEnum::Stickies)
	{
		bool bShouldAim = false;
		switch (pWeapon->GetWeaponID())
		{
		case TF_WEAPON_PIPEBOMBLAUNCHER:
			if (SDK::AttribHookValue(0, "stickies_detonate_stickies", pWeapon) == 1)
				bShouldAim = true;
			break;
		case TF_WEAPON_FLAREGUN:
		case TF_WEAPON_FLAREGUN_REVENGE:
			if (pWeapon->As<CTFFlareGun>()->GetFlareGunType() == FLAREGUN_SCORCHSHOT)
				bShouldAim = true;
		}

		if (bShouldAim)
		{
			for (auto pEntity : H::Entities.GetGroup(EntityEnum::WorldProjectile))
			{
				if (F::AimbotGlobal.ShouldIgnore(pEntity, pLocal, pWeapon))
					continue;

				Vec3 vPos = pEntity->GetCenter();
				Vec3 vAngleTo = Math::CalcAngle(vLocalPos, vPos);
				float flFOVTo = Math::CalcFov(vLocalAngles, vAngleTo);
				if (flFOVTo > Vars::Aimbot::General::AimFOV.Value)
					continue;

				float flDistTo = vLocalPos.DistTo(vPos);
				vTargets.emplace_back(pEntity, TargetEnum::Sticky, vPos, vAngleTo, flFOVTo, flDistTo);
			}
		}
	}

	if (Vars::Aimbot::General::Target.Value & Vars::Aimbot::General::TargetEnum::NPCs) // does not predict movement
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

			float flDistTo = vLocalPos.DistTo(vPos);
			vTargets.emplace_back(pEntity, TargetEnum::NPC, vPos, vAngleTo, flFOVTo, flDistTo);
		}
	}

	return vTargets;
}



float CAimbotProjectile::GetSplashRadius(CTFWeaponBase* pWeapon, CTFPlayer* pPlayer)
{
	float flRadius = 0.f;
	switch (pWeapon->GetWeaponID())
	{
	case TF_WEAPON_ROCKETLAUNCHER:
	case TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT:
	case TF_WEAPON_PARTICLE_CANNON:
	case TF_WEAPON_PIPEBOMBLAUNCHER:
		flRadius = TF_ROCKET_RADIUS;
		break;
	case TF_WEAPON_FLAREGUN:
	case TF_WEAPON_FLAREGUN_REVENGE:
		if (pWeapon->As<CTFFlareGun>()->GetFlareGunType() == FLAREGUN_SCORCHSHOT)
			flRadius = TF_FLARE_DET_RADIUS;
	}
	if (!flRadius)
		return 0.f;

	flRadius = SDK::AttribHookValue(flRadius, "mult_explosion_radius", pWeapon);
	switch (pWeapon->GetWeaponID())
	{
	case TF_WEAPON_ROCKETLAUNCHER:
	case TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT:
	case TF_WEAPON_PARTICLE_CANNON:
		if (pPlayer->InCond(TF_COND_BLASTJUMPING) && SDK::AttribHookValue(1.f, "rocketjump_attackrate_bonus", pWeapon) != 1.f)
			flRadius *= 0.8f;
	}
	return flRadius * Vars::Aimbot::Projectile::SplashRadius.Value / 100;
}

float CAimbotProjectile::GetSplashRadius(CBaseEntity* pProjectile, CTFWeaponBase* pWeapon, CTFPlayer* pPlayer, float flScale, CTFWeaponBase* pAirblast)
{
	float flRadius = 0.f;
	if (pAirblast)
	{
		pWeapon = pAirblast;
		pPlayer = pWeapon->m_hOwner()->As<CTFPlayer>();
	}
	switch (pProjectile->GetClassID())
	{
	case ETFClassID::CTFWeaponBaseGrenadeProj:
	case ETFClassID::CTFWeaponBaseMerasmusGrenade:
	case ETFClassID::CTFProjectile_Rocket:
	case ETFClassID::CTFProjectile_SentryRocket:
	case ETFClassID::CTFProjectile_EnergyBall:
		flRadius = TF_ROCKET_RADIUS;
		break;
	case ETFClassID::CTFGrenadePipebombProjectile:
		if (pProjectile->As<CTFGrenadePipebombProjectile>()->HasStickyEffects())
			flRadius = TF_ROCKET_RADIUS;
		break;
	case ETFClassID::CTFProjectile_Flare:
		if (pWeapon && pWeapon->As<CTFFlareGun>()->GetFlareGunType() == FLAREGUN_SCORCHSHOT)
			flRadius = TF_FLARE_DET_RADIUS;
	}
	if (pPlayer && pWeapon)
	{
		flRadius = SDK::AttribHookValue(flRadius, "mult_explosion_radius", pWeapon);
		switch (pProjectile->GetClassID())
		{
		case ETFClassID::CTFProjectile_Rocket:
		case ETFClassID::CTFProjectile_SentryRocket:
			if (pPlayer->InCond(TF_COND_BLASTJUMPING) && SDK::AttribHookValue(1.f, "rocketjump_attackrate_bonus", pWeapon) != 1.f)
				flRadius *= 0.8f;
		}
	}
	return flRadius * flScale;
}

static inline int GetSplashMode(CTFWeaponBase* pWeapon)
{
	if (Vars::Aimbot::Projectile::RocketSplashMode.Value)
	{
		switch (pWeapon->GetWeaponID())
		{
		case TF_WEAPON_ROCKETLAUNCHER:
		case TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT:
		case TF_WEAPON_PARTICLE_CANNON:
			return Vars::Aimbot::Projectile::RocketSplashMode.Value;
		}
	}

	return Vars::Aimbot::Projectile::RocketSplashModeEnum::Regular;
}

static inline float ArmTime(CTFWeaponBase* pWeapon)
{
	if (Vars::Aimbot::Projectile::Modifiers.Value & Vars::Aimbot::Projectile::ModifiersEnum::UseArmTime && pWeapon->GetWeaponID() == TF_WEAPON_PIPEBOMBLAUNCHER)
	{
		static auto tf_grenadelauncher_livetime = H::ConVars.FindVar("tf_grenadelauncher_livetime");
		const float flLiveTime = tf_grenadelauncher_livetime->GetFloat();
		return SDK::AttribHookValue(flLiveTime, "sticky_arm_time", pWeapon);
	}

	return 0.f;
}

static inline int GetHitboxPriority(int nHitbox, Target_t& tTarget, Info_t& tInfo, CBaseEntity* pProjectile = nullptr)
{
	if (!F::AimbotGlobal.IsHitboxValid(nHitbox, Vars::Aimbot::Projectile::Hitboxes.Value))
		return -1;

	int iHeadPriority = 0;
	int iBodyPriority = 1;
	int iFeetPriority = 2;

	if (Vars::Aimbot::Projectile::Hitboxes.Value & Vars::Aimbot::Projectile::HitboxesEnum::Auto)
	{
		bool bHeadshot = Vars::Aimbot::Projectile::Hitboxes.Value & Vars::Aimbot::Projectile::HitboxesEnum::Head
			&& tTarget.m_iTargetType == TargetEnum::Player;
		if (bHeadshot)
		{
			if (!pProjectile)
				bHeadshot = tInfo.m_pWeapon->GetWeaponID() == TF_WEAPON_COMPOUND_BOW;
			else
				bHeadshot = pProjectile->GetClassID() == ETFClassID::CTFProjectile_Arrow && pProjectile->As<CTFProjectile_Arrow>()->CanHeadshot();

			if (Vars::Aimbot::Projectile::Hitboxes.Value & Vars::Aimbot::Projectile::HitboxesEnum::BodyaimIfLethal
				&& bHeadshot && !pProjectile && tInfo.m_pWeapon->m_hOwner().GetEntryIndex() == I::EngineClient->GetLocalPlayer())
			{
				float flCharge = I::GlobalVars->curtime - tInfo.m_pWeapon->As<CTFPipebombLauncher>()->m_flChargeBeginTime();
				float flDamage = Math::RemapVal(flCharge, 0.f, 1.f, 50.f, 120.f);
				if (tInfo.m_pLocal->IsMiniCritBoosted())
					flDamage *= 1.36f;
				if (flDamage >= tTarget.m_pEntity->As<CTFPlayer>()->m_iHealth())
					bHeadshot = false;

				if (tInfo.m_pLocal->IsCritBoosted()) // for reliability
					bHeadshot = false;
			}
		}
		if (bHeadshot)
			tTarget.m_nAimedHitbox = HITBOX_HEAD;

		bool bLower = Vars::Aimbot::Projectile::Hitboxes.Value & Vars::Aimbot::Projectile::HitboxesEnum::PrioritizeFeet
			&& tTarget.m_iTargetType == TargetEnum::Player && tTarget.m_pEntity->As<CTFPlayer>()->IsOnGround() /*&& tInfo.m_flRadius*/;

		iHeadPriority = bHeadshot ? 0 : 2;
		iBodyPriority = bHeadshot ? -1 : bLower ? 1 : 0;
		iFeetPriority = bHeadshot ? -1 : bLower ? 0 : 1;
	}

	switch (nHitbox)
	{
	case BOUNDS_HEAD: return iHeadPriority;
	case BOUNDS_BODY: return iBodyPriority;
	case BOUNDS_FEET: return iFeetPriority;
	}

	return -1;
};

std::unordered_map<int, Vec3> CAimbotProjectile::GetDirectPoints()
{
	std::unordered_map<int, Vec3> mPoints = {};

	auto& tTarget = *m_tInfo.m_pTarget;

	const Vec3 vMins = tTarget.m_pEntity->m_vecMins(), vMaxs = tTarget.m_pEntity->m_vecMaxs();
	for (int i = 0; i < 3; i++)
	{
		int iPriority = GetHitboxPriority(i, tTarget, m_tInfo, m_tInfo.m_pProjectile);
		if (iPriority == -1)
			continue;

		switch (i)
		{
		case BOUNDS_HEAD:
			if (tTarget.m_nAimedHitbox == HITBOX_HEAD)
			{
				auto aBones = F::Backtrack.GetBones(tTarget.m_pEntity);
				if (!aBones)
					break;

				//Vec3 vOff = tTarget.m_pEntity->As<CBaseAnimating>()->GetHitboxOrigin(aBones, HITBOX_HEAD) - tTarget.m_pEntity->m_vecOrigin();

				// https://www.youtube.com/watch?v=_PSGD-pJUrM, might be better??
				Vec3 vCenter, vBBoxMins, vBBoxMaxs; tTarget.m_pEntity->As<CBaseAnimating>()->GetHitboxInfo(aBones, HITBOX_HEAD, &vCenter, &vBBoxMins, &vBBoxMaxs);
				Vec3 vOff = vCenter + (vBBoxMins + vBBoxMaxs) / 2 - tTarget.m_pEntity->m_vecOrigin();

				float flLow = 0.f;
				Vec3 vDelta = tTarget.m_vPos + m_tInfo.m_vTargetEye - m_tInfo.m_vLocalEye;
				if (vDelta.z > 0)
				{
					float flXY = vDelta.Length2D();
					if (flXY)
						flLow = Math::RemapVal(vDelta.z / flXY, 0.f, 0.5f, 0.f, 1.f);
					else
						flLow = 1.f;
				}

				float flLerp = (Vars::Aimbot::Projectile::HuntsmanLerp.Value + (Vars::Aimbot::Projectile::HuntsmanLerpLow.Value - Vars::Aimbot::Projectile::HuntsmanLerp.Value) * flLow) / 100.f;
				float flAdd = Vars::Aimbot::Projectile::HuntsmanAdd.Value + (Vars::Aimbot::Projectile::HuntsmanAddLow.Value - Vars::Aimbot::Projectile::HuntsmanAdd.Value) * flLow;
				vOff.z += flAdd;
				vOff.z = vOff.z + (vMaxs.z - vOff.z) * flLerp;

				vOff.x = std::clamp(vOff.x, vMins.x + Vars::Aimbot::Projectile::HuntsmanClamp.Value, vMaxs.x - Vars::Aimbot::Projectile::HuntsmanClamp.Value);
				vOff.y = std::clamp(vOff.y, vMins.y + Vars::Aimbot::Projectile::HuntsmanClamp.Value, vMaxs.y - Vars::Aimbot::Projectile::HuntsmanClamp.Value);
				vOff.z = std::clamp(vOff.z, vMins.z + Vars::Aimbot::Projectile::HuntsmanClamp.Value, vMaxs.z - Vars::Aimbot::Projectile::HuntsmanClamp.Value);
				mPoints[iPriority] = vOff;
			}
			else
				mPoints[iPriority] = Vec3(0, 0, vMaxs.z - Vars::Aimbot::Projectile::VerticalShift.Value);
			break;
		case BOUNDS_BODY: mPoints[iPriority] = Vec3(0, 0, (vMaxs.z - vMins.z) / 2); break;
		case BOUNDS_FEET: mPoints[iPriority] = Vec3(0, 0, vMins.z + Vars::Aimbot::Projectile::VerticalShift.Value); break;
		}
	}

	return mPoints;
}

static inline std::vector<std::pair<Vec3, int>> ComputeSphere(float flRadius, int iSamples)
{
	std::vector<std::pair<Vec3, int>> vPoints;
	vPoints.reserve(iSamples);

	float flRotateX = Vars::Aimbot::Projectile::SplashRotateX.Value < 0.f ? SDK::StdRandomFloat(0.f, 360.f) : Vars::Aimbot::Projectile::SplashRotateX.Value;
	float flRotateY = Vars::Aimbot::Projectile::SplashRotateY.Value < 0.f ? SDK::StdRandomFloat(0.f, 360.f) : Vars::Aimbot::Projectile::SplashRotateY.Value;

	int iPointType = Vars::Aimbot::Projectile::SplashGrates.Value ? PointTypeEnum::Out | PointTypeEnum::In : PointTypeEnum::Out;
	if (Vars::Aimbot::Projectile::RocketSplashMode.Value == Vars::Aimbot::Projectile::RocketSplashModeEnum::SpecialHeavy)
		iPointType |= PointTypeEnum::Out2 | PointTypeEnum::In2;

	float a = PI * (3.f - sqrtf(5.f));
	for (int n = 0; n < iSamples; n++)
	{
		float t = a * n;
		float y = 1 - (n / (iSamples - 1.f)) * 2;
		float r = sqrtf(1 - powf(y, 2));
		float x = cosf(t) * r;
		float z = sinf(t) * r;

		Vec3 vPoint = Vec3(x, y, z) * flRadius;
		vPoint = Math::RotatePoint(vPoint, {}, { flRotateX, flRotateY });

		vPoints.emplace_back(vPoint, iPointType);
	}
	vPoints.emplace_back(Vec3(0.f, 0.f, -1.f) * flRadius, iPointType);

	return vPoints;
};

template <class T>
static inline void TracePoint(Vec3& vPoint, int& iType, Vec3& vTargetEye, Info_t& tInfo, T& vPoints, std::function<bool(CGameTrace& trace, bool& bErase, bool& bNormal)> checkPoint, int i = 0)
{
	// if anyone knows ways to further optimize this or just a better method, let me know!

	int iOriginalType = iType;
	bool bErase = false, bNormal = false;

	CGameTrace trace = {};
	CTraceFilterWorldAndPropsOnly filter = {};

#if defined(SPLASH_DEBUG1) || defined(SPLASH_DEBUG2)
	auto drawTrace = [](bool bSuccess, Color_t tColor, CGameTrace& trace)
		{
			Vec3 vMins = Vec3(-1, -1, -1) / (bSuccess ? 1 : 2), vMaxs = Vec3(1, 1, 1) / (bSuccess ? 1 : 2);
			Vec3 vAngles = Math::VectorAngles(trace.plane.normal);
			G::BoxStorage.emplace_back(trace.endpos, vMins, vMaxs, vAngles, I::GlobalVars->curtime + 60.f, tColor.Alpha(tColor.a / (bSuccess ? 1 : 10)), Color_t(0, 0, 0, 0));
			G::LineStorage.emplace_back(std::pair<Vec3, Vec3>(trace.startpos, trace.endpos), I::GlobalVars->curtime + 60.f, tColor.Alpha(tColor.a / (bSuccess ? 1 : 10)));
		};
#endif

	if (iType & PointTypeEnum::Out)
	{
		SDK::TraceHull(vTargetEye, vPoint, tInfo.m_vHull * -1, tInfo.m_vHull, MASK_SOLID, &filter, &trace);
#ifdef SPLASH_DEBUG5
		s_mTraceCount["Splash out"]++;
#endif

		if (checkPoint(trace, bErase, bNormal))
		{
			if (i % Vars::Aimbot::Projectile::SplashNormalSkip.Value)
				vPoints.pop_back();
#ifdef SPLASH_DEBUG1
			else
				drawTrace(!bNormal, Vars::Colors::Local.Value, trace);
#endif
		}

		if (bErase)
			iType = 0;
		else if (bNormal)
			iType &= ~PointTypeEnum::Out;
		else
			iType &= ~PointTypeEnum::In;
	}
	if (iType & PointTypeEnum::Out2)
	{
		bErase = false, bNormal = false;
		size_t iOriginalSize = vPoints.size();

		{
			float flNormal = (tInfo.m_vLocalEye - vTargetEye).DotNormalized(vTargetEye - vPoint);
			if (bNormal = Vars::Aimbot::Projectile::Out2NormalMin.Value > flNormal || flNormal > Vars::Aimbot::Projectile::Out2NormalMax.Value)
				goto breakOutExtra;

			if (!(iOriginalType & PointTypeEnum::Out)) // don't do the same trace over again
			{
				SDK::Trace(vTargetEye, vPoint, MASK_SOLID, &filter, &trace);
#ifdef SPLASH_DEBUG5
				s_mTraceCount["Splash out 1"]++;
#endif
				bNormal = !trace.m_pEnt || trace.fraction == 1.f;
#ifdef SPLASH_DEBUG2
				drawTrace(!bNormal, Vars::Colors::IndicatorTextMid.Value, trace);
#endif
				if (bNormal)
					goto breakOutExtra;
			}

			switch (trace.m_pEnt->GetClassID())
			{	// make sure we get past entity or prop
			case ETFClassID::CWorld:
				if (trace.hitbox) filter.iTeam = trace.hitbox - 1;
				filter.pSkip = nullptr;
				break;
			default:
				filter.pSkip = trace.m_pEnt;
			}
			SDK::Trace(trace.endpos - (vTargetEye - vPoint).Normalized(), vPoint, MASK_SOLID | CONTENTS_NOSKIP, &filter, &trace);
			filter.pSkip = nullptr, filter.iTeam = -1;
#ifdef SPLASH_DEBUG5
			s_mTraceCount["Splash out 2"]++;
#endif
			bNormal = trace.fraction == 1.f || !trace.fraction || trace.fraction == trace.fractionleftsolid || trace.allsolid || trace.surface.flags & (/*SURF_NODRAW |*/ SURF_SKY);
#ifdef SPLASH_DEBUG2
			drawTrace(!bNormal, Vars::Colors::IndicatorTextBad.Value, trace);
#endif
			if (bNormal)
				goto breakOutExtra;

			if (checkPoint(trace, bErase, bNormal))
			{
				SDK::Trace(trace.endpos + trace.plane.normal, vTargetEye, MASK_SHOT, &filter, &trace);
#ifdef SPLASH_DEBUG5
				s_mTraceCount["Splash out rocket check"]++;
#endif
#ifdef SPLASH_DEBUG2
				drawTrace(trace.fraction >= 1.f, Vars::Colors::IndicatorTextMisc.Value, trace);
#endif
				if (trace.fraction < 1.f)
					vPoints.pop_back();
			}
		}

	breakOutExtra:
		if (vPoints.size() != iOriginalSize)
			iType = 0;
		else if (bErase || bNormal)
			iType &= ~PointTypeEnum::Out2;
	}
	if (iType & PointTypeEnum::In)
	{
		bErase = false, bNormal = false;
		size_t iOriginalSize = vPoints.size();

		if (bNormal = (tInfo.m_vLocalEye - vTargetEye).Dot(vTargetEye - vPoint) > 0.f)
			goto breakOut;

		if (tInfo.m_iSplashMode == Vars::Aimbot::Projectile::RocketSplashModeEnum::Regular) // just do this for non rockets, it's less expensive
		{
			SDK::Trace(vPoint, vTargetEye, MASK_SHOT, &filter, &trace);
#ifdef SPLASH_DEBUG5
			s_mTraceCount["Splash in check"]++;
#endif
			bNormal = trace.DidHit();
#ifdef SPLASH_DEBUG2
			drawTrace(!bNormal, Vars::Colors::IndicatorGood.Value, trace);
#endif
			if (bNormal)
				goto breakOut;

			SDK::TraceHull(vPoint, vTargetEye, tInfo.m_vHull * -1, tInfo.m_vHull, MASK_SOLID, &filter, &trace);
#ifdef SPLASH_DEBUG5
			s_mTraceCount["Splash in"]++;
#endif

			checkPoint(trace, bErase, bNormal);
#ifdef SPLASH_DEBUG2
			drawTrace(!bNormal, Vars::Colors::Local.Value, trace);
#endif
		}
		else // currently experimental, there may be a more efficient way to do this?
		{
			SDK::Trace(vPoint, vTargetEye, MASK_SOLID | CONTENTS_NOSKIP, &filter, &trace);
#ifdef SPLASH_DEBUG5
			s_mTraceCount["Splash in 1"]++;
#endif
			bNormal = trace.fraction == 1.f || !trace.fraction || trace.allsolid || trace.surface.flags & SURF_SKY;
#ifdef SPLASH_DEBUG2
			drawTrace(!bNormal, Vars::Colors::IndicatorMid.Value, trace);
#endif
			if (!bNormal && trace.surface.flags & SURF_NODRAW)
			{
				if (bNormal = !(iType & PointTypeEnum::In2))
					goto breakOut;

				CGameTrace trace2 = {};
				SDK::Trace(trace.endpos - (vPoint - vTargetEye).Normalized(), vTargetEye, MASK_SOLID | CONTENTS_NOSKIP, &filter, &trace2);
#ifdef SPLASH_DEBUG5
				s_mTraceCount["Splash in 2"]++;
#endif
				bNormal = trace2.fraction == 1.f || !trace2.fraction || trace2.fraction == trace2.fractionleftsolid || trace2.allsolid || trace2.surface.flags & (SURF_NODRAW | SURF_SKY);
#ifdef SPLASH_DEBUG2
				drawTrace(!bNormal, Vars::Colors::IndicatorBad.Value, trace2);
#endif
				if (!bNormal)
					trace = trace2;
			}
			if (bNormal)
				goto breakOut;

			if (checkPoint(trace, bErase, bNormal))
			{
				SDK::Trace(trace.endpos + trace.plane.normal, vTargetEye, MASK_SHOT, &filter, &trace);
#ifdef SPLASH_DEBUG5
				s_mTraceCount["Splash in rocket check"]++;
#endif
#ifdef SPLASH_DEBUG2
				drawTrace(!bNormal, Vars::Colors::IndicatorMisc.Value, trace);
#endif
				if (trace.fraction < 1.f)
					vPoints.pop_back();
			}
		}

	breakOut:
		if (vPoints.size() != iOriginalSize)
			iType = 0;
		else if (bErase || bNormal)
			iType &= ~PointTypeEnum::In;
		else
			iType &= ~PointTypeEnum::Out;
	}
}

// possibly add air splash for autodet weapons
std::vector<Point_t> CAimbotProjectile::GetSplashPoints(Vec3 vOrigin, std::vector<std::pair<Vec3, int>>& vSpherePoints, int iSimTime)
{
	std::vector<std::pair<Point_t, float>> vPointDistances = {};

	Vec3 vTargetEye = vOrigin + m_tInfo.m_vTargetEye;

	auto checkPoint = [&](CGameTrace& trace, bool& bErase, bool& bNormal)
		{
			bErase = !trace.m_pEnt || trace.fraction == 1.f || trace.surface.flags & SURF_SKY || !trace.m_pEnt->GetAbsVelocity().IsZero();
			if (bErase)
				return false;

			Point_t tPoint = { trace.endpos, {} };
			if (!m_tInfo.m_flGravity)
			{
				Vec3 vForward = (m_tInfo.m_vLocalEye - trace.endpos).Normalized();
				bNormal = vForward.Dot(trace.plane.normal) <= 0;
			}
			if (!bNormal)
			{
				CalculateAngle(m_tInfo.m_vLocalEye, tPoint.m_vPoint, iSimTime, tPoint.m_tSolution, true, !m_tInfo.m_iArmTime ? 0 : -1);
				if (m_tInfo.m_flGravity)
				{
					Vec3 vPos = m_tInfo.m_vLocalEye + Vec3(0, 0, (m_tInfo.m_flGravity * 800.f * pow(tPoint.m_tSolution.m_flTime, 2)) / 2);
					Vec3 vForward = (vPos - tPoint.m_vPoint).Normalized();
					bNormal = vForward.Dot(trace.plane.normal) <= 0;
				}
			}
			if (bNormal)
				return false;

			bErase = tPoint.m_tSolution.m_iCalculated == CalculatedEnum::Good;
			if (!bErase)
				return false;

			vPointDistances.emplace_back(tPoint, tPoint.m_vPoint.DistTo(vOrigin));
			return true;
		};

	int i = 0;
	for (auto it = vSpherePoints.begin(); it != vSpherePoints.end();)
	{
		Vec3 vPoint = it->first + vTargetEye;
		int& iType = it->second;

		Solution_t tSolution; CalculateAngle(m_tInfo.m_vLocalEye, vPoint, iSimTime, tSolution, false);

		if (tSolution.m_iCalculated == CalculatedEnum::Bad)
			iType = 0;
		else if (abs(tSolution.m_flTime - TICKS_TO_TIME(iSimTime)) < m_tInfo.m_flRadiusTime || m_tInfo.m_iArmTime && iSimTime == m_tInfo.m_iArmTime)
			TracePoint(vPoint, iType, vTargetEye, m_tInfo, vPointDistances, checkPoint, i++);

		if (!(iType & ~PointTypeEnum::In2))
			it = vSpherePoints.erase(it);
		else
			++it;
	}

	std::sort(vPointDistances.begin(), vPointDistances.end(), [&](const auto& a, const auto& b) -> bool
		{
			return a.second < b.second;
		});

	std::vector<Point_t> vPoints = {};
	int iSplashCount = std::min(
		m_tInfo.m_iArmTime && iSimTime == m_tInfo.m_iArmTime ? Vars::Aimbot::Projectile::SplashCountDirect.Value : m_tInfo.m_iSplashCount,
		int(vPointDistances.size())
	);
	for (int i = 0; i < iSplashCount; i++)
		vPoints.push_back(vPointDistances[i].first);

	const Vec3 vOriginal = m_tInfo.m_pTarget->m_pEntity->GetAbsOrigin();
	m_tInfo.m_pTarget->m_pEntity->SetAbsOrigin(vOrigin);
	for (auto it = vPoints.begin(); it != vPoints.end();)
	{
		auto& tPoint = *it;
		bool bValid = tPoint.m_tSolution.m_iCalculated != CalculatedEnum::Pending;
		if (bValid)
		{
			Vec3 vPos; m_tInfo.m_pTarget->m_pEntity->m_Collision()->CalcNearestPoint(tPoint.m_vPoint, &vPos);
			bValid = tPoint.m_vPoint.DistTo(vPos) < m_tInfo.m_flRadius;
		}

		if (bValid)
			++it;
		else
			it = vPoints.erase(it);
	}
	m_tInfo.m_pTarget->m_pEntity->SetAbsOrigin(vOriginal);

	return vPoints;
}

void CAimbotProjectile::SetupSplashPoints(Vec3& vPos, std::vector<std::pair<Vec3, int>>& vSpherePoints, std::vector<Vec3>& vSimplePoints)
{
	vSimplePoints.clear();
	Vec3 vTargetEye = vPos + m_tInfo.m_vTargetEye;

	auto checkPoint = [&](CGameTrace& trace, bool& bErase, bool& bNormal)
		{
			bErase = !trace.m_pEnt || trace.fraction == 1.f || trace.surface.flags & SURF_SKY || !trace.m_pEnt->GetAbsVelocity().IsZero();
			if (bErase)
				return false;

			Point_t tPoint = { trace.endpos, {} };
			if (!m_tInfo.m_flGravity)
			{
				Vec3 vForward = (m_tInfo.m_vLocalEye - trace.endpos).Normalized();
				bNormal = vForward.Dot(trace.plane.normal) <= 0;
			}
			if (!bNormal)
			{
				CalculateAngle(m_tInfo.m_vLocalEye, tPoint.m_vPoint, 0, tPoint.m_tSolution, false);
				if (m_tInfo.m_flGravity)
				{
					Vec3 vPos = m_tInfo.m_vLocalEye + Vec3(0, 0, (m_tInfo.m_flGravity * 800.f * pow(tPoint.m_tSolution.m_flTime, 2)) / 2);
					Vec3 vForward = (vPos - tPoint.m_vPoint).Normalized();
					bNormal = vForward.Dot(trace.plane.normal) <= 0;
				}
			}
			if (bNormal)
				return false;

			if (tPoint.m_tSolution.m_iCalculated != CalculatedEnum::Bad)
			{
				vSimplePoints.emplace_back(tPoint.m_vPoint);
				return true;
			}
			return false;
		};

	int i = 0;
	for (auto& vSpherePoint : vSpherePoints)
	{
		Vec3 vPoint = vSpherePoint.first + vTargetEye;
		int& iType = vSpherePoint.second;

		Solution_t tSolution; CalculateAngle(m_tInfo.m_vLocalEye, vPoint, 0, tSolution, false);

		if (tSolution.m_iCalculated != CalculatedEnum::Bad)
			TracePoint(vPoint, iType, vTargetEye, m_tInfo, vSimplePoints, checkPoint, i++);
	}
}

std::vector<Point_t> CAimbotProjectile::GetSplashPointsSimple(Vec3 vOrigin, std::vector<Vec3>& vSpherePoints, int iSimTime)
{
	std::vector<std::pair<Point_t, float>> vPointDistances = {};

	Vec3 vTargetEye = vOrigin + m_tInfo.m_vTargetEye;

	auto checkPoint = [&](Vec3& vPoint, bool& bErase)
		{
			Point_t tPoint = { vPoint, {} };
			CalculateAngle(m_tInfo.m_vLocalEye, tPoint.m_vPoint, iSimTime, tPoint.m_tSolution, true, !m_tInfo.m_iArmTime ? 0 : -1);

			bErase = tPoint.m_tSolution.m_iCalculated == CalculatedEnum::Good;
			if (!bErase)
				return false;

			vPointDistances.emplace_back(tPoint, tPoint.m_vPoint.DistTo(vOrigin));
			return true;
		};
	for (auto it = vSpherePoints.begin(); it != vSpherePoints.end();)
	{
		bool bErase = false;

		checkPoint(*it, bErase);

		if (bErase)
			it = vSpherePoints.erase(it);
		else
			++it;
	}

	std::sort(vPointDistances.begin(), vPointDistances.end(), [&](const auto& a, const auto& b) -> bool
		{
			return a.second < b.second;
		});

	std::vector<Point_t> vPoints = {};
	int iSplashCount = std::min(
		m_tInfo.m_iArmTime && iSimTime == m_tInfo.m_iArmTime ? Vars::Aimbot::Projectile::SplashCountDirect.Value : m_tInfo.m_iSplashCount,
		int(vPointDistances.size())
	);
	for (int i = 0; i < iSplashCount; i++)
		vPoints.push_back(vPointDistances[i].first);

	const Vec3 vOriginal = m_tInfo.m_pTarget->m_pEntity->GetAbsOrigin();
	m_tInfo.m_pTarget->m_pEntity->SetAbsOrigin(vOrigin);
	for (auto it = vPoints.begin(); it != vPoints.end();)
	{
		auto& tPoint = *it;
		bool bValid = tPoint.m_tSolution.m_iCalculated != CalculatedEnum::Pending;
		if (bValid)
		{
			Vec3 vPos = {}; m_tInfo.m_pTarget->m_pEntity->m_Collision()->CalcNearestPoint(tPoint.m_vPoint, &vPos);
			bValid = tPoint.m_vPoint.DistTo(vPos) < m_tInfo.m_flRadius;
		}

		if (bValid)
			++it;
		else
			it = vPoints.erase(it);
	}
	m_tInfo.m_pTarget->m_pEntity->SetAbsOrigin(vOriginal);

	return vPoints;
}

static inline float AABBLine(Vec3 vMins, Vec3 vMaxs, Vec3 vStart, Vec3 vDir)
{
	Vec3 a = {
		(vMins.x - vStart.x) / vDir.x,
		(vMins.y - vStart.y) / vDir.y,
		(vMins.z - vStart.z) / vDir.z
	};
	Vec3 b = {
		(vMaxs.x - vStart.x) / vDir.x,
		(vMaxs.y - vStart.y) / vDir.y,
		(vMaxs.z - vStart.z) / vDir.z
	};
	Vec3 c = {
		std::min(a.x, b.x),
		std::min(a.y, b.y),
		std::min(a.z, b.z)
	};
	return std::max(std::max(c.x, c.y), c.z);
}
static inline Vec3 PullPoint(Vec3 vPoint, Vec3 vLocalPos, Info_t& tInfo, Vec3 vMins, Vec3 vMaxs, Vec3 vTargetPos)
{
	auto HeightenLocalPos = [&]()
		{	// basic trajectory pass
			const float flGrav = tInfo.m_flGravity * 800.f;
			if (!flGrav)
				return vPoint;

			const Vec3 vDelta = vTargetPos - vLocalPos;
			const float flDist = vDelta.Length2D();

			const float flRoot = pow(tInfo.m_flVelocity, 4) - flGrav * (flGrav * pow(flDist, 2) + 2.f * vDelta.z * pow(tInfo.m_flVelocity, 2));
			if (flRoot < 0.f)
				return vPoint;
			float flPitch = atan((pow(tInfo.m_flVelocity, 2) - sqrt(flRoot)) / (flGrav * flDist));

			float flTime = flDist / (cos(flPitch) * tInfo.m_flVelocity) - tInfo.m_flOffsetTime;
			return vLocalPos + Vec3(0, 0, (flGrav * pow(flTime, 2)) / 2);
		};

	vLocalPos = HeightenLocalPos();
	Vec3 vForward, vRight, vUp; Math::AngleVectors(Math::CalcAngle(vLocalPos, vPoint), &vForward, &vRight, &vUp);
	vLocalPos += (vForward * tInfo.m_vOffset.x) + (vRight * tInfo.m_vOffset.y) + (vUp * tInfo.m_vOffset.z);
	return vLocalPos + (vPoint - vLocalPos) * fabsf(AABBLine(vMins + vTargetPos, vMaxs + vTargetPos, vLocalPos, vPoint - vLocalPos));
}



static inline void SolveProjectileSpeed(CTFWeaponBase* pWeapon, const Vec3& vLocalPos, const Vec3& vTargetPos, float& flVelocity, float& flDragTime, const float flGravity)
{
	if (!F::ProjSim.m_pObj->IsDragEnabled() || F::ProjSim.m_pObj->m_dragBasis.IsZero())
		return;

	const float flGrav = flGravity * 800.0f;
	const Vec3 vDelta = vTargetPos - vLocalPos;
	const float flDist = vDelta.Length2D();

	const float flRoot = pow(flVelocity, 4) - flGrav * (flGrav * pow(flDist, 2) + 2.f * vDelta.z * pow(flVelocity, 2));
	if (flRoot < 0.f)
		return;

	const float flPitch = atan((pow(flVelocity, 2) - sqrt(flRoot)) / (flGrav * flDist));
	const float flTime = flDist / (cos(flPitch) * flVelocity);

	float flDrag = 0.f;
	if (Vars::Aimbot::Projectile::DragOverride.Value)
		flDrag = Vars::Aimbot::Projectile::DragOverride.Value;
	else
	{
		switch (pWeapon->m_iItemDefinitionIndex()) // the remaps are dumb but they work so /shrug
		{
		case Demoman_m_GrenadeLauncher:
		case Demoman_m_GrenadeLauncherR:
		case Demoman_m_FestiveGrenadeLauncher:
		case Demoman_m_Autumn:
		case Demoman_m_MacabreWeb:
		case Demoman_m_Rainbow:
		case Demoman_m_SweetDreams:
		case Demoman_m_CoffinNail:
		case Demoman_m_TopShelf:
		case Demoman_m_Warhawk:
		case Demoman_m_ButcherBird:
		case Demoman_m_TheIronBomber: flDrag = Math::RemapVal(flVelocity, 1217.f, k_flMaxVelocity, 0.120f, 0.200f); break; // 0.120 normal, 0.200 capped, 0.300 v3000
		case Demoman_m_TheLochnLoad: flDrag = Math::RemapVal(flVelocity, 1504.f, k_flMaxVelocity, 0.070f, 0.085f); break; // 0.070 normal, 0.085 capped, 0.120 v3000
		case Demoman_m_TheLooseCannon: flDrag = Math::RemapVal(flVelocity, 1454.f, k_flMaxVelocity, 0.385f, 0.530f); break; // 0.385 normal, 0.530 capped, 0.790 v3000
		case Demoman_s_StickybombLauncher:
		case Demoman_s_StickybombLauncherR:
		case Demoman_s_FestiveStickybombLauncher:
		case Demoman_s_TheQuickiebombLauncher:
		case Demoman_s_TheScottishResistance: flDrag = Math::RemapVal(flVelocity, 922.f, k_flMaxVelocity, 0.085f, 0.190f); break; // 0.085 low, 0.190 capped, 0.230 v2400
		case Scout_s_TheFlyingGuillotine:
		case Scout_s_TheFlyingGuillotineG: flDrag = 0.310f; break;
		case Scout_t_TheSandman: flDrag = 0.180f; break;
		case Scout_t_TheWrapAssassin: flDrag = 0.285f; break;
		case Scout_s_MadMilk:
		case Scout_s_MutatedMilk:
		case Sniper_s_Jarate:
		case Sniper_s_FestiveJarate:
		case Sniper_s_TheSelfAwareBeautyMark: flDrag = 0.057f; break;
		}
	}

	float flOverride = Vars::Aimbot::Projectile::TimeOverride.Value;
	flDragTime = powf(flTime, 2) * flDrag / (flOverride ? flOverride : 1.5f); // rough estimate to prevent m_flTime being too low
	flVelocity = flVelocity - flVelocity * flTime * flDrag;
}
void CAimbotProjectile::CalculateAngle(const Vec3& vLocalPos, const Vec3& vTargetPos, int iSimTime, Solution_t& tOut, bool bAccuracy, int iTolerance)
{
	if (tOut.m_iCalculated != CalculatedEnum::Pending)
		return;

	const float flGrav = m_tInfo.m_flGravity * 800.f;

	float flPitch, flYaw;
	{	// basic trajectory pass
		float flVelocity = m_tInfo.m_flVelocity, flDragTime = 0.f;
		if (F::ProjSim.m_pObj->IsDragEnabled() && !F::ProjSim.m_pObj->m_dragBasis.IsZero() && m_tInfo.m_pWeapon)
		{
			Vec3 vForward, vRight, vUp; Math::AngleVectors(Math::CalcAngle(vLocalPos, vTargetPos), &vForward, &vRight, &vUp);
			Vec3 vShootPos = vLocalPos + (vForward * m_tInfo.m_vOffset.x) + (vRight * m_tInfo.m_vOffset.y) + (vUp * m_tInfo.m_vOffset.z);
			SolveProjectileSpeed(m_tInfo.m_pWeapon, vShootPos, vTargetPos, flVelocity, flDragTime, m_tInfo.m_flGravity);
		}

		Vec3 vDelta = vTargetPos - vLocalPos;
		float flDist = vDelta.Length2D();

		Vec3 vAngleTo = Math::CalcAngle(vLocalPos, vTargetPos);
		if (!flGrav)
			flPitch = -DEG2RAD(vAngleTo.x);
		else
		{	// arch
			float flRoot = pow(flVelocity, 4) - flGrav * (flGrav * pow(flDist, 2) + 2.f * vDelta.z * pow(flVelocity, 2));
			if (tOut.m_iCalculated = flRoot < 0.f ? CalculatedEnum::Bad : CalculatedEnum::Pending)
				return;
			flPitch = atan((pow(flVelocity, 2) - sqrt(flRoot)) / (flGrav * flDist));
		}
		tOut.m_flTime = flDist / (cos(flPitch) * flVelocity) - m_tInfo.m_flOffsetTime + flDragTime;
		tOut.m_flPitch = flPitch = -RAD2DEG(flPitch) - m_tInfo.m_vAngFix.x;
		tOut.m_flYaw = flYaw = vAngleTo.y - m_tInfo.m_vAngFix.y;
	}

	int iTimeTo = ceilf(tOut.m_flTime / TICK_INTERVAL);
	bool bGood = iTolerance != -1 ? abs(iTimeTo - iSimTime) <= iTolerance : iTimeTo <= iSimTime;
	if (!m_tInfo.m_vOffset.IsZero())
	{
		if (tOut.m_iCalculated = bGood ? CalculatedEnum::Pending : CalculatedEnum::Time)
			return;
	}
	else
	{
		tOut.m_iCalculated = bGood ? CalculatedEnum::Pending : CalculatedEnum::Time;
		return;
	}

	int iFlags = (bAccuracy ? ProjSimEnum::Redirect : ProjSimEnum::None) | ProjSimEnum::NoRandomAngles | ProjSimEnum::PredictCmdNum;
#ifdef SPLASH_DEBUG5
	if (iFlags & ProjSimEnum::Redirect)
	{
		if (Vars::Visuals::Trajectory::Override.Value)
		{
			if (!Vars::Visuals::Trajectory::Pipes.Value)
				s_mTraceCount["Setup trace calculate"]++;
		}
		else
		{
			switch (m_tInfo.m_pWeapon->GetWeaponID())
			{
			case TF_WEAPON_ROCKETLAUNCHER:
			case TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT:
			case TF_WEAPON_PARTICLE_CANNON:
			case TF_WEAPON_RAYGUN:
			case TF_WEAPON_DRG_POMSON:
			case TF_WEAPON_FLAREGUN:
			case TF_WEAPON_FLAREGUN_REVENGE:
			case TF_WEAPON_COMPOUND_BOW:
			case TF_WEAPON_CROSSBOW:
			case TF_WEAPON_SHOTGUN_BUILDING_RESCUE:
			case TF_WEAPON_SYRINGEGUN_MEDIC:
			case TF_WEAPON_FLAME_BALL:
				s_mTraceCount["Setup trace calculate"]++;
			}
		}
	}
#endif
	m_tProjInfo = {};
	if (tOut.m_iCalculated = !F::ProjSim.GetInfo(m_tInfo.m_pLocal, m_tInfo.m_pWeapon, { flPitch, flYaw, 0 }, m_tProjInfo, iFlags) ? CalculatedEnum::Bad : CalculatedEnum::Pending)
		return;

	{	// calculate trajectory from projectile origin
		float flVelocity = m_tInfo.m_flVelocity, flDragTime = 0.f;
		SolveProjectileSpeed(m_tInfo.m_pWeapon, m_tProjInfo.m_vPos, vTargetPos, flVelocity, flDragTime, m_tInfo.m_flGravity);

		Vec3 vDelta = vTargetPos - m_tProjInfo.m_vPos;
		float flDist = vDelta.Length2D();

		Vec3 vAngleTo = Math::CalcAngle(m_tProjInfo.m_vPos, vTargetPos);
		if (!flGrav)
			tOut.m_flPitch = -DEG2RAD(vAngleTo.x);
		else
		{	// arch
			float flRoot = pow(flVelocity, 4) - flGrav * (flGrav * pow(flDist, 2) + 2.f * vDelta.z * pow(flVelocity, 2));
			if (tOut.m_iCalculated = flRoot < 0.f ? CalculatedEnum::Bad : CalculatedEnum::Pending)
				return;
			tOut.m_flPitch = atan((pow(flVelocity, 2) - sqrt(flRoot)) / (flGrav * flDist));
		}
		tOut.m_flTime = flDist / (cos(tOut.m_flPitch) * flVelocity) + flDragTime;
	}

	{	// correct yaw
		Vec3 vShootPos = (m_tProjInfo.m_vPos - vLocalPos).To2D();
		Vec3 vTarget = vTargetPos - vLocalPos;
		Vec3 vForward; Math::AngleVectors(m_tProjInfo.m_vAng, &vForward); vForward.Normalize2D();
		float flB = 2 * (vShootPos.x * vForward.x + vShootPos.y * vForward.y);
		float flC = vShootPos.Length2DSqr() - vTarget.Length2DSqr();
		auto vSolutions = Math::SolveQuadratic(1.f, flB, flC);
		if (!vSolutions.empty())
		{
			vShootPos += vForward * vSolutions.front();
			tOut.m_flYaw = flYaw - (RAD2DEG(atan2(vShootPos.y, vShootPos.x)) - flYaw);
			flYaw = RAD2DEG(atan2(vShootPos.y, vShootPos.x));
		}
	}

	{	// correct pitch
		if (flGrav)
		{
			flPitch -= m_tProjInfo.m_vAng.x;
			tOut.m_flPitch = -RAD2DEG(tOut.m_flPitch) + flPitch - m_tInfo.m_vAngFix.x;
		}
		else
		{
			Vec3 vShootPos = Math::RotatePoint(m_tProjInfo.m_vPos - vLocalPos, {}, { 0, -flYaw, 0 }); vShootPos.y = 0;
			Vec3 vTarget = Math::RotatePoint(vTargetPos - vLocalPos, {}, { 0, -flYaw, 0 });
			Vec3 vForward; Math::AngleVectors(m_tProjInfo.m_vAng - Vec3(0, flYaw, 0), &vForward); vForward.y = 0; vForward.Normalize();
			float flB = 2 * (vShootPos.x * vForward.x + vShootPos.z * vForward.z);
			float flC = (powf(vShootPos.x, 2) + powf(vShootPos.z, 2)) - (powf(vTarget.x, 2) + powf(vTarget.z, 2));
			auto vSolutions = Math::SolveQuadratic(1.f, flB, flC);
			if (!vSolutions.empty())
			{
				vShootPos += vForward * vSolutions.front();
				tOut.m_flPitch = flPitch - (RAD2DEG(atan2(-vShootPos.z, vShootPos.x)) - flPitch);
			}
		}
	}

	iTimeTo = ceilf(tOut.m_flTime / TICK_INTERVAL);
	bGood = iTolerance != -1 ? abs(iTimeTo - iSimTime) <= iTolerance : iTimeTo <= iSimTime;
	tOut.m_iCalculated = bGood ? CalculatedEnum::Good : CalculatedEnum::Time;
}



bool CAimbotProjectile::TestAngle(const Vec3& vPoint, const Vec3& vAngles, int iSimTime, bool bSplash, bool bSecondTest)
{
	auto pLocal = m_tInfo.m_pLocal;
	auto pWeapon = m_tInfo.m_pWeapon;
	auto& tTarget = *m_tInfo.m_pTarget;

	int iFlags = ProjSimEnum::Redirect | ProjSimEnum::InitCheck | ProjSimEnum::NoRandomAngles | ProjSimEnum::PredictCmdNum;
#ifdef SPLASH_DEBUG5
	if (Vars::Visuals::Trajectory::Override.Value)
	{
		if (!Vars::Visuals::Trajectory::Pipes.Value)
			s_mTraceCount["Setup trace test"]++;
	}
	else
	{
		switch (pWeapon->GetWeaponID())
		{
		case TF_WEAPON_ROCKETLAUNCHER:
		case TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT:
		case TF_WEAPON_PARTICLE_CANNON:
		case TF_WEAPON_RAYGUN:
		case TF_WEAPON_DRG_POMSON:
		case TF_WEAPON_FLAREGUN:
		case TF_WEAPON_FLAREGUN_REVENGE:
		case TF_WEAPON_COMPOUND_BOW:
		case TF_WEAPON_CROSSBOW:
		case TF_WEAPON_SHOTGUN_BUILDING_RESCUE:
		case TF_WEAPON_SYRINGEGUN_MEDIC:
		case TF_WEAPON_FLAME_BALL:
			s_mTraceCount["Setup trace"]++;
		}
	}
	s_mTraceCount["Trace init check"]++;
#endif
	m_tProjInfo = {};
	if (!F::ProjSim.GetInfo(pLocal, pWeapon, vAngles, m_tProjInfo, iFlags)
		|| !F::ProjSim.Initialize(m_tProjInfo))
		return false;

	CGameTrace trace = {};
	CTraceFilterCollideable filter = {};
	filter.pSkip = bSplash ? tTarget.m_pEntity : pLocal;
	filter.iPlayer = bSplash ? PLAYER_NONE : PLAYER_DEFAULT;
	filter.bMisc = !bSplash;
	int nMask = MASK_SOLID;
	if (!bSplash && F::AimbotGlobal.FriendlyFire())
	{
		switch (pWeapon->GetWeaponID())
		{	// only weapons that actually hit teammates properly
		case TF_WEAPON_ROCKETLAUNCHER:
		case TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT:
		case TF_WEAPON_PARTICLE_CANNON:
		case TF_WEAPON_DRG_POMSON:
		case TF_WEAPON_FLAREGUN:
		case TF_WEAPON_SYRINGEGUN_MEDIC:
			filter.iPlayer = PLAYER_ALL;
		}
	}
	F::ProjSim.SetupTrace(filter, nMask, pWeapon);

#ifdef SPLASH_DEBUG4
	Vec3 vHull = m_tProjInfo.m_vHull.Max(1);
	G::BoxStorage.emplace_back(vPoint, vHull * -1, vHull, Vec3(), I::GlobalVars->curtime + 5.f, Color_t(255, 0, 0), Color_t(0, 0, 0, 0));
#endif

	if (!m_tProjInfo.m_flGravity)
	{
		SDK::TraceHull(m_tProjInfo.m_vPos, vPoint, m_tProjInfo.m_vHull * -1, m_tProjInfo.m_vHull, nMask, &filter, &trace);
#ifdef SPLASH_DEBUG5
		s_mTraceCount["Nograv trace"]++;
#endif
#ifdef SPLASH_DEBUG4
		G::LineStorage.emplace_back(std::pair<Vec3, Vec3>(m_tProjInfo.m_vPos, trace.endpos), I::GlobalVars->curtime + 5.f, Color_t(0, 0, 0));
#endif
		if (trace.fraction < 0.999f && trace.m_pEnt != tTarget.m_pEntity)
			return false;
	}

	bool bDidHit = false, bArmTime = false;
	const RestoreInfo_t tOriginal = { tTarget.m_pEntity->GetAbsOrigin(), tTarget.m_pEntity->m_vecMins(), tTarget.m_pEntity->m_vecMaxs() };
	tTarget.m_pEntity->SetAbsOrigin(tTarget.m_vPos);
	tTarget.m_pEntity->m_vecMins() = { std::clamp(tTarget.m_pEntity->m_vecMins().x, -24.f, 0.f), std::clamp(tTarget.m_pEntity->m_vecMins().y, -24.f, 0.f), tTarget.m_pEntity->m_vecMins().z };
	tTarget.m_pEntity->m_vecMaxs() = { std::clamp(tTarget.m_pEntity->m_vecMaxs().x, 0.f, 24.f), std::clamp(tTarget.m_pEntity->m_vecMaxs().y, 0.f, 24.f), tTarget.m_pEntity->m_vecMaxs().z };
	for (int n = 1; n <= iSimTime; n++)
	{
		Vec3 vOld = F::ProjSim.GetOrigin();
		F::ProjSim.RunTick(m_tProjInfo);
		Vec3 vNew = F::ProjSim.GetOrigin();

		if (bDidHit)
		{
			trace.endpos = vNew;
			continue;
		}

		if (!bSplash)
		{
			SDK::TraceHull(vOld, vNew, m_tProjInfo.m_vHull * -1, m_tProjInfo.m_vHull, nMask, &filter, &trace);
#ifdef SPLASH_DEBUG5
			s_mTraceCount["Direct trace"]++;
#endif
#ifdef SPLASH_DEBUG4
			G::LineStorage.emplace_back(std::pair<Vec3, Vec3>(vOld, trace.endpos), I::GlobalVars->curtime + 5.f, Color_t(255, 0, 0));
#endif
		}
		else
		{
			static Vec3 vStaticPos = {};
			if (n == 1 || bArmTime)
				vStaticPos = vOld;
			if (n % Vars::Aimbot::Projectile::SplashTraceInterval.Value && n != iSimTime && !bArmTime)
				continue;

			SDK::TraceHull(vStaticPos, vNew, m_tProjInfo.m_vHull * -1, m_tProjInfo.m_vHull, nMask, &filter, &trace);
#ifdef SPLASH_DEBUG5
			s_mTraceCount["Splash trace"]++;
#endif
#ifdef SPLASH_DEBUG4
			G::LineStorage.emplace_back(std::pair<Vec3, Vec3>(vStaticPos, trace.endpos), I::GlobalVars->curtime + 5.f, Color_t(255, 0, 0));
#endif
			vStaticPos = vNew;
		}
		if (trace.DidHit())
		{
			bool bTime = bSplash
				? trace.endpos.DistTo(vPoint) < m_tProjInfo.m_flVelocity * TICK_INTERVAL + m_tProjInfo.m_vHull.z
				: iSimTime - n < 5 || pWeapon->GetWeaponID() == TF_WEAPON_LUNCHBOX; // projectile so slow it causes problems if we don't waive this check
			bool bTarget = trace.m_pEnt == tTarget.m_pEntity || bSplash;
			bool bValid = bTarget && bTime;
			if (bValid && bSplash)
			{
				bValid = (trace.endpos - vPoint).IsZero(0.01f) || SDK::VisPosWorld(nullptr, tTarget.m_pEntity, trace.endpos, vPoint, nMask);
#ifdef SPLASH_DEBUG5
				s_mTraceCount["Splash vispos"]++;
#endif
#ifdef SPLASH_DEBUG4
				G::LineStorage.emplace_back(std::pair<Vec3, Vec3>(trace.endpos, vPoint), I::GlobalVars->curtime + 5.f, Color_t(0, 0, 255));
#endif
				if (bValid)
				{
					Vec3 vFrom = trace.endpos;
					switch (pWeapon->GetWeaponID())
					{
					case TF_WEAPON_ROCKETLAUNCHER:
					case TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT:
					case TF_WEAPON_PARTICLE_CANNON:
						vFrom += trace.plane.normal;
					}

					CGameTrace eyeTrace = {};
					SDK::Trace(vFrom, tTarget.m_vPos + m_tInfo.m_vTargetEye, MASK_SHOT, &filter, &eyeTrace);
					bValid = eyeTrace.fraction == 1.f;
#ifdef SPLASH_DEBUG5
					s_mTraceCount["Eye trace"]++;
#endif
#ifdef SPLASH_DEBUG4
					G::LineStorage.emplace_back(std::pair<Vec3, Vec3>(vFrom, trace.endpos), I::GlobalVars->curtime + 5.f, Color_t(255, 0, 255));
#endif
				}
			}

#ifdef SPLASH_DEBUG4
			G::BoxStorage.pop_back();
			if (bValid)
				G::BoxStorage.emplace_back(vPoint, vHull * -1, vHull, Vec3(), I::GlobalVars->curtime + 5.f, Color_t(0, 255, 0), Color_t(0, 0, 0, 0));
			else if (bTime)
				G::BoxStorage.emplace_back(vPoint, vHull * -1, vHull, Vec3(), I::GlobalVars->curtime + 5.f, Color_t(0, 0, 255), Color_t(0, 0, 0, 0));
			else
			{
				G::BoxStorage.emplace_back(vPoint, vHull * -1, vHull, Vec3(), I::GlobalVars->curtime + 5.f, Color_t(255, 0, 255), Color_t(0, 0, 0, 0));
				if (bSplash)
				{
					G::BoxStorage.emplace_back(trace.endpos, Vec3(-1, -1, -1), Vec3(1, 1, 1), Vec3(), I::GlobalVars->curtime + 5.f, Color_t(0, 0, 0), Color_t(0, 0, 0, 0));
					G::BoxStorage.emplace_back(vPoint, Vec3(-1, -1, -1), Vec3(1, 1, 1), Vec3(), I::GlobalVars->curtime + 5.f, Color_t(255, 255, 255), Color_t(0, 0, 0, 0));
				}
			}
#endif

			if (bValid)
			{
				if (bSplash)
				{
					int iPopCount = Vars::Aimbot::Projectile::SplashTraceInterval.Value - trace.fraction * Vars::Aimbot::Projectile::SplashTraceInterval.Value;
					for (int i = 0; i < iPopCount && !m_tProjInfo.m_vPath.empty(); i++)
						m_tProjInfo.m_vPath.pop_back();
				}

				// attempted to have a headshot check though this seems more detrimental than useful outside of smooth aimbot
				if (tTarget.m_nAimedHitbox == HITBOX_HEAD && !bSecondTest &&
					(Vars::Aimbot::General::AimType.Value == Vars::Aimbot::General::AimTypeEnum::Smooth
						|| Vars::Aimbot::General::AimType.Value == Vars::Aimbot::General::AimTypeEnum::Assistive))
				{	// loop and see if closest hitbox is head
					auto pModel = tTarget.m_pEntity->GetModel();
					if (!pModel) break;
					auto pHDR = I::ModelInfoClient->GetStudiomodel(pModel);
					if (!pHDR) break;
					auto pSet = pHDR->pHitboxSet(tTarget.m_pEntity->As<CTFPlayer>()->m_nHitboxSet());
					if (!pSet) break;

					auto aBones = F::Backtrack.GetBones(tTarget.m_pEntity);
					if (!aBones)
						break;

					Vec3 vOffset = tOriginal.m_vOrigin - tTarget.m_vPos;
					Vec3 vPos = trace.endpos + F::ProjSim.GetVelocity().Normalized() * 16 + vOffset;

					float flClosest = 0.f; int iClosest = -1;
					for (int nHitbox = 0; nHitbox < pSet->numhitboxes; ++nHitbox)
					{
						auto pBox = pSet->pHitbox(nHitbox);
						if (!pBox) continue;

						Vec3 vCenter; Math::VectorTransform({}, aBones[pBox->bone], vCenter);

						const float flDist = vPos.DistTo(vCenter);
						if (iClosest != -1 && flDist < flClosest || iClosest == -1)
						{
							flClosest = flDist;
							iClosest = nHitbox;
						}
					}
					if (iClosest != HITBOX_HEAD)
						break;
				}

				bDidHit = true;
			}
			else if (!bSplash && bTarget && pWeapon->GetWeaponID() == TF_WEAPON_PIPEBOMBLAUNCHER)
			{	// run for more ticks to check for splash
				iSimTime = n + 5;
				bSplash = bArmTime = true;
			}
			else
				break;

			if (!bSplash)
				trace.endpos = vNew;

			if (!bTarget || bSplash && !bArmTime)
				break;
		}
	}
	tTarget.m_pEntity->SetAbsOrigin(tOriginal.m_vOrigin);
	tTarget.m_pEntity->m_vecMins() = tOriginal.m_vMins;
	tTarget.m_pEntity->m_vecMaxs() = tOriginal.m_vMaxs;
	m_tProjInfo.m_vPath.push_back(trace.endpos);

	return bDidHit;
}

bool CAimbotProjectile::HandlePoint(const Vec3& vOrigin, int iSimTime, float flPitch, float flYaw, float flTime, const Vec3& vPoint, bool bSplash)
{
	bool bReturn = false;

	Vec3 vAngles; Aim(G::CurrentUserCmd->viewangles, { flPitch, flYaw, 0.f }, vAngles);

	const Vec3 vOriginal = m_tInfo.m_pTarget->m_vPos;
	m_tInfo.m_pTarget->m_vPos = vOrigin;
	if (!m_tInfo.m_pProjectile
		? TestAngle(vPoint, vAngles, iSimTime, bSplash)
		: TestAngle(m_tInfo.m_pProjectile, vPoint, vAngles, iSimTime, bSplash))
	{
		bReturn = m_iResult = true;
		m_flTimeTo = flTime + m_tInfo.m_flLatency;
	}
	else if (m_iResult != 1 && !m_tInfo.m_pProjectile)
	{
		switch (Vars::Aimbot::General::AimType.Value)
		{
		case Vars::Aimbot::General::AimTypeEnum::Smooth:
			if (Vars::Aimbot::General::AssistStrength.Value == 100.f)
				break;
			[[fallthrough]];
		case Vars::Aimbot::General::AimTypeEnum::Assistive:
		{
			Vec3 vPlainAngles = { flPitch, flYaw, 0.f };
			if (TestAngle(vPoint, vPlainAngles, iSimTime, bSplash, true))
				bReturn = m_iResult = 2;
		}
		}
	}
	m_tInfo.m_pTarget->m_vPos = vOriginal;

	if (bReturn)
	{
		m_vAngleTo = vAngles, m_vPredicted = vOrigin, m_vTarget = vPoint;
		if (m_bVisuals)
		{
			m_vPlayerPath.clear();
			int iCount = std::min(iSimTime + TIME_TO_TICKS(m_tInfo.m_flLatency) + 1, int(m_tMoveStorage.m_vPath.size()));
			for (int i = 0; i < iCount; i++)
				m_vPlayerPath.push_back(m_tMoveStorage.m_vPath[i]);
			m_vProjectilePath = m_tProjInfo.m_vPath;
		}
	}

	return bReturn;
}

bool CAimbotProjectile::HandleDirect(std::vector<Direct_t>& vDirectHistory)
{
	if (vDirectHistory.empty())
		return false;

	std::sort(vDirectHistory.begin(), vDirectHistory.end(), [&](const Direct_t& a, const Direct_t& b) -> bool
		{
			return a.m_iPriority < b.m_iPriority;
		});
	m_flTimeTo = vDirectHistory.front().m_flTime + m_tInfo.m_flLatency;

	for (auto& tHistory : vDirectHistory)
	{
		if (HandlePoint(tHistory.m_vOrigin, tHistory.m_iSimtime, tHistory.m_flPitch, tHistory.m_flYaw, tHistory.m_flTime, tHistory.m_vPoint))
			return true;
	}

	return false;
}

bool CAimbotProjectile::HandleSplash(std::vector<Splash_t>& vSplashHistory)
{
	if (vSplashHistory.empty())
		return false;

	std::sort(vSplashHistory.begin(), vSplashHistory.end(), [&](const Splash_t& a, const Splash_t& b) -> bool
		{
			return a.m_flTimeTo < b.m_flTimeTo;
		});

	float flSize = m_tInfo.m_pTarget->m_pEntity->GetSize().Length();
	int iPoints = !m_tInfo.m_flGravity ? Vars::Aimbot::Projectile::SplashPointsDirect.Value : Vars::Aimbot::Projectile::SplashPointsArc.Value;
	auto vSpherePoints = ComputeSphere(m_tInfo.m_flRadius + flSize, iPoints);
#ifdef SPLASH_DEBUG3
	for (auto& [vPoint, _] : vSpherePoints)
		G::BoxStorage.emplace_back(vSplashHistory.front().m_vOrigin + m_tInfo.m_vTargetEye + vPoint, Vec3(-1, -1, -1), Vec3(1, 1, 1), Vec3(), I::GlobalVars->curtime + 60.f, Color_t(0, 0, 0, 0), Vars::Colors::Local.Value);
#endif

	int iMulti = Vars::Aimbot::Projectile::SplashMode.Value;
	static std::vector<Vec3> vSimplePoints = {};
	if (iMulti == Vars::Aimbot::Projectile::SplashModeEnum::Single)
	{
		SetupSplashPoints(vSplashHistory.front().m_vOrigin, vSpherePoints, vSimplePoints);
		if (vSimplePoints.empty())
			return false;
	}

	float flLowestDistance = std::numeric_limits<float>::max();
	for (auto& tHistory : vSplashHistory)
	{
		if (m_tInfo.m_iArmTime && tHistory.m_iSimtime < m_tInfo.m_iArmTime)
			continue;

		std::vector<Point_t> vSplashPoints = {};
		if (iMulti == Vars::Aimbot::Projectile::SplashModeEnum::Multi)
			vSplashPoints = GetSplashPoints(tHistory.m_vOrigin, vSpherePoints, tHistory.m_iSimtime);
		else
			vSplashPoints = GetSplashPointsSimple(tHistory.m_vOrigin, vSimplePoints, tHistory.m_iSimtime);

		for (auto& tPoint : vSplashPoints)
		{
			float flDistance = tHistory.m_vOrigin.DistTo(tPoint.m_vPoint);
			if (flDistance > flLowestDistance)
				continue;

			if (HandlePoint(tHistory.m_vOrigin, tHistory.m_iSimtime, tPoint.m_tSolution.m_flPitch, tPoint.m_tSolution.m_flYaw, tPoint.m_tSolution.m_flTime, tPoint.m_vPoint, true))
				flLowestDistance = flDistance;
		}
	}

	return flLowestDistance != std::numeric_limits<float>::max();
}

int CAimbotProjectile::CanHit(Target_t& tTarget, CTFPlayer* pLocal, CTFWeaponBase* pWeapon, bool bVisuals)
{
	//if (Vars::Aimbot::General::Ignore.Value & Vars::Aimbot::General::IgnoreEnum::Unsimulated && H::Entities.GetChoke(tTarget.m_pEntity->entindex()) > Vars::Aimbot::General::TickTolerance.Value)
	//	return false;

	m_tProjInfo = {};
	if (!F::ProjSim.GetInfo(pLocal, pWeapon, {}, m_tProjInfo, ProjSimEnum::NoRandomAngles | ProjSimEnum::PredictCmdNum)
		|| !F::ProjSim.Initialize(m_tProjInfo, false))
		return false;

	m_tMoveStorage = {};
	F::MoveSim.Initialize(tTarget.m_pEntity, m_tMoveStorage);
	tTarget.m_vPos = tTarget.m_pEntity->m_vecOrigin();

	m_tInfo = { pLocal, pWeapon, &tTarget };
	m_tInfo.m_vLocalEye = pLocal->GetShootPos();
	m_tInfo.m_vTargetEye = tTarget.m_pEntity->As<CTFPlayer>()->GetViewOffset();
	m_tInfo.m_flLatency = F::Backtrack.GetReal() + TICKS_TO_TIME(F::Backtrack.GetAnticipatedChoke());

	Vec3 vVelocity = F::ProjSim.GetVelocity();
	m_tInfo.m_flVelocity = vVelocity.Length();
	m_tInfo.m_vAngFix = Math::VectorAngles(vVelocity);

	m_tInfo.m_vHull = m_tProjInfo.m_vHull.Min(3);
	m_tInfo.m_vOffset = m_tProjInfo.m_vPos - m_tInfo.m_vLocalEye; m_tInfo.m_vOffset.y *= -1;
	m_tInfo.m_flOffsetTime = m_tInfo.m_vOffset.Length() / m_tInfo.m_flVelocity; // silly

	float flSize = tTarget.m_pEntity->GetSize().Length();
	m_tInfo.m_flGravity = m_tProjInfo.m_flGravity;
	m_tInfo.m_iSplashCount = !m_tInfo.m_flGravity ? Vars::Aimbot::Projectile::SplashCountDirect.Value : Vars::Aimbot::Projectile::SplashCountArc.Value;
	m_tInfo.m_flRadius = GetSplashRadius(pWeapon, pLocal);
	m_tInfo.m_flRadiusTime = m_tInfo.m_flRadius / m_tInfo.m_flVelocity;
	m_tInfo.m_flBoundingTime = m_tInfo.m_flRadiusTime + flSize / m_tInfo.m_flVelocity;

	m_tInfo.m_iSplashMode = GetSplashMode(pWeapon);
	m_tInfo.m_iArmTime = TIME_TO_TICKS(ArmTime(pWeapon));



	int iMaxTime = TIME_TO_TICKS(std::min(m_tProjInfo.m_flLifetime, Vars::Aimbot::Projectile::MaxSimulationTime.Value));
	int iSplash = Vars::Aimbot::Projectile::SplashPrediction.Value && m_tInfo.m_flRadius ? Vars::Aimbot::Projectile::SplashPrediction.Value : Vars::Aimbot::Projectile::SplashPredictionEnum::Off;

	auto mDirectPoints = iSplash == Vars::Aimbot::Projectile::SplashPredictionEnum::Only ? std::unordered_map<int, Vec3>() : GetDirectPoints();

	std::vector<Direct_t> vDirectHistory = {};
	std::vector<Splash_t> vSplashHistory = {};

	for (int i = 1 - TIME_TO_TICKS(m_tInfo.m_flLatency); i <= iMaxTime; i++)
	{
		if (!m_tMoveStorage.m_bFailed)
		{
			F::MoveSim.RunTick(m_tMoveStorage);
			tTarget.m_vPos = m_tMoveStorage.m_vPredictedOrigin;
		}
		if (i < 0)
			continue;

		bool bDirectBreaks = true;
		if (iSplash)
		{
			Solution_t tSolution; CalculateAngle(m_tInfo.m_vLocalEye, tTarget.m_vPos, i, tSolution, false);
			if (tSolution.m_iCalculated != CalculatedEnum::Bad)
			{
				const float flTimeTo = tSolution.m_flTime - TICKS_TO_TIME(i);

				bDirectBreaks = false;
				if (flTimeTo < m_tInfo.m_flBoundingTime)
				{
					bDirectBreaks = flTimeTo < -m_tInfo.m_flBoundingTime && (!m_tInfo.m_iArmTime || m_tInfo.m_iArmTime < i);
					if (!bDirectBreaks)
						vSplashHistory.emplace_back(History_t(tTarget.m_vPos, i), fabsf(flTimeTo));
				}
			}
		}

		for (auto& [iIndex, vOffset] : mDirectPoints)
		{
			Vec3 vPoint = tTarget.m_vPos + vOffset;
			if (Vars::Aimbot::Projectile::HuntsmanPullPoint.Value && tTarget.m_nAimedHitbox == HITBOX_HEAD)
			{
				vPoint = PullPoint(vPoint, m_tInfo.m_vLocalEye, m_tInfo, tTarget.m_pEntity->m_vecMins() + m_tInfo.m_vHull, tTarget.m_pEntity->m_vecMaxs() - m_tInfo.m_vHull, tTarget.m_vPos);
				if (Vars::Aimbot::Projectile::HuntsmanPullNoZ.Value)
					vPoint.z = tTarget.m_vPos.z + vOffset.z;
			}

			Solution_t tSolution; CalculateAngle(m_tInfo.m_vLocalEye, vPoint, i, tSolution);
			switch (tSolution.m_iCalculated)
			{
			case CalculatedEnum::Good:
				if (m_tInfo.m_iArmTime && m_tInfo.m_iArmTime > i && !m_tMoveStorage.m_MoveData.m_vecVelocity.IsZero())
					break;
				vDirectHistory.emplace_back(History_t(tTarget.m_vPos, i), tSolution.m_flPitch, tSolution.m_flYaw, tSolution.m_flTime, vPoint, iIndex);
				[[fallthrough]];
			case CalculatedEnum::Bad:
				mDirectPoints.erase(iIndex);
			}
		}
		if (bDirectBreaks && mDirectPoints.empty())
			break;
	}

	m_iResult = false;
	switch (iSplash)
	{
	case Vars::Aimbot::Projectile::SplashPredictionEnum::Off:
		HandleDirect(vDirectHistory);
		break;
	case Vars::Aimbot::Projectile::SplashPredictionEnum::Include:
		if (!HandleDirect(vDirectHistory))
			HandleSplash(vSplashHistory);
		break;
	case Vars::Aimbot::Projectile::SplashPredictionEnum::Prefer:
		if (!HandleSplash(vSplashHistory))
			HandleDirect(vDirectHistory);
		break;
	case Vars::Aimbot::Projectile::SplashPredictionEnum::Only:
		HandleSplash(vSplashHistory);
	}
	F::MoveSim.Restore(m_tMoveStorage);

	tTarget.m_vPos = m_vTarget;
	tTarget.m_vAngleTo = m_vAngleTo;
	if (tTarget.m_iTargetType != TargetEnum::Player || !m_tMoveStorage.m_bFailed) // don't attempt to aim at players when movesim fails
	{
		bool bMain = m_iResult == 1;
		bool bAny = m_iResult;

		if (bVisuals && bAny && (Vars::Colors::BoundHitboxEdge.Value.a || Vars::Colors::BoundHitboxFace.Value.a || Vars::Colors::BoundHitboxEdgeIgnoreZ.Value.a || Vars::Colors::BoundHitboxFaceIgnoreZ.Value.a))
		{
			m_tInfo.m_vHull = m_tInfo.m_vHull.Max(1);
			float flProjectileTime = TICKS_TO_TIME(m_vProjectilePath.size());
			float flTargetTime = m_tMoveStorage.m_bFailed ? flProjectileTime : TICKS_TO_TIME(m_vPlayerPath.size());

			bool bBox = Vars::Visuals::Hitbox::BoundsEnabled.Value & Vars::Visuals::Hitbox::BoundsEnabledEnum::OnShot;
			bool bPoint = Vars::Visuals::Hitbox::BoundsEnabled.Value & Vars::Visuals::Hitbox::BoundsEnabledEnum::AimPoint;
			if (bBox)
			{
				if (Vars::Colors::BoundHitboxEdgeIgnoreZ.Value.a || Vars::Colors::BoundHitboxFaceIgnoreZ.Value.a)
					m_vBoxes.emplace_back(m_vPredicted, tTarget.m_pEntity->m_vecMins(), tTarget.m_pEntity->m_vecMaxs(), Vec3(), I::GlobalVars->curtime + (Vars::Visuals::Simulation::Timed.Value ? flTargetTime : Vars::Visuals::Hitbox::DrawDuration.Value), Vars::Colors::BoundHitboxEdgeIgnoreZ.Value, Vars::Colors::BoundHitboxFaceIgnoreZ.Value);
				if (Vars::Colors::BoundHitboxEdge.Value.a || Vars::Colors::BoundHitboxFace.Value.a)
					m_vBoxes.emplace_back(m_vPredicted, tTarget.m_pEntity->m_vecMins(), tTarget.m_pEntity->m_vecMaxs(), Vec3(), I::GlobalVars->curtime + (Vars::Visuals::Simulation::Timed.Value ? flTargetTime : Vars::Visuals::Hitbox::DrawDuration.Value), Vars::Colors::BoundHitboxEdge.Value, Vars::Colors::BoundHitboxFace.Value, true);
			}
			if (bMain && bPoint)
			{
				if (Vars::Colors::BoundHitboxEdgeIgnoreZ.Value.a || Vars::Colors::BoundHitboxFaceIgnoreZ.Value.a)
					m_vBoxes.emplace_back(m_vTarget, m_tInfo.m_vHull * -1, m_tInfo.m_vHull, Vec3(), I::GlobalVars->curtime + (Vars::Visuals::Simulation::Timed.Value ? flProjectileTime : Vars::Visuals::Hitbox::DrawDuration.Value), Vars::Colors::BoundHitboxEdgeIgnoreZ.Value, Vars::Colors::BoundHitboxFaceIgnoreZ.Value);
				if (Vars::Colors::BoundHitboxEdge.Value.a || Vars::Colors::BoundHitboxFace.Value.a)
					m_vBoxes.emplace_back(m_vTarget, m_tInfo.m_vHull * -1, m_tInfo.m_vHull, Vec3(), I::GlobalVars->curtime + (Vars::Visuals::Simulation::Timed.Value ? flProjectileTime : Vars::Visuals::Hitbox::DrawDuration.Value), Vars::Colors::BoundHitboxEdge.Value, Vars::Colors::BoundHitboxFace.Value, true);
			}
		}
	}

	return m_iResult;
}



bool CAimbotProjectile::Aim(const Vec3& vCurAngle, const Vec3& vToAngle, Vec3& vOut, int iMethod)
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

	Math::ClampAngles(vOut);
	return bReturn;
}

// assume angle calculated outside with other overload
void CAimbotProjectile::Aim(CUserCmd* pCmd, Vec3& vAngle, int iMethod)
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
		pCmd->viewangles = vAngle;
		I::EngineClient->SetViewAngles(vAngle);
		break;
	case Vars::Aimbot::General::AimTypeEnum::Silent:
		if (auto pWeapon = H::Entities.GetWeapon();
			G::Attacking == 1 || bUnsure || pWeapon && pWeapon->GetWeaponID() == TF_WEAPON_FLAMETHROWER)
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

static inline void CancelShot(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd, int& iLastTickCancel)
{
	switch (pWeapon->GetWeaponID())
	{
	case TF_WEAPON_COMPOUND_BOW:
	{
		pCmd->buttons |= IN_ATTACK2;
		pCmd->buttons &= ~IN_ATTACK;
		break;
	}
	case TF_WEAPON_CANNON:
	case TF_WEAPON_PIPEBOMBLAUNCHER:
	{
		for (int i = 0; i < MAX_WEAPONS; i++)
		{
			auto pSwap = pLocal->GetWeaponFromSlot(i);
			if (!pSwap || pSwap == pWeapon || !pSwap->CanBeSelected())
				continue;

			pCmd->weaponselect = pSwap->entindex();
			iLastTickCancel = pWeapon->entindex();
			break;
		}
	}
	}
}

static inline void DrawVisuals(int iResult, Target_t& tTarget, std::vector<Vec3>& vPlayerPath, std::vector<Vec3>& vProjectilePath, std::vector<DrawBox_t>& vBoxes)
{
	if (G::Attacking == 1 || iResult != 1 || !Vars::Aimbot::General::AutoShoot.Value || Vars::Debug::Info.Value)
	{
		bool bPlayerPath = Vars::Visuals::Simulation::PlayerPath.Value;
		bool bProjectilePath = Vars::Visuals::Simulation::ProjectilePath.Value && (G::Attacking == 1 || Vars::Debug::Info.Value) && iResult == 1;
		bool bBoxes = Vars::Visuals::Hitbox::BoundsEnabled.Value & (Vars::Visuals::Hitbox::BoundsEnabledEnum::OnShot | Vars::Visuals::Hitbox::BoundsEnabledEnum::AimPoint);
		bool bRealPath = Vars::Visuals::Simulation::RealPath.Value && iResult == 1;
		if (bPlayerPath || bProjectilePath || bBoxes || bRealPath)
		{
			G::PathStorage.clear();
			G::BoxStorage.clear();
			G::LineStorage.clear();

			if (bPlayerPath)
			{
				if (Vars::Colors::PlayerPathIgnoreZ.Value.a)
					G::PathStorage.emplace_back(vPlayerPath, Vars::Visuals::Simulation::Timed.Value ? -int(vPlayerPath.size()) : I::GlobalVars->curtime + Vars::Visuals::Simulation::DrawDuration.Value, Vars::Colors::PlayerPathIgnoreZ.Value, Vars::Visuals::Simulation::PlayerPath.Value);
				if (Vars::Colors::PlayerPath.Value.a)
					G::PathStorage.emplace_back(vPlayerPath, Vars::Visuals::Simulation::Timed.Value ? -int(vPlayerPath.size()) : I::GlobalVars->curtime + Vars::Visuals::Simulation::DrawDuration.Value, Vars::Colors::PlayerPath.Value, Vars::Visuals::Simulation::PlayerPath.Value, true);
			}
			if (bProjectilePath)
			{
				if (Vars::Colors::ProjectilePathIgnoreZ.Value.a)
					G::PathStorage.emplace_back(vProjectilePath, Vars::Visuals::Simulation::Timed.Value ? -int(vProjectilePath.size()) - TIME_TO_TICKS(F::Backtrack.GetReal()) : I::GlobalVars->curtime + Vars::Visuals::Simulation::DrawDuration.Value, Vars::Colors::ProjectilePathIgnoreZ.Value, Vars::Visuals::Simulation::ProjectilePath.Value);
				if (Vars::Colors::ProjectilePath.Value.a)
					G::PathStorage.emplace_back(vProjectilePath, Vars::Visuals::Simulation::Timed.Value ? -int(vProjectilePath.size()) - TIME_TO_TICKS(F::Backtrack.GetReal()) : I::GlobalVars->curtime + Vars::Visuals::Simulation::DrawDuration.Value, Vars::Colors::ProjectilePath.Value, Vars::Visuals::Simulation::ProjectilePath.Value, true);
			}
			if (bBoxes)
				G::BoxStorage.insert(G::BoxStorage.end(), vBoxes.begin(), vBoxes.end());
			if (bRealPath)
				F::Aimbot.Store(tTarget.m_pEntity, vPlayerPath.size());
		}
	}
}

bool CAimbotProjectile::RunMain(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd)
{
	const int nWeaponID = pWeapon->GetWeaponID();

	static int iStaticAimType = Vars::Aimbot::General::AimType.Value;
	const int iLastAimType = iStaticAimType;
	const int iRealAimType = Vars::Aimbot::General::AimType.Value;

	switch (nWeaponID)
	{
	case TF_WEAPON_COMPOUND_BOW:
	case TF_WEAPON_PIPEBOMBLAUNCHER:
	case TF_WEAPON_CANNON:
		if (!Vars::Aimbot::General::AutoShoot.Value && G::Attacking && !iRealAimType && iLastAimType)
			Vars::Aimbot::General::AimType.Value = iLastAimType;
		break;
	default:
		if (G::Throwing && !iRealAimType && iLastAimType)
			Vars::Aimbot::General::AimType.Value = iLastAimType;
	}
	iStaticAimType = Vars::Aimbot::General::AimType.Value;

	if (F::AimbotGlobal.ShouldHoldAttack(pWeapon))
		pCmd->buttons |= IN_ATTACK;
	if (!Vars::Aimbot::General::AimType.Value
		|| !F::AimbotGlobal.ShouldAim() && nWeaponID != TF_WEAPON_FLAMETHROWER)
		return false;

	if (Vars::Aimbot::Projectile::Modifiers.Value & Vars::Aimbot::Projectile::ModifiersEnum::ChargeWeapon && iRealAimType
		&& (nWeaponID == TF_WEAPON_COMPOUND_BOW || nWeaponID == TF_WEAPON_PIPEBOMBLAUNCHER || nWeaponID == TF_WEAPON_CANNON && G::LastUserCmd->buttons & IN_ATTACK))
	{
		pCmd->buttons |= IN_ATTACK;
		if (!G::CanPrimaryAttack && !G::Reloading && Vars::Aimbot::General::AimType.Value == Vars::Aimbot::General::AimTypeEnum::Silent)
			return false;
	}

	auto vTargets = F::AimbotGlobal.ManageTargets(GetTargets, pLocal, pWeapon);
	if (vTargets.empty())
		return false;

	if (!G::AimTarget.m_iEntIndex)
		G::AimTarget = { vTargets.front().m_pEntity->entindex(), I::GlobalVars->tickcount, 0 };

#if defined(SPLASH_DEBUG1) || defined(SPLASH_DEBUG2) || defined(SPLASH_DEBUG4)
	G::LineStorage.clear();
#endif
#if defined(SPLASH_DEBUG1) || defined(SPLASH_DEBUG2) || defined(SPLASH_DEBUG3) || defined(SPLASH_DEBUG4)
	G::BoxStorage.clear();
#endif
	for (auto& tTarget : vTargets)
	{
		m_flTimeTo = std::numeric_limits<float>::max();
		m_vPlayerPath.clear(); m_vProjectilePath.clear(); m_vBoxes.clear();

		const int iResult = CanHit(tTarget, pLocal, pWeapon);
		if (iResult != 1 && pWeapon->GetWeaponID() == TF_WEAPON_CANNON && Vars::Aimbot::Projectile::Modifiers.Value & Vars::Aimbot::Projectile::ModifiersEnum::ChargeWeapon
			&& !(G::OriginalCmd.buttons & (IN_ATTACK | IN_USE)))
		{
			float flTime = m_flTimeTo - GRENADE_CHECK_INTERVAL;
			float flCharge = pWeapon->As<CTFGrenadeLauncher>()->m_flDetonateTime() > 0.f
				? pWeapon->As<CTFGrenadeLauncher>()->m_flDetonateTime() - I::GlobalVars->curtime
				: 1.f;
			flCharge = floorf(flCharge / GRENADE_CHECK_INTERVAL) * GRENADE_CHECK_INTERVAL + F::ProjSim.GetDesync();
			if (flCharge < flTime)
			{
				if (pWeapon->As<CTFGrenadeLauncher>()->m_flDetonateTime() > 0.f)
					CancelShot(pLocal, pWeapon, pCmd, m_iLastTickCancel);
			}
			else
			{
				pCmd->buttons |= IN_ATTACK;
				if (m_iLastTickCancel)
					pCmd->weaponselect = m_iLastTickCancel = 0;
				G::OriginalCmd.buttons |= IN_USE;
			}
		}
		if (!iResult) continue;
		if (iResult == 2)
		{
			G::AimTarget = { tTarget.m_pEntity->entindex(), I::GlobalVars->tickcount, 0 };
			DrawVisuals(iResult, tTarget, m_vPlayerPath, m_vProjectilePath, m_vBoxes);
			Aim(pCmd, tTarget.m_vAngleTo);
			break;
		}

		G::AimTarget = { tTarget.m_pEntity->entindex(), I::GlobalVars->tickcount };
		G::AimPoint = { tTarget.m_vPos, I::GlobalVars->tickcount };

		if (Vars::Aimbot::General::AutoShoot.Value)
		{
			switch (nWeaponID)
			{
			case TF_WEAPON_COMPOUND_BOW:
			case TF_WEAPON_PIPEBOMBLAUNCHER:
				pCmd->buttons |= IN_ATTACK;
				if (pWeapon->As<CTFPipebombLauncher>()->m_flChargeBeginTime() > 0.f)
					pCmd->buttons &= ~IN_ATTACK;
				break;
			case TF_WEAPON_CANNON:
				pCmd->buttons |= IN_ATTACK;
				if (pWeapon->As<CTFGrenadeLauncher>()->m_flDetonateTime() > 0.f)
				{
					if (m_iLastTickCancel)
						pCmd->weaponselect = m_iLastTickCancel = 0;
					if (Vars::Aimbot::Projectile::Modifiers.Value & Vars::Aimbot::Projectile::ModifiersEnum::ChargeWeapon)
					{	// rerun, if we won't hit in the future, fire
						int iResult2 = 0;

						float flCharge = pWeapon->As<CTFGrenadeLauncher>()->m_flDetonateTime() - I::GlobalVars->curtime;
						flCharge = floorf(flCharge / GRENADE_CHECK_INTERVAL) * GRENADE_CHECK_INTERVAL + F::ProjSim.GetDesync();
						if (flCharge > GRENADE_CHECK_INTERVAL)
						{
							auto tTarget2 = tTarget;
							float flOldDetonateTime = pWeapon->As<CTFGrenadeLauncher>()->m_flDetonateTime();

							pWeapon->As<CTFGrenadeLauncher>()->m_flDetonateTime() -= GRENADE_CHECK_INTERVAL;
							iResult2 = CanHit(tTarget2, pLocal, pWeapon, false);

							pWeapon->As<CTFGrenadeLauncher>()->m_flDetonateTime() = flOldDetonateTime;
						}

						if (iResult2 != 1)
							pCmd->buttons &= ~IN_ATTACK;
					}
					else
						pCmd->buttons &= ~IN_ATTACK;
				}
				break;
			case TF_WEAPON_BAT_WOOD:
			case TF_WEAPON_BAT_GIFTWRAP:
			case TF_WEAPON_LUNCHBOX:
				pCmd->buttons |= IN_ATTACK2, pCmd->buttons &= ~IN_ATTACK;
				break;
			case TF_WEAPON_ROCKETLAUNCHER:
			case TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT:
			case TF_WEAPON_PARTICLE_CANNON:
			case TF_WEAPON_RAYGUN:
			case TF_WEAPON_DRG_POMSON:
			case TF_WEAPON_CROSSBOW:
				pCmd->buttons |= IN_ATTACK, pCmd->buttons &= ~IN_ATTACK2;
				if (pWeapon->m_iItemDefinitionIndex() == Soldier_m_TheBeggarsBazooka)
				{
					if (pWeapon->m_iClip1() > 0)
						pCmd->buttons &= ~IN_ATTACK;
				}
				break;
			default:
				pCmd->buttons |= IN_ATTACK;
			}
		}

		F::Aimbot.m_bRan = G::Attacking = SDK::IsAttacking(pLocal, pWeapon, pCmd, true);
		DrawVisuals(iResult, tTarget, m_vPlayerPath, m_vProjectilePath, m_vBoxes);

		Aim(pCmd, tTarget.m_vAngleTo);
		if (G::PSilentAngles)
		{
			switch (nWeaponID)
			{
			case TF_WEAPON_FLAMETHROWER: // angles show up anyways
			case TF_WEAPON_CLEAVER: // can't psilent with these weapons, they use SetContextThink
			case TF_WEAPON_JAR:
			case TF_WEAPON_JAR_MILK:
			case TF_WEAPON_JAR_GAS:
			case TF_WEAPON_BAT_WOOD:
			case TF_WEAPON_BAT_GIFTWRAP:
				G::PSilentAngles = false, G::SilentAngles = true;
			}
		}
		return true;
	}

	return false;
}

void CAimbotProjectile::Run(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd)
{
	const bool bSuccess = RunMain(pLocal, pWeapon, pCmd);
#ifdef SPLASH_DEBUG5
	if (Vars::Aimbot::General::AimType.Value && !s_mTraceCount.empty())
	{
		int iTraceCount = 0;
		for (auto& [_, iTraces] : s_mTraceCount)
			iTraceCount += iTraces;
		SDK::Output("Traces", std::format("{}", iTraceCount).c_str());
		for (auto& [sType, iTraces] : s_mTraceCount)
			SDK::Output("Traces", std::format("{}: {}", sType, iTraces).c_str());
	}
	s_mTraceCount.clear();
#endif

	float flAmount = 0.f;
	if (pWeapon->GetWeaponID() == TF_WEAPON_PIPEBOMBLAUNCHER)
	{
		const float flCharge = pWeapon->As<CTFPipebombLauncher>()->m_flChargeBeginTime() > 0.f ? I::GlobalVars->curtime - pWeapon->As<CTFPipebombLauncher>()->m_flChargeBeginTime() : 0.f;
		flAmount = Math::RemapVal(flCharge, 0.f, SDK::AttribHookValue(4.f, "stickybomb_charge_rate", pWeapon), 0.f, 1.f);
	}
	else if (pWeapon->GetWeaponID() == TF_WEAPON_CANNON)
	{
		const float flMortar = SDK::AttribHookValue(0.f, "grenade_launcher_mortar_mode", pWeapon);
		const float flCharge = pWeapon->As<CTFGrenadeLauncher>()->m_flDetonateTime() > 0.f ? I::GlobalVars->curtime - pWeapon->As<CTFGrenadeLauncher>()->m_flDetonateTime() : -flMortar;
		flAmount = flMortar ? Math::RemapVal(flCharge, -flMortar, 0.f, 0.f, 1.f) : 0.f;
	}

	if (pWeapon->GetWeaponID() == TF_WEAPON_PIPEBOMBLAUNCHER && G::OriginalCmd.buttons & IN_ATTACK && Vars::Aimbot::Projectile::AutoRelease.Value && flAmount > Vars::Aimbot::Projectile::AutoRelease.Value / 100)
		pCmd->buttons &= ~IN_ATTACK;
	else if (G::CanPrimaryAttack && Vars::Aimbot::Projectile::Modifiers.Value & Vars::Aimbot::Projectile::ModifiersEnum::CancelCharge)
	{
		if (m_bLastTickHeld && (G::LastUserCmd->buttons & IN_ATTACK && !(pCmd->buttons & IN_ATTACK) && !bSuccess || flAmount > 0.95f))
			CancelShot(pLocal, pWeapon, pCmd, m_iLastTickCancel);
	}

	m_bLastTickHeld = Vars::Aimbot::General::AimType.Value;
}



// TestAngle and CanHit shares a bunch of code, possibly merge somehow

bool CAimbotProjectile::TestAngle(CBaseEntity* pProjectile, const Vec3& vPoint, Vec3& vAngles, int iSimTime, bool bSplash)
{
	auto pLocal = m_tInfo.m_pLocal;
	auto pWeapon = m_tInfo.m_pWeapon;
	auto& tTarget = *m_tInfo.m_pTarget;

	m_tProjInfo = {};
	F::ProjSim.GetInfo(pProjectile, m_tProjInfo);
	CGameTrace trace = {};
	{
		CTraceFilterWorldAndPropsOnly filter = {};

		Vec3 vEyePos = pLocal->GetShootPos(); // m_tInfo.m_vLocalEye is not actually our shootpos here
		m_tProjInfo.m_vPos = pProjectile->GetAbsOrigin();

		Vec3 vPos = vPoint;
		if (m_tInfo.m_flGravity)
			vPos += Vec3(0, 0, (m_tInfo.m_flGravity * 800.f * pow(TICKS_TO_TIME(iSimTime), 2)) / 2);
		Vec3 vForward = (vPos - m_tProjInfo.m_vPos).Normalized();
		m_tProjInfo.m_vAng = Math::VectorAngles(vForward);

		SDK::Trace(m_tProjInfo.m_vPos, m_tProjInfo.m_vPos + vForward * MAX_TRACE_LENGTH, MASK_SOLID, &filter, &trace);
		vAngles = Math::CalcAngle(vEyePos, trace.endpos);
		vForward = (vEyePos - trace.endpos).Normalized();
		if (vForward.Dot(trace.plane.normal) <= 0)
			return false;

		SDK::Trace(vEyePos, trace.endpos, MASK_SOLID, &filter, &trace);
		if (trace.fraction < 0.999f)
			return false;

		if (!F::AutoAirblast.CanAirblastEntity(pLocal, pWeapon, pProjectile, vAngles))
			return false;
	}
	if (!F::ProjSim.Initialize(m_tProjInfo, false, true))
		return false;

	CTraceFilterCollideable filter = {};
	filter.pSkip = bSplash ? tTarget.m_pEntity : pLocal;
	filter.iPlayer = bSplash ? PLAYER_NONE : PLAYER_DEFAULT;
	int nMask = MASK_SOLID;
	F::ProjSim.SetupTrace(filter, nMask, pProjectile);

	if (!m_tProjInfo.m_flGravity)
	{
		SDK::TraceHull(m_tProjInfo.m_vPos, vPoint, m_tProjInfo.m_vHull * -1, m_tProjInfo.m_vHull, nMask, &filter, &trace);
		if (trace.fraction < 0.999f && trace.m_pEnt != tTarget.m_pEntity)
			return false;
	}

	bool bDidHit = false;
	const RestoreInfo_t tOriginal = { tTarget.m_pEntity->GetAbsOrigin(), tTarget.m_pEntity->m_vecMins(), tTarget.m_pEntity->m_vecMaxs() };
	tTarget.m_pEntity->SetAbsOrigin(tTarget.m_vPos);
	tTarget.m_pEntity->m_vecMins() = { std::clamp(tTarget.m_pEntity->m_vecMins().x, -24.f, 0.f), std::clamp(tTarget.m_pEntity->m_vecMins().y, -24.f, 0.f), tTarget.m_pEntity->m_vecMins().z };
	tTarget.m_pEntity->m_vecMaxs() = { std::clamp(tTarget.m_pEntity->m_vecMaxs().x, 0.f, 24.f), std::clamp(tTarget.m_pEntity->m_vecMaxs().y, 0.f, 24.f), tTarget.m_pEntity->m_vecMaxs().z };
	for (int n = 1; n <= iSimTime; n++)
	{
		Vec3 vOld = F::ProjSim.GetOrigin();
		F::ProjSim.RunTick(m_tProjInfo);
		Vec3 vNew = F::ProjSim.GetOrigin();

		if (bDidHit)
		{
			trace.endpos = vNew;
			continue;
		}

		if (!bSplash)
			SDK::TraceHull(vOld, vNew, m_tProjInfo.m_vHull * -1, m_tProjInfo.m_vHull, nMask, &filter, &trace);
		else
		{
			static Vec3 vStaticPos = {};
			if (n == 1)
				vStaticPos = vOld;
			if (n % Vars::Aimbot::Projectile::SplashTraceInterval.Value && n != iSimTime)
				continue;

			SDK::TraceHull(vStaticPos, vNew, m_tProjInfo.m_vHull * -1, m_tProjInfo.m_vHull, nMask, &filter, &trace);
			vStaticPos = vNew;
		}
		if (trace.DidHit())
		{
			bool bTime = bSplash
				? trace.endpos.DistTo(vPoint) < m_tProjInfo.m_flVelocity * TICK_INTERVAL + m_tProjInfo.m_vHull.z
				: iSimTime - n < 5;
			bool bTarget = trace.m_pEnt == tTarget.m_pEntity || bSplash;
			bool bValid = bTarget && bTime;
			if (bValid && bSplash)
			{
				bValid = SDK::VisPosWorld(nullptr, tTarget.m_pEntity, trace.endpos, vPoint, nMask);
				if (bValid)
				{
					Vec3 vFrom = trace.endpos;
					switch (pProjectile->GetClassID())
					{
					case ETFClassID::CTFProjectile_Rocket:
					case ETFClassID::CTFProjectile_SentryRocket:
					case ETFClassID::CTFProjectile_EnergyBall:
						vFrom += trace.plane.normal;
					}

					CGameTrace eyeTrace = {};
					SDK::Trace(vFrom, tTarget.m_vPos + tTarget.m_pEntity->As<CTFPlayer>()->GetViewOffset(), MASK_SHOT, &filter, &eyeTrace);
					bValid = eyeTrace.fraction == 1.f;
				}
			}

			if (bValid)
			{
				if (bSplash)
				{
					int iPopCount = Vars::Aimbot::Projectile::SplashTraceInterval.Value - trace.fraction * Vars::Aimbot::Projectile::SplashTraceInterval.Value;
					for (int i = 0; i < iPopCount && !m_tProjInfo.m_vPath.empty(); i++)
						m_tProjInfo.m_vPath.pop_back();
				}

				bDidHit = true;
			}
			else
				break;

			if (!bSplash)
				trace.endpos = vNew;

			if (!bTarget || bSplash)
				break;
		}
	}
	tTarget.m_pEntity->SetAbsOrigin(tOriginal.m_vOrigin);
	tTarget.m_pEntity->m_vecMins() = tOriginal.m_vMins;
	tTarget.m_pEntity->m_vecMaxs() = tOriginal.m_vMaxs;
	m_tProjInfo.m_vPath.push_back(trace.endpos);

	return bDidHit;
}


bool CAimbotProjectile::CanHit(Target_t& tTarget, CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CBaseEntity* pProjectile)
{
	m_tProjInfo = {};
	F::ProjSim.GetInfo(pProjectile, m_tProjInfo);
	if (!F::ProjSim.Initialize(m_tProjInfo, false, true))
		return false;

	m_tMoveStorage = {};
	F::MoveSim.Initialize(tTarget.m_pEntity, m_tMoveStorage);
	tTarget.m_vPos = tTarget.m_pEntity->m_vecOrigin();

	m_tInfo = { pLocal, m_tProjInfo.m_pWeapon, &tTarget, pProjectile };
	m_tInfo.m_flLatency = F::Backtrack.GetReal() + TICKS_TO_TIME(F::Backtrack.GetAnticipatedChoke());
	m_tInfo.m_vHull = pProjectile->m_vecMaxs().Min(3);
	{
		CGameTrace trace = {};
		CTraceFilterWorldAndPropsOnly filter = {};

		for (int i = TIME_TO_TICKS(m_tInfo.m_flLatency); i > 0; i--)
		{
			Vec3 vOld = F::ProjSim.GetOrigin();
			F::ProjSim.RunTick(m_tProjInfo);
			Vec3 vNew = F::ProjSim.GetOrigin();

			SDK::TraceHull(vOld, vNew, m_tProjInfo.m_vHull * -1, m_tProjInfo.m_vHull, MASK_SOLID, &filter, &trace);
			m_tProjInfo.m_vPos = trace.endpos;
		}
		m_tInfo.m_vLocalEye = m_tProjInfo.m_vPos; // just assume from the projectile without any offset, check validity later
		pProjectile->SetAbsOrigin(m_tProjInfo.m_vPos);
	}
	m_tInfo.m_vTargetEye = tTarget.m_pEntity->As<CTFPlayer>()->GetViewOffset();

	m_tInfo.m_flVelocity = m_tProjInfo.m_flVelocity;

	m_tInfo.m_flGravity = m_tProjInfo.m_flGravity;
	m_tInfo.m_iSplashCount = !m_tInfo.m_flGravity ? Vars::Aimbot::Projectile::SplashCountDirect.Value : Vars::Aimbot::Projectile::SplashCountArc.Value;

	float flSize = tTarget.m_pEntity->GetSize().Length();
	m_tInfo.m_flRadius = GetSplashRadius(pProjectile, m_tProjInfo.m_pWeapon, m_tProjInfo.m_pOwner, Vars::Aimbot::Projectile::SplashRadius.Value / 100, pWeapon);
	m_tInfo.m_flRadiusTime = m_tInfo.m_flRadius / m_tInfo.m_flVelocity;
	m_tInfo.m_flBoundingTime = m_tInfo.m_flRadiusTime + flSize / m_tInfo.m_flVelocity;



	int iMaxTime = TIME_TO_TICKS(Vars::Aimbot::Projectile::MaxSimulationTime.Value);
	int iSplash = Vars::Aimbot::Projectile::SplashPrediction.Value && m_tInfo.m_flRadius ? Vars::Aimbot::Projectile::SplashPrediction.Value : Vars::Aimbot::Projectile::SplashPredictionEnum::Off;

	auto mDirectPoints = iSplash == Vars::Aimbot::Projectile::SplashPredictionEnum::Only ? std::unordered_map<int, Vec3>() : GetDirectPoints();

	std::vector<Direct_t> vDirectHistory = {};
	std::vector<Splash_t> vSplashHistory = {};

	for (int i = 1 - TIME_TO_TICKS(m_tInfo.m_flLatency); i <= iMaxTime; i++)
	{
		if (!m_tMoveStorage.m_bFailed)
		{
			F::MoveSim.RunTick(m_tMoveStorage);
			tTarget.m_vPos = m_tMoveStorage.m_vPredictedOrigin;
		}
		if (i < 0)
			continue;

		bool bDirectBreaks = true;
		if (iSplash)
		{
			Solution_t tSolution; CalculateAngle(m_tInfo.m_vLocalEye, tTarget.m_vPos, i, tSolution, false);
			if (tSolution.m_iCalculated != CalculatedEnum::Bad)
			{
				const float flTimeTo = tSolution.m_flTime - TICKS_TO_TIME(i);

				bDirectBreaks = false;
				if (flTimeTo < m_tInfo.m_flBoundingTime)
				{
					bDirectBreaks = flTimeTo < -m_tInfo.m_flBoundingTime;
					if (!bDirectBreaks)
						vSplashHistory.emplace_back(History_t(tTarget.m_vPos, i), fabsf(flTimeTo));
				}
			}
		}

		for (auto& [iIndex, vOffset] : mDirectPoints)
		{
			Vec3 vPoint = tTarget.m_vPos + vOffset;
			if (Vars::Aimbot::Projectile::HuntsmanPullPoint.Value && tTarget.m_nAimedHitbox == HITBOX_HEAD)
			{
				vPoint = PullPoint(vPoint, m_tInfo.m_vLocalEye, m_tInfo, tTarget.m_pEntity->m_vecMins() + m_tInfo.m_vHull, tTarget.m_pEntity->m_vecMaxs() - m_tInfo.m_vHull, tTarget.m_vPos);
				if (Vars::Aimbot::Projectile::HuntsmanPullNoZ.Value)
					vPoint.z = tTarget.m_vPos.z + vOffset.z;
			}

			Solution_t tSolution; CalculateAngle(m_tInfo.m_vLocalEye, vPoint, i, tSolution);
			switch (tSolution.m_iCalculated)
			{
			case CalculatedEnum::Good:
				vDirectHistory.emplace_back(History_t(tTarget.m_vPos, i), tSolution.m_flPitch, tSolution.m_flYaw, tSolution.m_flTime, vPoint, iIndex);
				[[fallthrough]];
			case CalculatedEnum::Bad:
				mDirectPoints.erase(iIndex);
			}
		}
		if (bDirectBreaks && mDirectPoints.empty())
			break;
	}

	m_iResult = false;
	switch (iSplash)
	{
	case Vars::Aimbot::Projectile::SplashPredictionEnum::Off:
		HandleDirect(vDirectHistory);
		break;
	case Vars::Aimbot::Projectile::SplashPredictionEnum::Include:
		if (!HandleDirect(vDirectHistory))
			HandleSplash(vSplashHistory);
		break;
	case Vars::Aimbot::Projectile::SplashPredictionEnum::Prefer:
		if (!HandleSplash(vSplashHistory))
			HandleDirect(vDirectHistory);
		break;
	case Vars::Aimbot::Projectile::SplashPredictionEnum::Only:
		HandleSplash(vSplashHistory);
	}
	F::MoveSim.Restore(m_tMoveStorage);

	tTarget.m_vPos = m_vTarget;
	tTarget.m_vAngleTo = m_vAngleTo;
	if (tTarget.m_iTargetType != TargetEnum::Player || !m_tMoveStorage.m_bFailed) // don't attempt to aim at players when movesim fails
	{
		bool bMain = m_iResult == 1;

		if (bMain)
		{
			if (Vars::Colors::BoundHitboxEdge.Value.a || Vars::Colors::BoundHitboxFace.Value.a || Vars::Colors::BoundHitboxEdgeIgnoreZ.Value.a || Vars::Colors::BoundHitboxFaceIgnoreZ.Value.a)
			{
				m_tInfo.m_vHull = m_tInfo.m_vHull.Max(1);
				float flProjectileTime = TICKS_TO_TIME(m_vProjectilePath.size());
				float flTargetTime = m_tMoveStorage.m_bFailed ? flProjectileTime : TICKS_TO_TIME(m_vPlayerPath.size());

				bool bBox = Vars::Visuals::Hitbox::BoundsEnabled.Value & Vars::Visuals::Hitbox::BoundsEnabledEnum::OnShot;
				bool bPoint = Vars::Visuals::Hitbox::BoundsEnabled.Value & Vars::Visuals::Hitbox::BoundsEnabledEnum::AimPoint;
				if (bBox)
				{
					if (Vars::Colors::BoundHitboxEdgeIgnoreZ.Value.a || Vars::Colors::BoundHitboxFaceIgnoreZ.Value.a)
						m_vBoxes.emplace_back(m_vPredicted, tTarget.m_pEntity->m_vecMins(), tTarget.m_pEntity->m_vecMaxs(), Vec3(), I::GlobalVars->curtime + (Vars::Visuals::Simulation::Timed.Value ? flTargetTime : Vars::Visuals::Hitbox::DrawDuration.Value), Vars::Colors::BoundHitboxEdgeIgnoreZ.Value, Vars::Colors::BoundHitboxFaceIgnoreZ.Value);
					if (Vars::Colors::BoundHitboxEdge.Value.a || Vars::Colors::BoundHitboxFace.Value.a)
						m_vBoxes.emplace_back(m_vPredicted, tTarget.m_pEntity->m_vecMins(), tTarget.m_pEntity->m_vecMaxs(), Vec3(), I::GlobalVars->curtime + (Vars::Visuals::Simulation::Timed.Value ? flTargetTime : Vars::Visuals::Hitbox::DrawDuration.Value), Vars::Colors::BoundHitboxEdge.Value, Vars::Colors::BoundHitboxFace.Value, true);
				}
				if (bPoint)
				{
					if (Vars::Colors::BoundHitboxEdgeIgnoreZ.Value.a || Vars::Colors::BoundHitboxFaceIgnoreZ.Value.a)
						m_vBoxes.emplace_back(m_vTarget, m_tInfo.m_vHull * -1, m_tInfo.m_vHull, Vec3(), I::GlobalVars->curtime + (Vars::Visuals::Simulation::Timed.Value ? flProjectileTime : Vars::Visuals::Hitbox::DrawDuration.Value), Vars::Colors::BoundHitboxEdgeIgnoreZ.Value, Vars::Colors::BoundHitboxFaceIgnoreZ.Value);
					if (Vars::Colors::BoundHitboxEdge.Value.a || Vars::Colors::BoundHitboxFace.Value.a)
						m_vBoxes.emplace_back(m_vTarget, m_tInfo.m_vHull * -1, m_tInfo.m_vHull, Vec3(), I::GlobalVars->curtime + (Vars::Visuals::Simulation::Timed.Value ? flProjectileTime : Vars::Visuals::Hitbox::DrawDuration.Value), Vars::Colors::BoundHitboxEdge.Value, Vars::Colors::BoundHitboxFace.Value, true);
				}
			}
		}
	}

	return m_iResult;
}

bool CAimbotProjectile::AutoAirblast(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd, CBaseEntity* pProjectile)
{
	auto vTargets = F::AimbotGlobal.ManageTargets(GetTargets, pLocal, pWeapon);
	if (vTargets.empty())
		return false;

	//if (!G::AimTarget.m_iEntIndex)
	//	G::AimTarget = { vTargets.front().m_pEntity->entindex(), I::GlobalVars->tickcount, 0 };

	for (auto& tTarget : vTargets)
	{
		m_flTimeTo = std::numeric_limits<float>::max();
		m_vPlayerPath.clear(); m_vProjectilePath.clear(); m_vBoxes.clear();

		const bool bResult = CanHit(tTarget, pLocal, pWeapon, pProjectile);
		if (!bResult) continue;

		G::AimTarget = { tTarget.m_pEntity->entindex(), I::GlobalVars->tickcount };
		G::AimPoint = { tTarget.m_vPos, I::GlobalVars->tickcount };

		G::Attacking = true;
		DrawVisuals(1, tTarget, m_vPlayerPath, m_vProjectilePath, m_vBoxes);

		Aim(pCmd, tTarget.m_vAngleTo, Vars::Aimbot::General::AimTypeEnum::Silent);
		return true;
	}

	return false;
}