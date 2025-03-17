#include "../SDK/SDK.h"

MAKE_SIGNATURE(CStaticPropMgr_DrawStaticProps, "engine.dll", "4C 8B DC 49 89 5B ? 49 89 6B ? 49 89 73 ? 57 41 54 41 55 41 56 41 57 48 83 EC ? 4C 8B 3D", 0x0);

MAKE_HOOK(CStaticPropMgr_DrawStaticProps, S::CStaticPropMgr_DrawStaticProps(), void,
	void* rcx, IClientRenderable** pProps, int count, bool bShadowDepth, bool drawVCollideWireframe)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::CStaticPropMgr_DrawStaticProps.Map[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx, pProps, count, bShadowDepth, drawVCollideWireframe);
#endif

	G::DrawingProps = true;
	CALL_ORIGINAL(rcx, pProps, count, bShadowDepth, drawVCollideWireframe);
	G::DrawingProps = false;
}