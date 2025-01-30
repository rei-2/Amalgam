#include "../SDK/SDK.h"

#include "../Features/Players/PlayerUtils.h"

MAKE_HOOK(ISteamFriends_GetFriendPersonaName, U::Memory.GetVFunc(I::SteamFriends, 7), const char*,
	void* rcx, CSteamID steamIDFriend)
{
	if (Vars::Visuals::UI::StreamerMode.Value)
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