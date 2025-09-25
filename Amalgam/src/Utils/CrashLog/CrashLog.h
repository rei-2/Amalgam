#pragma once
#include "../Macros/Macros.h"
#include <Windows.h>

class CCrashLog
{
public:
	void Initialize(LPVOID lpParam = nullptr);
	void Unload();
};

ADD_FEATURE_CUSTOM(CCrashLog, CrashLog, U);