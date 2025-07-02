#pragma once
#include "../../../SDK/SDK.h"

Enum(ProjSim,
	None = 0,
	Trace = 1 << 0,
	InitCheck = 1 << 1,
	Quick = 1 << 2,
	NoRandomAngles = 1 << 3,
	PredictCmdNum = 1 << 4,
	MaxSpeed = 1 << 5
)

struct ProjectileInfo
{
	CTFPlayer* m_pOwner = nullptr;
	CTFWeaponBase* m_pWeapon = nullptr;
	uint32_t m_uType = 0;

	Vec3 m_vPos = {};
	Vec3 m_vAng = {};
	Vec3 m_vHull = {};

	float m_flVelocity = 0.f;
	float m_flGravity = 0.f;
	float m_flLifetime = 60.f;

	std::vector<Vec3> m_vPath = {};

	bool m_bQuick = false;
};

class CProjectileSimulation
{
	bool GetInfoMain(CTFPlayer* pPlayer, CTFWeaponBase* pWeapon, Vec3 vAngles, ProjectileInfo& tProjInfo, int iFlags, float flAutoCharge);

	const objectparams_t m_tPhysDefaultObjectParams = {
		NULL,
		1.0, //mass
		1.0, // inertia
		0.1f, // damping
		0.1f, // rotdamping
		0.05f, // rotIntertiaLimit
		"DEFAULT",
		NULL,// game data
		0.f, // volume (leave 0 if you don't have one or call physcollision->CollideVolume() to compute it)
		1.0f, // drag coefficient
		true,// enable collisions?
	};

public:
	bool GetInfo(CTFPlayer* pPlayer, CTFWeaponBase* pWeapon, Vec3 vAngles, ProjectileInfo& tProjInfo, int iFlags = ProjSimEnum::Trace | ProjSimEnum::InitCheck, float flAutoCharge = -1.f);
	void SetupTrace(CTraceFilterCollideable& filter, int& nMask, CTFWeaponBase* pWeapon, int nTick = 0, bool bQuick = false);

	void GetInfo(CBaseEntity* pProjectile, ProjectileInfo& tProjInfo);
	void SetupTrace(CTraceFilterCollideable& filter, int& nMask, CBaseEntity* pProjectile);

	bool Initialize(ProjectileInfo& tProjInfo, bool bSimulate = true, bool bWorld = false);
	void RunTick(ProjectileInfo& tProjInfo, bool bPath = true);
	Vec3 GetOrigin();
	Vec3 GetVelocity();

