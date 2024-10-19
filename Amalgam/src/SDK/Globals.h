#pragma once
#include "Definitions/Definitions.h"
#include "Definitions/Main/CUserCmd.h"
#include "../Utils/Signatures/Signatures.h"
#include "../Utils/Memory/Memory.h"

MAKE_SIGNATURE(RandomSeed, "client.dll", "0F B6 1D ? ? ? ? 89 9D", 0x0);

struct VelFixRecord
{
	Vec3 m_vecOrigin;
	float m_flSimulationTime;
};

struct DrawLine
{
	std::pair<Vec3, Vec3> m_vPair;
	float m_flTime;
	Color_t m_color;
	bool m_bZBuffer = false;
};

struct DrawPath
{
	std::deque<Vec3> m_vPath;
	float m_flTime;
	Color_t m_color;
	int m_iStyle;
	bool m_bZBuffer = false;
};

struct DrawBox
{
	Vec3 m_vecPos;
	Vec3 m_vecMins;
	Vec3 m_vecMaxs;
	Vec3 m_vecOrientation;
	float m_flTime;
	Color_t m_colorEdge;
	Color_t m_colorFace;
	bool m_bZBuffer = false;
};

namespace G
{
	inline bool Unload = false;

	inline bool IsAttacking = false;
	inline bool CanPrimaryAttack = false;
	inline bool CanSecondaryAttack = false;
	inline bool CanHeadshot = false;
	inline bool IsThrowing = false;
	inline float Lerp = 0.015f;

	inline EWeaponType PrimaryWeaponType = {}, SecondaryWeaponType = {};

	inline CUserCmd* CurrentUserCmd = nullptr;
	inline CUserCmd* LastUserCmd = nullptr;
	inline int Buttons = 0;

	inline std::pair<int, int> Target = { 0, 0 };
	inline Vec3 AimPosition = {};
	inline Vec3 ViewAngles = {};

	inline bool SilentAngles = false;
	inline bool PSilentAngles = false;

	inline int ShiftedTicks = 0;
	inline int ShiftedGoal = 0;
	inline int WaitForShift = 0;
	inline bool DoubleTap = false;
	inline bool AntiWarp = false;
	inline bool Warp = false;
	inline bool Recharge = false;
	inline int MaxShift = 24;

	inline bool AntiAim = false;
	inline int ChokeAmount = 0;
	inline int ChokeGoal = 0;
	inline int AnticipatedChoke = 0;
	inline bool Choking = false;

	inline bool UpdatingAnims = false;
	inline bool DrawingProps = false;
	inline bool FlipViewmodels = false;

	inline std::unordered_map<int, std::deque<VelFixRecord>> VelocityMap = {};

	inline std::vector<DrawLine> LineStorage = {};
	inline std::vector<DrawPath> PathStorage = {};
	inline std::vector<DrawBox> BoxStorage = {};

	inline int* RandomSeed()
	{
		static auto dest = U::Memory.RelToAbs(S::RandomSeed());
		return reinterpret_cast<int*>(dest);
	}
};