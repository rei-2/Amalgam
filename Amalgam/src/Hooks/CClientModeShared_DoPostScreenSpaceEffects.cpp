#include "../SDK/SDK.h"

#include "../Features/Visuals/Chams/Chams.h"
#include "../Features/Visuals/Glow/Glow.h"
#include "../Features/CameraWindow/CameraWindow.h"
#include "../Features/Visuals/Visuals.h"
#include "../Features/Visuals/Materials/Materials.h"

MAKE_HOOK(CClientModeShared_DoPostScreenSpaceEffects, U::Memory.GetVFunc(I::ClientModeShared, 39), bool,
	void* rcx, const CViewSetup* pSetup)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::CClientModeShared_DoPostScreenSpaceEffects[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx, pSetup);
#endif

	if (Vars::Visuals::UI::CleanScreenshots.Value && I::EngineClient->IsTakingScreenshot())
		return CALL_ORIGINAL(rcx, pSetup);

	auto pLocal = H::Entities.GetLocal();
	auto pWeapon = H::Entities.GetWeapon();

	if (pLocal && pWeapon)
	{
		F::Visuals.SplashRadius(pLocal);
		F::Visuals.ProjectileTrace(pLocal, pWeapon);
	}

	if (F::CameraWindow.m_bDrawing)
		return CALL_ORIGINAL(rcx, pSetup);

	F::Visuals.DrawBoxes();
	F::Visuals.DrawPaths();
	F::Visuals.DrawLines();
	F::Visuals.DrawSightlines();

	F::Chams.mEntities.clear();
	if (!I::EngineVGui->IsGameUIVisible() && pLocal && F::Materials.m_bLoaded)
	{
		F::Chams.RenderMain();
		F::Glow.RenderMain();
	}

	return CALL_ORIGINAL(rcx, pSetup);
}