	inline std::pair<CTFWeaponBase*, CTFPlayer*> GetEntities(CBaseEntity* pProjectile)
	{
		std::pair<CTFWeaponBase*, CTFPlayer*> paReturn;
		switch (pProjectile->GetClassID())
		{
		case ETFClassID::CBaseGrenade:
		case ETFClassID::CTFWeaponBaseGrenadeProj:
		case ETFClassID::CTFWeaponBaseMerasmusGrenade:
		case ETFClassID::CTFGrenadePipebombProjectile:
		case ETFClassID::CTFStunBall:
		case ETFClassID::CTFBall_Ornament:
		case ETFClassID::CTFProjectile_Jar:
		case ETFClassID::CTFProjectile_Cleaver:
		case ETFClassID::CTFProjectile_JarGas:
		case ETFClassID::CTFProjectile_JarMilk:
		case ETFClassID::CTFProjectile_SpellBats:
		case ETFClassID::CTFProjectile_SpellKartBats:
		case ETFClassID::CTFProjectile_SpellMeteorShower:
		case ETFClassID::CTFProjectile_SpellMirv:
		case ETFClassID::CTFProjectile_SpellPumpkin:
		case ETFClassID::CTFProjectile_SpellSpawnBoss:
		case ETFClassID::CTFProjectile_SpellSpawnHorde:
		case ETFClassID::CTFProjectile_SpellSpawnZombie:
		case ETFClassID::CTFProjectile_SpellTransposeTeleport:
		case ETFClassID::CTFProjectile_Throwable:
		case ETFClassID::CTFProjectile_ThrowableBreadMonster:
		case ETFClassID::CTFProjectile_ThrowableBrick:
		case ETFClassID::CTFProjectile_ThrowableRepel:
		{
			paReturn.first = pProjectile->As<CTFGrenadePipebombProjectile>()->m_hOriginalLauncher().Get()->As<CTFWeaponBase>();
			paReturn.second = pProjectile->As<CTFWeaponBaseGrenadeProj>()->m_hThrower().Get()->As<CTFPlayer>();
			break;
		}
		case ETFClassID::CTFBaseRocket:
		case ETFClassID::CTFFlameRocket:
		case ETFClassID::CTFProjectile_Arrow:
		case ETFClassID::CTFProjectile_GrapplingHook:
		case ETFClassID::CTFProjectile_HealingBolt:
		case ETFClassID::CTFProjectile_Rocket:
		case ETFClassID::CTFProjectile_BallOfFire:
		case ETFClassID::CTFProjectile_MechanicalArmOrb:
		case ETFClassID::CTFProjectile_SentryRocket:
		case ETFClassID::CTFProjectile_SpellFireball:
		case ETFClassID::CTFProjectile_SpellLightningOrb:
		case ETFClassID::CTFProjectile_SpellKartOrb:
		case ETFClassID::CTFProjectile_EnergyBall:
		case ETFClassID::CTFProjectile_Flare:
		{
			paReturn.first = pProjectile->As<CTFBaseRocket>()->m_hLauncher().Get()->As<CTFWeaponBase>();
			paReturn.second = paReturn.first ? paReturn.first->m_hOwner().Get()->As<CTFPlayer>() : nullptr;
			break;
		}
		case ETFClassID::CTFBaseProjectile:
		case ETFClassID::CTFProjectile_EnergyRing:
		//case ETFClassID::CTFProjectile_Syringe:
		{
			paReturn.first = pProjectile->As<CTFBaseProjectile>()->m_hLauncher().Get()->As<CTFWeaponBase>();
			paReturn.second = paReturn.first ? paReturn.first->m_hOwner().Get()->As<CTFPlayer>() : nullptr;
			break;
		}
		}
		return paReturn;
	}
	inline Vec3 GetVelocity(CBaseEntity* pProjectile)
	{
		switch (pProjectile->GetClassID())
		{
		case ETFClassID::CTFProjectile_Rocket:
		case ETFClassID::CTFProjectile_SentryRocket:
		case ETFClassID::CTFProjectile_EnergyBall:
			if (!pProjectile->As<CTFProjectile_Rocket>()->m_iDeflected())
				return pProjectile->As<CTFProjectile_Rocket>()->m_vInitialVelocity();
			break;
		case ETFClassID::CTFProjectile_Arrow:
			if (!pProjectile->As<CTFProjectile_Rocket>()->m_iDeflected())
				return { 
					pProjectile->As<CTFProjectile_Rocket>()->m_vInitialVelocity().x,
					pProjectile->As<CTFProjectile_Rocket>()->m_vInitialVelocity().y,
					pProjectile->GetAbsVelocity().z
				};
			break;
		}
		return pProjectile->GetAbsVelocity();
	}
	inline float GetGravity(CBaseEntity* pProjectile, CTFWeaponBase* pWeapon = nullptr)
	{
		float flReturn = 0.f;

		static auto sv_gravity = U::ConVars.FindVar("sv_gravity");
		float flGravity = sv_gravity->GetFloat() / 800.f;
		switch (pProjectile->GetClassID())
		{
		case ETFClassID::CBaseGrenade:
		case ETFClassID::CTFWeaponBaseGrenadeProj:
		case ETFClassID::CTFWeaponBaseMerasmusGrenade:
		case ETFClassID::CTFGrenadePipebombProjectile:
		case ETFClassID::CTFStunBall:
		case ETFClassID::CTFBall_Ornament:
		case ETFClassID::CTFProjectile_Jar:
		case ETFClassID::CTFProjectile_Cleaver:
		case ETFClassID::CTFProjectile_JarGas:
		case ETFClassID::CTFProjectile_JarMilk:
		case ETFClassID::CTFProjectile_SpellBats:
		case ETFClassID::CTFProjectile_SpellKartBats:
		case ETFClassID::CTFProjectile_SpellMeteorShower:
		case ETFClassID::CTFProjectile_SpellMirv:
		case ETFClassID::CTFProjectile_SpellPumpkin:
		case ETFClassID::CTFProjectile_SpellSpawnBoss:
		case ETFClassID::CTFProjectile_SpellSpawnHorde:
		case ETFClassID::CTFProjectile_SpellSpawnZombie:
		case ETFClassID::CTFProjectile_SpellTransposeTeleport:
		case ETFClassID::CTFProjectile_Throwable:
		case ETFClassID::CTFProjectile_ThrowableBreadMonster:
		case ETFClassID::CTFProjectile_ThrowableBrick:
		case ETFClassID::CTFProjectile_ThrowableRepel:
		case ETFClassID::CTFProjectile_SpellFireball:
			flReturn = 1.f;
			break;
		case ETFClassID::CTFProjectile_HealingBolt:
			flReturn = 0.2f * flGravity;
			break;
		case ETFClassID::CTFProjectile_Flare:
			flReturn = (pWeapon && pWeapon->As<CTFFlareGun>()->GetFlareGunType() == FLAREGUN_GRORDBORT ? 0.45f : 0.3f) * flGravity;
			break;
		case ETFClassID::CTFProjectile_Arrow:
			flReturn = pProjectile->As<CTFProjectile_Arrow>()->CanHeadshot()
				? Math::RemapVal(pProjectile->As<CTFProjectile_Arrow>()->m_vInitialVelocity().Length(), 1800.f, 2600.f, 0.5f, 0.1f) * flGravity
				: 0.2f * flGravity;
			break;
		}
		return flReturn;
	}

	IPhysicsEnvironment* m_pEnv = nullptr;
	IPhysicsObject* m_pObj = nullptr;
};

ADD_FEATURE(CProjectileSimulation, ProjSim);