#pragma once
#include "../../../SDK/SDK.h"

struct Notification_t
{
	std::string m_sText;
	Color_t m_cColor;
	float m_flTime;
};

class CNotifications
{
	std::vector<Notification_t> vNotifications;
	int iMaxNotifySize = 8;

public:
	void Add(const std::string& sText, Color_t cColor = { 255, 255, 255, 255 }, float flLifeTime = Vars::Logging::Lifetime.Value);
	void Draw();
};

ADD_FEATURE(CNotifications, Notifications)