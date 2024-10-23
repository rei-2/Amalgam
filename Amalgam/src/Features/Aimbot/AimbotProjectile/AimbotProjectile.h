#pragma once
#include "../../../SDK/SDK.h"

#include "../AimbotGlobal/AimbotGlobal.h"

enum PointType_
{
	PointType_None = 0,
	PointType_Regular = (1 << 0),
	PointType_Obscured = (1 << 1)
};

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
	CTFPlayer* m_pLocal = nullptr;
	CTFWeaponBase* m_pWeapon = nullptr;

	Vec3 m_vLocalEye = {};
	Vec3 m_vTargetEye = {};

	float m_flLatency = 0.f;
	int m_iLatency = 0;

	Vec3 m_vOffset = {};
	float m_flVelocity = 0.f;
	float m_flGravity = 0.f;
	float m_flRadius = 0.f;
	float m_flRadiusTime = 0.f;
	float m_flBoundingRadius = 0.f;
	float m_flUpFix = 0.f;
	float m_flOffsetTime = 0.f;
	int m_iSplashCount = 0;
	bool m_bRocket = false;
	int m_iPrimeTime = 0;
};

class CAimbotProjectile
{
	std::vector<Target_t> GetTargets(CTFPlayer* pLocal, CTFWeaponBase* pWeapon);
	std::vector<Target_t> SortTargets(CTFPlayer* pLocal, CTFWeaponBase* pWeapon);

	int GetHitboxPriority(int nHitbox, Target_t& target, Info_t& tInfo);
	std::unordered_map<int, Vec3> GetDirectPoints(Target_t& target, Info_t& tInfo);
	std::vector<Point_t> GetSplashPoints(Target_t& target, std::vector<std::pair<Vec3, int>>& vSpherePoints, Info_t& tInfo, int iSimTime);

	void CalculateAngle(const Vec3& vLocalPos, const Vec3& vTargetPos, Info_t& tInfo, int iSimTime, Solution_t& out, bool bAccuracy = true);
	bool TestAngle(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, Target_t& target, Vec3& vPoint, Vec3& vAngles, int iSimTime, bool bSplash, std::deque<Vec3>* pProjectilePath);

	int CanHit(Target_t& target, CTFPlayer* pLocal, CTFWeaponBase* pWeapon, std::deque<Vec3>* pPlayerPath, std::deque<Vec3>* pProjectilePath, std::vector<DrawBox>* pBoxes, float* pTimeTo);
	bool RunMain(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd);

	Vec3 Aim(Vec3 vCurAngle, Vec3 vToAngle, int iMethod = Vars::Aimbot::General::AimType.Value);
	void Aim(CUserCmd* pCmd, Vec3& vAngle);

	bool m_bLastTickHeld = false;

public:
	void Run(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd);

	int m_bLastTickCancel = 0;
};

ADD_FEATURE(CAimbotProjectile, AimbotProjectile)