#pragma once
#include "../../../SDK/SDK.h"

#include "../AimbotGlobal/AimbotGlobal.h"

class CAimbotMelee
{
	std::vector<Target_t> GetTargets(CTFPlayer* pLocal, CTFWeaponBase* pWeapon);
	bool AimFriendlyBuilding(CBaseObject* pBuilding);
	std::vector<Target_t> SortTargets(CTFPlayer* pLocal, CTFWeaponBase* pWeapon);

	int GetSwingTime(CTFWeaponBase* pWeapon);
	void SimulatePlayers(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, std::vector<Target_t> vTargets, Vec3& vEyePos);
	bool CanBackstab(CBaseEntity* pTarget, CTFPlayer* pLocal, Vec3 vEyeAngles);
	int CanHit(Target_t& tTarget, CTFPlayer* pLocal, CTFWeaponBase* pWeapon, Vec3 vEyePos);
	
	bool Aim(Vec3 vCurAngle, Vec3 vToAngle, Vec3& vOut, int iMethod = Vars::Aimbot::General::AimType.Value);
	void Aim(CUserCmd* pCmd, Vec3& vAngle, int iMethod = Vars::Aimbot::General::AimType.Value);

	bool FindNearestBuildPoint(CBaseObject* pBuilding, CTFPlayer* pLocal, Vec3& vPoint);
	bool RunSapper(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd);

	int m_iDoubletapTicks = 0;
	bool m_bShouldSwing = false;

	std::unordered_map<int, std::deque<TickRecord>> m_mRecordMap;
	std::unordered_map<int, std::vector<Vec3>> m_mPaths;

public:
	void Run(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd);
};

ADD_FEATURE(CAimbotMelee, AimbotMelee);