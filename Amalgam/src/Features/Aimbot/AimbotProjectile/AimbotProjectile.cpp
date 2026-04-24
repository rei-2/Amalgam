#include "AimbotProjectile.h"

#include "../Aimbot.h"
#include "../../Ticks/Ticks.h"
#include "../../EnginePrediction/EnginePrediction.h"
#include "../../World/World.h"
#include "../AutoAirblast/AutoAirblast.h"
#include <numeric>

//#define SPLASH_DEBUG1 // trace splash visualization
//#define SPLASH_DEBUG2 // plane splash visualization
//#define SPLASH_DEBUG3 // points visualization
//#define SPLASH_DEBUG4 // test visualization
//#define SPLASH_DEBUG5 // trace/face count

#ifdef SPLASH_DEBUG5
static std::map<std::string, int> s_mTraceCount = {};
#endif
#ifdef SPLASH_DEBUG2
//#include "../../Visuals/Visuals.h"
#endif

static inline std::vector<Target_t> GetTargets(CTFPlayer* pLocal, CTFWeaponBase* pWeapon)
{
	std::vector<Target_t> vTargets;

	const Vec3 vLocalPos = F::Ticks.GetShootPos();
	const Vec3 vLocalAngles = I::EngineClient->GetViewAngles();

	{
		auto eGroup = EntityEnum::Invalid;
		if (Vars::Aimbot::General::Target.Value & Vars::Aimbot::General::TargetEnum::Players)
			eGroup = !F::AimbotGlobal.FriendlyFire() || Vars::Aimbot::General::Ignore.Value & Vars::Aimbot::General::IgnoreEnum::Team ? EntityEnum::PlayerEnemy : EntityEnum::PlayerAll;
		switch (pWeapon->GetWeaponID())
		{
		case TF_WEAPON_CROSSBOW:
			if (Vars::Aimbot::Healing::AutoArrow.Value)
				eGroup = eGroup != EntityEnum::Invalid ? EntityEnum::PlayerAll : EntityEnum::PlayerTeam;
			break;
		case TF_WEAPON_LUNCHBOX:
			if (Vars::Aimbot::Healing::AutoSandvich.Value)
				eGroup = EntityEnum::PlayerTeam;
			break;
		}
		bool bHeal = pWeapon->GetWeaponID() == TF_WEAPON_CROSSBOW || pWeapon->GetWeaponID() == TF_WEAPON_LUNCHBOX;

		for (auto pEntity : H::Entities.GetGroup(eGroup))
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

		if (pWeapon->GetWeaponID() == TF_WEAPON_LUNCHBOX)
			return vTargets;
	}

	{
		auto eGroup = EntityEnum::Invalid;
		if (Vars::Aimbot::General::Target.Value & Vars::Aimbot::General::TargetEnum::Building)
			eGroup = EntityEnum::BuildingEnemy;
		if (Vars::Aimbot::Healing::AutoRepair.Value && pWeapon->GetWeaponID() == TF_WEAPON_SHOTGUN_BUILDING_RESCUE)
			eGroup = eGroup != EntityEnum::Invalid ? EntityEnum::BuildingAll : EntityEnum::BuildingTeam;
		for (auto pEntity : H::Entities.GetGroup(eGroup))
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

			float flDistTo = vLocalPos.DistToSqr(vPos);
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

				float flDistTo = vLocalPos.DistToSqr(vPos);
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

			float flDistTo = vLocalPos.DistToSqr(vPos);
			vTargets.emplace_back(pEntity, TargetEnum::NPC, vPos, vAngleTo, flFOVTo, flDistTo);
		}
	}

	return vTargets;
}



float CAimbotProjectile::GetSplashRadius(CTFWeaponBase* pWeapon, CTFPlayer* pPlayer, float flScale)
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
		break;
	case TF_WEAPON_JAR:
	case TF_WEAPON_JAR_MILK:
	case TF_WEAPON_JAR_GAS:
		return JAR_EXPLODE_RADIUS * flScale;
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
	return flRadius * flScale;
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
		break;
	case ETFClassID::CTFProjectile_Jar:
	case ETFClassID::CTFProjectile_JarMilk:
	case ETFClassID::CTFProjectile_JarGas:
		return JAR_EXPLODE_RADIUS * flScale;
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

static inline bool ShouldLob(Info_t& tInfo)
{
	return tInfo.m_flGravity && Vars::Aimbot::Projectile::Modifiers.Value & Vars::Aimbot::Projectile::ModifiersEnum::LobAngles;
}

static inline bool ShouldLob(MoveStorage& tMoveStorage, Info_t& tInfo)
{
	return tInfo.m_bIgnoreTiming && (tMoveStorage.m_bFailed || tMoveStorage.m_pPlayer->IsOnGround());
}

static inline bool AirSplash(CTFWeaponBase* pWeapon)
{
	if (!(Vars::Aimbot::Projectile::Modifiers.Value & Vars::Aimbot::Projectile::ModifiersEnum::AirSplash))
		return false;

	switch (pWeapon->GetWeaponID())
	{
	case TF_WEAPON_PIPEBOMBLAUNCHER:
	case TF_WEAPON_FLAREGUN:
		return true;
	}

	return false;
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

Directs_t CAimbotProjectile::GetDirects()
{
	Directs_t mDirects = {};

	if (Vars::Aimbot::Projectile::SplashPrediction.Value == Vars::Aimbot::Projectile::SplashPredictionEnum::Only && m_tInfo.m_flRadius)
		return mDirects;

	auto& tTarget = *m_tInfo.m_pTarget;
	uint8_t iFlags = PointFlagsEnum::Regular;
	if (ShouldLob(m_tInfo))
		iFlags |= PointFlagsEnum::Lob;

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
				mDirects[iPriority] = { vOff, iFlags };
			}
			else
				mDirects[iPriority] = { Vec3(0, 0, vMaxs.z - Vars::Aimbot::Projectile::VerticalShift.Value), iFlags };
			break;
		case BOUNDS_BODY:
			mDirects[iPriority] = { Vec3(0, 0, (vMaxs.z - vMins.z) / 2), iFlags };
			break;
		case BOUNDS_FEET:
			mDirects[iPriority] = { Vec3(0, 0, vMins.z + Vars::Aimbot::Projectile::VerticalShift.Value), iFlags };
			break;
		}
	}

	return mDirects;
}

Splashes_t CAimbotProjectile::GetSplashes()
{
	Splashes_t vSplashes = {};

	if (Vars::Aimbot::Projectile::SplashPrediction.Value == Vars::Aimbot::Projectile::SplashPredictionEnum::Off || !m_tInfo.m_flRadius)
		return vSplashes;

	vSplashes.push_back(PointFlagsEnum::Regular);
	if (ShouldLob(m_tInfo))
		vSplashes.push_back(PointFlagsEnum::Lob);

	return vSplashes;
}

static inline std::vector<Vec3> ComputePoints(float flRadius, int iSamples)
{
	std::vector<Vec3> vPoints = { { Vec3(0.f, 0.f, -1.f) * flRadius } };
	if (!iSamples)
		return vPoints;

	vPoints.reserve(iSamples + 1);

	float flRotateX = Vars::Aimbot::Projectile::SplashRotateX.Value < 0.f ? SDK::StdRandomFloat(0.f, 360.f) : Vars::Aimbot::Projectile::SplashRotateX.Value;
	float flRotateY = Vars::Aimbot::Projectile::SplashRotateY.Value < 0.f ? SDK::StdRandomFloat(0.f, 360.f) : Vars::Aimbot::Projectile::SplashRotateY.Value;
		
	float a = Math::PI * (3.f - sqrtf(5.f));
	for (int n = 0; n < iSamples; n++)
	{
		float t = a * n;
		float y = 1 - (n / (iSamples - 1.f)) * 2;
		float r = sqrtf(1 - powf(y, 2));
		float x = cosf(t) * r;
		float z = sinf(t) * r;

		Vec3 vPoint = Vec3(x, y, z) * flRadius;
		vPoint = Math::RotatePoint(vPoint, {}, { flRotateX, flRotateY });

		vPoints.push_back(vPoint);
	}

	return vPoints;
};

#if defined(SPLASH_DEBUG1) || defined(SPLASH_DEBUG2)
static inline void DrawTrace(bool bSuccess, Color_t tColor, CGameTrace& trace)
{
	Vec3 vMins = -Vec3::Get(bSuccess ? 1.f : 0.5f), vMaxs = Vec3::Get(bSuccess ? 1.f : 0.5f);
	Vec3 vAngles = Math::VectorAngles(trace.plane.normal);
	G::BoxStorage.emplace_back(trace.endpos, vMins, vMaxs, vAngles, I::GlobalVars->curtime + 60.f, tColor.Alpha(tColor.a / (bSuccess ? 1 : 10)), Color_t(0, 0, 0, 0));
	G::LineStorage.emplace_back(std::pair<Vec3, Vec3>(trace.startpos, trace.endpos), I::GlobalVars->curtime + 60.f, tColor.Alpha(tColor.a / (bSuccess ? 1 : 10)));
};
#endif

static inline void HandleTrace(const Vec3& vPoint, std::vector<Setup_t>& vPoints, const Vec3& vTargetEye, Info_t& tInfo, std::function<bool()> fCheckPointTrace, std::function<bool()> fCheckPointAir, CGameTrace& trace, ITraceFilter& filter, int i = 0)
{
	// out
	SDK::TraceHull(vTargetEye, vPoint, -tInfo.m_vHull, tInfo.m_vHull, MASK_SOLID, &filter, &trace);
#ifdef SPLASH_DEBUG5
	s_mTraceCount[__FUNCTION__": splash out"]++;
#endif

	if (fCheckPointAir())
		return;

	if (fCheckPointTrace())
#ifndef SPLASH_DEBUG1
		vPoints.emplace_back(trace.endpos);
#else
	{
		vPoints.emplace_back(trace.endpos);
		DrawTrace(true, Vars::Colors::IndicatorGood.Value, trace);
	}
	else
		DrawTrace(false, Vars::Colors::IndicatorGood.Value, trace);
#endif

	// in
	if ((tInfo.m_vLocalEye - vTargetEye).Dot(vTargetEye - vPoint) > 0.f)
		return;

	SDK::Trace(vPoint, vTargetEye, MASK_SHOT, &filter, &trace);
#ifdef SPLASH_DEBUG5
	s_mTraceCount[__FUNCTION__": splash in check"]++;
#endif
#ifdef SPLASH_DEBUG1
	DrawTrace(!trace.DidHit(), Vars::Colors::IndicatorMid.Value, trace);
#endif
	if (trace.DidHit())
		return;

	SDK::TraceHull(vPoint, vTargetEye, -tInfo.m_vHull, tInfo.m_vHull, MASK_SOLID, &filter, &trace);
#ifdef SPLASH_DEBUG5
	s_mTraceCount[__FUNCTION__": splash in"]++;
#endif

	if (fCheckPointTrace())
#ifndef SPLASH_DEBUG1
		vPoints.emplace_back(trace.endpos);
#else
	{
		vPoints.emplace_back(trace.endpos);
		DrawTrace(true, Vars::Colors::IndicatorBad.Value, trace);
	}
	else
		DrawTrace(false, Vars::Colors::IndicatorBad.Value, trace);
#endif
}

