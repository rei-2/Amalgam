#pragma once
#include "../../../SDK/SDK.h"

struct Notification_t
{
	std::string m_sText;
	float m_flCreateTime;
	float m_flLifeTime;
	float m_flPanTime;

	Color_t m_tAccent;
	Color_t m_tBackground;
	Color_t m_tActive;
};

class CNotifications
{
	std::deque<Notification_t> m_vNotifications;
	size_t m_iMaxNotifySize = 10;

public:
	void Add(const std::string& sText, float flLifeTime = Vars::Logging::Lifetime.Value, float flPanTime = 0.2f, const Color_t& tAccent = Vars::Menu::Theme::Accent.Value, const Color_t& tBackground = Vars::Menu::Theme::Background.Value, const Color_t& tActive = Vars::Menu::Theme::Active.Value);
	void Draw();
};

ADD_FEATURE(CNotifications, Notifications);