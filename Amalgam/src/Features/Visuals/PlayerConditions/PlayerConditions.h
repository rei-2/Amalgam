#pragma once
#include "../../../SDK/SDK.h"

class CPlayerConditions
{
public:
	std::vector<std::wstring> GetPlayerConditions(CTFPlayer* pEntity) const;
};

ADD_FEATURE(CPlayerConditions, PlayerConditions)