static float s_flTotal = 0.f;
static inline void HandleFace(Face_t& tFace, std::vector<Setup_t>& vPoints, float flDensity, float flRadius, float flCutoff, const Vec3& vTargetEye, const Vec3& vTargetCenter, const Vec3& vTargetOrigin, Info_t& tInfo, CGameTrace& trace, ITraceFilter& filter)
{
	float flRadiusSqr = powf(flRadius, 2);
	float flRadius2Sqr = flRadiusSqr * 4;
	int nMask = F::ProjSim.m_bPhysics ? MASK_SHOT | CONTENTS_DISPSOLID : MASK_SHOT;

	std::vector<Vec3> vVertices, vEpsilon;
	if (tFace.m_iType != FaceTypeEnum::Prop)
		vVertices = tFace.m_vVertices;
	else
		Math::ExpandPolygon(vVertices, tFace.m_vVertices, tFace.m_vNormal, SDK::StdRandomFloat(0.f, DIST_EPSILON), &vTargetEye);

	for (int i = 0, n = int(tFace.m_vVertices.size()), o = SDK::StdRandomInt(0, n - 1); ++i < n - 1;)
	{
		Vec3& vVertex1 = tFace.m_vVertices[o], &vVertex2 = tFace.m_vVertices[(o + i) % n], &vVertex3 = tFace.m_vVertices[(o + i + 1) % n];

		Vec3 vDir21 = vVertex2 - vVertex1, vDir31 = vVertex3 - vVertex1;
		float flArea = vDir21.Cross(vDir31).Length() / 2;
		float flSamples = flDensity * flArea / flRadius2Sqr;
		int iSamples = flSamples > flCutoff ? ceilf(flSamples) : fmodf(s_flTotal += flSamples, flCutoff) < flSamples;
		if (!iSamples)
			continue;

		// don't particularly like the hacky random epsilons
		int iFaceClosest = SDK::StdRandomInt(iSamples < 2 ? 0 : iSamples < 4 ? 1 : 2, iSamples < 2 ? 1 : 2);
		//int iEdgeClosest = SDK::StdRandomInt(iSamples < 8 ? 0 : iSamples < 16 ? 1 : 2, iSamples < 8 ? 1 : 2); // eats up a bit too much performance for my liking
		int iEdgeRandom = SDK::StdRandomInt(iSamples < 8 ? 0 : iSamples < 16 ? 1 : 2, iSamples < 3 ? 0 : iSamples < 12 ? 1 : 2);
		iEdgeRandom += iFaceClosest; //iEdgeClosest += iFaceClosest, iEdgeRandom += iEdgeClosest; //, iSamples += iEdgeRandom;
#ifdef SPLASH_DEBUG2
		Color_t tColor = { byte(SDK::StdRandomInt(0, 255)), byte(SDK::StdRandomInt(0, 255)), byte(SDK::StdRandomInt(0, 255)) };
#ifdef WORLD_DEBUG
		F::World.DrawFace({ { vVertex1, vVertex2, vVertex3 }, tFace.m_vNormal, tFace.m_iType }, DrawTypeEnum::Edges | DrawTypeEnum::Faces, tColor);
#endif
#ifdef DEBUG_TEXT
		F::Visuals.AddDebugText(std::format("{}:{}:{}"/*:{}"*/, iFaceClosest, /*iEdgeClosest,*/ iEdgeRandom, iSamples), (vVertex1 + vVertex2 + vVertex3) / 3, tColor);
#endif
#endif
		for (int s = 0; s < iSamples; s++)
		{
			Vec3 vPoint, vNormal = tFace.m_vNormal; bool bInside = true;
			if (s < iFaceClosest) // closest point
			{
				float flEpsilon = s == 0 && (tFace.m_iType == FaceTypeEnum::BoxBrush || iFaceClosest != 1 || SDK::StdRandomBool()) ? CALC_EPSILON : DIST_EPSILON;
				vPoint = Math::ClosestPointOnTriangle(vTargetOrigin, vVertex1, vVertex2, vVertex3, &bInside);
				vPoint += { SDK::StdRandomFloat(-flEpsilon, flEpsilon), SDK::StdRandomFloat(-flEpsilon, flEpsilon), SDK::StdRandomFloat(-flEpsilon, flEpsilon) };
			}
			//else if (s < iEdgeClosest) // closest point on edge
			//{
			//	float flEpsilon = SDK::StdRandomBool() ? CALC_EPSILON : DIST_EPSILON; bInside = false;
			//	switch (SDK::StdRandomInt(0, 2))
			//	{
			//	case 0: vPoint = Math::ClosestPointOnLine(vTargetOrigin, vVertex1, vVertex2); break;
			//	case 1: vPoint = Math::ClosestPointOnLine(vTargetOrigin, vVertex2, vVertex3); break;
			//	case 2: vPoint = Math::ClosestPointOnLine(vTargetOrigin, vVertex3, vVertex1); break;
			//	}
			//	vPoint += { SDK::StdRandomFloat(-flEpsilon, flEpsilon), SDK::StdRandomFloat(-flEpsilon, flEpsilon), SDK::StdRandomFloat(-flEpsilon, flEpsilon) };
			//}
			else if (s < iEdgeRandom) // random point on edge
			{
				float flEpsilon = SDK::StdRandomBool() ? CALC_EPSILON : DIST_EPSILON; bInside = false;
				switch (SDK::StdRandomInt(0, 2))
				{
				case 0: vPoint = vVertex1.Lerp(vVertex2, SDK::StdRandomFloat()); break;
				case 1: vPoint = vVertex2.Lerp(vVertex3, SDK::StdRandomFloat()); break;
				case 2: vPoint = vVertex3.Lerp(vVertex1, SDK::StdRandomFloat()); break;
				}
				vPoint += { SDK::StdRandomFloat(-flEpsilon, flEpsilon), SDK::StdRandomFloat(-flEpsilon, flEpsilon), SDK::StdRandomFloat(-flEpsilon, flEpsilon) };
			}
			else // random point on face
			{
				float flRandom1 = SDK::StdRandomFloat(), flRandom2 = SDK::StdRandomFloat();
				if (flRandom1 + flRandom2 > 1)
					flRandom1 = 1 - flRandom1, flRandom2 = 1 - flRandom2;
				vPoint = vVertex1 + vDir21 * flRandom1 + vDir31 * flRandom2;
			}
			vPoint += vNormal * (tInfo.m_vHull + CALC_EPSILON);
			if (vPoint.DistToSqr(vTargetCenter) > flRadiusSqr)
				continue;

			if (!bInside)
			{
				if (vEpsilon.empty())
					Math::ExpandPolygon(vEpsilon = vVertices, tFace.m_vNormal, -DIST_EPSILON);
				Math::ClosestPointOnPolygon(vPoint, vEpsilon, vNormal, &bInside);

				if (tFace.m_iType == FaceTypeEnum::Prop && !bInside)
					vPoint += tFace.m_vNormal * SDK::StdRandomFloat(0.f, DIST_EPSILON);
			}

#ifdef SPLASH_DEBUG5
			if (bInside) s_mTraceCount[__FUNCTION__": point contents"]++;
#endif
			if (bInside && I::EngineTrace->GetPointContents(vPoint) & MASK_SOLID)
				continue;

			int nSubMask = nMask;
			if (!bInside)
				vNormal = (vTargetEye - vPoint).Normalized(), nSubMask &= ~CONTENTS_MOVEABLE;

			SDK::Trace(vPoint + vNormal * tInfo.m_flNormalOffset, vTargetEye, nSubMask, &filter, &trace);
#ifdef SPLASH_DEBUG5
			s_mTraceCount[__FUNCTION__": vispos"]++;
#endif
#ifdef SPLASH_DEBUG2
			DrawTrace(trace.fraction == 1.f, Vars::Colors::IndicatorMisc.Value, trace);
#endif
			if (trace.fraction != 1.f)
				continue;

			vPoints.emplace_back(vPoint);
		}
	}
}

void CAimbotProjectile::SetupSplashPoints(Vec3& vOrigin, std::vector<Setup_t>& vSplashPoints, uint8_t iFlags)
{
	vSplashPoints.clear();

	CGameTrace trace = {};
	CTraceFilterWorldAndPropsOnly filter = {};

	m_tInfo.m_pTarget->m_vPos = vOrigin;
	Vec3 vTargetEye = vOrigin + m_tInfo.m_vTargetEye;
	Vec3 vTargetCenter = vOrigin + m_tInfo.m_pTarget->m_pEntity->GetOffset() / 2;
	float flRadius = m_tInfo.m_flRadius + m_tInfo.m_pTarget->m_pEntity->GetSize().Length() / 2;
	bool bAirSplash = AirSplash(m_tInfo.m_pWeapon);

	auto fCheckNormal = [&](const Vec3& vNormal, const Vec3& vPoint, Vec3* pAngle = nullptr)
	{
		if (pAngle)
		{
			Vec3 vForward, vRight, vUp; Math::AngleVectors(*pAngle, &vForward, &vRight, &vUp);
			Vec3 vShootPos = m_tInfo.m_vLocalEye + vForward * m_tInfo.m_vOffset.x + vRight * m_tInfo.m_vOffset.y + vUp * m_tInfo.m_vOffset.z;
			vForward = (vShootPos - vPoint).Normalized();
			return vForward.Dot(vNormal) > 0;
		}
		else
		{
			Vec3 vForward = (m_tInfo.m_vLocalEye - vPoint).Normalized();
			return vForward.Dot(vNormal) > 0;
		}
	};

	// Trace
	int iPoints = Vars::Aimbot::Projectile::SplashMode.Value == Vars::Aimbot::Projectile::SplashModeEnum::Face && !bAirSplash ? 0
		: !m_tInfo.m_flGravity ? Vars::Aimbot::Projectile::SplashPointsDirect.Value : Vars::Aimbot::Projectile::SplashPointsArc.Value;
	{
		auto vPoints = ComputePoints(flRadius, iPoints);

		for (int i = 0; i < vPoints.size(); i++)
		{
			auto fCheckPointTrace = [&]()
			{
				if (!trace.m_pEnt || trace.fraction == 1.f || trace.surface.flags & SURF_SKY || !trace.m_pEnt->GetAbsVelocity().IsZero())
					return false;

				Vec3 vPoint = trace.endpos, vAngle;
				if (!m_tInfo.m_flGravity)
					vAngle = Math::CalcAngle(m_tInfo.m_vLocalEye, trace.endpos);
				else
				{
					Point_t tPoint = { vPoint, {} };
					CalculateAngle(m_tInfo.m_vLocalEye, tPoint.m_vPoint, 0, tPoint.m_tSolution, iFlags);
					if (tPoint.m_tSolution.m_iCalculated == CalculateResultEnum::Bad)
						return false;
					vPoint -= Vec3(0, 0, m_tInfo.m_flGravity * powf(tPoint.m_tSolution.m_flTime, 2) / 2);
					vAngle = Vec3(tPoint.m_tSolution.m_flPitch, tPoint.m_tSolution.m_flYaw);
				}
				return fCheckNormal(trace.plane.normal, vPoint, &vAngle);
			};

			auto fCheckPointAir = [&]()
			{
				if (bAirSplash && !trace.DidHit())
				{
					if (!Vars::Aimbot::Projectile::SplashAirCount.Value)
						vSplashPoints.emplace_back(vPoints[i] * SDK::StdRandomFloat() + vTargetCenter, PointTypeEnum::Air);
					else for (float r = 0; r < Vars::Aimbot::Projectile::SplashAirCount.Value; r++)
						vSplashPoints.emplace_back(vPoints[i] * r / Vars::Aimbot::Projectile::SplashAirCount.Value + vTargetCenter, PointTypeEnum::Air);
				}
				return Vars::Aimbot::Projectile::SplashMode.Value == Vars::Aimbot::Projectile::SplashModeEnum::Face && i || !trace.DidHit();
			};

			Vec3 vPoint = vPoints[i] + vTargetCenter;

			Solution_t tSolution; CalculateAngle(m_tInfo.m_vLocalEye, vPoint, 0, tSolution, iFlags);
			if (tSolution.m_iCalculated == CalculateResultEnum::Bad)
				continue;

			HandleTrace(vPoint, vSplashPoints, vTargetEye, m_tInfo, fCheckPointTrace, fCheckPointAir, trace, filter);
		}
	}

	// Face
	if (float flDensity = !m_tInfo.m_flGravity ? Vars::Aimbot::Projectile::SplashDensityDirect.Value : Vars::Aimbot::Projectile::SplashDensityArc.Value;
		Vars::Aimbot::Projectile::SplashMode.Value == Vars::Aimbot::Projectile::SplashModeEnum::Face && flDensity)
	{
		Vec3 vMins = vTargetCenter - flRadius, vMaxs = vTargetCenter + flRadius;

		NormalValidCallback fNormalValid = [&](const std::vector<Vec3>& vVertices, const Vec3& vNormal)
		{
			Vec3 vPoint = std::reduce(vVertices.begin(), vVertices.end()) / vVertices.size(), vAngle;
			if (!m_tInfo.m_flGravity)
				vAngle = Math::CalcAngle(m_tInfo.m_vLocalEye, vPoint);
			else
			{
				Point_t tPoint = { vPoint, {} };
				CalculateAngle(m_tInfo.m_vLocalEye, tPoint.m_vPoint, 0, tPoint.m_tSolution, iFlags);
				if (tPoint.m_tSolution.m_iCalculated == CalculateResultEnum::Bad)
					return false;
				vPoint -= Vec3(0, 0, m_tInfo.m_flGravity * powf(tPoint.m_tSolution.m_flTime, 2) / 2);
				vAngle = Vec3(tPoint.m_tSolution.m_flPitch, tPoint.m_tSolution.m_flYaw);
			}
			return fCheckNormal(vNormal, vPoint, &vAngle);
		};
		F::World.SetNormalValidCallback(&fNormalValid);
		std::vector<Face_t> vFaces = F::World.GetFacesInAABB(vMins, vMaxs, MASK_SOLID, &filter);
		F::World.SetNormalValidCallback();
#ifdef SPLASH_DEBUG5
		SDK::Output("Faces", std::format("{}", vFaces.size()).c_str(), {}, OUTPUT_CONSOLE);
#endif

		float flCutoff = Vars::Aimbot::Projectile::SplashSamplesCutoff.Value * powf(vFaces.size(), 2); s_flTotal = SDK::StdRandomFloat();
		for (auto& tFace : vFaces)
		{
			HandleFace(tFace, vSplashPoints, flDensity, flRadius, flCutoff, vTargetEye, vTargetCenter, vOrigin, m_tInfo, trace, filter);
//#if defined(SPLASH_DEBUG2) && defined(WORLD_DEBUG)
//			F::World.DrawFace(tFace, DrawTypeEnum::Edges | DrawTypeEnum::Faces);
//#endif
		}
	}
	
	if (vSplashPoints.size() > 1)
		std::shuffle(vSplashPoints.begin() + 1, vSplashPoints.end(), SDK::Random);

#ifdef SPLASH_DEBUG3
	for (auto& tSetup : vSplashPoints)
		G::BoxStorage.emplace_back(tSetup.m_vPoint, Vec3::Get(-1), Vec3::Get(1), Vec3(), I::GlobalVars->curtime + 60.f, Vars::Colors::Local.Value, Color_t(0, 0, 0, 0));
#endif
}

