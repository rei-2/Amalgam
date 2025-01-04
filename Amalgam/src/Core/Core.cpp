#include "Core.h"

#include "../SDK/SDK.h"
#include "../BytePatches/BytePatches.h"
#include "../Features/Visuals/Materials/Materials.h"
#include "../Features/Configs/Configs.h"
#include "../Features/Commands/Commands.h"
#include "../Features/ImGui/Menu/Menu.h"
#include "../Features/Visuals/Visuals.h"
#include "../SDK/Events/Events.h"

static inline bool CheckDXLevel()
{
	auto mat_dxlevel = I::CVar->FindVar("mat_dxlevel");
	if (mat_dxlevel->GetInt() < 90)
	{
		MessageBox(nullptr, "You are running with graphics options that Amalgam does not support.\n-dxlevel must be at least 90.", "Error", MB_ICONERROR);
		return false;
	}

	return true;
}

void CCore::Load()
{
	while (!U::Memory.FindSignature("client.dll", "48 8B 0D ? ? ? ? 48 8B 10 48 8B 19 48 8B C8 FF 92"))
	{
		Sleep(500);
		if (m_bUnload = m_bFailed = U::KeyHandler.Down(VK_F11, true))
			return;
	}
	Sleep(500);

	SDK::GetTeamFortressWindow();
	if (m_bUnload = m_bFailed =
			!U::Signatures.Initialize()
		 || !U::Interfaces.Initialize() || !CheckDXLevel()
		 || !U::Hooks.Initialize()
		 || !U::BytePatches.Initialize()
		 || !H::Events.Initialize()
		)
		return;
	U::ConVars.Initialize();
	F::Materials.LoadMaterials();
	F::Commands.Initialize();

	F::Configs.LoadConfig(F::Configs.sCurrentConfig, false);
	F::Menu.ConfigLoaded = true;

	SDK::Output("Amalgam", "Loaded", { 175, 150, 255, 255 }, true, false, false, true);
}

void CCore::Loop()
{
	while (true)
	{
		bool bShouldUnload = U::KeyHandler.Down(VK_F11) && SDK::IsGameWindowInFocus() || m_bUnload;
		if (bShouldUnload)
			break;

		Sleep(15);
	}
}

void CCore::Unload()
{
	if (m_bFailed)
	{
		SDK::Output("Amalgam", "Failed", { 175, 150, 255, 255 }, false, false, false, true);
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