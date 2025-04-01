#pragma once
#include "../../../SDK/SDK.h"

Enum(ProjSim,
	None = 0,
	Trace = 1 << 0,
	InitCheck = 1 << 1,
	Quick = 1 << 2,
	NoRandomAngles = 1 << 3,
	PredictCmdNum = 1 << 4
)

struct ProjectileInfo
{
	uint32_t m_uType = 0;

	Vec3 m_vPos = {};
	Vec3 m_vAng = {};
	Vec3 m_vHull = {};

	float m_flVelocity = 0.f;
	float m_flGravity = 0.f;
	bool m_bNoSpin = false;
	float m_flLifetime = 60.f;

	CTFPlayer* m_pOwner = nullptr;

	std::deque<Vec3> m_vPath = {};

	bool m_bQuick = false;
};

class CProjectileSimulation
{
	bool GetInfoMain(CTFPlayer* pPlayer, CTFWeaponBase* pWeapon, Vec3 vAngles, ProjectileInfo& tProjInfo, int iFlags, float flAutoCharge);

	const objectparams_t g_PhysDefaultObjectParams =
	{
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
	bool Initialize(ProjectileInfo& tProjInfo, bool bSimulate = true, bool bVelocities = true);
	void RunTick(ProjectileInfo& tProjInfo, bool bPath = true);
	Vec3 GetOrigin();
	Vec3 GetVelocity();

	IPhysicsEnvironment* env = nullptr;
	IPhysicsObject* obj = nullptr;
};

ADD_FEATURE(CProjectileSimulation, ProjSim)