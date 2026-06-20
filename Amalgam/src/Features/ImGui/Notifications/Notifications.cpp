#include "Notifications.h"

#include "../Easings/Easings.h"
#include "../Menu/Components.h"
#include <ImGui/imgui_internal.h>

#define EASE_IN Ease::OutCubic
#define EASE_OUT Ease::OutCubic
#define EASE_Y Ease::InOutCubic

void CNotifications::Add(const std::string& sText, const char* sIcon, Color_t tColor, float flLifeTime, float flPanTime)
{
	float flTime = SDK::PlatFloatTime();
	m_vNotifications.emplace_back(sText, sIcon, flTime, flLifeTime, flPanTime, tColor);
	if (m_vNotifications.size() > Vars::Logging::MaxNotifications.Value)
	{
		for (int i = 0; i < m_vNotifications.size() - Vars::Logging::MaxNotifications.Value; i++)
		{
			auto& tNotification = m_vNotifications[i];
			if (!tNotification.m_flLifeTime)
				continue;

			tNotification.m_flCreateTime = flTime - tNotification.m_flPanTime;
			tNotification.m_flLifeTime = 0.f;
		}
		//m_vNotifications.pop_front();
	}
}

void CNotifications::Add(const std::string& sText, Color_t tColor, float flLifeTime, float flPanTime)
{
	Add(sText, nullptr, tColor, flLifeTime, flPanTime);
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
static inline int X()
{
	return ShouldReverseX() ? -1 : 1;
}
static inline int Y()
{
	return ShouldReverseY() ? -1 : 1;
}

void CNotifications::Draw()
{
	using namespace ImGui;

	for (auto it = m_vNotifications.begin(); it != m_vNotifications.end();)
	{
		if (it->m_flCreateTime + it->m_flLifeTime + it->m_flPanTime * 3 < SDK::PlatFloatTime())
			it = m_vNotifications.erase(it);
		else
			++it;
	}
	if (m_vNotifications.empty())
		return;

	ImDrawList* pDrawList = GetForegroundDrawList();

	float h = H::Draw.Scale(40);
	float y = ShouldReverseY() ? GetIO().DisplaySize.y - H::Draw.Scale(8) - h : H::Draw.Scale(8);
	for (auto& tNotification : m_vNotifications)
	{
		bool bIcon = tNotification.m_sIcon;

		float w = CalcTextSize(tNotification.m_sText.c_str()).x + H::Draw.Scale(bIcon ? 54 : 30);
		float x = ShouldReverseX() ? GetIO().DisplaySize.x - H::Draw.Scale(8) - w : H::Draw.Scale(8);

		float flEaseX = 1.f, flEaseY = 1.f;
		float flTime = SDK::PlatFloatTime();
		float flCreate = tNotification.m_flCreateTime;
		float flPan = tNotification.m_flPanTime;
		float flLife = tNotification.m_flLifeTime + flPan * 2;
		if (flPan)
		{
			if (float flDelta = flTime - flCreate; flDelta < flPan)
				flEaseX = EASE_IN(Math::RemapVal(flDelta, 0.f, flPan, 0.f, 1.f));
			else if (float flDelta = flCreate + flLife - flTime; flDelta < flPan)
				flEaseX = EASE_OUT(Math::RemapVal(flDelta, 0.f, flPan, 0.f, 1.f));
			if (float flDelta = flCreate + flLife + flPan - flTime; flDelta < flPan)
				flEaseY = EASE_Y(Math::RemapVal(flDelta, 0.f, flPan, 0.f, 1.f));
		}
		flLife = Math::RemapVal(flCreate + flLife - flPan - flTime, 0.f, flLife, 0.f, 1.f);

		x -= (w + H::Draw.Scale(8)) * (1.f - flEaseX) * X();

		ImU32 uBackground = F::Render.Background0;
		ImU32 uBorder = F::Render.Background2;
		ImU32 uActive = F::Render.Active;
		ImU32 uAccentOpaque = ColorByteToInt(tNotification.m_tColor.Alpha(255));
		ImU32 uAccentTransparent = ColorByteToInt(tNotification.m_tColor.Alpha(50));

		ImVec2 vDrawPos = { x, y };
		ImVec2 vSize = { w, h };
		float flInset = H::Draw.Scale();

		pDrawList->AddRectFilled(vDrawPos + ImVec2(flInset, flInset), vDrawPos + vSize - ImVec2(flInset, flInset), uBackground, H::Draw.Scale(3));
		pDrawList->PushClipRect(vDrawPos + ImVec2(flInset, h - flInset - H::Draw.Scale(2)), vDrawPos + ImVec2(w - flInset, h - flInset), false);
		pDrawList->AddRectFilled(vDrawPos + ImVec2(flInset, flInset), vDrawPos + vSize - ImVec2(flInset, flInset), uAccentTransparent, H::Draw.Scale(3));
		pDrawList->PopClipRect();
		pDrawList->PushClipRect(vDrawPos + ImVec2(flInset, h - flInset - H::Draw.Scale(2)), vDrawPos + ImVec2((w - flInset) * flLife, h - flInset), false);
		pDrawList->AddRectFilled(vDrawPos + ImVec2(flInset, flInset), vDrawPos + vSize - ImVec2(flInset, flInset), uAccentOpaque, H::Draw.Scale(3));
		pDrawList->PopClipRect();
		flInset += H::Draw.Scale(0.5f) - 0.5f - H::Draw.Scale();
		pDrawList->AddRect(vDrawPos + ImVec2(flInset, flInset), vDrawPos + vSize - ImVec2(flInset, flInset), uBorder, H::Draw.Scale(4), ImDrawFlags_None, H::Draw.Scale());
		if (!bIcon)
			pDrawList->AddText(vDrawPos + ImVec2(H::Draw.Scale(15), H::Draw.Scale(13)), uActive, tNotification.m_sText.c_str());
		else
		{
			pDrawList->AddText(vDrawPos + ImVec2(H::Draw.Scale(39), H::Draw.Scale(13)), uActive, tNotification.m_sText.c_str());
			pDrawList->AddText(F::Render.IconFont, F::Render.IconFont->LegacySize, vDrawPos + ImVec2(H::Draw.Scale(12), H::Draw.Scale(12)), uAccentOpaque, tNotification.m_sIcon);
		}

		y += (h + H::Draw.Scale(8, Scale_Round)) * (flEaseY) * Y();
	}
}