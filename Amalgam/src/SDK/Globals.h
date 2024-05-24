#pragma once
#include "Definitions/Definitions.h"
#include "Definitions/Main/CUserCmd.h"
#include "../Utils/Signatures/Signatures.h"
#include "../Utils/Memory/Memory.h"

MAKE_SIGNATURE(Get_RandomSeed, "client.dll", "0F B6 1D ? ? ? ? 89 9D", 0x0);

struct DormantData
{
	Vec3 Location;
	float LastUpdate = 0.f;
};

struct VelFixRecord
{
	Vec3 m_vecOrigin;
	float m_flHeight;
	float m_flSimulationTime;
};

struct DrawBullet
{
	std::pair<Vec3, Vec3> m_line;
	float m_flTime;
	Color_t m_color;
	bool m_bZBuffer = false;
};

struct DrawLine
{
	std::deque<std::pair<Vec3, Vec3>> m_line;
	float m_flTime;
	Color_t m_color;
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
	inline float Lerp = 0.015f;

	inline EWeaponType WeaponType = {};
	inline int WeaponDefIndex = 0;

	inline CUserCmd* CurrentUserCmd = nullptr;
	inline CUserCmd* LastUserCmd = nullptr;
	inline int Buttons = 0;

	inline std::pair<int, int> Target = { 0, 0 };
	inline Vec3 AimPosition = {};
	inline Vec3 ViewAngles = {};
	inline Vec3 PunchAngles = {};

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
	inline bool Busy = false;
	inline int ChokeAmount = 0;
	inline int ChokeGoal = 0;
	inline int AnticipatedChoke = 0;
	inline bool Choking = false;

	inline bool UpdatingAnims = false;
	inline bool AnimateKart = false;
	inline bool DrawingProps = false;

	inline std::unordered_map<int, DormantData> DormancyMap = {};
	inline std::unordered_map<int, int> ChokeMap = {};
	inline std::unordered_map<int, VelFixRecord> VelocityMap = {};

	inline std::vector<DrawBullet> BulletsStorage = {};
	inline std::vector<DrawLine> LinesStorage = {};
	inline std::vector<DrawBox> BoxesStorage = {};

	inline int* RandomSeed()
	{
		static auto dest = U::Memory.RelToAbs(S::Get_RandomSeed());
		return reinterpret_cast<int*>(dest);
	}
};