#include "CBaseAnimating.h"

#include "../../SDK.h"

mstudiohitboxset_t* CBaseAnimating::GetHitboxSet()
{
	auto pHDR = GetStudiomodel();
	if (!pHDR)
		return nullptr;

	return pHDR->pHitboxSet(m_nHitboxSet());
}

int CBaseAnimating::GetHitboxGroup(int nHitbox)
{
	auto pSet = GetHitboxSet();
	if (!pSet)
		return -1;
	auto pBox = pSet->pHitbox(nHitbox);
	if (!pBox)
		return -1;

	return pBox->group;
}

int CBaseAnimating::GetNumOfHitboxes()
{
	auto pSet = GetHitboxSet();
	if (!pSet)
		return 0;

	return pSet->numhitboxes;
}

Vec3 CBaseAnimating::GetHitboxOrigin(matrix3x4* aBones, int nHitbox, Vec3 vOffset)
{
	auto pSet = GetHitboxSet();
	if (!pSet || pSet->numhitboxes <= nHitbox)
		return {};
	auto pBox = pSet->pHitbox(nHitbox);
	if (!pBox)
		return {};

	Vec3 vOut; Math::VectorTransform(vOffset, aBones[pBox->bone], vOut);
	return vOut;
}

Vec3 CBaseAnimating::GetHitboxCenter(matrix3x4* aBones, int nHitbox, Vec3 vOffset)
{
	auto pSet = GetHitboxSet();
	if (!pSet || pSet->numhitboxes <= nHitbox)
		return {};
	auto pBox = pSet->pHitbox(nHitbox);
	if (!pBox)
		return {};

	Vec3 vOut; Math::VectorTransform((pBox->bbmin + pBox->bbmax) / 2 + vOffset, aBones[pBox->bone], vOut);
	return vOut;
}

void CBaseAnimating::GetHitboxInfo(matrix3x4* aBones, int nHitbox, Vec3* pCenter, Vec3* pMins, Vec3* pMaxs, matrix3x4* pMatrix, Vec3 vOffset)
{
	auto pSet = GetHitboxSet();
	if (!pSet || pSet->numhitboxes <= nHitbox)
		return;
	auto pBox = pSet->pHitbox(nHitbox);
	if (!pBox)
		return;

	if (pMins)
		*pMins = pBox->bbmin;

	if (pMaxs)
		*pMaxs = pBox->bbmax;

	if (pCenter)
		Math::VectorTransform(vOffset, aBones[pBox->bone], *pCenter);

	if (pMatrix)
		memcpy(*pMatrix, aBones[pBox->bone], sizeof(matrix3x4));
}