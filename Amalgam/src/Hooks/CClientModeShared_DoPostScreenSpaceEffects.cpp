#include "../SDK/SDK.h"

#include "../Features/Visuals/Chams/Chams.h"
#include "../Features/Visuals/Glow/Glow.h"
#include "../Features/CameraWindow/CameraWindow.h"
#include "../Features/Visuals/Visuals.h"

MAKE_HOOK(CClientModeShared_DoPostScreenSpaceEffects, U::Memory.GetVFunc(I::ClientModeShared, 39), bool,
	void* rcx, const CViewSetup* pSetup)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::CClientModeShared_DoPostScreenSpaceEffects[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx, pSetup);
#endif

	F::Chams.mEntities.clear();

	if (I::EngineVGui->IsGameUIVisible() || Vars::Visuals::UI::CleanScreenshots.Value && I::EngineClient->IsTakingScreenshot())
		return CALL_ORIGINAL(rcx, pSetup);

	auto pLocal = H::Entities.GetLocal();
	auto pWeapon = H::Entities.GetWeapon();
	if (pLocal)
	{
		F::Visuals.SplashRadius(pLocal);
		if (pWeapon)
			F::Visuals.ProjectileTrace(pLocal, pWeapon);
	}

	if (F::CameraWindow.m_bDrawing)
		return CALL_ORIGINAL(rcx, pSetup);

	F::Visuals.DrawBoxes();
	F::Visuals.DrawPaths();
	F::Visuals.DrawLines();
	F::Visuals.DrawSightlines();
	if (pLocal)
	{
		F::Chams.RenderMain();
		F::Glow.RenderMain();
	}

	return CALL_ORIGINAL(rcx, pSetup);
}