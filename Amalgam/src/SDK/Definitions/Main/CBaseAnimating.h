#pragma once
#include "CBaseEntity.h"
#include "../Misc/Studio.h"

MAKE_SIGNATURE(CBaseAnimating_FrameAdvance, "client.dll", "48 89 5C 24 ? 48 89 6C 24 ? 57 48 81 EC ? ? ? ? 44 0F 29 54 24", 0x0);
MAKE_SIGNATURE(CBaseAnimating_GetBonePosition, "client.dll", "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 8B DA 49 8B F1", 0x0);
MAKE_SIGNATURE(CBaseAnimating_SequenceDuration, "client.dll", "48 89 5C 24 ? 57 48 83 EC ? 80 B9 ? ? ? ? ? 48 8B D9 8B B9", 0x0);

class CBaseAnimating : public CBaseEntity
{
public:
	NETVAR(m_nSequence, int, "CBaseAnimating", "m_nSequence");
	NETVAR(m_nForceBone, int, "CBaseAnimating", "m_nForceBone");
	NETVAR(m_vecForce, Vec3, "CBaseAnimating", "m_vecForce");
	NETVAR(m_nSkin, int, "CBaseAnimating", "m_nSkin");
	NETVAR(m_nBody, int, "CBaseAnimating", "m_nBody");
	NETVAR(m_nHitboxSet, int, "CBaseAnimating", "m_nHitboxSet");
	NETVAR(m_flModelScale, float, "CBaseAnimating", "m_flModelScale");
	NETVAR(m_flModelWidthScale, float, "CBaseAnimating", "m_flModelWidthScale");
	NETVAR(m_flPlaybackRate, float, "CBaseAnimating", "m_flPlaybackRate");
	NETVAR(m_flEncodedController, void*, "CBaseAnimating", "m_flEncodedController");
	NETVAR(m_bClientSideAnimation, bool, "CBaseAnimating", "m_bClientSideAnimation");
	NETVAR(m_bClientSideFrameReset, bool, "CBaseAnimating", "m_bClientSideFrameReset");
	NETVAR(m_nNewSequenceParity, int, "CBaseAnimating", "m_nNewSequenceParity");
	NETVAR(m_nResetEventsParity, int, "CBaseAnimating", "m_nResetEventsParity");
	NETVAR(m_nMuzzleFlashParity, int, "CBaseAnimating", "m_nMuzzleFlashParity");
	NETVAR(m_hLightingOrigin, EHANDLE, "CBaseAnimating", "m_hLightingOrigin");
	NETVAR(m_hLightingOriginRelative, EHANDLE, "CBaseAnimating", "m_hLightingOriginRelative");
	NETVAR(m_flCycle, float, "CBaseAnimating", "m_flCycle");
	NETVAR(m_fadeMinDist, float, "CBaseAnimating", "m_fadeMinDist");
	NETVAR(m_fadeMaxDist, float, "CBaseAnimating", "m_fadeMaxDist");
	NETVAR(m_flFadeScale, float, "CBaseAnimating", "m_flFadeScale");
	inline std::array<float, 24>& m_flPoseParameter()
	{
		static int nOffset = U::NetVars.GetNetVar("CBaseAnimating", "m_flPoseParameter");
		return *reinterpret_cast<std::array<float, 24>*>(uintptr_t(this) + nOffset);
	}

	NETVAR_OFF(GetModelPtr, CStudioHdr*, "CBaseAnimating", "m_nMuzzleFlashParity", 16);
	NETVAR_OFF(m_bSequenceLoops, bool, "CBaseAnimating", "m_flFadeScale", 13);
	inline CUtlVector<matrix3x4>* GetCachedBoneData()
	{
		static int nOffset = U::NetVars.GetNetVar("CBaseAnimating", "m_hLightingOrigin") - 88;
		return reinterpret_cast<CUtlVector<matrix3x4>*>(uintptr_t(this) + nOffset);
	}

	VIRTUAL_ARGS(GetAttachment, bool, 71, (int number, Vec3& origin), this, number, std::ref(origin))

	SIGNATURE_ARGS(FrameAdvance, float, CBaseAnimating, (float flInterval), this, flInterval);
	SIGNATURE_ARGS(GetBonePosition, float, CBaseAnimating, (int iBone, Vector& origin, QAngle& angles), this, iBone, std::ref(origin), std::ref(angles));
	SIGNATURE(SequenceDuration, float, CBaseAnimating, this);
	inline float SequenceDuration(int iSequence)
	{
		int iOriginalSequence = m_nSequence();
		m_nSequence() = iSequence;
		bool bReturn = S::CBaseAnimating_SequenceDuration.Call<float>(this);
		m_nSequence() = iOriginalSequence;
		return bReturn;
	}

	int GetHitboxGroup(int nHitbox);
	int GetNumOfHitboxes();
	Vec3 GetHitboxOrigin(matrix3x4* aBones, int nHitbox, Vec3 vOffset = {});
	Vec3 GetHitboxCenter(matrix3x4* aBones, int nHitbox, Vec3 vOffset = {});
	void GetHitboxInfo(matrix3x4* aBones, int nHitbox, Vec3* pCenter = nullptr, Vec3* pMins = nullptr, Vec3* pMaxs = nullptr, matrix3x4* pMatrix = nullptr, Vec3 vOffset = {});
};

class CBaseAnimatingOverlay : public CBaseAnimating
{
public:

};

class CCurrencyPack : public CBaseAnimating
{
public:
	NETVAR(m_bDistributed, bool, "CCurrencyPack", "m_bDistributed");
};

class CHalloweenPickup : public CBaseAnimating
{
public:
};

class CHalloweenGiftPickup : public CHalloweenPickup
{
public:
	NETVAR(m_hTargetPlayer, EHANDLE, "CHalloweenGiftPickup", "m_hTargetPlayer");
};