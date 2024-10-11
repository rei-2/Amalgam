#pragma once
#include "../Utils/Feature/Feature.h"

class CCore
{
public:
	void Load();
	void Loop();
	void Unload();

	bool bUnload = false;
	bool bEarly = false;
};

ADD_FEATURE_CUSTOM(CCore, Core, U);