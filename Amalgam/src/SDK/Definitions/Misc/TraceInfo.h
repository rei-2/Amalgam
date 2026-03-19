#pragma once
#include "CUtlVector.h"
#include "BaseTypes.h"
#include "../Main/CGameTrace.h"
#include "../Types.h"

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