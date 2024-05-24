#include "../SDK/SDK.h"

#include "../Features/Players/PlayerUtils.h"

MAKE_SIGNATURE(CVoiceStatus_IsPlayerBlocked, "client.dll", "40 53 48 81 EC ? ? ? ? 48 8B D9 4C 8D 44 24", 0x0);
MAKE_SIGNATURE(VGuiMenuBuilder_AddMenuItem, "client.dll", "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B EA 49 8B F9 48 8B 51 ? 49 8B F0 48 8B D9 48 85 D2 74 ? 49 8B C9 E8 ? ? ? ? 85 C0 74 ? 48 8B 0B 48 8B 01 FF 90 ? ? ? ? 48 8B 0B 4C 8B C6 4C 8B 4B ? 48 8B D5 48 89 7B ? 48 C7 44 24 ? ? ? ? ? 48 8B 01 FF 90 ? ? ? ? 48 8B 13 3B 82 ? ? ? ? 73 ? 3B 82 ? ? ? ? 7F ? 48 8B 92 ? ? ? ? 8B C8 48 03 C9 39 44 CA ? 75 ? 39 44 CA ? 75 ? 48 8B 04 CA EB ? 33 C0 48 8B 5C 24 ? 48 8B 6C 24 ? 48 8B 74 24 ? 48 83 C4 ? 5F C3 CC CC CC CC 48 89 5C 24", 0x0);
MAKE_SIGNATURE(CTFClientScoreBoardDialog_OnCommand, "client.dll", "48 89 5C 24 ? 57 48 83 EC ? 48 8B DA 48 8B F9 48 8B CB 48 8D 15 ? ? ? ? E8 ? ? ? ? 48 85 C0 74 ? 48 8B 0D", 0x0);
MAKE_SIGNATURE(CVoiceStatus_IsPlayerBlocked_Call, "client.dll", "84 C0 48 8D 0D ? ? ? ? 48 8D 15 ? ? ? ? 48 0F 45 D1 4C 8D 0D", 0x0);
MAKE_SIGNATURE(VGuiMenuBuilder_AddMenuItem_Call, "client.dll", "48 8B 0D ? ? ? ? 4C 8D 84 24 ? ? ? ? 48 8D 94 24 ? ? ? ? 48 8B 01 FF 90 ? ? ? ? 44 8B 84 24 ? ? ? ? 8B 94 24", 0x0);

static int PlayerIndex;
static std::string PlayerName;
static uint32_t FriendsID;

MAKE_HOOK(CVoiceStatus_IsPlayerBlocked, S::CVoiceStatus_IsPlayerBlocked(), bool, __fastcall,
    void* ecx, int playerIndex)
{
    static auto dwDesired = S::CVoiceStatus_IsPlayerBlocked_Call();
    const auto dwRetAddr = std::uintptr_t(_ReturnAddress());

    if (!Vars::Visuals::UI::ScoreboardPlayerlist.Value)
        return CALL_ORIGINAL(ecx, playerIndex);

    if (dwRetAddr == dwDesired)
        PlayerIndex = playerIndex;

    return CALL_ORIGINAL(ecx, playerIndex);
}

MAKE_HOOK(VGuiMenuBuilder_AddMenuItem, S::VGuiMenuBuilder_AddMenuItem(), void*, __fastcall,
    void* ecx, const char* pszButtonText, const char* pszCommand, const char* pszCategoryName)
{
    static auto dwDesired = S::VGuiMenuBuilder_AddMenuItem_Call();
    const auto dwRetAddr = std::uintptr_t(_ReturnAddress());

    if (!Vars::Visuals::UI::ScoreboardPlayerlist.Value)
        return CALL_ORIGINAL(ecx, pszButtonText, pszCommand, pszCategoryName);

    if (dwRetAddr == dwDesired && PlayerIndex != -1)
    {
        auto ret = CALL_ORIGINAL(ecx, pszButtonText, pszCommand, pszCategoryName);

        PlayerInfo_t pi{};
        if (I::EngineClient->GetPlayerInfo(PlayerIndex, &pi) && !pi.fakeplayer)
        {
            PlayerName = pi.name;
            FriendsID = pi.friendsID;

            const bool bIgnored = F::PlayerUtils.HasTag(FriendsID, "Ignored");
            const bool bCheater = F::PlayerUtils.HasTag(FriendsID, "Cheater");

            CALL_ORIGINAL(ecx, std::format("{} {}", bIgnored ? "Unignore" : "Ignore", PlayerName).c_str(), "ignoreplayer", "tags");
            CALL_ORIGINAL(ecx, std::format("{} {}", bCheater ? "Unmark" : "Mark", PlayerName).c_str(), "markplayer", "tags");
        }

        return ret;
    }

    return CALL_ORIGINAL(ecx, pszButtonText, pszCommand, pszCategoryName);
}

MAKE_HOOK(CTFClientScoreBoardDialog_OnCommand, S::CTFClientScoreBoardDialog_OnCommand(), void, __fastcall,
    void* ecx, const char* command)
{
    if (!Vars::Visuals::UI::ScoreboardPlayerlist.Value)
        return CALL_ORIGINAL(ecx, command);

    auto uHash = FNV1A::Hash(command);
    if (uHash == FNV1A::HashConst("ignoreplayer"))
    {
        if (!F::PlayerUtils.HasTag(FriendsID, "Ignored"))
            F::PlayerUtils.AddTag(FriendsID, "Ignored", true, PlayerName);
        else
            F::PlayerUtils.RemoveTag(FriendsID, "Ignored", true, PlayerName);
    }
    else if (uHash == FNV1A::HashConst("markplayer"))
    {
        if (!F::PlayerUtils.HasTag(FriendsID, "Cheater"))
            F::PlayerUtils.AddTag(FriendsID, "Cheater", true, PlayerName);
        else
            F::PlayerUtils.RemoveTag(FriendsID, "Cheater", true, PlayerName);
    }

    CALL_ORIGINAL(ecx, command);
}