#include "Core.h"

#include "../SDK/SDK.h"
#include "../BytePatches/BytePatches.h"
#include "../Features/Commands/Commands.h"
#include "../Features/Configs/Configs.h"
#include "../Features/ImGui/Menu/Menu.h"
#include "../Features/EnginePrediction/EnginePrediction.h"
#include "../Features/Visuals/Materials/Materials.h"
#include "../Features/Visuals/Visuals.h"
#include "../SDK/Events/Events.h"
#include <Psapi.h>

static inline std::string GetProcessName(DWORD dwProcessID)
{
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwProcessID);
	if (!hProcess)
		return "";

	char buffer[MAX_PATH];
	if (!GetModuleBaseName(hProcess, nullptr, buffer, sizeof(buffer) / sizeof(char)))
	{
		CloseHandle(hProcess);
		return "";
	}

	CloseHandle(hProcess);
	return buffer;
}

static inline bool CheckDXLevel()
{
	auto mat_dxlevel = U::ConVars.FindVar("mat_dxlevel");
	if (mat_dxlevel->GetInt() < 90)
	{
		//const char* sMessage = "You are running with graphics options that Amalgam does not support.\n-dxlevel must be at least 90.";
		const char* sMessage = "You are running with graphics options that Amalgam does not support.\nIt is recommended for -dxlevel to be at least 90.";
		U::Core.AppendFailText(sMessage);
		SDK::Output("Amalgam", sMessage, { 175, 150, 255 }, true, true);
		//return false;
	}

	return true;
}

void CCore::AppendFailText(const char* sMessage)
{
	m_ssFailStream << std::format("{}\n", sMessage);
	OutputDebugStringA(std::format("{}\n", sMessage).c_str());
}

void CCore::LogFailText()
{
	m_ssFailStream << "\nBuilt @ " __DATE__ ", " __TIME__ ", " __CONFIGURATION__ "\n";
	m_ssFailStream << "Ctrl + C to copy. \n";
	try
	{
		std::ofstream file;
		file.open(F::Configs.m_sConfigPath + "fail_log.txt", std::ios_base::app);
		file << m_ssFailStream.str() + "\n\n\n";
		file.close();
		m_ssFailStream << "Logged to Amalgam\\fail_log.txt. ";
	}
	catch (...) {}

	SDK::Output("Failed to load", m_ssFailStream.str().c_str(), {}, false, true, false, false, false, false, MB_OK | MB_ICONERROR);
}

