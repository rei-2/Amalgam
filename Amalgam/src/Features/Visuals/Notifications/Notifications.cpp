#include "Notifications.h"

static inline float EaseInOutCubic(float x)
{
	return x < 0.5f ? 4 * powf(x, 3) : 1 - powf(-2 * x + 2, 3) / 2;
}

static Color_t BlendColors(const Color_t& a, const Color_t& b, float ratio) 
{
    Color_t result;
    result.r = static_cast<byte>(a.r * (1.0f - ratio) + b.r * ratio);
    result.g = static_cast<byte>(a.g * (1.0f - ratio) + b.g * ratio);
    result.b = static_cast<byte>(a.b * (1.0f - ratio) + b.b * ratio);
    result.a = static_cast<byte>(a.a * (1.0f - ratio) + b.a * ratio);
    return result;
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
	const int iRounding = H::Draw.Scale(3);
	const int iBottomPadding = H::Draw.Scale(4, Scale_Round);
	const int iBarRounding = std::max(1, iRounding / 2);

	int y = !ShouldReverseY() ? H::Draw.Scale(8, Scale_Round) : H::Draw.m_nScreenH - H::Draw.Scale(8, Scale_Round) - H::Draw.Scale(32, Scale_Round);
	for (auto& tNotification : m_vNotifications)
	{
		Vec2 textSize = H::Draw.GetTextSize(tNotification.m_sText.c_str(), fFont);
		int textPadding = H::Draw.Scale(8, Scale_Round);
		
		int w = textSize.x + textPadding * 2;
		int h = H::Draw.Scale(24, Scale_Round) + iBottomPadding;

		int x = !ShouldReverseX() ? H::Draw.Scale(8, Scale_Round) : H::Draw.m_nScreenW - H::Draw.Scale(8, Scale_Round) - w;

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
				x -= (w + H::Draw.Scale(8, Scale_Round)) * flEase * (!ShouldReverseX() ? 1 : -1);
			}
		}

		float flProgress = Math::RemapVal(flTime - flCreate, 0.f, tNotification.m_flLifeTime, 0.f, 1.f);
		flProgress = std::clamp(flProgress, 0.0f, 1.0f);

		int barHeight = H::Draw.Scale(2, Scale_Round);  
		int barY = y + h - barHeight - iBottomPadding;
		int totalBarWidth = w - 2 * iRounding;

		H::Draw.FillRoundRect(x, y, w, h, iRounding, tNotification.m_tBackground);

		Color_t barColor = tNotification.m_tAccent;
		Color_t dimmedAccent = BlendColors(tNotification.m_tAccent, tNotification.m_tBackground, 0.5f);
		Color_t barBackgroundColor = dimmedAccent;

		H::Draw.FillRoundRect(x + iRounding, barY, totalBarWidth, barHeight, iBarRounding, barBackgroundColor);

		int barWidth = static_cast<int>(totalBarWidth * flProgress);
		if (barWidth > 0)
			H::Draw.FillRoundRect(x + iRounding, barY, barWidth, barHeight, iBarRounding, barColor);

		Color_t borderColor = BlendColors(tNotification.m_tBackground, Color_t(255, 255, 255, 50), 0.1f);
		H::Draw.LineRoundRect(x, y, w, h, iRounding, borderColor);

		H::Draw.StringOutlined(
			fFont,
			x + textPadding,
			y + (h - barHeight - iBottomPadding) / 2,
			tNotification.m_tActive,
			tNotification.m_tBackground.Alpha(150),
			ALIGN_LEFT,
			tNotification.m_sText.c_str()
		);

		y += (h + H::Draw.Scale(8, Scale_Round)) * (1.f - flEase) * (!ShouldReverseY() ? 1 : -1);
	}
}