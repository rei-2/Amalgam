#include "Interfaces.h"

#include "../../Core/Core.h"
#include "../../Utils/Assert/Assert.h"

#define Validate(x) if (!x) { U::Core.AppendFailText("CNullInterfaces::Initialize() failed to initialize "#x); m_bFailed = true; }
#define ValidateNonLethal(x) if (!x) { const char* sMessage = "CNullInterfaces::Initialize() failed to initialize "#x; MessageBox(nullptr, sMessage, "Warning", MB_OK | MB_ICONERROR); U::Core.AppendFailText(sMessage); }

MAKE_SIGNATURE(Get_TFPartyClient, "client.dll", "48 8B 05 ? ? ? ? C3 CC CC CC CC CC CC CC CC 48 89 5C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 56", 0x0);
MAKE_SIGNATURE(Get_SteamNetworkingUtils, "client.dll", "40 53 48 83 EC ? 48 8B D9 48 8D 15 ? ? ? ? 33 C9 FF 15 ? ? ? ? 33 C9", 0x0);

bool CNullInterfaces::Initialize()
{
	I::TFPartyClient = S::Get_TFPartyClient.Call<CTFPartyClient*>();
	Validate(I::TFPartyClient);

	I::KeyValuesSystem = U::Memory.GetModuleExport<IKeyValuesSystem*(*)()>("vstdlib.dll", "KeyValuesSystem")();
	Validate(I::KeyValuesSystem);

	const HSteamPipe hsNewPipe = I::SteamClient->CreateSteamPipe();
	Validate(hsNewPipe);

	const HSteamPipe hsNewUser = I::SteamClient->ConnectToGlobalUser(hsNewPipe);
	Validate(hsNewUser);

	I::SteamFriends = I::SteamClient->GetISteamFriends(hsNewUser, hsNewPipe, STEAMFRIENDS_INTERFACE_VERSION);
	Validate(I::SteamFriends);

	I::SteamUtils = I::SteamClient->GetISteamUtils(hsNewUser, STEAMUTILS_INTERFACE_VERSION);
	Validate(I::SteamUtils);

	I::SteamApps = I::SteamClient->GetISteamApps(hsNewUser, hsNewPipe, STEAMAPPS_INTERFACE_VERSION);
	ValidateNonLethal(I::SteamApps);

	I::SteamUserStats = I::SteamClient->GetISteamUserStats(hsNewUser, hsNewPipe, STEAMUSERSTATS_INTERFACE_VERSION);
	Validate(I::SteamUserStats);

	I::SteamUser = I::SteamClient->GetISteamUser(hsNewUser, hsNewPipe, STEAMUSER_INTERFACE_VERSION);
	Validate(I::SteamUser);

	S::Get_SteamNetworkingUtils.Call<ISteamNetworkingUtils*>(&I::SteamNetworkingUtils);
	Validate(I::SteamNetworkingUtils);

	return !m_bFailed;
}