std::vector<Point_t> CAimbotProjectile::GetSplashPoints(Vec3 vOrigin, std::vector<Setup_t>& vSplashPoints, int iSimTime, uint8_t iFlags, bool bFirst)
{
	std::vector<Point_t> vPoints = {};

	m_tInfo.m_pTarget->m_vPos = vOrigin;
	Vec3 vTargetEye = vOrigin + m_tInfo.m_vTargetEye;
	float flRadiusSqr = powf(m_tInfo.m_flRadius, 2), flRadiusAirSqr = flRadiusSqr;
	if (Vars::Aimbot::Projectile::Modifiers.Value & Vars::Aimbot::Projectile::ModifiersEnum::AirSplash && m_tInfo.m_pWeapon->GetWeaponID() == TF_WEAPON_PIPEBOMBLAUNCHER)
	{
		static auto tf_grenadelauncher_livetime = H::ConVars.FindVar("tf_grenadelauncher_livetime");
		static auto tf_sticky_radius_ramp_time = H::ConVars.FindVar("tf_sticky_radius_ramp_time");
		static auto tf_sticky_airdet_radius = H::ConVars.FindVar("tf_sticky_airdet_radius");
		float flLiveTime = tf_grenadelauncher_livetime->GetFloat();
		float flRampTime = tf_sticky_radius_ramp_time->GetFloat();
		float flAirdetRadius = tf_sticky_airdet_radius->GetFloat();
		flRadiusAirSqr = powf(m_tInfo.m_flRadius * Math::RemapVal(TICKS_TO_TIME(iSimTime), flLiveTime, flLiveTime + flRampTime, flAirdetRadius, 1.f), 2);
	}
	bool bLob = iFlags & CalculateFlagsEnum::LobAngle;
	int iTolerance = m_tInfo.m_bIgnoreTiming && bLob ? std::numeric_limits<int>::max() : m_tInfo.m_iArmTime ? -1 : 0;
	int iLimit = !bFirst ? m_tInfo.m_iSplashRestrict : std::max(m_tInfo.m_iSplashRestrict, Vars::Aimbot::Projectile::SplashRestrictFirst.Value);
	bool bSort = !bLob || !m_tInfo.m_bIgnoreTiming;

	for (auto it = vSplashPoints.begin(); it != vSplashPoints.end();)
	{
		Point_t tPoint = { it->m_vPoint, {}, it->m_iType };

		CalculateAngle(m_tInfo.m_vLocalEye, tPoint.m_vPoint, iSimTime, tPoint.m_tSolution, iFlags, iTolerance);
		if (tPoint.m_tSolution.m_iCalculated != CalculateResultEnum::Good)
		{
			if (tPoint.m_tSolution.m_iCalculated == CalculateResultEnum::Bad)
				it = vSplashPoints.erase(it);
			else
				++it;
			continue;
		}
		else if (tPoint.m_iType == PointTypeEnum::Air && tPoint.m_tSolution.m_flTime < TICKS_TO_TIME(m_tInfo.m_iArmTime))
		{
			it = vSplashPoints.erase(it);
			continue;
		}

		vPoints.push_back(tPoint);
		it = vSplashPoints.erase(it);
		if (!bSort && vPoints.size() == iLimit)
			break;
	}
	if (vPoints.empty())
		return vPoints;

	if (bSort)
	{
		std::sort(vPoints.begin(), vPoints.end(), [&](const auto& a, const auto& b) -> bool
		{
			return a.m_vPoint.DistToSqr(vOrigin) < b.m_vPoint.DistToSqr(vOrigin);
		});
		vPoints.resize(std::min(iLimit, int(vPoints.size())));
	}

	const Vec3 vOriginal = m_tInfo.m_pTarget->m_pEntity->GetAbsOrigin();
	m_tInfo.m_pTarget->m_pEntity->SetAbsOrigin(vOrigin);
	for (auto it = vPoints.begin(); it != vPoints.end();)
	{
		auto& tPoint = *it;
		bool bValid = tPoint.m_tSolution.m_iCalculated != CalculateResultEnum::Pending;
		if (bValid)
		{
			Vec3 vPos = {}; m_tInfo.m_pTarget->m_pEntity->m_Collision()->CalcNearestPoint(tPoint.m_vPoint, &vPos);
			float flRadiusSqrCheck = tPoint.m_iType == PointTypeEnum::Geometry ? flRadiusSqr : flRadiusAirSqr;
			bValid = tPoint.m_vPoint.DistToSqr(vPos) < flRadiusSqrCheck;
		}

		if (bValid)
			++it;
		else
			it = vPoints.erase(it);
	}
	m_tInfo.m_pTarget->m_pEntity->SetAbsOrigin(vOriginal);

	return vPoints;
}

static inline Vec3 PullPoint(const Vec3& vPoint, Vec3 vLocalPos, Info_t& tInfo, const Vec3& vTargetPos, const Vec3& vMins, const Vec3& vMaxs)
{
	auto fHeightenLocalPos = [&]()
	{	// basic trajectory pass
		float flGrav = tInfo.m_flGravity;
		if (!flGrav)
			return vPoint;

		Vec3 vDelta = vTargetPos - vLocalPos;
		float flDist = vDelta.Length2D();

		float flRoot = powf(tInfo.m_flVelocity, 4) - flGrav * (flGrav * powf(flDist, 2) + 2.f * vDelta.z * powf(tInfo.m_flVelocity, 2));
		if (flRoot < 0.f)
			return vPoint;
		float flPitch = atan((powf(tInfo.m_flVelocity, 2) - sqrt(flRoot)) / (flGrav * flDist));

		float flTime = flDist / (cos(flPitch) * tInfo.m_flVelocity) - tInfo.m_flOffsetTime;
		return vLocalPos + Vec3(0, 0, flGrav * powf(flTime, 2) / 2);
	};

	vLocalPos = fHeightenLocalPos();
	Vec3 vForward, vRight, vUp; Math::AngleVectors(Math::CalcAngle(vLocalPos, vPoint), &vForward, &vRight, &vUp);
	vLocalPos += (vForward * tInfo.m_vOffset.x) + (vRight * tInfo.m_vOffset.y) + (vUp * tInfo.m_vOffset.z);
	return Math::PullPoint(vPoint, vLocalPos, vTargetPos, vMins, vMaxs);
}



