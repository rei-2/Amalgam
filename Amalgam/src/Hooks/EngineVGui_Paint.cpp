#include "../SDK/SDK.h"
#include "../Features/CameraWindow/CameraWindow.h"
#include "../Features/CritHack/CritHack.h"
#include "../Features/Visuals/ESP/ESP.h"
#include "../Features/Visuals/Notifications/Notifications.h"
#include "../Features/Visuals/PlayerArrows/PlayerArrows.h"
#include "../Features/Visuals/Radar/Radar.h"
#include "../Features/Visuals/SpectatorList/SpectatorList.h"
#include "../Features/Visuals/Visuals.h"

void Paint()
{
	H::Draw.UpdateW2SMatrix();
	H::Draw.UpdateScreenSize();

	I::MatSystemSurface->StartDrawing();
	{
		if (Vars::Visuals::UI::CleanScreenshots.Value && I::EngineClient->IsTakingScreenshot())
			return I::MatSystemSurface->FinishDrawing();

		F::Notifications.Draw();

		auto pLocal = H::Entities.GetLocal();
		if (I::EngineVGui->IsGameUIVisible() || !pLocal)
			return I::MatSystemSurface->FinishDrawing();

		F::CameraWindow.Draw();
		F::Visuals.DrawServerHitboxes(pLocal);
		F::Visuals.DrawAntiAim(pLocal);

		F::Visuals.PickupTimers();
		F::ESP.Run(pLocal);
		F::PlayerArrows.Run(pLocal);
		F::Radar.Run(pLocal);

		F::Visuals.DrawAimbotFOV(pLocal);
		F::Visuals.DrawSeedPrediction(pLocal);
		F::Visuals.DrawOnScreenConditions(pLocal);
		F::Visuals.DrawOnScreenPing(pLocal);
		F::SpectatorList.Run(pLocal);
		F::CritHack.Draw(pLocal);
		F::Visuals.DrawTickbaseText(pLocal);
		F::Visuals.DrawDebugInfo(pLocal);
	}
	I::MatSystemSurface->FinishDrawing();
}

MAKE_HOOK(EngineVGui_Paint, U::Memory.GetVFunc(I::EngineVGui, 14), void, __fastcall,
	void* ecx, int iMode)
{
	CALL_ORIGINAL(ecx, iMode);

	if (iMode & PAINT_UIPANELS)
		Paint();
}