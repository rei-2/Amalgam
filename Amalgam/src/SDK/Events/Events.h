#pragma once

#include "../SDK.h"

class CEventListener : public CGameEventListener
{
private:
	bool m_bFailed = false;

public:
	virtual void FireGameEvent(IGameEvent* pEvent) override;

	bool Initialize();
	void Unload();
};

ADD_FEATURE_CUSTOM(CEventListener, Events, H);