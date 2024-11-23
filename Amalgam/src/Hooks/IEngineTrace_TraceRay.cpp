#include "../SDK/SDK.h"

#ifdef DEBUG_TRACES
MAKE_HOOK(IEngineTrace_TraceRay, U::Memory.GetVFunc(I::EngineTrace, 4), void,
	void* rcx, const Ray_t& ray, unsigned int fMask, ITraceFilter* pTraceFilter, trace_t* pTrace)
{
	CALL_ORIGINAL(rcx, ray, fMask, pTraceFilter, pTrace);
	if (Vars::Debug::VisualizeTraces.Value)
	{
		Vec3 vStart = ray.m_Start + ray.m_StartOffset;
		Vec3 vEnd = Vars::Debug::VisualizeTraceHits.Value ? pTrace->endpos : ray.m_Delta + vStart;
		G::LineStorage.push_back({ { vStart, vEnd }, I::GlobalVars->curtime + 0.015f, { 0, 0, 255, 255 }, bool(GetAsyncKeyState(VK_MENU) & 0x8000) });
		if (!ray.m_IsRay)
		{
			G::BoxStorage.push_back({ ray.m_Start, ray.m_Extents * -1, ray.m_Extents, {}, I::GlobalVars->curtime + 0.015f, { 0, 0, 255, 255 }, { 0, 0, 0, 0 }, bool(GetAsyncKeyState(VK_MENU) & 0x8000) });
			G::BoxStorage.push_back({ vEnd - ray.m_StartOffset, ray.m_Extents * -1, ray.m_Extents, {}, I::GlobalVars->curtime + 0.015f, { 0, 0, 255, 255 }, { 0, 0, 0, 0 }, bool(GetAsyncKeyState(VK_MENU) & 0x8000) });
		}
	}
}
#endif