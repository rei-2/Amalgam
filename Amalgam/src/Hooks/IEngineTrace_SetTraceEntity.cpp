#include "../SDK/SDK.h"

MAKE_SIGNATURE(CM_BoxTrace, "engine.dll", "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 56 48 83 EC ? 45 0F B6 F1", 0x0);
MAKE_SIGNATURE(CM_TraceToLeaf_True, "engine.dll", "48 8B C4 55 57 48 81 EC ? ? ? ? 48 8B B9 ? ? ? ? 48 8B E9 44 0F 29 40 ? 44 0F 28 C3 44 0F 29 48 ? 44 0F 28 CA 85 D2 0F 88 ? ? ? ? 3B 97 ? ? ? ? 0F 8D ? ? ? ? 48 63 89 ? ? ? ? 48 89 70 ? 4C 89 60 ? 4C 89 68 ? 44 8B AC 8D ? ? ? ? 4C 89 70 ? 45 33 F6 4C 89 78 ? 0F 29 78 ? 0F 57 FF 4C 63 FA 49 C1 E7 ? 4C 03 BF ? ? ? ? 48 89 58 ? 48 8B C1 48 C1 E0 ? 41 0F B7 77 ? 41 0F B7 57 ? 4C 8B A4 28 ? ? ? ? 03 D6 3B F2 0F 83 ? ? ? ? 8B C2 8B DE 48 89 84 24 ? ? ? ? 41 BA ? ? ? ? 48 85 DB 0F 88 ? ? ? ? 3B B7 ? ? ? ? 0F 8D ? ? ? ? 48 8B 87 ? ? ? ? 0F B7 0C 58 3B 8F ? ? ? ? 0F 8D ? ? ? ? 48 8B 87 ? ? ? ? 4C 8D 1C C8 45 39 2C 8C 0F 84 ? ? ? ? 45 89 2C 8C 41 8B 03 8B 8D ? ? ? ? 23 C1 0F 84 ? ? ? ? 3D ? ? ? ? 0F 85 ? ? ? ? 0F BA E1 ? 0F 83 ? ? ? ? 41 0F B7 4B ? 41 0F B7 43 ? 4C 8B 8D ? ? ? ? 66 41 3B CA 75 ? 41 3B 81 ? ? ? ? 0F 8D ? ? ? ? 48 8D 14 40 4D 8B C6 48 C1 E2 ? 48 83 C2 ? 49 03 91 ? ? ? ? 0F B7 0A 66 41 3B CA 75 ? 48 8D 05 ? ? ? ? EB ? 41 3B 89 ? ? ? ? 0F 8D ? ? ? ? 48 8B C1 48 C1 E0 ? 49 03 81 ? ? ? ? F6 40 ? ? 0F 85 ? ? ? ? 49 FF C0 48 83 C2 ? 49 83 F8 ? 7C ? EB ? 41 3B 41 ? 0F 8D ? ? ? ? 48 8B D0 45 8B C6 48 C1 E2 ? 44 8B D1 48 83 C2 ? 85 C9 74 ? 49 03 51 ? 66 90 0F B7 0A B8 ? ? ? ? 66 3B C8 75 ? 48 8D 05 ? ? ? ? EB ? 41 3B 89 ? ? ? ? 0F 8D ? ? ? ? 48 8B C1 48 C1 E0 ? 49 03 81 ? ? ? ? F6 40 ? ? 75 ? 41 FF C0 48 83 C2 ? 45 3B C2 7C ? 49 8B D3 48 8B CD E8 ? ? ? ? 0F 2E BD ? ? ? ? 0F 84 ? ? ? ? 41 BA ? ? ? ? FF C6 48 FF C3 48 3B 9C 24 ? ? ? ? 0F 8C ? ? ? ? 44 38 B5 ? ? ? ? 0F 85 ? ? ? ? 41 0F B7 57 ? 66 85 D2 0F 84 ? ? ? ? 48 63 8D ? ? ? ? 48 8B C1 0F 29 74 24", 0x0);
MAKE_SIGNATURE(CM_TraceToLeaf_False, "engine.dll", "48 8B C4 55 57 48 81 EC ? ? ? ? 48 8B B9 ? ? ? ? 48 8B E9 44 0F 29 40 ? 44 0F 28 C3 44 0F 29 48 ? 44 0F 28 CA 85 D2 0F 88 ? ? ? ? 3B 97 ? ? ? ? 0F 8D ? ? ? ? 48 63 89 ? ? ? ? 48 89 70 ? 4C 89 60 ? 4C 89 68 ? 44 8B AC 8D ? ? ? ? 4C 89 70 ? 45 33 F6 4C 89 78 ? 0F 29 78 ? 0F 57 FF 4C 63 FA 49 C1 E7 ? 4C 03 BF ? ? ? ? 48 89 58 ? 48 8B C1 48 C1 E0 ? 41 0F B7 77 ? 41 0F B7 57 ? 4C 8B A4 28 ? ? ? ? 03 D6 3B F2 0F 83 ? ? ? ? 8B C2 8B DE 48 89 84 24 ? ? ? ? 41 BA ? ? ? ? 48 85 DB 0F 88 ? ? ? ? 3B B7 ? ? ? ? 0F 8D ? ? ? ? 48 8B 87 ? ? ? ? 0F B7 0C 58 3B 8F ? ? ? ? 0F 8D ? ? ? ? 48 8B 87 ? ? ? ? 4C 8D 1C C8 45 39 2C 8C 0F 84 ? ? ? ? 45 89 2C 8C 41 8B 03 8B 8D ? ? ? ? 23 C1 0F 84 ? ? ? ? 3D ? ? ? ? 0F 85 ? ? ? ? 0F BA E1 ? 0F 83 ? ? ? ? 41 0F B7 4B ? 41 0F B7 43 ? 4C 8B 8D ? ? ? ? 66 41 3B CA 75 ? 41 3B 81 ? ? ? ? 0F 8D ? ? ? ? 48 8D 14 40 4D 8B C6 48 C1 E2 ? 48 83 C2 ? 49 03 91 ? ? ? ? 0F B7 0A 66 41 3B CA 75 ? 48 8D 05 ? ? ? ? EB ? 41 3B 89 ? ? ? ? 0F 8D ? ? ? ? 48 8B C1 48 C1 E0 ? 49 03 81 ? ? ? ? F6 40 ? ? 0F 85 ? ? ? ? 49 FF C0 48 83 C2 ? 49 83 F8 ? 7C ? EB ? 41 3B 41 ? 0F 8D ? ? ? ? 48 8B D0 45 8B C6 48 C1 E2 ? 44 8B D1 48 83 C2 ? 85 C9 74 ? 49 03 51 ? 66 90 0F B7 0A B8 ? ? ? ? 66 3B C8 75 ? 48 8D 05 ? ? ? ? EB ? 41 3B 89 ? ? ? ? 0F 8D ? ? ? ? 48 8B C1 48 C1 E0 ? 49 03 81 ? ? ? ? F6 40 ? ? 75 ? 41 FF C0 48 83 C2 ? 45 3B C2 7C ? 49 8B D3 48 8B CD E8 ? ? ? ? 0F 2E BD ? ? ? ? 0F 84 ? ? ? ? 41 BA ? ? ? ? FF C6 48 FF C3 48 3B 9C 24 ? ? ? ? 0F 8C ? ? ? ? 44 38 B5 ? ? ? ? 0F 85 ? ? ? ? 41 0F B7 57 ? 66 85 D2 0F 84 ? ? ? ? 48 63 8D ? ? ? ? 48 8B C1 0F 29 B4 24", 0x0);
MAKE_SIGNATURE(CM_ClipBoxToBrush_True, "engine.dll", "48 89 5C 24 ? 57 48 81 EC ? ? ? ? 44 0F B7 42", 0x0);
MAKE_SIGNATURE(CM_ClipBoxToBrush_False, "engine.dll", "48 8B C4 48 89 58 ? 57 48 81 EC ? ? ? ? 44 0F B7 42", 0x0);
MAKE_SIGNATURE(IEngineTrace_TraceRay_SetTraceEntity_Call, "engine.dll", "40 38 7E ? 0F 85", 0x0);

