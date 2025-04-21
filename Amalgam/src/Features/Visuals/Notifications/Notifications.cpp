#include "Notifications.h"

static float EaseInOutCubic(float x)
{
	return x < 0.5f ? 4.f * x * x * x : 1 - powf(-2.f * x + 2.f, 3.f) / 2.f;
}

void CNotifications::Add(const std::string& sText, float flLifeTime, float flPanTime, const Color_t& tAccent, const Color_t& tBackground, const Color_t& tActive)
{
	m_vNotifications.emplace_back(sText, float(SDK::PlatFloatTime()), flLifeTime, flPanTime, tAccent, tBackground, tActive);
	while (m_vNotifications.size() > m_iMaxNotifySize)
		m_vNotifications.pop_front();
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

	int y = 8;
	const auto& fFont = H::Fonts.GetFont(FONT_INDICATORS);
	for (auto& tNotification : m_vNotifications)
	{
		int x = 8;
		Vec2 vSize = H::Draw.GetTextSize(tNotification.m_sText.c_str(), fFont);
		vSize.x += H::Draw.Scale(23, Scale_Round); vSize.y = H::Draw.Scale(32, Scale_Round);

		float flEase = 0.f;
		if (float flPan = tNotification.m_flPanTime)
		{
			float flTime = SDK::PlatFloatTime();
			float flCreate = tNotification.m_flCreateTime;
			float flLife = std::min(flTime - flCreate, flCreate + tNotification.m_flLifeTime - flTime);
			if (flLife < flPan)
			{
				float flRatio = Math::RemapVal(flLife, flPan, 0.f, 0.f, 1.f);
				flEase = EaseInOutCubic(flRatio);
				x -= (vSize.x + 8) * flEase;
			}
		}

		H::Draw.GradientRect(x + 1, y + 1, vSize.x - 2, vSize.y - 2, tNotification.m_tBackground.Alpha(255), tNotification.m_tBackground.Alpha(127), true);
		H::Draw.FillRect(x + 1, y + 1, H::Draw.Scale(2, Scale_Round), vSize.y - 2, tNotification.m_tAccent.Alpha(255));
		H::Draw.LineRect(x, y, vSize.x, vSize.y, tNotification.m_tBackground.Alpha(255));
		H::Draw.StringOutlined(fFont, x + H::Draw.Scale(13, Scale_Round), y + H::Draw.Scale(9, Scale_Round), tNotification.m_tActive.Alpha(255), Vars::Menu::Theme::Background.Value, ALIGN_TOPLEFT, tNotification.m_sText.c_str());

		y += (vSize.y + 8) * (1.f - flEase);
	}
}