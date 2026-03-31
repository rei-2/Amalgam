#pragma once
#include "../../../SDK/SDK.h"

#include "../AimbotGlobal/AimbotGlobal.h"
#include <optional>

class CAutoDetonate
{
private:
	bool CheckEntity(CBaseEntity* pEntity, CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd, CBaseEntity* pProjectile, float flRadius, Vec3 vOrigin);
	bool CheckEntities(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd, CBaseEntity* pProjectile, float flRadius, Vec3 vOrigin);
	bool CheckTargets(CTFPlayer* pLocal, EntityEnum::EntityEnum iGroup, float flRadiusScale, CUserCmd* pCmd);
	bool CheckSelf(CTFPlayer* pLocal, EntityEnum::EntityEnum iGroup);
	bool Check(CTFPlayer* pLocal, CUserCmd* pCmd, EntityEnum::EntityEnum iGroup, int iFlag);

	bool GetRadius(EntityEnum::EntityEnum iGroup, CBaseEntity* pProjectile, float& flRadius, CTFWeaponBase*& pWeapon);
	Vec3 GetOrigin(CBaseEntity* pProjectile, EntityEnum::EntityEnum iGroup, float flLatency = 0.f);

	void PredictPlayers(CTFPlayer* pLocal, float flLatency = 0.f, bool bLocal = false);
	void RestorePlayers();

	std::unordered_map<CBaseEntity*, Vec3> m_mRestore = {};
	std::optional<Vec3> m_vAimPos = {};

public:
	void Run(CTFPlayer* pLocal, CUserCmd* pCmd);
};

ADD_FEATURE(CAutoDetonate, AutoDetonate);