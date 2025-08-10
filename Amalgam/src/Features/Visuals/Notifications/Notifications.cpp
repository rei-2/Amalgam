#include "Notifications.h"

static inline float EaseInOutCubic(float x)
{
	return x < 0.5f ? 4 * powf(x, 3) : 1 - powf(-2 * x + 2, 3) / 2;
}

void CNotifications::Add(const std::string& sText, Color_t tColor, float flLifeTime, float flPanTime)
{
	m_vNotifications.emplace_back(sText, float(SDK::PlatFloatTime()), flLifeTime, flPanTime, tColor);
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
		if (it->m_flCreateTime + it->m_flLifeTime + it->m_flPanTime < SDK::PlatFloatTime())
			it = m_vNotifications.erase(it);
		else
			++it;
	}
	if (m_vNotifications.empty())
		return;

	const auto& fFont = H::Fonts.GetFont(FONT_INDICATORS);

	int h = H::Draw.Scale(40, Scale_Round);
	int y = !ShouldReverseY() ? H::Draw.Scale(8, Scale_Round) : H::Draw.m_nScreenH - H::Draw.Scale(8, Scale_Round) - h;
	for (auto& tNotification : m_vNotifications)
	{
		int w = H::Draw.GetTextSize(tNotification.m_sText.c_str(), fFont).x + H::Draw.Scale(30, Scale_Round);
		int x = !ShouldReverseX() ? H::Draw.Scale(8, Scale_Round) : H::Draw.m_nScreenW - H::Draw.Scale(8, Scale_Round) - w;

		float flEaseX = 1.f, flEaseY = 1.f;
		float flTime = SDK::PlatFloatTime();
		float flCreate = tNotification.m_flCreateTime;
		float flLife = tNotification.m_flLifeTime;
		float flPan = tNotification.m_flPanTime;
		if (flPan)
		{
			float flDelta = std::min(flTime - flCreate, flCreate + flLife - flTime);
			if (flDelta < flPan)
				flEaseX = EaseInOutCubic(Math::RemapVal(flDelta, 0.f, flPan, 0.f, 1.f));

			flDelta = flCreate + flLife + flPan - flTime;
			if (flDelta < flPan)
				flEaseY = EaseInOutCubic(Math::RemapVal(flDelta, 0.f, flPan, 0.f, 1.f));
		}
		flLife = Math::RemapVal(flCreate + flLife - flPan - flTime, 0.f, flLife, 0.f, 1.f);

		x -= (w + H::Draw.Scale(8, Scale_Round)) * (1.f - flEaseX) * (!ShouldReverseX() ? 1 : -1);

		Color_t tAccent = tNotification.m_tColor;
		Color_t tBackground = Vars::Menu::Theme::Background.Value;
		Color_t tActive = Vars::Menu::Theme::Active.Value;

		H::Draw.FillRoundRect(x + 1, y + 1, w - 2, h - 2, H::Draw.Scale(3, Scale_Round), tBackground, 16);
		H::Draw.StartClipping(x + 1, y + h - 1 - H::Draw.Scale(2, Scale_Round), w - 2, H::Draw.Scale(2, Scale_Round));
		H::Draw.FillRoundRect(x + 1, y + 1, w - 2, h - 2, H::Draw.Scale(3, Scale_Round), tAccent.Alpha(50), 16);
		H::Draw.StartClipping(x + 1, y + h - 1 - H::Draw.Scale(2, Scale_Round), (w - 2) * flLife, H::Draw.Scale(2, Scale_Round));
		H::Draw.FillRoundRect(x + 1, y + 1, w - 2, h - 2, H::Draw.Scale(3, Scale_Round), tAccent.Alpha(255), 16);
		H::Draw.EndClipping();
		H::Draw.LineRoundRect(x, y, w, h, H::Draw.Scale(4, Scale_Round), tBackground.Lerp({ 127, 127, 127 }, 2.f / 9), 16);
		H::Draw.StringOutlined(fFont, x + H::Draw.Scale(15, Scale_Round), y + H::Draw.Scale(13, Scale_Round), tActive.Alpha(255), Vars::Menu::Theme::Background.Value, ALIGN_TOPLEFT, tNotification.m_sText.c_str());

		y += (h + H::Draw.Scale(8, Scale_Round)) * (flEaseY) * (!ShouldReverseY() ? 1 : -1);
	}
}