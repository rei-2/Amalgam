#pragma once
#include "../Interfaces/IGameEvents.h"

class CGameEventListener : public IGameEventListener2
{
public:
	virtual void FireGameEvent(IGameEvent* event) = 0;

private:
	bool m_bRegisteredForEvents;
};