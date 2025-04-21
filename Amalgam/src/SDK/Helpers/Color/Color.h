#pragma once
#include "../../Definitions/Types.h"
#include "../../Definitions/Main/CTFPlayer.h"

class CColor
{
public:
	Color_t GetTeamColor(int iLocalTeam, int iTargetTeam, bool bRelative);
	Color_t GetEntityDrawColor(CTFPlayer* pLocal, CBaseEntity* pEntity, bool bRelative, CBaseEntity* pTarget = nullptr, int* pType = nullptr);
	Color_t GetScoreboardColor(int iIndex);
};

ADD_FEATURE_CUSTOM(CColor, Color, H)