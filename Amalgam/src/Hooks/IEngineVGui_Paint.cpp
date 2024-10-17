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

		// dumb FUCKN+IG CHeck for fonts
		int w = 0, h = 0; I::MatSystemSurface->GetTextSize(H::Fonts.GetFont(FONT_ESP).m_dwFont, L"", w, h);
		if (!h)
			H::Fonts.Reload(Vars::Menu::DPI.Map[DEFAULT_BIND]);

		F::Notifications.Draw();

		auto pLocal = H::Entities.GetLocal();
		if (I::EngineVGui->IsGameUIVisible() || !pLocal)
			return I::MatSystemSurface->FinishDrawing();

		F::CameraWindow.Draw();
		F::Visuals.DrawServerHitboxes(pLocal);
		F::Visuals.DrawAntiAim(pLocal);

		F::Visuals.PickupTimers();
		F::ESP.Draw();
		F::PlayerArrows.Run(pLocal);
		F::Radar.Run(pLocal);

		F::Visuals.DrawAimbotFOV(pLocal);
		F::Visuals.DrawSeedPrediction(pLocal);
		F::Visuals.DrawOnScreenConditions(pLocal);
		F::Visuals.DrawOnScreenPing(pLocal);
		F::SpectatorList.Run(pLocal);
		F::CritHack.Draw(pLocal);
		F::Visuals.DrawTicks(pLocal);
		F::Visuals.DrawDebugInfo(pLocal);
	}
	I::MatSystemSurface->FinishDrawing();
}

MAKE_HOOK(IEngineVGui_Paint, U::Memory.GetVFunc(I::EngineVGui, 14), void, __fastcall,
	void* rcx, int iMode)
{
	CALL_ORIGINAL(rcx, iMode);

	if (iMode & PAINT_UIPANELS)
		Paint();
}