#define MAX_CHECK_COUNT_DEPTH 2
typedef uint32 TraceCounter_t;
typedef CUtlVector<TraceCounter_t> CTraceCounterVec;

struct TraceInfo_t
{
	Vector m_start;
	Vector m_end;
	Vector m_mins;
	Vector m_maxs;
	Vector m_extents;
	Vector m_delta;
	Vector m_invDelta;
	trace_t m_trace;
	trace_t m_stabTrace;
	int m_contents;
	bool m_ispoint;
	bool m_isswept;
	void* m_pBSPData;
	Vector m_DispStabDir;
	int m_bDispHit;
	bool m_bCheckPrimary;
	int m_nCheckDepth;
	TraceCounter_t m_Count[MAX_CHECK_COUNT_DEPTH];
	CTraceCounterVec m_BrushCounters[MAX_CHECK_COUNT_DEPTH];
	CTraceCounterVec m_DispCounters[MAX_CHECK_COUNT_DEPTH];
};

static int iOriginalMask = false;
static bool bOriginalStartSolid = false;

MAKE_HOOK(IEngineTrace_SetTraceEntity, U::Memory.GetVFunc(I::EngineTrace, 20), void,
	void* rcx, ICollideable* pCollideable, trace_t* pTrace)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::IEngineTrace_SetTraceEntity[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx, ray, fMask, pTraceFilter, pTrace);
