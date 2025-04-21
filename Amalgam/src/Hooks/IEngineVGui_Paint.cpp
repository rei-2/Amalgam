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

MAKE_HOOK(IEngineVGui_Paint, U::Memory.GetVFunc(I::EngineVGui, 14), void,
	void* rcx, int iMode)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::IEngineVGui_Paint[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx, iMode);
#endif

	if (G::Unload)
		return CALL_ORIGINAL(rcx, iMode);

	if (iMode & PAINT_INGAMEPANELS && (!Vars::Visuals::UI::CleanScreenshots.Value || !I::EngineClient->IsTakingScreenshot()))
	{
		H::Draw.UpdateW2SMatrix();
		H::Draw.UpdateScreenSize();

		H::Draw.Start(true);
		if (auto pLocal = H::Entities.GetLocal())
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
		H::Draw.End();
	}

	CALL_ORIGINAL(rcx, iMode);

	if (iMode & PAINT_UIPANELS && (!Vars::Visuals::UI::CleanScreenshots.Value || !I::EngineClient->IsTakingScreenshot()))
	{
		H::Draw.Start();
		{
			F::Notifications.Draw();
		}
		H::Draw.End();
	}
}