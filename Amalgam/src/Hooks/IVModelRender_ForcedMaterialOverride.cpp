#include "../SDK/SDK.h"

#include "../Features/Visuals/Chams/Chams.h"
#include "../Features/Visuals/Glow/Glow.h"
#include "../Features/Visuals/Materials/Materials.h"

MAKE_HOOK(IVModelRender_ForcedMaterialOverride, U::Memory.GetVirtual(I::ModelRender, 1), void,
	IVModelRender* rcx, IMaterial* mat, OverrideType_t type)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::IVModelRender_ForcedMaterialOverride[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx, mat, type);
#endif

	if (F::Chams.m_bRendering || F::Glow.m_bRendering)
		return;

	CALL_ORIGINAL(rcx, mat, type);
}