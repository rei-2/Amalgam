#pragma once
#include "../Utils/Feature/Feature.h"

class CCore
{
public:
	void Load();
	void Loop();
	void Unload();

	bool m_bUnload = false;
	bool m_bFailed = false;
};

ADD_FEATURE_CUSTOM(CCore, Core, U);