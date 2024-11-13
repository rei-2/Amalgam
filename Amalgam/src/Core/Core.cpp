#include "Core.h"

#include "../SDK/SDK.h"
#include "../BytePatches/BytePatches.h"
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
		Sleep(500);
		if (bUnload = bEarly = U::KeyHandler.Down(VK_F11, true))
			return;
	}
	Sleep(500);

	// Check the DirectX version
	if (bUnload)
		return;

		SDK::Output("Core", "SDK::GetTeamFortressWindow();", {}, false, false, false, true);
	SDK::GetTeamFortressWindow();
		SDK::Output("Core", "U::Signatures.Initialize();", {}, false, false, false, true);
	U::Signatures.Initialize();
		SDK::Output("Core", "U::Interfaces.Initialize();", {}, false, false, false, true);
	U::Interfaces.Initialize();
		SDK::Output("Core", "U::ConVars.Initialize();", {}, false, false, false, true);
	U::ConVars.Initialize();
		SDK::Output("Core", "F::Materials.LoadMaterials();", {}, false, false, false, true);
	F::Materials.LoadMaterials();
		SDK::Output("Core", "F::Commands.Initialize();", {}, false, false, false, true);
	F::Commands.Initialize();
		SDK::Output("Core", "U::Hooks.Initialize();", {}, false, false, false, true);
	U::Hooks.Initialize();
		SDK::Output("Core", "U::BytePatches.Initialize();", {}, false, false, false, true);
	U::BytePatches.Initialize();
		SDK::Output("Core", "H::Events.Initialize();", {}, false, false, false, true);
	H::Events.Initialize();

	F::Configs.LoadConfig(F::Configs.sCurrentConfig, false);
	F::Menu.ConfigLoaded = true;

	SDK::Output("Amalgam", "Loaded", { 175, 150, 255, 255 }, true, false, false, true);
}

void CCore::Loop()
{
	while (true)
	{
		bool bShouldUnload = U::KeyHandler.Down(VK_F11) && SDK::IsGameWindowInFocus() || bUnload;
		if (bShouldUnload)
			break;

		Sleep(15);
	}
}

void CCore::Unload()
{
	if (bEarly)
	{
		SDK::Output("Amalgam", "Cancelled", { 175, 150, 255, 255 }, true, false, false, true);
		return;
	}

	G::Unload = true;
		SDK::Output("Core", "U::Hooks.Unload();", {}, false, false, false, true);
	U::Hooks.Unload();
		SDK::Output("Core", "U::BytePatches.Unload();", {}, false, false, false, true);
	U::BytePatches.Unload();
		SDK::Output("Core", "H::Events.Unload();", {}, false, false, false, true);
	H::Events.Unload();

	if (F::Menu.IsOpen)
		I::MatSystemSurface->SetCursorAlwaysVisible(false);
	F::Visuals.RestoreWorldModulation();
	if (I::Input->CAM_IsThirdPerson())
	{
		if (auto pLocal = H::Entities.GetLocal())
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
		SDK::Output("Core", "U::ConVars.Unload();", {}, false, false, false, true);
	U::ConVars.Unload();
		SDK::Output("Core", "F::Materials.UnloadMaterials();", {}, false, false, false, true);
	F::Materials.UnloadMaterials();

	SDK::Output("Amalgam", "Unloaded", { 175, 150, 255, 255 }, true, false, false, true);
}