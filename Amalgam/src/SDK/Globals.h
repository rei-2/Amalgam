#pragma once
#include "Definitions/Definitions.h"
#include "Definitions/Main/CUserCmd.h"
#include "../Utils/Signatures/Signatures.h"
#include "../Utils/Memory/Memory.h"

MAKE_SIGNATURE(RandomSeed, "client.dll", "0F B6 1D ? ? ? ? 89 9D", 0x0);

struct DrawLine_t
{
	std::pair<Vec3, Vec3> m_vPair;
	float m_flTime;
	Color_t m_color;
	bool m_bZBuffer = false;
};

struct DrawPath_t
{
	std::deque<Vec3> m_vPath;
	float m_flTime;
	Color_t m_color;
	int m_iStyle;
	bool m_bZBuffer = false;
};

struct DrawBox_t
{
	Vec3 m_vPos;
	Vec3 m_vMins;
	Vec3 m_vMaxs;
	Vec3 m_vRotation;
	float m_flTime;
	Color_t m_tColorEdge;
	Color_t m_tColorFace;
	bool m_bZBuffer = false;
};

struct MoveData_t
{
	Vec3 m_vMove = {};
	Vec3 m_vView = {};
	int m_iButtons = 0;
};

namespace G
{
	inline bool Unload = false;

	inline int Attacking = 0;
	inline bool Reloading = false;
	inline bool CanPrimaryAttack = false;
	inline bool CanSecondaryAttack = false;
	inline bool CanHeadshot = false;
	inline bool Throwing = false;
	inline float Lerp = 0.015f;

	inline EWeaponType PrimaryWeaponType = {}, SecondaryWeaponType = {};

	inline CUserCmd* CurrentUserCmd = nullptr;
	inline CUserCmd* LastUserCmd = nullptr;
	inline MoveData_t OriginalMove = {};

	inline std::pair<int, int> Target = { 0, 0 };
	inline std::pair<Vec3, int> AimPosition = {};

	inline bool SilentAngles = false;
	inline bool PSilentAngles = false;

	inline bool AntiAim = false;
	inline bool Choking = false;

	inline bool UpdatingAnims = false;
	inline bool DrawingProps = false;
	inline bool FlipViewmodels = false;

	inline std::vector<DrawLine_t> LineStorage = {};
	inline std::vector<DrawPath_t> PathStorage = {};
	inline std::vector<DrawBox_t> BoxStorage = {};

	inline int* RandomSeed()
	{
		static auto dest = U::Memory.RelToAbs(S::RandomSeed());
		return reinterpret_cast<int*>(dest);
	}
};