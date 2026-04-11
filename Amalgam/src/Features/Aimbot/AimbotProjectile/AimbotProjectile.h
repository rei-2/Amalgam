#pragma once
#include "../../../SDK/SDK.h"

#include "../AimbotGlobal/AimbotGlobal.h"
#include "../../Simulation/MovementSimulation/MovementSimulation.h"
#include "../../Simulation/ProjectileSimulation/ProjectileSimulation.h"

Enum(PointFlags, None = 0, Regular = 1 << 0, Lob = 1 << 1)
Enum(PointType, Direct, Geometry, Air)
Enum(CalculateFlags, None = 0, TwoPass = 1 << 0, SetupClip = 1 << 1, AccountDrag = 1 << 2, LobAngle = 1 << 3, Accuracy = TwoPass | SetupClip | AccountDrag)
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
	float m_flBoundsTime = 0.f;
	float m_flOffsetTime = 0.f;
	int m_iSplashRestrict = 0;
	int m_iArmTime = 0;
	float m_flNormalOffset = 0.f;
	bool m_bIgnoreTiming = false;
};

#pragma pack(1)
struct Solution_t
{
	float m_flPitch = 0.f;
	float m_flYaw = 0.f;
	float m_flTime = 0.f;
	uint8_t m_iCalculated = CalculateResultEnum::Pending;
};
#pragma pack()

struct Setup_t
{
	Vec3 m_vPoint = {};
	uint8_t m_iType = PointTypeEnum::Geometry;
};
struct Point_t
{
	Vec3 m_vPoint = {};
	Solution_t m_tSolution = {};
	uint8_t m_iType = PointTypeEnum::Direct;
};

struct Offset_t
{
	Vec3 m_vOffset;
	uint8_t m_iFlags;
};
using Directs_t = std::unordered_map<uint8_t, Offset_t>;
using Splashes_t = std::vector<uint8_t>;

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
using DirectHistory_t = std::unordered_map<uint8_t, std::vector<Direct_t>>;
using SplashHistory_t = std::unordered_map<uint8_t, std::vector<Splash_t>>;

class CAimbotProjectile
{
private:
	Directs_t GetDirects();
	Splashes_t GetSplashes();
	void SetupSplashPoints(Vec3& vOrigin, std::vector<Setup_t>& vSplashPoints, uint8_t iFlags = CalculateFlagsEnum::None);
	std::vector<Point_t> GetSplashPoints(Vec3 vOrigin, std::vector<Setup_t>& vSplashPoints, int iSimTime, uint8_t iFlags = CalculateFlagsEnum::Accuracy, bool bFirst = false);

	void CalculateAngle(const Vec3& vLocalPos, const Vec3& vTargetPos, int iSimTime, Solution_t& tOut, uint8_t iFlags = CalculateFlagsEnum::Accuracy, int iTolerance = -1);
	bool TestAngle(const Vec3& vPoint, const Vec3& vAngles, int iSimTime, uint8_t iType, uint8_t iFlags, bool bSecondTest = false);

	bool HandlePoint(const Vec3& vOrigin, int iSimTime, float flPitch, float flYaw, float flTime, const Vec3& vPoint, uint8_t iType = PointTypeEnum::Direct, uint8_t iFlags = PointFlagsEnum::Regular);
	bool HandleDirect(DirectHistory_t& vDirectHistory);
	bool HandleSplash(SplashHistory_t& vSplashHistory);

	int CanHit(Target_t& tTarget, CTFPlayer* pLocal, CTFWeaponBase* pWeapon, bool bUpdate = true);
	bool RunMain(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd);

	bool CanHit(Target_t& tTarget, CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CBaseEntity* pProjectile);
	bool TestAngle(CBaseEntity* pProjectile, const Vec3& vPoint, Vec3& vAngles, int iSimTime, uint8_t iType, uint8_t iFlags);

	bool Aim(const Vec3& vCurAngle, const Vec3& vToAngle, Vec3& vOut, int iMethod = Vars::Aimbot::General::AimType.Value);
	void Aim(CUserCmd* pCmd, Vec3& vAngles, int iMethod = Vars::Aimbot::General::AimType.Value);

	Info_t m_tInfo = {};
	MoveStorage m_tMoveStorage = {};
	ProjectileInfo m_tProjInfo = {};
	std::vector<Setup_t> m_vSplashPoints = {};

	bool m_bLastTickHeld = false;

	float m_flTimeTo = std::numeric_limits<float>::max();
	std::vector<Vec3> m_vPlayerPath = {};
	std::vector<Vec3> m_vProjectilePath = {};
	std::vector<DrawBox_t> m_vBoxes = {};

	Vec3 m_vAngleTo = {};
	Vec3 m_vPredicted = {};
	Vec3 m_vTarget = {};

	int m_iResult = false;
	bool m_bUpdate = true;

public:
	void Run(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd);
	float GetSplashRadius(CTFWeaponBase* pWeapon, CTFPlayer* pPlayer, float flScale = 1.f);

	bool AutoAirblast(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd, CBaseEntity* pProjectile);
	float GetSplashRadius(CBaseEntity* pProjectile, CTFWeaponBase* pWeapon = nullptr, CTFPlayer* pPlayer = nullptr, float flScale = 1.f, CTFWeaponBase* pAirblast = nullptr);

	int m_iLastTickCancel = 0;
};

ADD_FEATURE(CAimbotProjectile, AimbotProjectile);