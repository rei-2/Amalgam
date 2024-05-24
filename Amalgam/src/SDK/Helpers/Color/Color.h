#pragma once
#include "../../Definitions/Types.h"
#include "../../Definitions/Main/CTFPlayer.h"

class CColor
{
public:
	Color_t GetTeamColor(int iLocalTeam, int iTargetTeam, bool bOther);
	Color_t GetEntityDrawColor(CTFPlayer* pLocal, CBaseEntity* pEntity, bool enableOtherColors);
};

ADD_FEATURE_CUSTOM(CColor, Color, H)