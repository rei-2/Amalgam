#pragma once
#include "Interface.h"
#include "../Steam/Steam_API.h"

MAKE_INTERFACE_VERSION(ISteamClient, SteamClient, "steamclient64.dll", STEAMCLIENT_INTERFACE_VERSION);
MAKE_INTERFACE_NULL(ISteamFriends, SteamFriends);
MAKE_INTERFACE_NULL(ISteamUtils, SteamUtils);
MAKE_INTERFACE_NULL(ISteamApps, SteamApps);
MAKE_INTERFACE_NULL(ISteamUserStats, SteamUserStats);
MAKE_INTERFACE_NULL(ISteamUser, SteamUser);
MAKE_INTERFACE_NULL(ISteamNetworkingUtils, SteamNetworkingUtils);