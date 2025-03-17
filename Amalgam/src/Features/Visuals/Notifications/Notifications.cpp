#include "Notifications.h"

static float EaseInOutCubic(float x)
{
	return x < 0.5f ? 4.f * x * x * x : 1 - powf(-2.f * x + 2.f, 3.f) / 2.f;
}

void CNotifications::Add(const std::string& sText, float flLifeTime, float flPanTime)
{
	vNotifications.emplace_back(sText, float(SDK::PlatFloatTime()), flLifeTime, flPanTime);
}

void CNotifications::Draw()
{
	while (vNotifications.size() > iMaxNotifySize)
		vNotifications.pop_front();
	for (auto it = vNotifications.begin(); it != vNotifications.end();)
	{
		if (it->m_flCreateTime + it->m_flLifeTime <= SDK::PlatFloatTime())
			it = vNotifications.erase(it);
		else
			++it;
	}

	if (vNotifications.empty())
		return;

	int y = 8;
	const auto& fFont = H::Fonts.GetFont(FONT_INDICATORS);
	for (auto& tNotification : vNotifications)
	{
		int x = 8;
		int w, h; I::MatSystemSurface->GetTextSize(fFont.m_dwFont, SDK::ConvertUtf8ToWide(tNotification.m_sText).c_str(), w, h);
		w += H::Draw.Scale(23, Scale_Round); h = H::Draw.Scale(32, Scale_Round);

		float flEase = 0.f;
		if (float flPan = tNotification.m_flPanTime)
		{
			float flTime = SDK::PlatFloatTime();
			float flCreate = tNotification.m_flCreateTime;
			float flLife = std::min(flTime - flCreate, flCreate + tNotification.m_flLifeTime - flTime);
			if (flLife < flPan)
			{
				float flRatio = Math::RemapValClamped(flLife, flPan, 0.f, 0.f, 1.f);
				flEase = EaseInOutCubic(flRatio);
				x -= (w + 8) * flEase;
			}
		}

		auto& cAccent = Vars::Menu::Theme::Accent.Value, & cActive = Vars::Menu::Theme::Active.Value, & cBackground = Vars::Menu::Theme::Background.Value;
		H::Draw.GradientRect(x + 1, y + 1, w - 2, h - 2, { cBackground.r, cBackground.g, cBackground.b, 255 }, { cBackground.r, cBackground.g, cBackground.b, 127 }, true);
		H::Draw.FillRect(x + 1, y + 1, H::Draw.Scale(2, Scale_Round), h - 2, { cAccent.r, cAccent.g, cAccent.b, 255 });
		H::Draw.LineRect(x, y, w, h, { cBackground.r, cBackground.g, cBackground.b, 255 });
		H::Draw.StringOutlined(fFont, x + H::Draw.Scale(13, Scale_Round), y + H::Draw.Scale(9, Scale_Round), { cActive.r, cActive.g, cActive.b, 255 }, Vars::Menu::Theme::Background.Value, ALIGN_TOPLEFT, tNotification.m_sText.c_str());

		y += (h + 8) * (1.f - flEase);
	}
}