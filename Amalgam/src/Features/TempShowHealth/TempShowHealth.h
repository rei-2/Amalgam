#pragma once
#include "../../SDK/SDK.h"

class CTempShowHealth
{
public:
	void Run(CTFPlayer* pLocal);
	bool m_bEnabled = true; // Always enabled by default
};

ADD_FEATURE(CTempShowHealth, TempShowHealth);