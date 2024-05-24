#include "../SDK/SDK.h"
#include "../Features/Players/PlayerUtils.h"

MAKE_SIGNATURE(CTFPlayerResource_GetPlayerConnectionState, "client.dll", "85 D2 74 ? 83 FA ? 7F", 0x0);
MAKE_SIGNATURE(CPlayerResource_GetTeamColor, "client.dll", "83 FA ? 77 ? 48 63 C2 48 05", 0x0);
MAKE_SIGNATURE(CTFPlayer_Resource_Call, "client.dll", "8B E8 85 C0 75 ? 48 8B 0D", 0x0);

int iCurPlayer;
unsigned char _color[4];

__inline Color_t GetScoreboardColor(int iIndex, bool enableOtherColors)
{
    Color_t out = { 0, 0, 0, 0 };

    PlayerInfo_t pi{}; bool bTagColor = false; Color_t plTagColor;
    if (I::EngineClient->GetPlayerInfo(iIndex, &pi))
    {
        std::string _; PriorityLabel_t plTag;
        if (bTagColor = F::PlayerUtils.GetSignificantTag(pi.friendsID, &_, &plTag))
            plTagColor = plTag.Color;
    }

    if (iIndex == I::EngineClient->GetLocalPlayer())
        out = Vars::Colors::Local.Value;
    else if (H::Entities.IsFriend(iIndex))
        out = F::PlayerUtils.mTags["Friend"].Color;
    else if (bTagColor)
        out = plTagColor;

    return out;
}

MAKE_HOOK(CTFPlayerResource_GetPlayerConnectionState, S::CTFPlayerResource_GetPlayerConnectionState(), MM_PlayerConnectionState_t, __fastcall,
    void* ecx, int iIndex)
{
    static auto dwDesired = S::CTFPlayer_Resource_Call();
    const auto dwRetAddr = std::uintptr_t(_ReturnAddress());

    const auto result = CALL_ORIGINAL(ecx, iIndex);

    if (result != MM_WAITING_FOR_PLAYER && dwRetAddr == dwDesired)
        iCurPlayer = iIndex;
    else
        iCurPlayer = 0;

    return result;
}

MAKE_HOOK(CPlayerResource_GetTeamColor, S::CPlayerResource_GetTeamColor(), unsigned char*, __fastcall,
    void* ecx, int iIndex)
{
    if (!Vars::Visuals::UI::ScoreboardColors.Value || !iCurPlayer)
        return CALL_ORIGINAL(ecx, iIndex);

    const Color_t cReturn = GetScoreboardColor(iCurPlayer, Vars::Colors::Relative.Value);
    if (!cReturn.a)
        return CALL_ORIGINAL(ecx, iIndex);

    _color[0] = cReturn.r; _color[1] = cReturn.g; _color[2] = cReturn.b; _color[3] = 255;
    return _color;
}