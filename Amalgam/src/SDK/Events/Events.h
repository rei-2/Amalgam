#pragma once

#include "../SDK.h"

class CEventListener : public CGameEventListener
{
public:
	bool Initialize();
	void Unload();

	virtual void FireGameEvent(IGameEvent* pEvent) override;
};

ADD_FEATURE_CUSTOM(CEventListener, Events, H);