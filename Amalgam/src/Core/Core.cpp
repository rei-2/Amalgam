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
	auto mat_dxlevel = U::ConVars.FindVar("mat_dxlevel");
	if (mat_dxlevel->GetInt() < 90)
	{
		//const char* sMessage = "You are running with graphics options that Amalgam does not support.\n-dxlevel must be at least 90.";
		const char* sMessage = "You are running with graphics options that Amalgam does not support.\nIt is recommended for -dxlevel to be at least 90.";
		U::Core.AppendFailText(sMessage);
		SDK::Output("Amalgam", sMessage, { 175, 150, 255, 255 }, true, false, false, true);
		//return false;
	}

	return true;
}

void CCore::AppendFailText(const char* sMessage)
{
	ssFailStream << std::format("{}\n", sMessage);
	OutputDebugStringA(std::format("{}\n", sMessage).c_str());
}

void CCore::Load()
{
	if (m_bUnload = m_bFailed = !SDK::GetTeamFortressWindow())
	{
		AppendFailText("Game window not found");
		return;
	}
	while (!U::Memory.FindSignature("client.dll", "48 8B 0D ? ? ? ? 48 8B 10 48 8B 19 48 8B C8 FF 92"))
	{
		Sleep(500);
		if (m_bUnload = m_bFailed = U::KeyHandler.Down(VK_F11, true))
		{
			U::Core.AppendFailText("Cancelled load");
			return;
		}
	}
	Sleep(500);

	if (m_bUnload = m_bFailed = !U::Signatures.Initialize() || !U::Interfaces.Initialize() || !CheckDXLevel())
		return;
	if (m_bUnload = m_bFailed2 = !U::Hooks.Initialize() || !U::BytePatches.Initialize() || !H::Events.Initialize())
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
		ssFailStream << "\nCtrl + C to copy. Logged to Amalgam\\fail_log.txt. (1)\n";
		ssFailStream << "Built @ " __DATE__ ", " __TIME__;

		SDK::Output("Failed to load", ssFailStream.str().c_str(), {}, false, false, false, true, MB_OK | MB_ICONERROR);

		ssFailStream << "\n\n\n\n";
		std::ofstream file;
		file.open(F::Configs.sConfigPath + "\\fail_log.txt", std::ios_base::app);
		file << ssFailStream.str();
		file.close();

		return;
	}

	G::Unload = true;
	m_bFailed2 = !U::Hooks.Unload();
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

	if (m_bFailed2)
	{
		ssFailStream << "\nCtrl + C to copy. Logged to Amalgam\\fail_log.txt. (2)\n";
		ssFailStream << "Built @ " __DATE__ ", " __TIME__;

		SDK::Output("Failed to load", ssFailStream.str().c_str(), {}, false, false, false, true, MB_OK | MB_ICONERROR);

		ssFailStream << "\n\n\n\n";
		std::ofstream file;
		file.open(F::Configs.sConfigPath + "\\fail_log.txt", std::ios_base::app);
		file << ssFailStream.str();
		file.close();

		return;
	}

	SDK::Output("Amalgam", "Unloaded", { 175, 150, 255, 255 }, true, false, false, true);
}