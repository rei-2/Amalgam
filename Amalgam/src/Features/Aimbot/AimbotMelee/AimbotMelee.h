#pragma once
#include "../../../SDK/SDK.h"

#include "../AimbotGlobal/AimbotGlobal.h"

class CAimbotMelee
{
	std::vector<Target_t> GetTargets(CTFPlayer* pLocal, CTFWeaponBase* pWeapon);
	bool AimFriendlyBuilding(CBaseObject* pBuilding);
	std::vector<Target_t> SortTargets(CTFPlayer* pLocal, CTFWeaponBase* pWeapon);

	int GetSwingTime(CTFWeaponBase* pWeapon);
	void SimulatePlayers(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, std::vector<Target_t> targets, Vec3& vEyePos,
						 std::unordered_map<CBaseEntity*, std::deque<TickRecord>>& pRecordMap,
						 std::unordered_map<CBaseEntity*, std::deque<std::pair<Vec3, Vec3>>>& simLines);
	bool CanBackstab(CBaseEntity* pTarget, CTFPlayer* pLocal, Vec3 eyeAngles);
	int CanHit(Target_t& target, CTFPlayer* pLocal, CTFWeaponBase* pWeapon, Vec3 vEyePos, std::deque<TickRecord> newRecords);
	
	bool IsAttacking(const CUserCmd* pCmd, CTFWeaponBase* pWeapon);
	void Aim(CUserCmd* pCmd, Vec3& vAngle);
	Vec3 Aim(Vec3 vCurAngle, Vec3 vToAngle, int iMethod = Vars::Aimbot::General::AimType.Value);

	bool FindNearestBuildPoint(CBaseObject* pBuilding, CTFPlayer* pLocal, Vec3& vPoint);
	bool RunSapper(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd);

	Target_t lockedTarget;
	int iAimType = 0;
	int iDoubletapTicks = 0;
	int iDoubletapMax = 0;
public:
	void Run(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd);
};

ADD_FEATURE(CAimbotMelee, AimbotMelee)