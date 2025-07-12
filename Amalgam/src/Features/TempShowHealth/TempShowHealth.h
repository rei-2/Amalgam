#pragma once
#include "../../SDK/SDK.h"

class CTempShowHealth
{
public:
	void Run(CTFPlayer* pLocal);
	bool m_bEnabled = false; // Disabled by default
};

ADD_FEATURE(CTempShowHealth, TempShowHealth);