void CCore::Load()
{
	// File-based logging for Core.Load debugging
	FILE* log_file = fopen("C:\\temp\\amalgam_debug.log", "a");
	if (log_file) {
		fprintf(log_file, "Core::Load: Entry\n");
		fclose(log_file);
	}

	try
	{
		if (m_bUnload = m_bFailed = FNV1A::Hash32(GetProcessName(GetCurrentProcessId()).c_str()) != FNV1A::Hash32Const("tf_win64.exe"))
		{
			AppendFailText("Invalid process");
			return;
		}

		FILE* log_file1 = fopen("C:\\temp\\amalgam_debug.log", "a");
		if (log_file1) {
			fprintf(log_file1, "Core::Load: Process check passed, waiting for signatures\n");
			fclose(log_file1);
		}

	float flTime = 0.f;
	while (!U::Memory.FindSignature("client.dll", "48 8B 0D ? ? ? ? 48 8B 10 48 8B 19 48 8B C8 FF 92") || !SDK::GetTeamFortressWindow())
	{
		Sleep(500), flTime += 0.5f;
		if (m_bUnload = m_bFailed = flTime >= 60.f)
		{
			AppendFailText("Failed to load");
			return;
		}
		if (m_bUnload = m_bFailed = U::KeyHandler.Down(VK_F11, true))
		{
			AppendFailText("Cancelled load");
			return;
		}
	}
	Sleep(500);

	FILE* log_file2 = fopen("C:\\temp\\amalgam_debug.log", "a");
	if (log_file2) {
		fprintf(log_file2, "Core::Load: About to initialize Signatures\n");
		fclose(log_file2);
	}

	if (!U::Signatures.Initialize())
	{
		FILE* log_file = fopen("C:\\temp\\amalgam_debug.log", "a");
		if (log_file) {
			fprintf(log_file, "Core::Load: Signatures.Initialize() FAILED\n");
			fclose(log_file);
		}
		m_bUnload = m_bFailed = true;
		return;
	}

	FILE* log_file3 = fopen("C:\\temp\\amalgam_debug.log", "a");
	if (log_file3) {
		fprintf(log_file3, "Core::Load: Signatures initialized, about to initialize Interfaces\n");
		fclose(log_file3);
	}

	if (!U::Interfaces.Initialize())
	{
		FILE* log_file = fopen("C:\\temp\\amalgam_debug.log", "a");
		if (log_file) {
			fprintf(log_file, "Core::Load: Interfaces.Initialize() FAILED\n");
			fclose(log_file);
		}
		m_bUnload = m_bFailed = true;
		return;
	}

	FILE* log_file4 = fopen("C:\\temp\\amalgam_debug.log", "a");
	if (log_file4) {
		fprintf(log_file4, "Core::Load: Interfaces initialized, checking DX level\n");
		fclose(log_file4);
	}

	if (!CheckDXLevel())
	{
		FILE* log_file = fopen("C:\\temp\\amalgam_debug.log", "a");
		if (log_file) {
			fprintf(log_file, "Core::Load: CheckDXLevel() FAILED\n");
			fclose(log_file);
		}
		m_bUnload = m_bFailed = true;
		return;
	}

	FILE* log_file5 = fopen("C:\\temp\\amalgam_debug.log", "a");
	if (log_file5) {
		fprintf(log_file5, "Core::Load: About to initialize Hooks\n");
		fclose(log_file5);
	}

	if (!U::Hooks.Initialize())
	{
		FILE* log_file = fopen("C:\\temp\\amalgam_debug.log", "a");
		if (log_file) {
			fprintf(log_file, "Core::Load: Hooks.Initialize() FAILED\n");
			fclose(log_file);
		}
		m_bUnload = m_bFailed2 = true;
		return;
	}

	FILE* log_file6 = fopen("C:\\temp\\amalgam_debug.log", "a");
	if (log_file6) {
		fprintf(log_file6, "Core::Load: Hooks initialized, about to initialize BytePatches\n");
		fclose(log_file6);
	}

	if (!U::BytePatches.Initialize())
	{
		FILE* log_file = fopen("C:\\temp\\amalgam_debug.log", "a");
		if (log_file) {
			fprintf(log_file, "Core::Load: BytePatches.Initialize() FAILED\n");
			fclose(log_file);
		}
		m_bUnload = m_bFailed2 = true;
		return;
	}

	FILE* log_file7 = fopen("C:\\temp\\amalgam_debug.log", "a");
	if (log_file7) {
		fprintf(log_file7, "Core::Load: BytePatches initialized, about to initialize Events\n");
		fclose(log_file7);
	}

	if (!H::Events.Initialize())
	{
		FILE* log_file = fopen("C:\\temp\\amalgam_debug.log", "a");
		if (log_file) {
			fprintf(log_file, "Core::Load: Events.Initialize() FAILED\n");
			fclose(log_file);
		}
		m_bUnload = m_bFailed2 = true;
		return;
	}

	FILE* log_file8 = fopen("C:\\temp\\amalgam_debug.log", "a");
	if (log_file8) {
		fprintf(log_file8, "Core::Load: All systems initialized successfully\n");
		fclose(log_file8);
	}
	F::Materials.LoadMaterials();
	U::ConVars.Initialize();
	F::Commands.Initialize();

	F::Configs.LoadConfig(F::Configs.m_sCurrentConfig, false);
	F::Configs.m_bConfigLoaded = true;

	SDK::Output("Amalgam", "Loaded", { 175, 150, 255 }, true, true, true);
	}
	catch (...)
	{
		AppendFailText("Exception during initialization");
		m_bUnload = m_bFailed = true;
		return;
	}
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
		LogFailText();
		return;
	}

	G::Unload = true;
	m_bFailed2 = !U::Hooks.Unload() || m_bFailed2;
	U::BytePatches.Unload();
	H::Events.Unload();

	if (F::Menu.m_bIsOpen)
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

	Sleep(250);
	F::EnginePrediction.Unload();
	U::ConVars.Unload();
	F::Materials.UnloadMaterials();

	if (m_bFailed2)
	{
		LogFailText();
		return;
	}

	SDK::Output("Amalgam", "Unloaded", { 175, 150, 255 }, true, true);
}