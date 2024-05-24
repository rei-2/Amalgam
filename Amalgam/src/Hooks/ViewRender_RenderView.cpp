#include "../SDK/SDK.h"

#include "../Features/CameraWindow/CameraWindow.h"

MAKE_HOOK(ViewRender_RenderView, U::Memory.GetVFunc(I::ViewRender, 6), void, __fastcall,
	void* ecx, const CViewSetup& view, ClearFlags_t nClearFlags, RenderViewInfo_t whatToDraw)
{
	CALL_ORIGINAL(ecx, view, nClearFlags, whatToDraw);
	if (Vars::Visuals::UI::CleanScreenshots.Value && I::EngineClient->IsTakingScreenshot() || G::Unload)
		return;

	F::CameraWindow.RenderView(ecx, view);
}