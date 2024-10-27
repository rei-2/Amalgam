#pragma once
#include "../../../SDK/SDK.h"

struct Notification_t
{
	std::string m_sText;
	float m_flCreateTime;
	float m_flLifeTime;
	float m_flPanTime;
};

class CNotifications
{
	std::deque<Notification_t> vNotifications;
	unsigned iMaxNotifySize = 10;

public:
	void Add(const std::string& sText, float flLifeTime = Vars::Logging::Lifetime.Value, float flPanTime = 0.2f);
	void Draw();
};

ADD_FEATURE(CNotifications, Notifications)