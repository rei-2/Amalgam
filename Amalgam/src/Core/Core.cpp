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

	
	// What is this here for??
	// Check the DirectX version
	/*if (bUnload)
		return;*/

	if(!(bUnload = bEarly = !SDK::GetTeamFortressWindow()))
	{
		if (!(bUnload = bEarly = !U::Signatures.Initialize()))
		{
			if (!(bUnload = bEarly = !U::Interfaces.Initialize()))
			{
				U::ConVars.Initialize();
				if (!(bUnload = !U::Hooks.Initialize()))
				{
					if (!(bUnload = !U::BytePatches.Initialize()))
					{
						if (!(bUnload = !H::Events.Initialize()))
						{
							F::Materials.LoadMaterials();
							F::Commands.Initialize();

							F::Configs.LoadConfig(F::Configs.sCurrentConfig, false);
							F::Menu.ConfigLoaded = true;

							SDK::Output("Amalgam", "Loaded", { 175, 150, 255, 255 }, true, false, false, true);
						}
					}
				}
			}
		}
	}
}

void CCore::Loop()
{
	while (true)
	{
		bool bShouldUnload = bUnload || U::KeyHandler.Down(VK_F11) && SDK::IsGameWindowInFocus();
		if (bShouldUnload)
			break;

		Sleep(15);
	}
}

void CCore::Unload()
{
	if (bEarly) 
	{
		// Cant use console here since I::CVar is not initialized yet
		SDK::Output("Amalgam", "Cancelled", { 175, 150, 255, 255 }, false, false, false, true);
		return;
	}

	G::Unload = true;
	U::Hooks.Unload();
	U::BytePatches.Unload();
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
	U::ConVars.Unload();
	F::Materials.UnloadMaterials();

	SDK::Output("Amalgam", "Unloaded", { 175, 150, 255, 255 }, true, false, false, true);
}