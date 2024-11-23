#include "../SDK/SDK.h"

#include "../Features/Visuals/Materials/Materials.h"

MAKE_HOOK(CViewRender_LevelShutdown, U::Memory.GetVFunc(I::ViewRender, 2), void,
	void* rcx)
{
	F::Materials.UnloadMaterials();

	CALL_ORIGINAL(rcx);
}