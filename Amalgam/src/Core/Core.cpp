#include "Core.h"

#include "../SDK/SDK.h"
#include "../Features/Visuals/Materials/Materials.h"
#include "../Features/Configs/Configs.h"
#include "../Features/Commands/Commands.h"
#include "../Features/ImGui/Menu/Menu.h"
#include "../Features/Visuals/Visuals.h"

void CCore::Load()
{
	// Check the DirectX version

	U::Signatures.Initialize();
	U::Interfaces.Initialize();
	U::Hooks.Initialize();
	U::ConVars.Initialize();
	F::Materials.LoadMaterials();
	F::Commands.Initialize();

	F::Configs.LoadConfig(F::Configs.sCurrentConfig, false);
	F::Menu.ConfigLoaded = true;

	SDK::Output("Amalgam", "Loaded", { 175, 150, 255, 255 });
}

void CCore::Unload()
{
	G::Unload = true;

	U::Hooks.Unload();
	U::ConVars.Unload();
	F::Materials.UnloadMaterials();

	F::Visuals.RestoreWorldModulation();
	Vars::Visuals::World::SkyboxChanger.Value = "Off"; // hooks won't run, remove here
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

	SDK::Output("Amalgam", "Unloaded", { 175, 150, 255, 255 });
}

bool CCore::ShouldUnload()
{
	return SDK::IsGameWindowInFocus() && GetAsyncKeyState(VK_F11) & 0x8000 || bUnload;
}