static inline float GetDrag(float flVelocity, ProjectileInfo* pInfo, int iFlags)
{	// stupid, would like not to hardcode magic values
	if (!(iFlags & CalculateFlagsEnum::AccountDrag))
		return 0.f;

	static float flLastVelocity = 0.f, flRegularDrag = 0.f, flLobDrag = 0.f;
	if (flLastVelocity != flVelocity)
	{
		auto fGetDrag = [&](std::function<float()> fGetTypeDrag)
		{
			if (!F::ProjSim.m_bPhysics)
				return 0.f;

			if (Vars::Aimbot::Projectile::DragOverride.Value)
				return Vars::Aimbot::Projectile::DragOverride.Value;

			return fGetTypeDrag();
		};
		auto fGetRegularDrag = [&]()
		{
			switch (pInfo->m_uType)
			{
			case FNV1A::Hash32Const("models/weapons/w_models/w_grenade_grenadelauncher.mdl"):
				if (!SDK::AttribHookValue(0, "grenade_no_spin", pInfo->m_pWeapon))
					return Math::RemapVal(flVelocity, 1217.f, k_flMaxVelocity, 0.120f, 0.200f); // 0.120 normal, 0.200 capped, 0.300 v3000
				else
					return Math::RemapVal(flVelocity, 1217.f, k_flMaxVelocity, 0.060f, 0.085f); // 0.060 normal, 0.085 capped, 0.120 v3000
			case FNV1A::Hash32Const("models/weapons/w_models/w_cannonball.mdl"):
				return Math::RemapVal(flVelocity, 1454.f, k_flMaxVelocity, 0.385f, 0.530f); // 0.385 normal, 0.530 capped, 0.790 v3000
			case FNV1A::Hash32Const("models/weapons/w_models/w_stickybomb.mdl"):
				return Math::RemapVal(flVelocity, 922.f, k_flMaxVelocity, 0.090f, 0.190f); // 0.085 low, 0.190 capped, 0.230 v2400
			case FNV1A::Hash32Const("models/workshop_partner/weapons/c_models/c_sd_cleaver/c_sd_cleaver.mdl"):
				return 0.310f;
			case FNV1A::Hash32Const("models/weapons/w_models/w_baseball.mdl"):
				return 0.180f;
			case FNV1A::Hash32Const("models/weapons/c_models/c_xms_festive_ornament.mdl"):
				return 0.285f;
			case FNV1A::Hash32Const("models/weapons/c_models/urinejar.mdl"):
			case FNV1A::Hash32Const("models/workshop/weapons/c_models/c_madmilk/c_madmilk.mdl"):
			case FNV1A::Hash32Const("models/weapons/c_models/c_breadmonster/c_breadmonster.mdl"):
			case FNV1A::Hash32Const("models/weapons/c_models/c_breadmonster/c_breadmonster_milk.mdl"):
				return 0.057f;
			case FNV1A::Hash32Const("models/weapons/c_models/c_gascan/c_gascan.mdl"):
				return 0.530f;
			}
			return 0.f;
		};
		auto fGetLobDrag = [&]()
		{
			if (!(Vars::Aimbot::Projectile::Modifiers.Value & Vars::Aimbot::Projectile::ModifiersEnum::LobAngles))
				return 0.f;

			switch (pInfo->m_uType)
			{
			case FNV1A::Hash32Const("models/weapons/w_models/w_grenade_grenadelauncher.mdl"):
				if (!SDK::AttribHookValue(0, "grenade_no_spin", pInfo->m_pWeapon))
					return Math::RemapVal(flVelocity, 1217.f, k_flMaxVelocity, 0.056f, 0.062f);
				else
					return Math::RemapVal(flVelocity, 1217.f, k_flMaxVelocity, 0.030f, 0.033f);
			case FNV1A::Hash32Const("models/weapons/w_models/w_cannonball.mdl"):
				return Math::RemapVal(flVelocity, 1454.f, k_flMaxVelocity, 0.099f, 0.092f);
			case FNV1A::Hash32Const("models/weapons/w_models/w_stickybomb.mdl"):
				return Math::RemapVal(flVelocity, 922.f, k_flMaxVelocity, 0.048f, 0.060f);
			case FNV1A::Hash32Const("models/workshop_partner/weapons/c_models/c_sd_cleaver/c_sd_cleaver.mdl"):
				return 0.075f;
			case FNV1A::Hash32Const("models/weapons/w_models/w_baseball.mdl"):
				return 0.057f;
			case FNV1A::Hash32Const("models/weapons/c_models/c_xms_festive_ornament.mdl"):
				return 0.072f;
			case FNV1A::Hash32Const("models/weapons/c_models/urinejar.mdl"):
			case FNV1A::Hash32Const("models/workshop/weapons/c_models/c_madmilk/c_madmilk.mdl"):
			case FNV1A::Hash32Const("models/weapons/c_models/c_breadmonster/c_breadmonster.mdl"):
			case FNV1A::Hash32Const("models/weapons/c_models/c_breadmonster/c_breadmonster_milk.mdl"):
				return 0.030f;
			case FNV1A::Hash32Const("models/weapons/c_models/c_gascan/c_gascan.mdl"):
				return 0.089f;
			}
			return 0.f;
		};

		flLastVelocity = flVelocity;
		flRegularDrag = fGetDrag(fGetRegularDrag);
		flLobDrag = fGetDrag(fGetLobDrag);
	}

	return !(iFlags & CalculateFlagsEnum::LobAngle) ? flRegularDrag : flLobDrag;
}

static inline bool GetAngle(const Vec3& vLocalPos, const Vec3& vTargetPos, int iFlags, Info_t& tInfo, float& flPitch, float& flYaw, float& flTime)
{
	float flVelocity = tInfo.m_flVelocity;
	float flGrav = tInfo.m_flGravity;
	Vec3 vDelta = vTargetPos - vLocalPos;
	float flDist = vDelta.Length2D();
	float flDrag = GetDrag(flVelocity, F::ProjSim.m_pCurrent, iFlags);

	Vec3 vAngleTo = Math::CalcAngle(vLocalPos, vTargetPos);

	flYaw = vAngleTo.y;

	if (!flGrav)
		flPitch = -Math::Deg2Rad(vAngleTo.x);
	else
	{
		float flRoot = powf(flVelocity, 4) - flGrav * (flGrav * powf(flDist, 2) + 2.f * vDelta.z * powf(flVelocity, 2));
		if (flRoot < 0.f)
			return false;

		if (!(iFlags & CalculateFlagsEnum::LobAngle))
			flPitch = atan((powf(flVelocity, 2) - sqrt(flRoot)) / (flGrav * flDist));
		else
			flPitch = atan((powf(flVelocity, 2) + sqrt(flRoot)) / (flGrav * flDist));

		if (flDrag)
		{	// drag is handled in a stupid manner, would like something better than this
			float flTime = flDist / (flVelocity * cos(flPitch));
			flVelocity *= 1 - flDrag * flTime;

			flRoot = powf(flVelocity, 4) - flGrav * (flGrav * powf(flDist, 2) + 2.f * vDelta.z * powf(flVelocity, 2));
			if (flRoot < 0.f)
				return false;

			if (!(iFlags & CalculateFlagsEnum::LobAngle))
				flPitch = atan((powf(flVelocity, 2) - sqrt(flRoot)) / (flGrav * flDist));
			else
				flPitch = atan((powf(flVelocity, 2) + sqrt(flRoot)) / (flGrav * flDist));

			//flVelocity = flVelocity / (flDrag * flTime) * (1.f - powf(Math::E, -flDrag * flTime));
			flVelocity = flVelocity / (1 + 0.5f * (1 - powf(Math::E, -flDrag * flTime)));
			flVelocity *= 1 + vDelta.z / flVelocity * TICK_INTERVAL * flTime;
		}
	}

	flTime = flDist / (flVelocity * cos(flPitch));

	if (Vars::Aimbot::Projectile::TimeOverride.Value)
		flTime *= Vars::Aimbot::Projectile::TimeOverride.Value;

	return true;
}

void CAimbotProjectile::CalculateAngle(const Vec3& vLocalPos, const Vec3& vTargetPos, int iSimTime, Solution_t& tOut, uint8_t iFlags, int iTolerance)
{
	if (tOut.m_iCalculated != CalculateResultEnum::Pending)
		return;

	float flPitch, flYaw;
	{	// basic trajectory pass
		//Vec3 vForward, vRight, vUp; Math::AngleVectors(Math::CalcAngle(vLocalPos, vTargetPos), &vForward, &vRight, &vUp);
		//Vec3 vShootPos = vLocalPos + vForward * m_tInfo.m_vOffset.x + vRight * m_tInfo.m_vOffset.y + vUp * m_tInfo.m_vOffset.z;
		if (!GetAngle(vLocalPos, vTargetPos, iFlags, m_tInfo, flPitch, flYaw, tOut.m_flTime))
		{
			tOut.m_iCalculated = CalculateResultEnum::Bad;
			return;
		}

		tOut.m_flTime -= m_tInfo.m_flOffsetTime;
		tOut.m_flPitch = flPitch = -Math::Rad2Deg(flPitch) - m_tInfo.m_vAngFix.x;
		tOut.m_flYaw = flYaw -= m_tInfo.m_vAngFix.y;
	}

	int iTimeTo = ceilf(tOut.m_flTime / TICK_INTERVAL);
	bool bGood = iTolerance != -1 ? abs(iTimeTo - iSimTime) <= iTolerance : iTimeTo <= iSimTime;
	if (!(iFlags & CalculateFlagsEnum::TwoPass) || m_tInfo.m_vOffset.IsZero())
	{
		tOut.m_iCalculated = bGood ? CalculateResultEnum::Good : CalculateResultEnum::Time;
		return;
	}
	else if (!bGood)
	{
		tOut.m_iCalculated = CalculateResultEnum::Time;
		return;
	}

	int iSimFlags = (iFlags & CalculateFlagsEnum::SetupClip ? ProjSimEnum::Redirect : ProjSimEnum::None) | ProjSimEnum::NoRandomAngles | ProjSimEnum::PredictCmdNum;
#ifdef SPLASH_DEBUG5
	if (iSimFlags & ProjSimEnum::Redirect)
	{
		if (Vars::Visuals::Trajectory::Override.Value)
		{
			if (Vars::Visuals::Trajectory::ForwardRedirect.Value)
				s_mTraceCount[__FUNCTION__": setup trace"]++;
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
				s_mTraceCount[__FUNCTION__": setup trace"]++;
			}
		}
	}
#endif
	m_tProjInfo = {};
	if (!F::ProjSim.GetInfo(m_tInfo.m_pLocal, m_tInfo.m_pWeapon, { flPitch, flYaw, 0 }, m_tProjInfo, iSimFlags))
	{
		tOut.m_iCalculated = CalculateResultEnum::Bad;
		return;
	}

	{	// calculate trajectory from projectile origin
		if (!GetAngle(m_tProjInfo.m_vPos, vTargetPos, iFlags, m_tInfo, tOut.m_flPitch, tOut.m_flYaw, tOut.m_flTime))
		{
			tOut.m_iCalculated = CalculateResultEnum::Bad;
			return;
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
				tOut.m_flYaw = flYaw - (Math::Rad2Deg(atan2(vShootPos.y, vShootPos.x)) - flYaw);
				flYaw = Math::Rad2Deg(atan2(vShootPos.y, vShootPos.x));
			}
		}

		{	// correct pitch
			if (m_tInfo.m_flGravity)
			{
				flPitch -= m_tProjInfo.m_vAng.x;
				tOut.m_flPitch = -Math::Rad2Deg(tOut.m_flPitch) + flPitch - m_tInfo.m_vAngFix.x;
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
					tOut.m_flPitch = flPitch - (Math::Rad2Deg(atan2(-vShootPos.z, vShootPos.x)) - flPitch);
				}
			}
		}
	}

	iTimeTo = ceilf(tOut.m_flTime / TICK_INTERVAL);
	bGood = iTolerance != -1 ? abs(iTimeTo - iSimTime) <= iTolerance : iTimeTo <= iSimTime;
	tOut.m_iCalculated = bGood ? CalculateResultEnum::Good : CalculateResultEnum::Time;
}



bool CAimbotProjectile::TestAngle(const Vec3& vPoint, const Vec3& vAngles, int iSimTime, uint8_t iType, uint8_t iFlags, bool bSecondTest)
{
	auto pLocal = m_tInfo.m_pLocal;
	auto pWeapon = m_tInfo.m_pWeapon;
	auto& tTarget = *m_tInfo.m_pTarget;

	int iSimFlags = ProjSimEnum::Redirect | ProjSimEnum::InitCheck | ProjSimEnum::PredictCmdNum | (Vars::Aimbot::General::NoSpread.Value ? ProjSimEnum::CorrectRandomAngles : ProjSimEnum::NoRandomAngles);
#ifdef SPLASH_DEBUG5
	if (Vars::Visuals::Trajectory::Override.Value)
	{
		if (Vars::Visuals::Trajectory::ForwardRedirect.Value)
			s_mTraceCount[__FUNCTION__": setup trace"]++;
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
			s_mTraceCount[__FUNCTION__": setup trace"]++;
		}
	}
	s_mTraceCount[std::format(__FUNCTION__": setup clip ({}, {})", iType, iFlags)]++;
#endif
	m_tProjInfo = {};
	if (!F::ProjSim.GetInfo(pLocal, pWeapon, vAngles, m_tProjInfo, iSimFlags)
		|| !F::ProjSim.Initialize(m_tProjInfo))
		return false;

	CGameTrace trace = {};
	CTraceFilterCollideable filter = {};
	filter.pSkip = iType == PointTypeEnum::Direct ? pLocal : tTarget.m_pEntity;
	filter.iPlayer = iType == PointTypeEnum::Direct ? PLAYER_DEFAULT : PLAYER_NONE;
	filter.bMisc = iType == PointTypeEnum::Direct;
	int nMask = MASK_SOLID;
	if (iType == PointTypeEnum::Direct && F::AimbotGlobal.FriendlyFire())
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
	G::BoxStorage.emplace_back(vPoint, -vHull, vHull, Vec3(), I::GlobalVars->curtime + 5.f, Color_t(0, 0, 0), Color_t(0, 0, 0, 0));
