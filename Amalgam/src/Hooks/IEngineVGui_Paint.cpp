#include "../SDK/SDK.h"

#include "../Features/Visuals/Visuals.h"
#include "../Features/Ticks/Ticks.h"
#include "../Features/CritHack/CritHack.h"
#include "../Features/Visuals/SpectatorList/SpectatorList.h"
#include "../Features/Backtrack/Backtrack.h"
#include "../Features/Visuals/PlayerConditions/PlayerConditions.h"
#include "../Features/NoSpread/NoSpreadHitscan/NoSpreadHitscan.h"
#include "../Features/Visuals/Radar/Radar.h"
#include "../Features/Aimbot/Aimbot.h"
#include "../Features/Visuals/PlayerArrows/PlayerArrows.h"
#include "../Features/Visuals/ESP/ESP.h"
#include "../Features/Visuals/CameraWindow/CameraWindow.h"
#include "../Features/Visuals/Notifications/Notifications.h"
#include "../Features/Aimbot/AutoHeal/AutoHeal.h"
#include "../Features/Visuals/UberTracker/UberTracker.h"
#include "../Features/Visuals/HealthBarESP/HealthBarESP.h"
#include "../Features/Visuals/PlayerTrails/PlayerTrails.h"
#include "../Features/Visuals/StickyESP/StickyESP.h"
#include "../Features/Visuals/CritHeals/CritHeals.h"
#include "../Features/Visuals/FocusFire/FocusFire.h"
#include "../Features/Visuals/PylonESP/PylonESP.h"
#include "../Features/Visuals/EnemyCam/EnemyCam.h"

MAKE_HOOK(IEngineVGui_Paint, U::Memory.GetVirtual(I::EngineVGui, 14), void,
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
		H::Draw.UpdateScreenSize();
		H::Draw.UpdateW2SMatrix();
		H::Draw.Start(true);
		if (auto pLocal = H::Entities.GetLocal())
		{
			F::CameraWindow.Draw();
			F::EnemyCam.Draw();
			F::Visuals.DrawAntiAim(pLocal);

			F::Visuals.DrawPickupTimers();
			F::ESP.Draw();
			F::PlayerArrows.Run(pLocal);
			F::Aimbot.Draw(pLocal);
			F::Radar.Run(pLocal);

#ifdef DEBUG_VACCINATOR
			F::AutoHeal.Draw(pLocal);
#endif
			F::NoSpreadHitscan.Draw(pLocal);
			F::PlayerConditions.Draw(pLocal);
			F::Backtrack.Draw(pLocal);
			F::SpectatorList.Draw(pLocal);
			F::CritHack.Draw(pLocal);
			F::Ticks.Draw(pLocal);
			F::UberTracker.Draw();
			F::HealthBarESP.Draw();
			F::PlayerTrails.Draw();
			F::StickyESP.Draw();
			F::CritHeals.Draw();
			F::FocusFire.Draw();
			F::PylonESP.Draw();
			F::Visuals.DrawDebugInfo(pLocal);
		}
		H::Draw.End();
	}

	CALL_ORIGINAL(rcx, iMode);

	if (iMode & PAINT_UIPANELS && (!Vars::Visuals::UI::CleanScreenshots.Value || !I::EngineClient->IsTakingScreenshot()))
	{
		H::Draw.UpdateScreenSize();
		H::Draw.Start();
		{
			F::Notifications.Draw();
		}
		H::Draw.End();
	}
}