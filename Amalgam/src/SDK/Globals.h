#pragma once
#include "Definitions/Definitions.h"
#include "Definitions/Main/CUserCmd.h"
#include "../Utils/Signatures/Signatures.h"
#include "../Utils/Memory/Memory.h"

MAKE_SIGNATURE(RandomSeed, "client.dll", "0F B6 1D ? ? ? ? 89 9D", 0x0);

struct DrawLine_t
{
	std::pair<Vec3, Vec3> m_paOrigin;
	float m_flTime;
	Color_t m_tColor;
	bool m_bZBuffer = false;
};

struct DrawPath_t
{
	std::vector<Vec3> m_vPath;
	float m_flTime;
	Color_t m_tColor;
	int m_iStyle;
	bool m_bZBuffer = false;
};

struct DrawBox_t
{
	Vec3 m_vOrigin;
	Vec3 m_vMins;
	Vec3 m_vMaxs;
	Vec3 m_vAngles;
	float m_flTime;
	Color_t m_tColorEdge;
	Color_t m_tColorFace;
	bool m_bZBuffer = false;
};

struct DrawSphere_t
{
	Vec3 m_vOrigin;
	float m_flRadius;
	int m_nTheta;
	int m_nPhi;
	float m_flTime;
	Color_t m_tColorEdge;
	Color_t m_tColorFace;
	bool m_bZBuffer = false;
};

struct DrawSwept_t
{
	std::pair<Vec3, Vec3> m_paOrigin;
	Vec3 m_vMins;
	Vec3 m_vMaxs;
	Vec3 m_vAngles;
	float m_flTime;
	Color_t m_tColor;
	bool m_bZBuffer = false;
};

struct AimTarget_t
{
	int m_iEntIndex = 0;
	int m_iTickCount = 0;
	int m_iDuration = 32;
};

struct AimPoint_t
{
	Vec3 m_vOrigin = {};
	int m_iTickCount = 0;
	int m_iDuration = 32;
};

namespace G
{
	inline bool Unload = false;

	inline int Attacking = 0;
	inline bool Reloading = false;
	inline bool CanPrimaryAttack = false;
	inline bool CanSecondaryAttack = false;
	inline bool CanHeadshot = false;
	inline int Throwing = false;
	inline float Lerp = 0.015f;

	inline EWeaponType PrimaryWeaponType = {}, SecondaryWeaponType = {};

	inline CUserCmd* CurrentUserCmd = nullptr;
	inline CUserCmd* LastUserCmd = nullptr;
	inline CUserCmd OriginalCmd = {};

	inline AimTarget_t AimTarget = {};
	inline AimPoint_t AimPoint = {};

	inline bool SilentAngles = false;
	inline bool PSilentAngles = false;

	inline bool AntiAim = false;
	inline bool Choking = false;

	inline bool UpdatingAnims = false;
	inline bool FlipViewmodels = false;

	inline std::vector<DrawLine_t> LineStorage = {};
	inline std::vector<DrawPath_t> PathStorage = {};
	inline std::vector<DrawBox_t> BoxStorage = {};
	inline std::vector<DrawSphere_t> SphereStorage = {};
	inline std::vector<DrawSwept_t> SweptStorage = {};

	inline int* RandomSeed()
	{
		static auto pRandomSeed = reinterpret_cast<int*>(U::Memory.RelToAbs(S::RandomSeed()));
		return pRandomSeed;
	}
};