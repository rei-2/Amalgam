#pragma once
#include "../../../SDK/SDK.h"

#include "../AimbotGlobal/AimbotGlobal.h"

struct Solution_t
{
	float m_flPitch = 0.f;
	float m_flYaw = 0.f;
	float m_flTime = 0.f;
	int m_iCalculated = 0;
};
struct Point_t
{
	Vec3 m_vPoint = {};
	Solution_t m_Solution = {};
};
struct Info_t
{
	Vec3 vOffset = {};
	float flVelocity = 0.f;
	float flGravity = 0.f;
	float flRadius = 0.f;
	float flSphere = 0.f;
	float flUpFix = 0.f;
	float flOffset = 0.f;
	int iPrimeTime = 0;
};

class CAimbotProjectile
{
	std::vector<Target_t> GetTargets(CTFPlayer* pLocal, CTFWeaponBase* pWeapon);
	std::vector<Target_t> SortTargets(CTFPlayer* pLocal, CTFWeaponBase* pWeapon);

	int GetHitboxPriority(int nHitbox, CTFPlayer* pLocal, CTFWeaponBase* pWeapon, Target_t& target);
	std::unordered_map<int, Vec3> GetDirectPoints(Target_t& target, bool bPlayer, CTFPlayer* pLocal, CTFWeaponBase* pWeapon);
	std::vector<Point_t> GetSplashPoints(Target_t& target, std::vector<Vec3>& vSpherePoints, CTFPlayer* pLocal, CTFWeaponBase* pWeapon, Info_t& tInfo, int iSimTime);
	
	void CalculateAngle(const Vec3& vLocalPos, const Vec3& vTargetPos, Info_t& tInfo, int iSimTime, CTFPlayer* pLocal, CTFWeaponBase* pWeapon, Solution_t& out, bool bAccuracy = true);
	bool TestAngle(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, Target_t& target, Vec3& vPoint, Vec3& vAngles, int iSimTime, bool bSplash, std::deque<std::pair<Vec3, Vec3>>* pProjLines);
	
	int CanHit(Target_t& target, CTFPlayer* pLocal, CTFWeaponBase* pWeapon, std::deque<std::pair<Vec3, Vec3>>* pMoveLines, std::deque<std::pair<Vec3, Vec3>>* pProjLines, std::vector<DrawBox>* pBoxes, float* pTimeTo);
	bool RunMain(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd);

	void Aim(CUserCmd* pCmd, Vec3& vAngle);
	Vec3 Aim(Vec3 vCurAngle, Vec3 vToAngle, int iMethod = Vars::Aimbot::General::AimType.Value);

	bool bLastTickHeld = false;

public:
	void Run(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd);

	int bLastTickCancel = 0;
};

ADD_FEATURE(CAimbotProjectile, AimbotProjectile)