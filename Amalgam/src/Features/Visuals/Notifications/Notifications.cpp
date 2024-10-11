#include "Notifications.h"

void CNotifications::Add(const std::string& sText, float flLifeTime, float flPanTime)
{
	vNotifications.push_back({ sText, float(SDK::PlatFloatTime()), flLifeTime, flPanTime });
}

void CNotifications::Draw()
{
	while (vNotifications.size() > iMaxNotifySize)
		vNotifications.erase(vNotifications.begin());

	for (auto it = vNotifications.begin(); it != vNotifications.end();)
	{
		if (it->m_flCreateTime + it->m_flLifeTime <= SDK::PlatFloatTime())
			it = vNotifications.erase(it);
		else
			++it;
	}

	if (vNotifications.empty())
		return;

	int y = 2;
	const auto& fFont = H::Fonts.GetFont(FONT_INDICATORS);
	for (auto& tNotification : vNotifications)
	{
		int x = 2;
		int w, h; I::MatSystemSurface->GetTextSize(fFont.m_dwFont, SDK::ConvertUtf8ToWide(tNotification.m_sText).c_str(), w, h);
		w += 4; h += 4;

		if (float flPan = tNotification.m_flPanTime)
		{
			float flTime = SDK::PlatFloatTime();
			float flCreate = tNotification.m_flCreateTime;
			float flLife = std::min(flTime - flCreate, flCreate + tNotification.m_flLifeTime - flTime);
			if (flLife < flPan)
				x -= powf(Math::RemapValClamped(flLife, flPan, 0.f, 0.f, 1.f), 2.f) * w;
		}

		auto& cAccent = Vars::Menu::Theme::Accent.Value, & cActive = Vars::Menu::Theme::Active.Value, & cBackground = Vars::Menu::Theme::Background.Value;
		H::Draw.Line(x, y, x, y + h, { cAccent.r, cAccent.g, cAccent.b, 255 });
		H::Draw.GradientRect(x + 1, y, w, h, { cBackground.r, cBackground.g, cBackground.b, 255 }, { cBackground.r, cBackground.g, cBackground.b, 0 }, true);
		H::Draw.String(fFont, x + 6, y + 2, { cActive.r, cActive.g, cActive.b, 255 }, ALIGN_TOPLEFT, tNotification.m_sText.c_str());

		y += h + 2;
	}
}