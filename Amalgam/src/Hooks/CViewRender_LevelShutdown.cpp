#include "../SDK/SDK.h"

#include "../Features/Spectate/Spectate.h"
#include "../Features/Visuals/FlatTextures/FlatTextures.h"

MAKE_HOOK(CViewRender_LevelShutdown, U::Memory.GetVirtual(I::ViewRender, 2), void,
	void* rcx)
{
	F::Spectate.m_iIntendedTarget = -1;
	F::FlatTextures.OnLevelShutdown();

	CALL_ORIGINAL(rcx);
}