#endif

	static const auto dwDesired = S::IEngineTrace_TraceRay_SetTraceEntity_Call();
	const auto dwRetAddr = uintptr_t(_ReturnAddress());

	CALL_ORIGINAL(rcx, pCollideable, pTrace);

	if (dwRetAddr && dwDesired && iOriginalMask & CONTENTS_NOSTARTSOLID)
		pTrace->startsolid = false;
}

MAKE_HOOK(CM_BoxTrace, S::CM_BoxTrace(), void,
	const Ray_t& ray, int headnode, int brushmask, bool computeEndpt, trace_t& tr)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::IEngineTrace_SetTraceEntity[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx, ray, fMask, pTraceFilter, pTrace);
#endif

	CALL_ORIGINAL(ray, headnode, brushmask, computeEndpt, tr);

	iOriginalMask = brushmask;
}

MAKE_HOOK(CM_TraceToLeaf_True, S::CM_TraceToLeaf_True(), void,
	TraceInfo_t* pTraceInfo, int ndxLeaf, float startFrac, float endFrac)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::IEngineTrace_SetTraceEntity[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx, ray, fMask, pTraceFilter, pTrace);
#endif

	if (pTraceInfo->m_contents & CONTENTS_NOSTARTSOLID)
	{
		bOriginalStartSolid = pTraceInfo->m_trace.startsolid;
		pTraceInfo->m_trace.startsolid = false;
		CALL_ORIGINAL(pTraceInfo, ndxLeaf, startFrac, endFrac);
		pTraceInfo->m_trace.startsolid = bOriginalStartSolid;
		return;
	}

	CALL_ORIGINAL(pTraceInfo, ndxLeaf, startFrac, endFrac);
}

MAKE_HOOK(CM_TraceToLeaf_False, S::CM_TraceToLeaf_False(), void,
	TraceInfo_t* pTraceInfo, int ndxLeaf, float startFrac, float endFrac)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::IEngineTrace_SetTraceEntity[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx, ray, fMask, pTraceFilter, pTrace);
#endif

	if (pTraceInfo->m_contents & CONTENTS_NOSTARTSOLID)
	{
		bOriginalStartSolid = pTraceInfo->m_trace.startsolid;
		pTraceInfo->m_trace.startsolid = false;
		CALL_ORIGINAL(pTraceInfo, ndxLeaf, startFrac, endFrac);
		pTraceInfo->m_trace.startsolid = bOriginalStartSolid;
		return;
	}

	CALL_ORIGINAL(pTraceInfo, ndxLeaf, startFrac, endFrac);
}

MAKE_HOOK(CM_ClipBoxToBrush_True, S::CM_ClipBoxToBrush_True(), void,
	TraceInfo_t* pTraceInfo, void* brush)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::IEngineTrace_SetTraceEntity[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx, ray, fMask, pTraceFilter, pTrace);
#endif

	CALL_ORIGINAL(pTraceInfo, brush);

	if (pTraceInfo->m_contents & CONTENTS_NOSTARTSOLID && pTraceInfo->m_trace.startsolid)
	{
		bOriginalStartSolid = true;
		pTraceInfo->m_trace.startsolid = false;
	}
}

MAKE_HOOK(CM_ClipBoxToBrush_False, S::CM_ClipBoxToBrush_False(), void,
	TraceInfo_t* pTraceInfo, void* brush)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::IEngineTrace_SetTraceEntity[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx, ray, fMask, pTraceFilter, pTrace);
#endif

	CALL_ORIGINAL(pTraceInfo, brush);

	if (pTraceInfo->m_contents & CONTENTS_NOSTARTSOLID && pTraceInfo->m_trace.startsolid)
	{
		bOriginalStartSolid = true;
		pTraceInfo->m_trace.startsolid = false;
	}
}