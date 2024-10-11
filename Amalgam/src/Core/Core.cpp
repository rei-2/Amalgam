#include "Core.h"

#include "../SDK/SDK.h"
#include "../Features/Visuals/Materials/Materials.h"
#include "../Features/Configs/Configs.h"
#include "../Features/Commands/Commands.h"
#include "../Features/ImGui/Menu/Menu.h"
#include "../Features/Visuals/Visuals.h"
#include "../SDK/Events/Events.h"

void CCore::Load()
{
	while (!U::Memory.FindSignature("client.dll", "48 8B 0D ? ? ? ? 48 8B 10 48 8B 19 48 8B C8 FF 92"))
	{
		bUnload = bEarly = U::KeyHandler.Down(VK_F11, true);
		if (bUnload)
			return;

		Sleep(500);
	}

	Sleep(500);

	// Check the DirectX version
	if (bUnload)
		return;

	SDK::GetTeamFortressWindow();
	U::Signatures.Initialize();
	U::Interfaces.Initialize();
	U::Hooks.Initialize();
	U::ConVars.Initialize();
	F::Materials.LoadMaterials();
	F::Commands.Initialize();
	H::Events.Initialize();

	F::Configs.LoadConfig(F::Configs.sCurrentConfig, false);
	F::Menu.ConfigLoaded = true;

	SDK::Output("Amalgam", "Loaded", { 175, 150, 255, 255 });
}

void CCore::Loop()
{
	while (true)
	{
		bool bShouldUnload = U::KeyHandler.Down(VK_F11) && SDK::IsGameWindowInFocus() || bUnload;
		if (bShouldUnload)
			break;

		Sleep(50);
	}
}

void CCore::Unload()
{
	if (bEarly)
	{
		SDK::Output("Amalgam", "Cancelled", { 175, 150, 255, 255 });
		return;
	}

	G::Unload = true;

	U::Hooks.Unload();
	H::Events.Unload();
	U::ConVars.Unload();

	if (F::Menu.IsOpen)
		I::MatSystemSurface->SetCursorAlwaysVisible(F::Menu.IsOpen = false);
	F::Visuals.RestoreWorldModulation();
	if (I::Input->CAM_IsThirdPerson())
	{
		auto pLocal = H::Entities.GetLocal();
		if (pLocal)
		{
			I::Input->CAM_ToFirstPerson();
			pLocal->ThirdPersonSwitch();
		}
	}
	if (auto cl_wpn_sway_interp = U::ConVars.FindVar("cl_wpn_sway_interp"))
		cl_wpn_sway_interp->SetValue(0.f);
	if (auto cl_wpn_sway_scale = U::ConVars.FindVar("cl_wpn_sway_scale"))
		cl_wpn_sway_scale->SetValue(0.f);

	Sleep(250);

	F::Materials.UnloadMaterials();

	SDK::Output("Amalgam", "Unloaded", { 175, 150, 255, 255 });
}