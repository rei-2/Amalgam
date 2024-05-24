#pragma once
#include "../../../SDK/SDK.h"

class LCESP
{
public:
	std::vector<std::wstring> GetPlayerConditions(CTFPlayer* pEntity) const;
};

ADD_FEATURE(LCESP, LocalConditions)