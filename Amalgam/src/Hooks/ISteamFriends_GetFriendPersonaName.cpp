#include "../SDK/SDK.h"

#include "../Features/Players/PlayerUtils.h"

MAKE_SIGNATURE(GetPlayerNameForSteamID_GetFriendPersonaName_Call, "client.dll", "41 B9 ? ? ? ? 44 8B C3 48 8B C8", 0x0);

MAKE_HOOK(ISteamFriends_GetFriendPersonaName, U::Memory.GetVFunc(I::SteamFriends, 7), const char*,
	void* rcx, CSteamID steamIDFriend)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::ISteamFriends_GetFriendPersonaName[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx, steamIDFriend);
#endif

	static const auto dwDesired = S::GetPlayerNameForSteamID_GetFriendPersonaName_Call();
	const auto dwRetAddr = uintptr_t(_ReturnAddress());

	if (Vars::Visuals::UI::StreamerMode.Value && dwRetAddr == dwDesired)
	{
		if (I::SteamUser->GetSteamID() == steamIDFriend)
		{
			if (Vars::Visuals::UI::StreamerMode.Value >= Vars::Visuals::UI::StreamerModeEnum::Local)
				return "Local";
		}
		else if (I::SteamFriends->HasFriend(steamIDFriend, k_EFriendFlagImmediate))
		{
			if (Vars::Visuals::UI::StreamerMode.Value >= Vars::Visuals::UI::StreamerModeEnum::Friends)
				return "Friend";
		}
		else
		{
			if (Vars::Visuals::UI::StreamerMode.Value >= Vars::Visuals::UI::StreamerModeEnum::Party)
				return "Party";
		}
	}

	return CALL_ORIGINAL(rcx, steamIDFriend);
}