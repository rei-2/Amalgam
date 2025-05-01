#include "Notifications.h"

static inline float EaseInOutCubic(float x)
{
	return x < 0.5f ? 4 * powf(x, 3) : 1 - powf(-2 * x + 2, 3) / 2;
}

void CNotifications::Add(const std::string& sText, float flLifeTime, float flPanTime, const Color_t& tAccent, const Color_t& tBackground, const Color_t& tActive)
{
	m_vNotifications.emplace_back(sText, float(SDK::PlatFloatTime()), flLifeTime, flPanTime, tAccent, tBackground, tActive);
	while (m_vNotifications.size() > m_iMaxNotifySize)
		m_vNotifications.pop_front();
}

static inline bool ShouldReverseX()
{
	switch (Vars::Logging::NotificationPosition.Value)
	{
	case Vars::Logging::NotificationPositionEnum::TopLeft:
	case Vars::Logging::NotificationPositionEnum::BottomLeft:
		return false;
	case Vars::Logging::NotificationPositionEnum::TopRight:
	case Vars::Logging::NotificationPositionEnum::BottomRight:
		return true;
	}
	return false;
}

static inline bool ShouldReverseY()
{
	switch (Vars::Logging::NotificationPosition.Value)
	{
	case Vars::Logging::NotificationPositionEnum::TopLeft:
	case Vars::Logging::NotificationPositionEnum::TopRight:
		return false;
	case Vars::Logging::NotificationPositionEnum::BottomLeft:
	case Vars::Logging::NotificationPositionEnum::BottomRight:
		return true;
	}
	return false;
}

void CNotifications::Draw()
{
	for (auto it = m_vNotifications.begin(); it != m_vNotifications.end();)
	{
		if (it->m_flCreateTime + it->m_flLifeTime <= SDK::PlatFloatTime())
			it = m_vNotifications.erase(it);
		else
			++it;
	}
	if (m_vNotifications.empty())
		return;

	const auto& fFont = H::Fonts.GetFont(FONT_INDICATORS);

	int y = !ShouldReverseY() ? H::Draw.Scale(8, Scale_Round) : H::Draw.m_nScreenH - H::Draw.Scale(8, Scale_Round) - H::Draw.Scale(32, Scale_Round);
	for (auto& tNotification : m_vNotifications)
	{
		Vec2 vSize = { H::Draw.GetTextSize(tNotification.m_sText.c_str(), fFont).x + H::Draw.Scale(23, Scale_Round), H::Draw.Scale(32, Scale_Round) };

		int x = !ShouldReverseX() ? H::Draw.Scale(8, Scale_Round) : H::Draw.m_nScreenW - H::Draw.Scale(8, Scale_Round) - vSize.x;

		float flTime = SDK::PlatFloatTime();
		float flCreate = tNotification.m_flCreateTime;
		float flEase = 0.f;
		if (float flPan = tNotification.m_flPanTime)
		{
			float flLife = std::min(flTime - flCreate, flCreate + tNotification.m_flLifeTime - flTime);
			if (flLife < flPan)
			{
				float flRatio = Math::RemapVal(flLife, flPan, 0.f, 0.f, 1.f);
				flEase = EaseInOutCubic(flRatio);
				x -= (vSize.x + H::Draw.Scale(8, Scale_Round)) * flEase * (!ShouldReverseX() ? 1 : -1);
			}
		}

		float flLife = Math::RemapVal(flCreate + tNotification.m_flLifeTime - flTime, 0.f, tNotification.m_flLifeTime, 0.f, 1.f);
		H::Draw.GradientRect(x + 1, y + 1, vSize.x - 2, vSize.y - 2, tNotification.m_tBackground.Alpha(255), tNotification.m_tBackground.Alpha(127), true);
		H::Draw.FillRect(x + 1, y + 1, H::Draw.Scale(2, Scale_Round), vSize.y - 2, tNotification.m_tAccent.Alpha(127));
		H::Draw.FillRect(x + 1, y + 1, H::Draw.Scale(2, Scale_Round), (vSize.y - 2) * flLife, tNotification.m_tAccent.Alpha(255));
		//H::Draw.LineRoundRect(x, y, vSize.x, vSize.y, 2, tNotification.m_tBackground.Alpha(255), 8);
		H::Draw.LineRect(x, y, vSize.x, vSize.y, tNotification.m_tBackground.Alpha(255));
		H::Draw.StringOutlined(fFont, x + H::Draw.Scale(13, Scale_Round), y + H::Draw.Scale(9, Scale_Round), tNotification.m_tActive.Alpha(255), Vars::Menu::Theme::Background.Value, ALIGN_TOPLEFT, tNotification.m_sText.c_str());

		y += (vSize.y + H::Draw.Scale(8, Scale_Round)) * (1.f - flEase) * (!ShouldReverseY() ? 1 : -1);
	}
}