#pragma once

#include "../SDK.h"

class CEventListener : public CGameEventListener
{
private:
	bool m_bFailed = false;

public:
	bool Initialize();
	void Unload();

	virtual void FireGameEvent(IGameEvent* pEvent) override;
};

ADD_FEATURE_CUSTOM(CEventListener, Events, H);