#endif

	if (!m_tProjInfo.m_flGravity)
	{
		SDK::TraceHull(m_tProjInfo.m_vPos, vPoint, -m_tProjInfo.m_vHull, m_tProjInfo.m_vHull, nMask, &filter, &trace);
#ifdef SPLASH_DEBUG5
		s_mTraceCount[__FUNCTION__": nograv trace"]++;
#endif
#ifdef SPLASH_DEBUG4
		G::LineStorage.emplace_back(std::pair<Vec3, Vec3>(m_tProjInfo.m_vPos, trace.endpos), I::GlobalVars->curtime + 5.f, Color_t(0, 0, 0));
#endif
		if (Math::FullFraction(m_tProjInfo.m_vPos, vPoint, trace) < 0.999f && trace.m_pEnt != tTarget.m_pEntity)
			return false;
	}

#ifdef SPLASH_DEBUG4
	G::BoxStorage.pop_back();
#endif

	bool bDidHit = false;
	Vec3 vNew = F::ProjSim.GetOrigin();
	int iTimingTolerance = TIME_TO_TICKS(m_tInfo.m_flBoundsTime);
	float flRadiusSqr = iType != PointTypeEnum::Direct ? powf(m_tProjInfo.m_flVelocity * TICK_INTERVAL + m_tProjInfo.m_vHull.z, 2) : std::numeric_limits<float>::max();
	uint8_t iTraceInterval = iFlags == PointFlagsEnum::Lob ? Vars::Aimbot::Projectile::LobTraceInterval.Value
		: iType != PointTypeEnum::Direct ? Vars::Aimbot::Projectile::SplashTraceInterval.Value
		: Vars::Aimbot::Projectile::DirectTraceInterval.Value;

	const RestoreInfo_t tOriginal = { tTarget.m_pEntity->GetAbsOrigin(), tTarget.m_pEntity->m_vecMins(), tTarget.m_pEntity->m_vecMaxs() };
	tTarget.m_pEntity->SetAbsOrigin(tTarget.m_vPos);
	tTarget.m_pEntity->m_vecMins() = { std::clamp(tTarget.m_pEntity->m_vecMins().x, -24.f, 0.f), std::clamp(tTarget.m_pEntity->m_vecMins().y, -24.f, 0.f), tTarget.m_pEntity->m_vecMins().z };
	tTarget.m_pEntity->m_vecMaxs() = { std::clamp(tTarget.m_pEntity->m_vecMaxs().x, 0.f, 24.f), std::clamp(tTarget.m_pEntity->m_vecMaxs().y, 0.f, 24.f), tTarget.m_pEntity->m_vecMaxs().z };
	for (int n = 1; n <= iSimTime; n++)
	{
		F::ProjSim.RunTick(m_tProjInfo);

		if (bDidHit)
		{
			trace.endpos = F::ProjSim.GetOrigin();
			continue;
		}
		if (iTraceInterval != 1 && n % iTraceInterval && n != iSimTime)
			continue;

		Vec3 vOld = vNew; vNew = F::ProjSim.GetOrigin();
		SDK::TraceHull(vOld, vNew, -m_tProjInfo.m_vHull, m_tProjInfo.m_vHull, nMask, &filter, &trace);
#ifdef SPLASH_DEBUG5
		s_mTraceCount[std::format(__FUNCTION__": trace ({})", iTraceInterval)]++;
#endif
#ifdef SPLASH_DEBUG4
		G::LineStorage.emplace_back(std::pair<Vec3, Vec3>(vOld, trace.endpos), I::GlobalVars->curtime + 5.f, Color_t(255, 0, 0));
#endif

		bool bHit = false;
		switch (iType)
		{
		case PointTypeEnum::Direct:
		case PointTypeEnum::Geometry:
			bHit = trace.DidHit(); break;
		case PointTypeEnum::Air:
			bHit = trace.endpos.DistToSqr(vPoint) < flRadiusSqr || trace.DidHit(); break;
		}

		if (bHit)
		{
			bool bValid = false, bTarget = true;
			switch (iType)
			{
			case PointTypeEnum::Direct:
				bTarget = trace.m_pEnt == tTarget.m_pEntity, bValid = bTarget && iSimTime - n < iTimingTolerance; break;
			case PointTypeEnum::Geometry:
				bValid = trace.endpos.DistToSqr(vPoint) < flRadiusSqr; break;
			case PointTypeEnum::Air:
				bValid = !trace.DidHit(); break;
			}
			if (bValid && iType != PointTypeEnum::Direct)
			{
				CGameTrace eyeTrace = {};
				SDK::Trace(trace.endpos + trace.plane.normal * m_tInfo.m_flNormalOffset, tTarget.m_vPos + m_tInfo.m_vTargetEye, MASK_SHOT, &filter, &eyeTrace);
				bValid = eyeTrace.fraction == 1.f;
#ifdef SPLASH_DEBUG5
				s_mTraceCount[__FUNCTION__": splash eye trace"]++;
#endif
#ifdef SPLASH_DEBUG4
				//G::LineStorage.emplace_back(std::pair<Vec3, Vec3>(trace.endpos, trace.endpos + trace.plane.normal * m_tInfo.m_flNormalOffset), I::GlobalVars->curtime + 5.f, Color_t(255, 0, 255));
				G::LineStorage.emplace_back(std::pair<Vec3, Vec3>(eyeTrace.startpos, eyeTrace.endpos), I::GlobalVars->curtime + 5.f, Color_t(255, 0, 255));
				G::BoxStorage.emplace_back(trace.endpos + m_tInfo.m_flNormalOffset * trace.plane.normal, -vHull, vHull, Vec3(), I::GlobalVars->curtime + 5.f, Color_t(255, 0, 255), Color_t(0, 0, 0, 0));
#endif
			}

#ifdef SPLASH_DEBUG4
			if (bValid)
				G::BoxStorage.emplace_back(vPoint, -vHull, vHull, Vec3(), I::GlobalVars->curtime + 5.f, Color_t(0, 255, 0), Color_t(0, 0, 0, 0));
			else
				G::BoxStorage.emplace_back(vPoint, -vHull, vHull, Vec3(), I::GlobalVars->curtime + 5.f, Color_t(255, 0, 0), Color_t(0, 0, 0, 0));
#endif

			if (bValid)
			{
				if (iTraceInterval != 1 && iType != PointTypeEnum::Direct)
				{
					int iInterval = n % iTraceInterval ? n % iTraceInterval : iTraceInterval;
					int iPopCount = ceilf(iInterval - trace.fraction * iInterval);
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

					float flLowestDistance = std::numeric_limits<float>::max(); int iClosest = -1;
					for (int nHitbox = 0; nHitbox < pSet->numhitboxes; ++nHitbox)
					{
						auto pBox = pSet->pHitbox(nHitbox);
						if (!pBox) continue;

						Vec3 vCenter; Math::VectorTransform({}, aBones[pBox->bone], vCenter);

						const float flDistance = vPos.DistToSqr(vCenter);
						if (flDistance < flLowestDistance)
							iClosest = nHitbox, flLowestDistance = flDistance;
					}
					if (iClosest != HITBOX_HEAD)
						break;
				}

				bDidHit = true;
			}
			else if (bTarget && iType == PointTypeEnum::Direct && m_tInfo.m_iArmTime)
			{	// run for more ticks to check for splash
				iSimTime = n + iTimingTolerance / 2;
				iType = PointTypeEnum::Geometry;
				filter.pSkip = tTarget.m_pEntity;
				filter.iPlayer = PLAYER_NONE;
				filter.bMisc = false;
				continue;
			}
			else
				break;

			if (iType == PointTypeEnum::Direct)
				trace.endpos = vNew;

			if (!bTarget || iType != PointTypeEnum::Direct)
				break;
		}
	}
	tTarget.m_pEntity->SetAbsOrigin(tOriginal.m_vOrigin);
	tTarget.m_pEntity->m_vecMins() = tOriginal.m_vMins;
	tTarget.m_pEntity->m_vecMaxs() = tOriginal.m_vMaxs;
	m_tProjInfo.m_vPath.push_back(trace.endpos);

	return bDidHit;
}

bool CAimbotProjectile::HandlePoint(const Vec3& vOrigin, int iSimTime, float flPitch, float flYaw, float flTime, const Vec3& vPoint, uint8_t iType, uint8_t iFlags)
{
	bool bReturn = false;

	Vec3 vAngles; Aim(G::CurrentUserCmd->viewangles, { flPitch, flYaw, 0.f }, vAngles);
	m_tInfo.m_pTarget->m_vPos = vOrigin;

	int iOriginalSimTime = iSimTime;
	if (m_tInfo.m_iArmTime && iType == PointTypeEnum::Geometry || m_tInfo.m_bIgnoreTiming && iFlags == PointFlagsEnum::Lob)
	{
		if (flTime > m_tProjInfo.m_flLifetime)
			return false;

		iSimTime = std::ceil(flTime / I::GlobalVars->interval_per_tick); //TIME_TO_TICKS(flTime);
	}

	if (!m_tInfo.m_pProjectile
		? TestAngle(vPoint, vAngles, iSimTime, iType, iFlags)
		: TestAngle(m_tInfo.m_pProjectile, vPoint, vAngles, iSimTime, iType, iFlags))
	{
		bReturn = m_iResult = true;
		m_flTimeTo = flTime + m_tInfo.m_flLatency;
	}
	else if (!m_iResult && !m_tInfo.m_pProjectile)
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
			if (TestAngle(vPoint, vPlainAngles, iSimTime, iType, true))
				bReturn = m_iResult = 2;
		}
		}
	}

	if (bReturn && m_bUpdate)
	{
		m_vAngleTo = vAngles, m_vTarget = vPoint, m_vPredicted = vOrigin;
		m_vPlayerPath.clear(), m_vProjectilePath = m_tProjInfo.m_vPath;
		if (m_tMoveStorage.m_vPath.empty())
			return true;

		if (m_tInfo.m_iArmTime && iType != PointTypeEnum::Air)
		{
			int iCount = std::min(iOriginalSimTime + TIME_TO_TICKS(m_tInfo.m_flLatency) + 1, int(m_tMoveStorage.m_vPath.size()));
			m_vPlayerPath.insert(m_vPlayerPath.end(), m_tMoveStorage.m_vPath.begin(), m_tMoveStorage.m_vPath.begin() + iCount);
		}
		else
		{
			size_t iCount = std::min(m_tProjInfo.m_vPath.size() + TIME_TO_TICKS(m_tInfo.m_flLatency), m_tMoveStorage.m_vPath.size());
			m_vPlayerPath.insert(m_vPlayerPath.end(), m_tMoveStorage.m_vPath.begin(), m_tMoveStorage.m_vPath.begin() + iCount);
			m_vPredicted = m_vPlayerPath.back();
		}
	}

	return bReturn && m_iResult == 1;
}

bool CAimbotProjectile::HandleDirect(DirectHistory_t& mDirectHistory)
{
	bool bReturn = false;
	if (mDirectHistory.empty())
		return bReturn;

	auto it = mDirectHistory.begin();
	uint8_t iType = it->first;
	auto& vDirectHistory = it->second;

	std::sort(vDirectHistory.begin(), vDirectHistory.end(), [&](const Direct_t& a, const Direct_t& b) -> bool
	{
		return a.m_iPriority < b.m_iPriority;
	});
	m_flTimeTo = vDirectHistory.front().m_flTime + m_tInfo.m_flLatency;

	for (auto& tHistory : vDirectHistory)
	{
		if (HandlePoint(tHistory.m_vOrigin, tHistory.m_iSimtime, tHistory.m_flPitch, tHistory.m_flYaw, tHistory.m_flTime, tHistory.m_vPoint, PointTypeEnum::Direct, iType))
		{
			bReturn = true;
			break;
		}
	}

	mDirectHistory.erase(it);
	return bReturn;
}

