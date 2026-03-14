#pragma once
#include "../../../SDK/SDK.h"

#include "../AimbotGlobal/AimbotGlobal.h"
#include "../../Simulation/MovementSimulation/MovementSimulation.h"
#include "../../Simulation/ProjectileSimulation/ProjectileSimulation.h"

Enum(CalculateFlags, None = 0, TwoPass = 1 << 0, SetupClip = 1 << 1, AccountDrag = 1 << 2, Accuracy = TwoPass | SetupClip | AccountDrag)
Enum(CalculateResult, Pending, Good, Time, Bad)

struct Info_t
{
	CTFPlayer* m_pLocal = nullptr;
	CTFWeaponBase* m_pWeapon = nullptr;
	Target_t* m_pTarget = nullptr;
	CBaseEntity* m_pProjectile = nullptr;

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
	int m_iArmTime = 0;
	float m_flNormalOffset = 0.f;
};

struct Solution_t
{
	float m_flPitch = 0.f;
	float m_flYaw = 0.f;
	float m_flTime = 0.f;
	int m_iCalculated = CalculateResultEnum::Pending;
};
struct Point_t
{
	Vec3 m_vPoint = {};
	Solution_t m_tSolution = {};
};

struct History_t
{
	Vec3 m_vOrigin;
	int m_iSimtime;
};
struct Direct_t : History_t
{
	float m_flPitch;
	float m_flYaw;
	float m_flTime;
	Vec3 m_vPoint;
	int m_iPriority;
};
struct Splash_t : History_t
{
	float m_flTimeTo;
};

class CAimbotProjectile
{
private:
	std::unordered_map<int, Vec3> GetDirectPoints();
	void SetupSplashPoints(Vec3& vOrigin, std::vector<Vec3>& vSplashPoints);
	std::vector<Point_t> GetSplashPoints(Vec3 vOrigin, std::vector<Vec3>& vSplashPoints, int iSimTime);

	void CalculateAngle(const Vec3& vLocalPos, const Vec3& vTargetPos, int iSimTime, Solution_t& tOut, int iFlags = CalculateFlagsEnum::Accuracy, int iTolerance = -1);
	bool TestAngle(const Vec3& vPoint, const Vec3& vAngles, int iSimTime, bool bSplash, bool bSecondTest = false);

	bool HandlePoint(const Vec3& vOrigin, int iSimTime, float flPitch, float flYaw, float flTime, const Vec3& vPoint, bool bSplash = false);
	bool HandleDirect(std::vector<Direct_t>& vDirectHistory);
	bool HandleSplash(std::vector<Splash_t>& vSplashHistory);

	int CanHit(Target_t& tTarget, CTFPlayer* pLocal, CTFWeaponBase* pWeapon, bool bVisuals = true);
	bool RunMain(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd);

	bool CanHit(Target_t& tTarget, CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CBaseEntity* pProjectile);
	bool TestAngle(CBaseEntity* pProjectile, const Vec3& vPoint, Vec3& vAngles, int iSimTime, bool bSplash);

	bool Aim(const Vec3& vCurAngle, const Vec3& vToAngle, Vec3& vOut, int iMethod = Vars::Aimbot::General::AimType.Value);
	void Aim(CUserCmd* pCmd, Vec3& vAngle, int iMethod = Vars::Aimbot::General::AimType.Value);

	Info_t m_tInfo = {};
	MoveStorage m_tMoveStorage = {};
	ProjectileInfo m_tProjInfo = {};
	std::vector<Vec3> m_vSplashPoints = {};

	bool m_bLastTickHeld = false;

	float m_flTimeTo = std::numeric_limits<float>::max();
	std::vector<Vec3> m_vPlayerPath = {};
	std::vector<Vec3> m_vProjectilePath = {};
	std::vector<DrawBox_t> m_vBoxes = {};

	Vec3 m_vAngleTo = {};
	Vec3 m_vPredicted = {};
	Vec3 m_vTarget = {};

	int m_iResult = false;
	bool m_bVisuals = true;

public:
	void Run(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd);
	float GetSplashRadius(CTFWeaponBase* pWeapon, CTFPlayer* pPlayer);

	bool AutoAirblast(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd, CBaseEntity* pProjectile);
	float GetSplashRadius(CBaseEntity* pProjectile, CTFWeaponBase* pWeapon = nullptr, CTFPlayer* pPlayer = nullptr, float flScale = 1.f, CTFWeaponBase* pAirblast = nullptr);

	int m_iLastTickCancel = 0;
};

ADD_FEATURE(CAimbotProjectile, AimbotProjectile);