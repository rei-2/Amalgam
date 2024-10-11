#include "../SDK/SDK.h"

#include "../Features/Visuals/Chams/Chams.h"
#include "../Features/Visuals/Glow/Glow.h"
#include "../Features/CameraWindow/CameraWindow.h"
#include "../Features/Visuals/Visuals.h"

MAKE_HOOK(CClientModeShared_DoPostScreenSpaceEffects, U::Memory.GetVFunc(I::ClientModeShared, 39), bool, __fastcall,
	void* rcx, const CViewSetup* pSetup)
{
	F::Chams.mEntities.clear();

	if (I::EngineVGui->IsGameUIVisible() || Vars::Visuals::UI::CleanScreenshots.Value && I::EngineClient->IsTakingScreenshot())
		return CALL_ORIGINAL(rcx, pSetup);

	static std::once_flag onceFlag;
	std::call_once(onceFlag, []
		{
			F::Glow.Init();
			F::CameraWindow.Init();
		});

	F::Visuals.DrawBoxes();
	F::Visuals.DrawBulletLines();
	F::Visuals.DrawSimLines();
	F::Visuals.DrawSightlines();

	auto pLocal = H::Entities.GetLocal();
	auto pWeapon = H::Entities.GetWeapon();
	if (pLocal)
	{
		F::Chams.RenderMain();
		F::Glow.RenderMain();

		if (pWeapon)
			F::Visuals.ProjectileTrace(pLocal, pWeapon);
		F::Visuals.SplashRadius(pLocal);
	}

	return CALL_ORIGINAL(rcx, pSetup);
}