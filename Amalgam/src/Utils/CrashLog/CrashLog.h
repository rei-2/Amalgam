#pragma once
#include "../Macros/Macros.h"

class CCrashLog
{
public:
	void Initialize();
	void Unload();
};

ADD_FEATURE_CUSTOM(CCrashLog, CrashLog, U);