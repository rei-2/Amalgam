#pragma once
#include "../Utils/Feature/Feature.h"

class CCore
{
public:
	void Load();
	void Loop();
	void Unload();

	bool m_bUnload = false;

private:
	bool m_bFailed = false;
	bool m_bFailed2 = false;
};

ADD_FEATURE_CUSTOM(CCore, Core, U);