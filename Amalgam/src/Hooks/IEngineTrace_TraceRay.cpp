#include "../SDK/SDK.h"

MAKE_HOOK(IEngineTrace_TraceRay, U::Memory.GetVirtual(I::EngineTrace, 4), void,
	void* rcx, const Ray_t& ray, unsigned int fMask, ITraceFilter* pTraceFilter, trace_t* pTrace)
{
	DEBUG_RETURN(IEngineTrace_TraceRay, rcx, ray, fMask, pTraceFilter, pTrace);

	CALL_ORIGINAL(rcx, ray, fMask, pTraceFilter, pTrace);
#ifdef DEBUG_TRACES
	if (Vars::Debug::VisualizeTraces.Value)
	{
		Vec3 vStart = ray.m_Start + ray.m_StartOffset;
		Vec3 vEnd = Vars::Debug::VisualizeTraceHits.Value ? pTrace->endpos : ray.m_Delta + vStart;
		G::LineStorage.emplace_back(std::pair<Vec3, Vec3>(vStart, vEnd), I::GlobalVars->curtime + 0.015f, Color_t(0, 0, 255), bool(GetAsyncKeyState(VK_MENU) & 0x8000));
		if (!ray.m_IsRay)
		{
			G::BoxStorage.emplace_back(ray.m_Start, -ray.m_Extents, ray.m_Extents, Vec3(), I::GlobalVars->curtime + 0.015f, Color_t(0, 0, 255), Color_t(0, 0, 0, 0), bool(GetAsyncKeyState(VK_MENU) & 0x8000));
			G::BoxStorage.emplace_back(vEnd - ray.m_StartOffset, -ray.m_Extents, ray.m_Extents, Vec3(), I::GlobalVars->curtime + 0.015f, Color_t(0, 0, 255), Color_t(0, 0, 0, 0), bool(GetAsyncKeyState(VK_MENU) & 0x8000));
		}
	}
#endif
}