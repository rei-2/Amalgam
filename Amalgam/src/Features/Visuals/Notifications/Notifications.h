#pragma once
#include "../../../SDK/SDK.h"

struct Notification_t
{
	std::string m_sText;
	float m_flCreateTime;
	float m_flLifeTime;
	float m_flPanTime;

	Color_t m_tColor;
};

class CNotifications
{
private:
	std::deque<Notification_t> m_vNotifications;
	size_t m_iMaxNotifySize = 10;

public:
	void Add(const std::string& sText, Color_t tColor = Vars::Menu::Theme::Accent.Value, float flLifeTime = Vars::Logging::Lifetime.Value, float flPanTime = 0.2f);
	void Draw();
};

ADD_FEATURE(CNotifications, Notifications);