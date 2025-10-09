#pragma once
#include "../../../SDK/SDK.h"

#include "../AimbotGlobal/AimbotGlobal.h"

Enum(PointType, None = 0, Regular = 1 << 0, Obscured = 1 << 1, ObscuredExtra = 1 << 2, ObscuredMulti = 1 << 3)
Enum(Calculated, Pending, Good, Time, Bad)

// Removed DormantData_t and container usage to avoid allocator/construct issues in older stdlib

struct Solution_t
{
	float m_flPitch = 0.f;
	float m_flYaw = 0.f;
	float m_flTime = 0.f;
	int m_iCalculated = CalculatedEnum::Pending;
};
struct Point_t
{
	Vec3 m_vPoint = {};
	Solution_t m_tSolution = {};
};
struct Info_t
{
	CTFPlayer* m_pLocal = nullptr;
	CTFWeaponBase* m_pWeapon = nullptr;

	Vec3 m_vLocalEye = {};
	Vec3 m_vTargetEye = {};

	float m_flLatency = 0.f;

	Vec3 m_vHull = {};
	Vec3 m_vOffset = {};
	Vec3 m_vAngFix = {};
	float m_flVelocity = 0.f;
	float m_flGravity = 0.f;
	float m_flRadius = 0.f;
	float m_flRadiusTime = 0.f;
	float m_flBoundingTime = 0.f;
	float m_flOffsetTime = 0.f;
	int m_iSplashCount = 0;
	int m_iSplashMode = 0;
	float m_flPrimeTime = 0;
	int m_iPrimeTime = 0;
};

class CAimbotProjectile
{
private:
	std::vector<Target_t> GetTargets(CTFPlayer* pLocal, CTFWeaponBase* pWeapon);
	std::vector<Target_t> SortTargets(CTFPlayer* pLocal, CTFWeaponBase* pWeapon);

	std::unordered_map<int, Vec3> GetDirectPoints(Target_t& tTarget, CBaseEntity* pProjectile = nullptr);
	std::vector<Point_t> GetSplashPoints(Target_t& tTarget, std::vector<std::pair<Vec3, int>>& vSpherePoints, int iSimTime);
	void SetupSplashPoints(Target_t& tTarget, std::vector<std::pair<Vec3, int>>& vSpherePoints, std::vector<std::pair<Vec3, Vec3>>& vSimplePoints);
	std::vector<Point_t> GetSplashPointsSimple(Target_t& tTarget, std::vector<std::pair<Vec3, Vec3>>& vSpherePoints, int iSimTime);

	void CalculateAngle(const Vec3& vLocalPos, const Vec3& vTargetPos, int iSimTime, Solution_t& out, bool bAccuracy = true);
	bool TestAngle(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, Target_t& tTarget, Vec3& vPoint, Vec3& vAngles, int iSimTime, bool bSplash, bool* pHitSolid = nullptr, std::vector<Vec3>* pProjectilePath = nullptr);

	int CanHit(Target_t& tTarget, CTFPlayer* pLocal, CTFWeaponBase* pWeapon);
	bool RunMain(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd);

	bool CanHit(Target_t& tTarget, CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CBaseEntity* pProjectile);
	bool TestAngle(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CBaseEntity* pProjectile, Target_t& tTarget, Vec3& vPoint, Vec3& vAngles, int iSimTime, bool bSplash, std::vector<Vec3>* pProjectilePath = nullptr);

	bool Aim(Vec3 vCurAngle, Vec3 vToAngle, Vec3& vOut, int iMethod = Vars::Aimbot::General::AimType.Value);
	void Aim(CUserCmd* pCmd, Vec3& vAngle, int iMethod = Vars::Aimbot::General::AimType.Value);

	Info_t m_tInfo = {};

	bool m_bLastTickHeld = false;

	float m_flTimeTo = std::numeric_limits<float>::max();
	std::vector<Vec3> m_vPlayerPath = {};
	std::vector<Vec3> m_vProjectilePath = {};
	std::vector<DrawBox_t> m_vBoxes = {};

    // dormant targeting removed

public:
	void Run(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd);
	float GetSplashRadius(CTFWeaponBase* pWeapon, CTFPlayer* pPlayer);
	
	bool AutoAirblast(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd, CBaseEntity* pProjectile);
	float GetSplashRadius(CBaseEntity* pProjectile, CTFWeaponBase* pWeapon = nullptr, CTFPlayer* pPlayer = nullptr, float flScale = 1.f, CTFWeaponBase* pAirblast = nullptr);

	int m_iLastTickCancel = 0;
};

ADD_FEATURE(CAimbotProjectile, AimbotProjectile);