#pragma once
#include "../../../SDK/SDK.h"

class CPlayerConditions
{
public:
	std::vector<std::string> Get(CTFPlayer* pEntity);
	void Draw(CTFPlayer* pLocal);
};

ADD_FEATURE(CPlayerConditions, PlayerConditions)