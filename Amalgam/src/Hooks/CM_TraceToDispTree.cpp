#include "../SDK/SDK.h"

#include "../SDK/Definitions/Misc/TraceInfo.h"

MAKE_SIGNATURE(CM_TraceToDispTree_True, "engine.dll", "48 8B C4 48 89 58 ? 48 89 70 ? 57 48 81 EC ? ? ? ? F3 0F 10 01", 0x0);
MAKE_SIGNATURE(CM_TraceToDispTree_False, "engine.dll", "48 8B C4 48 89 58 ? 48 89 70 ? 57 48 83 EC ? F3 0F 10 01", 0x0);
MAKE_SIGNATURE(IntersectRayWithTriangle, "engine.dll", "48 8B C4 48 81 EC ? ? ? ? 80 BC 24", 0x0);
MAKE_SIGNATURE(CDispCollTree_SweepAABBTriIntersect, "engine.dll", "48 8B C4 48 89 70 ? 48 89 78 ? 55 41 56 41 57 48 8D 68", 0x0);
MAKE_SIGNATURE(CDispCollTree_EdgeCrossAxis_0, "engine.dll", "48 83 EC ? B8 ? ? ? ? 66 44 3B C0 75 ? B0 ? 48 83 C4 ? C3 48 8B 89 ? ? ? ? F3 0F 10 05 ? ? ? ? 0F 29 74 24 ? 0F 29 7C 24 ? 44 0F 29 04 24 41 0F B7 C0 66 45 85 C0 79 ? 25 ? ? ? ? 48 8D 04 40 F3 44 0F 10 04 81", 0x0);

#define DISPCOLL_DIST_EPSILON 0.03125f

static int s_iMask = 0;
static std::optional<Vec3> s_vOriginal = {};
static CDispCollTri* s_pTri = nullptr;

MAKE_HOOK(CM_TraceToDispTree_True, S::CM_TraceToDispTree_True(), float,
	TraceInfo_t* pTraceInfo, CDispCollTree* pDispTree, float startFrac, float endFrac)
{
	DEBUG_RETURN(CM_TraceToDispTree_True, pTraceInfo, pDispTree, startFrac, endFrac);

	s_iMask = pTraceInfo->m_contents;

	return CALL_ORIGINAL(pTraceInfo, pDispTree, startFrac, endFrac);
}

MAKE_HOOK(CM_TraceToDispTree_False, S::CM_TraceToDispTree_False(), float,
	TraceInfo_t* pTraceInfo, CDispCollTree* pDispTree, float startFrac, float endFrac)
{
	DEBUG_RETURN(CM_TraceToDispTree_False, pTraceInfo, pDispTree, startFrac, endFrac);

	s_iMask = pTraceInfo->m_contents;

	return CALL_ORIGINAL(pTraceInfo, pDispTree, startFrac, endFrac);
}

MAKE_HOOK(IntersectRayWithTriangle, S::IntersectRayWithTriangle(), float,
	const Ray_t& ray, const Vector& v1, const Vector& v2, const Vector& v3, bool oneSided)
{
	if (s_iMask & CONTENTS_DISPSOLID)
		oneSided = false;

	return CALL_ORIGINAL(ray, v1, v2, v3, oneSided);
}

MAKE_HOOK(CDispCollTree_SweepAABBTriIntersect, S::CDispCollTree_SweepAABBTriIntersect(), void,
	void* rcx, const Ray_t& ray, const Vector& rayDir, int iTri, CDispCollTri* pTri, CBaseTrace* pTrace)
{
	DEBUG_RETURN(CDispCollTree_SweepAABBTriIntersect, rcx, ray, rayDir, iTri, pTri, pTrace);

	if (s_iMask & CONTENTS_DISPSOLID && pTri->m_vecNormal.Dot(ray.m_Delta) > DISPCOLL_DIST_EPSILON)
	{
		s_vOriginal = pTri->m_vecNormal, s_pTri = pTri;
		pTri->m_vecNormal = -pTri->m_vecNormal;
	}

	CALL_ORIGINAL(rcx, ray, rayDir, iTri, pTri, pTrace);

	if (s_vOriginal)
		pTri->m_vecNormal = *s_vOriginal, s_vOriginal = std::nullopt;
}

MAKE_HOOK(CDispCollTree_EdgeCrossAxis_0, S::CDispCollTree_EdgeCrossAxis_0(), void,
	void* rcx, const Ray_t& ray, unsigned short iPlane, CDispCollHelper* pHelper)
{
	DEBUG_RETURN(CDispCollTree_EdgeCrossAxis_0, rcx, ray, iPlane, pHelper);

	if (s_vOriginal)
		s_pTri->m_vecNormal = *s_vOriginal, s_vOriginal = std::nullopt;

	CALL_ORIGINAL(rcx, ray, iPlane, pHelper);
}