bool CAimbotProjectile::HandleSplash(SplashHistory_t& mSplashHistory)
{
	bool bReturn = false;
	if (mSplashHistory.empty())
		return bReturn;

	auto it = mSplashHistory.begin();
	uint8_t iType = it->first;
	auto& vSplashHistory = it->second;

	std::sort(vSplashHistory.begin(), vSplashHistory.end(), [&](const Splash_t& a, const Splash_t& b) -> bool
	{
		return a.m_flTimeTo < b.m_flTimeTo;
	});
	uint8_t iFlags = CalculateFlagsEnum::None;
	if (iType == PointFlagsEnum::Lob)
		iFlags |= CalculateFlagsEnum::LobAngle;
	SetupSplashPoints(vSplashHistory.front().m_vOrigin, m_vSplashPoints, iFlags);
	if (!m_vSplashPoints.empty())
	{
		iFlags |= CalculateFlagsEnum::Accuracy;
		float flLowestDistance = std::numeric_limits<float>::max(); bool bFirst = true;
		for (auto& tHistory : vSplashHistory)
		{
			std::vector<Point_t> vSplashPoints = {};
			vSplashPoints = GetSplashPoints(tHistory.m_vOrigin, m_vSplashPoints, tHistory.m_iSimtime, iFlags, bFirst); bFirst = false;

			for (auto& tPoint : vSplashPoints)
			{
				float flDistance = tHistory.m_vOrigin.DistToSqr(tPoint.m_vPoint);
				if (flDistance > flLowestDistance)
					continue;

				if (HandlePoint(tHistory.m_vOrigin, tHistory.m_iSimtime, tPoint.m_tSolution.m_flPitch, tPoint.m_tSolution.m_flYaw, tPoint.m_tSolution.m_flTime, tPoint.m_vPoint, tPoint.m_iType, iType))
				{
					bReturn = true;
					flLowestDistance = flDistance;
				}
			}
			if (m_tInfo.m_bIgnoreTiming && iType == PointFlagsEnum::Lob)
				break;
		}
	}
	
	mSplashHistory.erase(it);
	return bReturn;
}

