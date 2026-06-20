#pragma once
#include "../../../SDK/SDK.h"

struct Notification_t
{
	std::string m_sText = "";
	const char* m_sIcon = nullptr;
	float m_flCreateTime = 0.f;
	float m_flLifeTime = 0.f;
	float m_flPanTime = 0.f;

	Color_t m_tColor = {};
};

class CNotifications
{
private:
	std::deque<Notification_t> m_vNotifications;

public:
	void Add(const std::string& sText, const char* sIcon, Color_t tColor = Vars::Menu::Theme::Accent.Value, float flLifeTime = Vars::Logging::NotificationTime.Value, float flPanTime = 0.2f);
	void Add(const std::string& sText, Color_t tColor = Vars::Menu::Theme::Accent.Value, float flLifeTime = Vars::Logging::NotificationTime.Value, float flPanTime = 0.2f);
	void Draw();
};

ADD_FEATURE(CNotifications, Notifications);