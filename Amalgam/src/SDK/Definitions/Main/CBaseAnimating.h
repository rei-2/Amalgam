#pragma once
#include "CBaseEntity.h"
#include "../Misc/Studio.h"
#include "../../../Utils/Math/Math.h"

MAKE_SIGNATURE(CBaseAnimating_FrameAdvance, "client.dll", "48 89 5C 24 ? 48 89 6C 24 ? 57 48 81 EC ? ? ? ? 44 0F 29 54 24", 0x0);
MAKE_SIGNATURE(CBaseAnimating_GetBonePosition, "client.dll", "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 8B DA 49 8B F1", 0x0);

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

	NETVAR_OFF(GetModelPtr, CStudioHdr*, "CBaseAnimating", "m_nMuzzleFlashParity", 12);

	int GetHitboxGroup(int nHitbox)
	{
		auto pModel = GetModel();
		if (!pModel) return -1;
		auto pHDR = I::ModelInfoClient->GetStudiomodel(pModel);
		if (!pHDR) return -1;
		auto pSet = pHDR->pHitboxSet(m_nHitboxSet());
		if (!pSet) return -1;
		auto pBox = pSet->pHitbox(nHitbox);
		if (!pBox) return -1;

		return pBox->group;
	}

	int GetNumOfHitboxes()
	{
		auto pModel = GetModel();
		if (!pModel) return 0;
		auto pHDR = I::ModelInfoClient->GetStudiomodel(pModel);
		if (!pHDR) return 0;
		auto pSet = pHDR->pHitboxSet(m_nHitboxSet());
		if (!pSet) return 0;

		return pSet->numhitboxes;
	}

	Vec3 GetHitboxPos(int nHitbox, Vec3 vOffset = {})
	{
		auto pModel = GetModel();
		if (!pModel) return {};
		auto pHDR = I::ModelInfoClient->GetStudiomodel(pModel);
		if (!pHDR) return {};
		auto pSet = pHDR->pHitboxSet(m_nHitboxSet());
		if (!pSet) return {};
		auto pBox = pSet->pHitbox(nHitbox);
		if (!pBox) return {};

		matrix3x4 BoneMatrix[128] = {};
		if (!SetupBones(BoneMatrix, 128, BONE_USED_BY_ANYTHING, I::GlobalVars->curtime))
			return {};

		Vec3 vOut = {};
		Math::VectorTransform(vOffset, BoneMatrix[pBox->bone], vOut);
		return vOut;
	}

	void GetHitboxInfo(int nHitbox, Vec3* pCenter = nullptr, Vec3* pMins = nullptr, Vec3* pMaxs = nullptr, matrix3x4* pMatrix = nullptr, Vec3 vOffset = {})
	{
		auto pModel = GetModel();
		if (!pModel) return;
		auto pHDR = I::ModelInfoClient->GetStudiomodel(pModel);
		if (!pHDR) return;
		auto pSet = pHDR->pHitboxSet(m_nHitboxSet());
		if (!pSet) return;
		auto pBox = pSet->pHitbox(nHitbox);
		if (!pBox) return;

		matrix3x4 BoneMatrix[128] = {};
		if (!SetupBones(BoneMatrix, 128, BONE_USED_BY_ANYTHING, I::GlobalVars->curtime))
			return;

		if (pMins)
			*pMins = pBox->bbmin;

		if (pMaxs)
			*pMaxs = pBox->bbmax;

		if (pCenter)
			Math::VectorTransform(vOffset, BoneMatrix[pBox->bone], *pCenter);

		if (pMatrix)
			memcpy(*pMatrix, BoneMatrix[pBox->bone], sizeof(matrix3x4));
	}

	std::array<float, 24>& m_flPoseParameter()
	{
		static int nOffset = U::NetVars.GetNetVar("CBaseAnimating", "m_flPoseParameter");
		return *reinterpret_cast<std::array<float, 24>*>(std::uintptr_t(this) + nOffset);
	}

	CUtlVector<matrix3x4>* GetCachedBoneData()
	{
		static int nOffset = U::NetVars.GetNetVar("CBaseAnimating", "m_hLightingOrigin") - 88;
		return reinterpret_cast<CUtlVector<matrix3x4>*>(std::uintptr_t(this) + nOffset);
	}

	float FrameAdvance(float flInterval)
	{
		return S::CBaseAnimating_FrameAdvance.As<float(__thiscall*)(void*, float)>()(this, flInterval);
	}

	void GetBonePosition(int iBone, Vector& origin, QAngle& angles)
	{
		S::CBaseAnimating_GetBonePosition.As<void(__thiscall*)(void*, int, Vector&, QAngle&)>()(this, iBone, origin, angles);
	}

	__inline bool GetAttachment(int number, Vec3& origin)
	{
		return reinterpret_cast<bool(__thiscall*)(void*, int, Vec3&)>(U::Memory.GetVFunc(this, 71))(this, number, origin);
	}
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