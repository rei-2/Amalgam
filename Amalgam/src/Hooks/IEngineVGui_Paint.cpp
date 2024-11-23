#include "../SDK/SDK.h"
#include "../Features/CameraWindow/CameraWindow.h"
#include "../Features/CritHack/CritHack.h"
#include "../Features/NoSpread/NoSpreadHitscan/NoSpreadHitscan.h"
#include "../Features/Visuals/ESP/ESP.h"
#include "../Features/Visuals/Notifications/Notifications.h"
#include "../Features/Visuals/PlayerArrows/PlayerArrows.h"
#include "../Features/Visuals/PlayerConditions/PlayerConditions.h"
#include "../Features/Visuals/Radar/Radar.h"
#include "../Features/Visuals/SpectatorList/SpectatorList.h"
#include "../Features/Visuals/Visuals.h"

static void Paint()
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
			H::Fonts.Reload(Vars::Menu::Scale.Map[DEFAULT_BIND]);

		auto pLocal = H::Entities.GetLocal();
		if (pLocal && !I::EngineVGui->IsGameUIVisible())
		{
			F::CameraWindow.Draw();
			F::Visuals.DrawServerHitboxes(pLocal);
			F::Visuals.DrawAntiAim(pLocal);

			F::Visuals.DrawPickupTimers();
			F::ESP.Draw();
			F::PlayerArrows.Run(pLocal);
			F::Visuals.DrawFOV(pLocal);
			F::Radar.Run(pLocal);

			F::NoSpreadHitscan.Draw(pLocal);
			F::PlayerConditions.Draw(pLocal);
			F::Visuals.DrawPing(pLocal);
			F::SpectatorList.Draw(pLocal);
			F::CritHack.Draw(pLocal);
			F::Visuals.DrawTicks(pLocal);
			F::Visuals.DrawDebugInfo(pLocal);
		}

		F::Notifications.Draw();
	}
	I::MatSystemSurface->FinishDrawing();
}

MAKE_HOOK(IEngineVGui_Paint, U::Memory.GetVFunc(I::EngineVGui, 14), void,
	void* rcx, int iMode)
{
	CALL_ORIGINAL(rcx, iMode);

	if (iMode & PAINT_UIPANELS)
		Paint();
}