int CAimbotProjectile::CanHit(Target_t& tTarget, CTFPlayer* pLocal, CTFWeaponBase* pWeapon, bool bUpdate)
{
	//if (Vars::Aimbot::General::Ignore.Value & Vars::Aimbot::General::IgnoreEnum::Unsimulated && H::Entities.GetChoke(tTarget.m_pEntity->entindex()) > Vars::Aimbot::General::TickTolerance.Value)
	//	return false;

	m_tMoveStorage = {};
	if (!F::MoveSim.Initialize(tTarget.m_pEntity, m_tMoveStorage) && tTarget.m_iTargetType == TargetEnum::Player)
		return false;

	m_tProjInfo = {};
	if (!F::ProjSim.GetInfo(pLocal, pWeapon, {}, m_tProjInfo, ProjSimEnum::NoRandomAngles | ProjSimEnum::PredictCmdNum)
		|| !F::ProjSim.Initialize(m_tProjInfo, false))
		return false;

	m_tInfo = { pLocal, pWeapon, &tTarget };
	m_tInfo.m_vLocalEye = pLocal->GetShootPos();
	m_tInfo.m_vTargetEye = tTarget.m_pEntity->As<CTFPlayer>()->GetViewOffset();
	m_tInfo.m_flLatency = F::Backtrack.GetReal() + TICKS_TO_TIME(F::Backtrack.GetAnticipatedChoke());
	tTarget.m_vPos = tTarget.m_pEntity->m_vecOrigin();

	Vec3 vVelocity = F::ProjSim.GetVelocity();
	m_tInfo.m_flVelocity = vVelocity.Length();
	m_tInfo.m_vAngFix = Math::VectorAngles(vVelocity);

	m_tInfo.m_vHull = m_tProjInfo.m_vHull.Min(3);
	m_tInfo.m_vOffset = m_tProjInfo.m_vPos - m_tInfo.m_vLocalEye; m_tInfo.m_vOffset.y *= -1;
	m_tInfo.m_flOffsetTime = m_tInfo.m_vOffset.Length() / m_tInfo.m_flVelocity;

	m_tInfo.m_flGravity = m_tProjInfo.m_flGravity;
	m_tInfo.m_iSplashRestrict = !m_tInfo.m_flGravity ? Vars::Aimbot::Projectile::SplashRestrictDirect.Value : Vars::Aimbot::Projectile::SplashRestrictArc.Value;
	m_tInfo.m_flRadius = GetSplashRadius(pWeapon, pLocal, Vars::Aimbot::Projectile::SplashRadius.Value / 100);
	m_tInfo.m_flBoundsTime = tTarget.m_pEntity->GetSize().Length() / m_tInfo.m_flVelocity;
	m_tInfo.m_flRadiusTime = m_tInfo.m_flBoundsTime + m_tInfo.m_flRadius / m_tInfo.m_flVelocity;
	m_tInfo.m_iArmTime = TIME_TO_TICKS(ArmTime(pWeapon));
	switch (pWeapon->GetWeaponID())
	{
	case TF_WEAPON_ROCKETLAUNCHER:
	case TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT:
	case TF_WEAPON_PARTICLE_CANNON:
		m_tInfo.m_flNormalOffset = 1.f;
	}
	m_tInfo.m_bIgnoreTiming = Vars::Aimbot::Projectile::LobAnglesUnderpredict.Value && m_tInfo.m_flRadius;



	Directs_t mDirects = GetDirects();
	Splashes_t vSplashes = GetSplashes();

	DirectHistory_t mDirectHistory = {};
	SplashHistory_t mSplashHistory = {};

	int iMaxTime = TIME_TO_TICKS(std::min(m_tProjInfo.m_flLifetime, Vars::Aimbot::Projectile::MaxSimulationTime.Value));
	for (int i = 1 - TIME_TO_TICKS(m_tInfo.m_flLatency); i <= iMaxTime; i++)
	{
		if (!m_tMoveStorage.m_bFailed)
		{
			F::MoveSim.RunTick(m_tMoveStorage);
			tTarget.m_vPos = m_tMoveStorage.m_vPredictedOrigin;
		}
		if (i < 0)
			continue;

		for (auto& [iIndex, tOffset] : mDirects)
		{
			if (m_tInfo.m_iArmTime && m_tInfo.m_iArmTime > i && !m_tMoveStorage.m_MoveData.m_vecVelocity.IsZero())
				break;

			Vec3& vOffset = tOffset.m_vOffset;
			uint8_t iType = tOffset.m_iFlags & -tOffset.m_iFlags;

			Vec3 vPoint = tTarget.m_vPos + vOffset;
			if (Vars::Aimbot::Projectile::HuntsmanPullPoint.Value && tTarget.m_nAimedHitbox == HITBOX_HEAD)
			{
				vPoint = PullPoint(vPoint, m_tInfo.m_vLocalEye, m_tInfo, tTarget.m_vPos, tTarget.m_pEntity->m_vecMins() + m_tInfo.m_vHull, tTarget.m_pEntity->m_vecMaxs() - m_tInfo.m_vHull);
				if (Vars::Aimbot::Projectile::HuntsmanPullNoZ.Value)
					vPoint.z = tTarget.m_vPos.z + vOffset.z;
			}

			uint8_t iFlags = CalculateFlagsEnum::Accuracy;
			if (iType == PointFlagsEnum::Lob)
				iFlags |= CalculateFlagsEnum::LobAngle;
			int iTolerance = m_tInfo.m_bIgnoreTiming && iType == PointFlagsEnum::Lob ? std::numeric_limits<int>::max() : -1;

			Solution_t tSolution;
			switch (iType)
			{
			case PointFlagsEnum::Lob:
				if (ShouldLob(m_tMoveStorage, m_tInfo))
					goto end;
				tSolution.m_iCalculated = CalculateResultEnum::Bad; break;
			default: end:
				CalculateAngle(m_tInfo.m_vLocalEye, vPoint, i, tSolution, iFlags, iTolerance);
			}
			switch (tSolution.m_iCalculated)
			{
			case CalculateResultEnum::Good:
				mDirectHistory[iType].emplace_back(History_t(tTarget.m_vPos, i), tSolution.m_flPitch, tSolution.m_flYaw, tSolution.m_flTime, vPoint, iIndex);
				[[fallthrough]];
			case CalculateResultEnum::Bad:
				tOffset.m_iFlags &= ~iType;
				if (!(tOffset.m_iFlags /*& (PointFlagsEnum::Regular | PointFlagsEnum::Lob)*/))
					mDirects.erase(iIndex);
			}
		}

		for (auto it = vSplashes.begin(); it != vSplashes.end();)
		{
			uint8_t iFlags = CalculateFlagsEnum::AccountDrag;
			if (*it == PointFlagsEnum::Lob && !m_tInfo.m_bIgnoreTiming)
				iFlags |= CalculateFlagsEnum::LobAngle;

			Solution_t tSolution; CalculateAngle(m_tInfo.m_vLocalEye, tTarget.m_vPos, i, tSolution, iFlags);
			if (tSolution.m_iCalculated == CalculateResultEnum::Bad && mDirects.empty())
			{
				it = vSplashes.erase(it);
				continue;
			}

			const float flTimeTo = tSolution.m_flTime - TICKS_TO_TIME(i);
			if (flTimeTo > m_tInfo.m_flRadiusTime || m_tInfo.m_iArmTime && m_tInfo.m_iArmTime > i)
			{
				++it;
				continue;
			}
			if (flTimeTo < -m_tInfo.m_flRadiusTime && (!m_tInfo.m_iArmTime || m_tInfo.m_iArmTime < i))
			{
				it = vSplashes.erase(it);
				continue;
			}
			if (*it == PointFlagsEnum::Lob && !ShouldLob(m_tMoveStorage, m_tInfo))
			{
				++it;
				continue;
			}

			mSplashHistory[*it].emplace_back(History_t(tTarget.m_vPos, i), fabsf(flTimeTo));
			++it;
		}

		if (mDirects.empty() && vSplashes.empty())
			break;
	}

	m_iResult = false, m_bUpdate = bUpdate;
	if (!m_tInfo.m_flRadius || Vars::Aimbot::Projectile::SplashPrediction.Value < Vars::Aimbot::Projectile::SplashPredictionEnum::Prefer)
		goto direct;
	else
		goto splash;
	while (!mDirectHistory.empty() || !mSplashHistory.empty())
	{
		direct: if (HandleDirect(mDirectHistory)) break;
		splash: if (HandleSplash(mSplashHistory)) break;
	}
	F::MoveSim.Restore(m_tMoveStorage);
	if (!bUpdate)
		return m_iResult;

	tTarget.m_vPos = m_vTarget;
	tTarget.m_vAngleTo = m_vAngleTo;

	bool bMain = m_iResult == 1;
	bool bAny = m_iResult;
	if (bAny && (Vars::Colors::BoundHitboxEdge.Value.a || Vars::Colors::BoundHitboxFace.Value.a || Vars::Colors::BoundHitboxEdgeIgnoreZ.Value.a || Vars::Colors::BoundHitboxFaceIgnoreZ.Value.a))
	{
		float flProjectileTime = 0.f, flTargetTime = 0.f;
		bool bBox = Vars::Visuals::Hitbox::BoundsEnabled.Value & Vars::Visuals::Hitbox::BoundsEnabledEnum::OnShot;
		bool bPoint = Vars::Visuals::Hitbox::BoundsEnabled.Value & Vars::Visuals::Hitbox::BoundsEnabledEnum::AimPoint;
		bool bTimed = !Vars::Visuals::Prediction::PlayerDrawDuration.Value;
		if (bTimed)
		{
			flProjectileTime = TICKS_TO_TIME(m_vProjectilePath.size());
			flTargetTime = m_tMoveStorage.m_bFailed ? flProjectileTime : TICKS_TO_TIME(m_vPlayerPath.size());
		}
		if (bBox)
		{
			float flDuration = bTimed ? flTargetTime : Vars::Visuals::Hitbox::DrawDuration.Value;
			if (Vars::Colors::BoundHitboxEdgeIgnoreZ.Value.a || Vars::Colors::BoundHitboxFaceIgnoreZ.Value.a)
				m_vBoxes.emplace_back(m_vPredicted, tTarget.m_pEntity->m_vecMins(), tTarget.m_pEntity->m_vecMaxs(), Vec3(), I::GlobalVars->curtime + flDuration, Vars::Colors::BoundHitboxEdgeIgnoreZ.Value, Vars::Colors::BoundHitboxFaceIgnoreZ.Value);
			if (Vars::Colors::BoundHitboxEdge.Value.a || Vars::Colors::BoundHitboxFace.Value.a)
				m_vBoxes.emplace_back(m_vPredicted, tTarget.m_pEntity->m_vecMins(), tTarget.m_pEntity->m_vecMaxs(), Vec3(), I::GlobalVars->curtime + flDuration, Vars::Colors::BoundHitboxEdge.Value, Vars::Colors::BoundHitboxFace.Value, true);
		}
		if (bMain && bPoint)
		{
			float flDuration = bTimed ? flProjectileTime : Vars::Visuals::Hitbox::DrawDuration.Value;
			if (Vars::Colors::BoundHitboxEdgeIgnoreZ.Value.a || Vars::Colors::BoundHitboxFaceIgnoreZ.Value.a)
				m_vBoxes.emplace_back(m_vTarget, Vec3::Get(-1), Vec3::Get(1), Vec3(), I::GlobalVars->curtime + flDuration, Vars::Colors::BoundHitboxEdgeIgnoreZ.Value, Vars::Colors::BoundHitboxFaceIgnoreZ.Value);
			if (Vars::Colors::BoundHitboxEdge.Value.a || Vars::Colors::BoundHitboxFace.Value.a)
				m_vBoxes.emplace_back(m_vTarget, Vec3::Get(-1), Vec3::Get(1), Vec3(), I::GlobalVars->curtime + flDuration, Vars::Colors::BoundHitboxEdge.Value, Vars::Colors::BoundHitboxFace.Value, true);
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

	if (iMethod != Vars::Aimbot::General::AimTypeEnum::Silent || Vars::Misc::Game::AntiCheatCompatibility.Value)
		Math::ClampAngles(vOut);
	return bReturn;
}

// assume angle calculated outside with other overload
void CAimbotProjectile::Aim(CUserCmd* pCmd, Vec3& vAngles, int iMethod)
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
		if (auto pWeapon = H::Entities.GetWeapon();
			G::Attacking == 1 || bUnsure || pWeapon && pWeapon->GetWeaponID() == TF_WEAPON_FLAMETHROWER)
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
	if (iResult == 1)
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
	}

	if (G::Attacking == 1 || iResult != 1 || !Vars::Aimbot::General::AutoShoot.Value || Vars::Debug::Info.Value)
	{
		bool bPlayerPath = Vars::Visuals::Prediction::PlayerPath.Value;
		bool bProjectilePath = Vars::Visuals::Prediction::ProjectilePath.Value && (G::Attacking == 1 || Vars::Debug::Info.Value) && iResult == 1;
		bool bBoxes = Vars::Visuals::Hitbox::BoundsEnabled.Value & (Vars::Visuals::Hitbox::BoundsEnabledEnum::OnShot | Vars::Visuals::Hitbox::BoundsEnabledEnum::AimPoint);
		bool bRealPath = Vars::Visuals::Prediction::RealPath.Value && iResult == 1;
		if (bPlayerPath || bProjectilePath || bBoxes || bRealPath)
		{
			G::PathStorage.clear();
			G::BoxStorage.clear();
			G::LineStorage.clear();

			if (bPlayerPath)
			{
				float flDuration = Vars::Visuals::Prediction::PlayerDrawDuration.Value;
				if (Vars::Colors::PlayerPathIgnoreZ.Value.a)
					G::PathStorage.emplace_back(vPlayerPath, !flDuration ? -int(vPlayerPath.size()) : I::GlobalVars->curtime + flDuration, Vars::Colors::PlayerPathIgnoreZ.Value, Vars::Visuals::Prediction::PlayerPath.Value);
				if (Vars::Colors::PlayerPath.Value.a)
					G::PathStorage.emplace_back(vPlayerPath, !flDuration ? -int(vPlayerPath.size()) : I::GlobalVars->curtime + flDuration, Vars::Colors::PlayerPath.Value, Vars::Visuals::Prediction::PlayerPath.Value, true);
			}
			if (bProjectilePath)
			{
				float flDuration = Vars::Visuals::Prediction::ProjectileDrawDuration.Value;
				if (Vars::Colors::ProjectilePathIgnoreZ.Value.a)
					G::PathStorage.emplace_back(vProjectilePath, !flDuration ? -int(vProjectilePath.size()) - TIME_TO_TICKS(F::Backtrack.GetReal()) : I::GlobalVars->curtime + flDuration, Vars::Colors::ProjectilePathIgnoreZ.Value, Vars::Visuals::Prediction::ProjectilePath.Value);
				if (Vars::Colors::ProjectilePath.Value.a)
					G::PathStorage.emplace_back(vProjectilePath, !flDuration ? -int(vProjectilePath.size()) - TIME_TO_TICKS(F::Backtrack.GetReal()) : I::GlobalVars->curtime + flDuration, Vars::Colors::ProjectilePath.Value, Vars::Visuals::Prediction::ProjectilePath.Value, true);
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
#if defined(SPLASH_DEBUG2) && defined(WORLD_DEBUG)
	G::TriangleStorage.clear();
#endif
#if defined(SPLASH_DEBUG2) && defined(DEBUG_TEXT)
	F::Visuals.ClearDebugText();
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
		for (auto& iTraces : s_mTraceCount | std::views::values)
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

bool CAimbotProjectile::TestAngle(CBaseEntity* pProjectile, const Vec3& vPoint, Vec3& vAngles, int iSimTime, uint8_t iType, uint8_t iFlags)
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
			vPos += Vec3(0, 0, m_tInfo.m_flGravity * pow(TICKS_TO_TIME(iSimTime), 2) / 2);
		Vec3 vForward = (vPos - m_tProjInfo.m_vPos).Normalized();
		m_tProjInfo.m_vAng = Math::VectorAngles(vForward);

		SDK::Trace(m_tProjInfo.m_vPos, m_tProjInfo.m_vPos + vForward * MAX_TRACE_LENGTH, MASK_SOLID, &filter, &trace);
		vAngles = Math::CalcAngle(vEyePos, trace.endpos);
		vForward = (vEyePos - trace.endpos).Normalized();
		if (vForward.Dot(trace.plane.normal) <= 0)
			return false;

		SDK::Trace(vEyePos, trace.endpos, MASK_SOLID, &filter, &trace);
		if (Math::FullFraction(vEyePos, trace.endpos, trace) < 0.999f)
			return false;

		if (!F::AutoAirblast.CanAirblastEntity(pLocal, pWeapon, pProjectile, vAngles))
			return false;
	}
	if (!F::ProjSim.Initialize(m_tProjInfo, false, true))
		return false;

	CTraceFilterCollideable filter = {};
	filter.pSkip = iType == PointTypeEnum::Direct ? pLocal : tTarget.m_pEntity;
	filter.iPlayer = iType == PointTypeEnum::Direct ? PLAYER_DEFAULT : PLAYER_NONE;
	int nMask = MASK_SOLID;
	F::ProjSim.SetupTrace(filter, nMask, pProjectile);

	if (!m_tProjInfo.m_flGravity)
	{
		SDK::TraceHull(m_tProjInfo.m_vPos, vPoint, -m_tProjInfo.m_vHull, m_tProjInfo.m_vHull, nMask, &filter, &trace);
		if (Math::FullFraction(m_tProjInfo.m_vPos, vPoint, trace) < 0.999f && trace.m_pEnt != tTarget.m_pEntity)
			return false;
	}

	bool bDidHit = false;
	Vec3 vNew = F::ProjSim.GetOrigin();
	int iTimingTolerance = TIME_TO_TICKS(m_tInfo.m_flBoundsTime);
	float flRadiusSqr = powf(m_tProjInfo.m_flVelocity * TICK_INTERVAL + m_tProjInfo.m_vHull.z, 2);
	uint8_t iTraceInterval = iFlags == PointFlagsEnum::Lob ? Vars::Aimbot::Projectile::LobTraceInterval.Value
		: iType != PointTypeEnum::Direct ? Vars::Aimbot::Projectile::SplashTraceInterval.Value
		: Vars::Aimbot::Projectile::DirectTraceInterval.Value;

	const RestoreInfo_t tOriginal = { tTarget.m_pEntity->GetAbsOrigin(), tTarget.m_pEntity->m_vecMins(), tTarget.m_pEntity->m_vecMaxs() };
	tTarget.m_pEntity->SetAbsOrigin(tTarget.m_vPos);
	tTarget.m_pEntity->m_vecMins() = { std::clamp(tTarget.m_pEntity->m_vecMins().x, -24.f, 0.f), std::clamp(tTarget.m_pEntity->m_vecMins().y, -24.f, 0.f), tTarget.m_pEntity->m_vecMins().z };
	tTarget.m_pEntity->m_vecMaxs() = { std::clamp(tTarget.m_pEntity->m_vecMaxs().x, 0.f, 24.f), std::clamp(tTarget.m_pEntity->m_vecMaxs().y, 0.f, 24.f), tTarget.m_pEntity->m_vecMaxs().z };
	for (int n = 1; n <= iSimTime; n++)
	{
		F::ProjSim.RunTick(m_tProjInfo);

		if (bDidHit)
		{
			trace.endpos = F::ProjSim.GetOrigin();
			continue;
		}
		if (iTraceInterval != 1 && n % iTraceInterval && n != iSimTime)
			continue;

		Vec3 vOld = vNew; vNew = F::ProjSim.GetOrigin();
		SDK::TraceHull(vOld, vNew, -m_tProjInfo.m_vHull, m_tProjInfo.m_vHull, nMask, &filter, &trace);

		bool bHit = false;
		switch (iType)
		{
		case PointTypeEnum::Direct:
		case PointTypeEnum::Geometry:
			bHit = trace.DidHit(); break;
		case PointTypeEnum::Air:
			bHit = trace.endpos.DistToSqr(vPoint) < flRadiusSqr || trace.DidHit(); break;
		}

		if (bHit)
		{
			bool bValid = false, bTarget = true;
			switch (iType)
			{
			case PointTypeEnum::Direct:
				bTarget = trace.m_pEnt == tTarget.m_pEntity, bValid = bTarget && iSimTime - n < iTimingTolerance; break;
			case PointTypeEnum::Geometry:
				bValid = trace.endpos.DistToSqr(vPoint) < flRadiusSqr; break;
			case PointTypeEnum::Air:
				bValid = !trace.DidHit(); break;
			}
			if (bValid && iType != PointTypeEnum::Direct)
			{
				CGameTrace eyeTrace = {};
				SDK::Trace(trace.endpos + trace.plane.normal * m_tInfo.m_flNormalOffset, tTarget.m_vPos + m_tInfo.m_vTargetEye, MASK_SHOT, &filter, &eyeTrace);
				bValid = eyeTrace.fraction == 1.f;
			}

			if (bValid)
			{
				if (iTraceInterval != 1 && iType != PointTypeEnum::Direct)
				{
					int iInterval = n % iTraceInterval ? n % iTraceInterval : iTraceInterval;
					int iPopCount = ceilf(iInterval - trace.fraction * iInterval);
					for (int i = 0; i < iPopCount && !m_tProjInfo.m_vPath.empty(); i++)
						m_tProjInfo.m_vPath.pop_back();
				}

				bDidHit = true;
			}
			else
				break;

			if (iType == PointTypeEnum::Direct)
				trace.endpos = vNew;

			if (!bTarget || iType != PointTypeEnum::Direct)
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
	m_tMoveStorage = {};
	if (!F::MoveSim.Initialize(tTarget.m_pEntity, m_tMoveStorage) && tTarget.m_iTargetType == TargetEnum::Player)
		return false;

	m_tProjInfo = {};
	F::ProjSim.GetInfo(pProjectile, m_tProjInfo);
	if (!F::ProjSim.Initialize(m_tProjInfo, false, true))
		return false;

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

			SDK::TraceHull(vOld, vNew, -m_tProjInfo.m_vHull, m_tProjInfo.m_vHull, MASK_SOLID, &filter, &trace);
			m_tProjInfo.m_vPos = trace.endpos;
		}
		m_tInfo.m_vLocalEye = m_tProjInfo.m_vPos; // just assume from the projectile without any offset, check validity later
		pProjectile->SetAbsOrigin(m_tProjInfo.m_vPos);
	}
	m_tInfo.m_vTargetEye = tTarget.m_pEntity->As<CTFPlayer>()->GetViewOffset();
	tTarget.m_vPos = tTarget.m_pEntity->m_vecOrigin();

	m_tInfo.m_flVelocity = m_tProjInfo.m_flVelocity;

	m_tInfo.m_flGravity = m_tProjInfo.m_flGravity;
	m_tInfo.m_iSplashRestrict = !m_tInfo.m_flGravity ? Vars::Aimbot::Projectile::SplashRestrictDirect.Value : Vars::Aimbot::Projectile::SplashRestrictArc.Value;

	float flSize = tTarget.m_pEntity->GetSize().Length();
	m_tInfo.m_flRadius = GetSplashRadius(pProjectile, m_tProjInfo.m_pWeapon, m_tProjInfo.m_pOwner, Vars::Aimbot::Projectile::SplashRadius.Value / 100, pWeapon);
	m_tInfo.m_flBoundsTime = tTarget.m_pEntity->GetSize().Length() / m_tInfo.m_flVelocity;
	m_tInfo.m_flRadiusTime = m_tInfo.m_flBoundsTime + m_tInfo.m_flRadius / m_tInfo.m_flVelocity;
	m_tInfo.m_bIgnoreTiming = Vars::Aimbot::Projectile::LobAnglesUnderpredict.Value && m_tInfo.m_flRadius;



	Directs_t mDirects = GetDirects();
	Splashes_t vSplashes = GetSplashes();

	DirectHistory_t mDirectHistory = {};
	SplashHistory_t mSplashHistory = {};

	int iMaxTime = TIME_TO_TICKS(Vars::Aimbot::Projectile::MaxSimulationTime.Value);
	for (int i = 1 - TIME_TO_TICKS(m_tInfo.m_flLatency); i <= iMaxTime; i++)
	{
		if (!m_tMoveStorage.m_bFailed)
		{
			F::MoveSim.RunTick(m_tMoveStorage);
			tTarget.m_vPos = m_tMoveStorage.m_vPredictedOrigin;
		}
		if (i < 0)
			continue;

		for (auto& [iIndex, tOffset] : mDirects)
		{
			Vec3& vOffset = tOffset.m_vOffset;
			uint8_t iType = tOffset.m_iFlags & -tOffset.m_iFlags;

			Vec3 vPoint = tTarget.m_vPos + vOffset;
			if (Vars::Aimbot::Projectile::HuntsmanPullPoint.Value && tTarget.m_nAimedHitbox == HITBOX_HEAD)
			{
				vPoint = PullPoint(vPoint, m_tInfo.m_vLocalEye, m_tInfo, tTarget.m_pEntity->m_vecMins() + m_tInfo.m_vHull, tTarget.m_pEntity->m_vecMaxs() - m_tInfo.m_vHull, tTarget.m_vPos);
				if (Vars::Aimbot::Projectile::HuntsmanPullNoZ.Value)
					vPoint.z = tTarget.m_vPos.z + vOffset.z;
			}

			uint8_t iFlags = CalculateFlagsEnum::Accuracy;
			if (iType == PointFlagsEnum::Lob)
				iFlags |= CalculateFlagsEnum::LobAngle;
			int iTolerance = m_tInfo.m_bIgnoreTiming && iType == PointFlagsEnum::Lob ? std::numeric_limits<int>::max() : -1;

			Solution_t tSolution;
			switch (iType)
			{
			case PointFlagsEnum::Lob:
				if (ShouldLob(m_tMoveStorage, m_tInfo))
					goto end;
				tSolution.m_iCalculated = CalculateResultEnum::Bad; break;
			default: end:
				CalculateAngle(m_tInfo.m_vLocalEye, vPoint, i, tSolution, iFlags, iTolerance);
			}
			switch (tSolution.m_iCalculated)
			{
			case CalculateResultEnum::Good:
				mDirectHistory[iType].emplace_back(History_t(tTarget.m_vPos, i), tSolution.m_flPitch, tSolution.m_flYaw, tSolution.m_flTime, vPoint, iIndex);
				[[fallthrough]];
			case CalculateResultEnum::Bad:
				tOffset.m_iFlags &= ~iType;
				if (!(tOffset.m_iFlags /*& (PointFlagsEnum::Regular | PointFlagsEnum::Lob)*/))
					mDirects.erase(iIndex);
			}
		}

		for (auto it = vSplashes.begin(); it != vSplashes.end();)
		{
			uint8_t iFlags = CalculateFlagsEnum::AccountDrag;
			if (*it == PointFlagsEnum::Lob && !m_tInfo.m_bIgnoreTiming)
				iFlags |= CalculateFlagsEnum::LobAngle;

			Solution_t tSolution; CalculateAngle(m_tInfo.m_vLocalEye, tTarget.m_vPos, i, tSolution, iFlags);
			if (tSolution.m_iCalculated == CalculateResultEnum::Bad && mDirects.empty())
			{
				it = vSplashes.erase(it);
				continue;
			}

			const float flTimeTo = tSolution.m_flTime - TICKS_TO_TIME(i);
			if (flTimeTo > m_tInfo.m_flRadiusTime)
			{
				++it;
				continue;
			}
			if (flTimeTo < -m_tInfo.m_flRadiusTime)
			{
				it = vSplashes.erase(it);
				continue;
			}
			if (*it == PointFlagsEnum::Lob && !ShouldLob(m_tMoveStorage, m_tInfo))
			{
				++it;
				continue;
			}

			mSplashHistory[*it].emplace_back(History_t(tTarget.m_vPos, i), fabsf(flTimeTo));
			++it;
		}

		if (mDirects.empty() && vSplashes.empty())
			break;
	}

	m_iResult = false, m_bUpdate = true;
	if (!m_tInfo.m_flRadius || Vars::Aimbot::Projectile::SplashPrediction.Value < Vars::Aimbot::Projectile::SplashPredictionEnum::Prefer)
		goto direct;
	else
		goto splash;
	while (!mDirectHistory.empty() || !mSplashHistory.empty())
	{
		direct: if (HandleDirect(mDirectHistory)) break;
		splash: if (HandleSplash(mSplashHistory)) break;
	}
	F::MoveSim.Restore(m_tMoveStorage);

	tTarget.m_vPos = m_vTarget;
	tTarget.m_vAngleTo = m_vAngleTo;
		
	bool bMain = m_iResult == 1;
	if (bMain)
	{
		if (Vars::Colors::BoundHitboxEdge.Value.a || Vars::Colors::BoundHitboxFace.Value.a || Vars::Colors::BoundHitboxEdgeIgnoreZ.Value.a || Vars::Colors::BoundHitboxFaceIgnoreZ.Value.a)
		{
			float flProjectileTime = 0.f, flTargetTime = 0.f;
			bool bBox = Vars::Visuals::Hitbox::BoundsEnabled.Value & Vars::Visuals::Hitbox::BoundsEnabledEnum::OnShot;
			bool bPoint = Vars::Visuals::Hitbox::BoundsEnabled.Value & Vars::Visuals::Hitbox::BoundsEnabledEnum::AimPoint;
			bool bTimed = !Vars::Visuals::Prediction::PlayerDrawDuration.Value;
			if (bTimed)
			{
				flProjectileTime = TICKS_TO_TIME(m_vProjectilePath.size());
				flTargetTime = m_tMoveStorage.m_bFailed ? flProjectileTime : TICKS_TO_TIME(m_vPlayerPath.size());
			}
			if (bBox)
			{
				float flDuration = bTimed ? flTargetTime : Vars::Visuals::Hitbox::DrawDuration.Value;
				if (Vars::Colors::BoundHitboxEdgeIgnoreZ.Value.a || Vars::Colors::BoundHitboxFaceIgnoreZ.Value.a)
					m_vBoxes.emplace_back(m_vPredicted, tTarget.m_pEntity->m_vecMins(), tTarget.m_pEntity->m_vecMaxs(), Vec3(), I::GlobalVars->curtime + flDuration, Vars::Colors::BoundHitboxEdgeIgnoreZ.Value, Vars::Colors::BoundHitboxFaceIgnoreZ.Value);
				if (Vars::Colors::BoundHitboxEdge.Value.a || Vars::Colors::BoundHitboxFace.Value.a)
					m_vBoxes.emplace_back(m_vPredicted, tTarget.m_pEntity->m_vecMins(), tTarget.m_pEntity->m_vecMaxs(), Vec3(), I::GlobalVars->curtime + flDuration, Vars::Colors::BoundHitboxEdge.Value, Vars::Colors::BoundHitboxFace.Value, true);
			}
			if (bPoint)
			{
				float flDuration = bTimed ? flProjectileTime : Vars::Visuals::Hitbox::DrawDuration.Value;
				if (Vars::Colors::BoundHitboxEdgeIgnoreZ.Value.a || Vars::Colors::BoundHitboxFaceIgnoreZ.Value.a)
					m_vBoxes.emplace_back(m_vTarget, Vec3::Get(-1), Vec3::Get(1), Vec3(), I::GlobalVars->curtime + flDuration, Vars::Colors::BoundHitboxEdgeIgnoreZ.Value, Vars::Colors::BoundHitboxFaceIgnoreZ.Value);
				if (Vars::Colors::BoundHitboxEdge.Value.a || Vars::Colors::BoundHitboxFace.Value.a)
					m_vBoxes.emplace_back(m_vTarget, Vec3::Get(-1), Vec3::Get(1), Vec3(), I::GlobalVars->curtime + flDuration, Vars::Colors::BoundHitboxEdge.Value, Vars::Colors::BoundHitboxFace.Value, true);
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

		G::Attacking = true;
		DrawVisuals(1, tTarget, m_vPlayerPath, m_vProjectilePath, m_vBoxes);

		Aim(pCmd, tTarget.m_vAngleTo, Vars::Aimbot::General::AimTypeEnum::Silent);
		return true;
	}

	return false;
}