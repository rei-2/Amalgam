#include "../SDK/SDK.h"

#include "../Features/Spectate/Spectate.h"

MAKE_HOOK(CViewRender_LevelShutdown, U::Memory.GetVirtual(I::ViewRender, 2), void,
	void* rcx)
{
	F::Spectate.m_iIntendedTarget = -1;

	CALL_ORIGINAL(rcx);
}