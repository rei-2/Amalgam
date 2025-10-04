#include "Draw.h"

#include "../../SDK.h"
#include "Icons.h"
#include "../../Definitions/Interfaces.h"
#include "../../../Utils/Math/Math.h"
#include "../../../Utils/Timer/Timer.h"
#include <array>
#include <ranges>

MAKE_SIGNATURE(CHudBaseDeathNotice_GetIcon, "client.dll", "40 53 48 81 EC ? ? ? ? 48 8B DA", 0x0);
MAKE_SIGNATURE(RenderLine, "engine.dll", "48 89 5C 24 ? 48 89 74 24 ? 44 89 44 24", 0x0);
MAKE_SIGNATURE(RenderBox, "engine.dll", "48 83 EC ? 8B 84 24 ? ? ? ? 4D 8B D8", 0x0);
MAKE_SIGNATURE(RenderWireframeBox, "engine.dll", "48 89 5C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 55 41 54 41 55 41 56 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 49 8B F9", 0x0);
MAKE_SIGNATURE(RenderWireframeSweptBox, "engine.dll", "48 8B C4 48 89 58 ? 48 89 50 ? 48 89 48 ? 55 56 57 41 54 41 55 41 56 41 57 48 8D A8 ? ? ? ? 48 81 EC ? ? ? ? 0F 29 70", 0x0);
MAKE_SIGNATURE(RenderTriangle, "engine.dll", "48 83 EC ? 41 8B C1 44 88 4C 24", 0x0);
MAKE_SIGNATURE(RenderSphere, "engine.dll", "48 8B C4 44 89 48 ? F3 0F 11 48", 0x0);

void CDraw::Start(bool bBadFontCheck)
{
	I::MatSystemSurface->StartDrawing();
	I::MatSystemSurface->DisableClipping(true);

	if (bBadFontCheck)
	{
		static Timer tTimer = {};
		if (tTimer.Run(1.f))
		{
			if (!GetTextSize("", H::Fonts.GetFont(FONT_ESP)).y)
				H::Fonts.Reload(Vars::Menu::Scale[DEFAULT_BIND]);
		}
	}
}

void CDraw::End()
{
	I::MatSystemSurface->FinishDrawing();
}

void CDraw::StartClipping(int x, int y, int w, int h)
{
	I::MatSystemSurface->DisableClipping(false);
	I::MatSystemSurface->SetClippingRect(x, y, x + w, y + h);
}

void CDraw::EndClipping()
{
	I::MatSystemSurface->DisableClipping(true);
}

void CDraw::UpdateScreenSize()
{
	I::MatSystemSurface->GetScreenSize(m_nScreenW, m_nScreenH);
}

void CDraw::UpdateW2SMatrix()
{
	CViewSetup tViewSetup;
	if (I::BaseClientDLL->GetPlayerView(tViewSetup))
	{
		static VMatrix mWorldToView;
		static VMatrix mViewToProjection;
		static VMatrix mWorldToPixels;
		I::RenderView->GetMatricesForView(tViewSetup, &mWorldToView, &mViewToProjection, &m_mWorldToProjection, &mWorldToPixels);
	}
}

Vec2 CDraw::GetTextSize(const char* text, const Font_t& tFont)
{
	int w = 0, h = 0;
	I::MatSystemSurface->GetTextSize(tFont.m_dwFont, SDK::ConvertUtf8ToWide(text).c_str(), w, h);
	return { float(w), float(h) };
}

Vec2 CDraw::GetTextSize(const wchar_t* text, const Font_t& tFont)
{
	int w = 0, h = 0;
	I::MatSystemSurface->GetTextSize(tFont.m_dwFont, text, w, h);
	return { float(w), float(h) };
}

static wchar_t wstr[1024] = { '\0' };
void CDraw::String(const Font_t& tFont, int x, int y, const Color_t& tColor, const EAlign& eAlign, const char* str)
{
	wsprintfW(wstr, L"%hs", str);
	const auto dwFont = tFont.m_dwFont;

	Vec2 vSize = GetTextSize(str, tFont);
	switch (eAlign)
	{
	case ALIGN_TOPLEFT: break;
	case ALIGN_TOP: x -= vSize.x / 2; break;
	case ALIGN_TOPRIGHT: x -= vSize.x; break;
	case ALIGN_LEFT: y -= vSize.y / 2; break;
	case ALIGN_CENTER: x -= vSize.x / 2; y -= vSize.y / 2; break;
	case ALIGN_RIGHT: x -= vSize.x; y -= vSize.y / 2; break;
	case ALIGN_BOTTOMLEFT: y -= vSize.y; break;
	case ALIGN_BOTTOM: x -= vSize.x / 2; y -= vSize.y; break;
	case ALIGN_BOTTOMRIGHT: x -= vSize.x; y -= vSize.y; break;
	}

	I::MatSystemSurface->DrawSetTextPos(x, y);
	I::MatSystemSurface->DrawSetTextFont(dwFont);
	I::MatSystemSurface->DrawSetTextColor(tColor.r, tColor.g, tColor.b, tColor.a);
	I::MatSystemSurface->DrawPrintText(wstr, int(wcslen(wstr)));
}
void CDraw::String(const Font_t& tFont, int x, int y, const Color_t& tColor, const EAlign& eAlign, const wchar_t* wstr)
{
	const auto dwFont = tFont.m_dwFont;

	Vec2 vSize = GetTextSize(wstr, tFont);
	switch (eAlign)
	{
	case ALIGN_TOPLEFT: break;
	case ALIGN_TOP: x -= vSize.x / 2; break;
	case ALIGN_TOPRIGHT: x -= vSize.x; break;
	case ALIGN_LEFT: y -= vSize.y / 2; break;
	case ALIGN_CENTER: x -= vSize.x / 2; y -= vSize.y / 2; break;
	case ALIGN_RIGHT: x -= vSize.x; y -= vSize.y / 2; break;
	case ALIGN_BOTTOMLEFT: y -= vSize.y; break;
	case ALIGN_BOTTOM: x -= vSize.x / 2; y -= vSize.y; break;
	case ALIGN_BOTTOMRIGHT: x -= vSize.x; y -= vSize.y; break;
	}

	I::MatSystemSurface->DrawSetTextPos(x, y);
	I::MatSystemSurface->DrawSetTextFont(dwFont);
	I::MatSystemSurface->DrawSetTextColor(tColor.r, tColor.g, tColor.b, tColor.a);
	I::MatSystemSurface->DrawPrintText(wstr, int(wcslen(wstr)));
}
void CDraw::StringOutlined(const Font_t& tFont, int x, int y, const Color_t& tColor, const Color_t& tColorOut, const EAlign& eAlign, const char* str)
{
	wsprintfW(wstr, L"%hs", str);
	const auto dwFont = tFont.m_dwFont;

	Vec2 vSize = GetTextSize(wstr, tFont);
	switch (eAlign)
	{
	case ALIGN_TOPLEFT: break;
	case ALIGN_TOP: x -= vSize.x / 2; break;
	case ALIGN_TOPRIGHT: x -= vSize.x; break;
	case ALIGN_LEFT: y -= vSize.y / 2; break;
	case ALIGN_CENTER: x -= vSize.x / 2; y -= vSize.y / 2; break;
	case ALIGN_RIGHT: x -= vSize.x; y -= vSize.y / 2; break;
	case ALIGN_BOTTOMLEFT: y -= vSize.y; break;
	case ALIGN_BOTTOM: x -= vSize.x / 2; y -= vSize.y; break;
	case ALIGN_BOTTOMRIGHT: x -= vSize.x; y -= vSize.y; break;
	}

	auto tColorOutline = tColorOut;
	std::vector<std::pair<int, int>> vOutline = { { 1, 1 } };
	if (!Vars::Menu::CheapText.Value)
	{
		vOutline = { { -1, 0 }, { 0, -1 }, { 1, 0 }, { 0, 1 }, { -1, -1 }, { 1, 1 }, { -1, 1 }, { 1, -1 } };
		tColorOutline.a /= 2;
	}
	if (tColorOutline.a)
	{
		for (auto& [x2, y2] : vOutline)
		{
			I::MatSystemSurface->DrawSetTextPos(x + x2, y + y2);
			I::MatSystemSurface->DrawSetTextFont(dwFont);
			I::MatSystemSurface->DrawSetTextColor(tColorOutline.r, tColorOutline.g, tColorOutline.b, tColorOutline.a);
			I::MatSystemSurface->DrawPrintText(wstr, int(wcslen(wstr)));
		}
	}

	I::MatSystemSurface->DrawSetTextPos(x, y);
	I::MatSystemSurface->DrawSetTextFont(dwFont);
	I::MatSystemSurface->DrawSetTextColor(tColor.r, tColor.g, tColor.b, tColor.a);
	I::MatSystemSurface->DrawPrintText(wstr, int(wcslen(wstr)));
}
void CDraw::StringOutlined(const Font_t& tFont, int x, int y, const Color_t& tColor, const Color_t& tColorOut, const EAlign& eAlign, const wchar_t* wstr)
{
	const auto dwFont = tFont.m_dwFont;

	Vec2 vSize = GetTextSize(wstr, tFont);
	switch (eAlign)
	{
	case ALIGN_TOPLEFT: break;
	case ALIGN_TOP: x -= vSize.x / 2; break;
	case ALIGN_TOPRIGHT: x -= vSize.x; break;
	case ALIGN_LEFT: y -= vSize.y / 2; break;
	case ALIGN_CENTER: x -= vSize.x / 2; y -= vSize.y / 2; break;
	case ALIGN_RIGHT: x -= vSize.x; y -= vSize.y / 2; break;
	case ALIGN_BOTTOMLEFT: y -= vSize.y; break;
	case ALIGN_BOTTOM: x -= vSize.x / 2; y -= vSize.y; break;
	case ALIGN_BOTTOMRIGHT: x -= vSize.x; y -= vSize.y; break;
	}

	auto tColorOutline = tColorOut;
	std::vector<std::pair<int, int>> vOutline = { { 1, 1 } };
	if (!Vars::Menu::CheapText.Value)
	{
		vOutline = { { -1, 0 }, { 0, -1 }, { 1, 0 }, { 0, 1 }, { -1, -1 }, { 1, 1 }, { -1, 1 }, { 1, -1 } };
		tColorOutline.a /= 2;
	}
	if (tColorOutline.a)
	{
		for (auto& [x2, y2] : vOutline)
		{
			I::MatSystemSurface->DrawSetTextPos(x + x2, y + y2);
			I::MatSystemSurface->DrawSetTextFont(dwFont);
			I::MatSystemSurface->DrawSetTextColor(tColorOutline.r, tColorOutline.g, tColorOutline.b, tColorOutline.a);
			I::MatSystemSurface->DrawPrintText(wstr, int(wcslen(wstr)));
		}
	}

	I::MatSystemSurface->DrawSetTextPos(x, y);
	I::MatSystemSurface->DrawSetTextFont(dwFont);
	I::MatSystemSurface->DrawSetTextColor(tColor.r, tColor.g, tColor.b, tColor.a);
	I::MatSystemSurface->DrawPrintText(wstr, int(wcslen(wstr)));
}

void CDraw::Line(int x1, int y1, int x2, int y2, const Color_t& tColor)
{
	I::MatSystemSurface->DrawSetColor(tColor.r, tColor.g, tColor.b, tColor.a);
	I::MatSystemSurface->DrawLine(x1, y1, x2, y2);
}

void CDraw::FillPolygon(std::vector<Vertex_t> vVertices, const Color_t& tColor)
{
	static int iId = 0;
	if (!I::MatSystemSurface->IsTextureIDValid(iId))
		iId = I::MatSystemSurface->CreateNewTextureID();

	I::MatSystemSurface->DrawSetColor(tColor.r, tColor.g, tColor.b, tColor.a);
	I::MatSystemSurface->DrawSetTexture(iId);
	I::MatSystemSurface->DrawTexturedPolygon(int(vVertices.size()), vVertices.data());
}
void CDraw::LinePolygon(std::vector<Vertex_t> vVertices, const Color_t& tColor)
{
	static int id = 0;
	if (!I::MatSystemSurface->IsTextureIDValid(id))
		id = I::MatSystemSurface->CreateNewTextureID();

	I::MatSystemSurface->DrawSetColor(tColor.r, tColor.g, tColor.b, tColor.a);
	I::MatSystemSurface->DrawSetTexture(id);
	I::MatSystemSurface->DrawTexturedPolyLine(vVertices.data(), int(vVertices.size()));
}

void CDraw::FillRect(int x, int y, int w, int h, const Color_t& tColor)
{
	I::MatSystemSurface->DrawSetColor(tColor.r, tColor.g, tColor.b, tColor.a);
	I::MatSystemSurface->DrawFilledRect(x, y, x + w, y + h);
}
void CDraw::LineRect(int x, int y, int w, int h, const Color_t& tColor)
{
	I::MatSystemSurface->DrawSetColor(tColor.r, tColor.g, tColor.b, tColor.a);
	I::MatSystemSurface->DrawOutlinedRect(x, y, x + w, y + h);
}
void CDraw::GradientRect(int x, int y, int w, int h, const Color_t& tColorTop, const Color_t& tColorBottom, bool bHorizontal)
{
	I::MatSystemSurface->DrawSetColor(tColorTop.r, tColorTop.g, tColorTop.b, tColorTop.a);
	I::MatSystemSurface->DrawFilledRectFade(x, y, x + w, y + h, tColorTop.a, tColorBottom.a, bHorizontal);
	I::MatSystemSurface->DrawSetColor(tColorBottom.r, tColorBottom.g, tColorBottom.b, tColorBottom.a);
	I::MatSystemSurface->DrawFilledRectFade(x, y, x + w, y + h, tColorTop.a, tColorBottom.a, bHorizontal);
}
void CDraw::FillRectOutline(int x, int y, int w, int h, const Color_t& tColor, const Color_t& tColorOut)
{
	FillRect(x, y, w, h, tColor);
	LineRect(x - 1, y - 1, w + 2, h + 2, tColorOut);
}
void CDraw::LineRectOutline(int x, int y, int w, int h, const Color_t& tColor, const Color_t& tColorOut, bool bInside)
{
	LineRect(x, y, w, h, tColor);
	LineRect(x - 1, y - 1, w + 2, h + 2, tColorOut);
	if (bInside)
		LineRect(x + 1, y + 1, w - 2, h - 2, tColorOut);
}
void CDraw::FillRectPercent(int x, int y, int w, int h, float t, const Color_t& tColor, const Color_t& tColorOut, const EAlign& eAlign, bool bAdjust)
{
	if (!bAdjust)
		FillRect(x - 1, y - 1, w + 2, h + 2, tColorOut);
	int nw = w, nh = h;
	switch (eAlign)
	{
	case ALIGN_LEFT: nw *= t; break;
	case ALIGN_RIGHT: nw *= t; x += w - nw; break;
	case ALIGN_TOP: nh *= t; break;
	case ALIGN_BOTTOM: nh *= t; y += h - nh; break;
	}
	if (bAdjust)
		FillRect(x - 1, y - 1, nw + 2, nh + 2, tColorOut);
	FillRect(x, y, nw, nh, tColor);
}
void CDraw::FillRoundRect(int x, int y, int w, int h, int iRadius, const Color_t& tColor, int iCount)
{
	std::vector<Vertex_t> vVertices = {};

	int _iCount = std::max(iCount / 4, 2);
	float flDelta = 90.f / (_iCount - 1);
	for (int i = 0; i < 4; i++)
	{
		const int _x = x + ((i < 2) ? (w - iRadius) : iRadius);
		const int _y = y + ((i % 3) ? (h - iRadius) : iRadius);

		const float a = 90.f * i;
		for (int j = 0; j < _iCount; j++)
		{
			const float _a = DEG2RAD(a + j * flDelta);
			vVertices.emplace_back(Vertex_t({ { _x + iRadius * sinf(_a), _y - iRadius * cosf(_a) } }));
		}
	}

	FillPolygon(vVertices, tColor);
}
void CDraw::LineRoundRect(int x, int y, int w, int h, int iRadius, const Color_t& tColor, int iCount)
{
	std::vector<Vertex_t> vVertices = {};

	w -= 1, h -= 1;
	int _iCount = std::max(iCount / 4, 2);
	float flDelta = 90.f / (_iCount - 1);
	for (int i = 0; i < 4; i++)
	{
		const int _x = x + ((i < 2) ? (w - iRadius) : iRadius);
		const int _y = y + ((i % 3) ? (h - iRadius) : iRadius);

		const float a = 90.f * i;
		for (int j = 0; j < _iCount; j++)
		{
			const float _a = DEG2RAD(a + j * flDelta);
			vVertices.emplace_back(Vertex_t({ { _x + iRadius * sinf(_a), _y - iRadius * cosf(_a) } }));
		}
	}

	LinePolygon(vVertices, tColor);
}

void CDraw::FillCircle(int x, int y, float iRadius, int iSegments, const Color_t clr)
{
	std::vector<Vertex_t> vVertices = {};

	const float step = static_cast<float>(PI) * 2.0f / iSegments;
	for (float a = 0; a < PI * 2.0f; a += step)
		vVertices.emplace_back(Vector2D{ iRadius * cosf(a) + x, iRadius * sinf(a) + y });

	FillPolygon(vVertices, clr);
}
void CDraw::LineCircle(int x, int y, float iRadius, int iSegments, const Color_t& clr)
{
	I::MatSystemSurface->DrawSetColor(clr.r, clr.g, clr.b, clr.a);
	I::MatSystemSurface->DrawOutlinedCircle(x, y, iRadius, iSegments);
}

void CDraw::Texture(int x, int y, int w, int h, int iId, const EAlign& eAlign)
{
	switch (eAlign)
	{
	case ALIGN_TOPLEFT: break;
	case ALIGN_TOP: x -= w / 2; break;
	case ALIGN_TOPRIGHT: x -= w; break;
	case ALIGN_LEFT: y -= h / 2; break;
	case ALIGN_CENTER: x -= w / 2; y -= h / 2; break;
	case ALIGN_RIGHT: x -= w; y -= h / 2; break;
	case ALIGN_BOTTOMLEFT: y -= h; break;
	case ALIGN_BOTTOM: x -= w / 2; y -= h; break;
	case ALIGN_BOTTOMRIGHT: x -= w; y -= h; break;
	}

	int nTexture = 0;
	if (ICONS::TEXTURES[iId].first != -1)
		nTexture = ICONS::TEXTURES[iId].first;
	else
	{
		nTexture = ICONS::TEXTURES[iId].first = I::MatSystemSurface->CreateNewTextureID();
		I::MatSystemSurface->DrawSetTextureFile(nTexture, ICONS::TEXTURES[iId].second.c_str(), false, true);
	}

	I::MatSystemSurface->DrawSetColor(255, 255, 255, 255);
	I::MatSystemSurface->DrawSetTexture(nTexture);
	I::MatSystemSurface->DrawTexturedRect(x, y, x + w, y + h);
}
CHudTexture* CDraw::GetIcon(const char* szIcon, int eIconFormat)
{
	return S::CHudBaseDeathNotice_GetIcon.Call<CHudTexture*>(nullptr, szIcon, eIconFormat);
}
int CDraw::CreateTextureFromArray(const unsigned char* rgba, int w, int h)
{
	const int nTextureIdOut = I::MatSystemSurface->CreateNewTextureID(true);
	I::MatSystemSurface->DrawSetTextureRGBAEx(nTextureIdOut, rgba, w, h, IMAGE_FORMAT_RGBA8888);
	return nTextureIdOut;
}
void CDraw::DrawHudTexture(float x, float y, float s, const CHudTexture* pTexture, Color_t clr)
{
	if (!pTexture)
		return;

	if (pTexture->bRenderUsingFont)
	{
		I::MatSystemSurface->DrawSetTextFont(pTexture->hFont);
		I::MatSystemSurface->DrawSetTextColor(clr.r, clr.g, clr.b, clr.a);
		I::MatSystemSurface->DrawSetTextPos(x, y);
		I::MatSystemSurface->DrawUnicodeChar(pTexture->cCharacterInFont);
	}
	else if (pTexture->textureId != -1)
	{
		I::MatSystemSurface->DrawSetTexture(pTexture->textureId);
		I::MatSystemSurface->DrawSetColor(clr.r, clr.g, clr.b, clr.a);
		I::MatSystemSurface->DrawTexturedSubRect(x, y, x + pTexture->Width() * s, y + pTexture->Height() * s, pTexture->texCoords[0], pTexture->texCoords[1], pTexture->texCoords[2], pTexture->texCoords[3]);
	}
}
void CDraw::DrawHudTextureByName(float x, float y, float s, const char* sTexture, Color_t clr)
{
	const CHudTexture* pIcon = GetIcon(sTexture, 0);

	if (!pIcon)
		return;

	if (pIcon->bRenderUsingFont)
	{
		I::MatSystemSurface->DrawSetTextFont(pIcon->hFont);
		I::MatSystemSurface->DrawSetTextColor(clr.r, clr.g, clr.b, clr.a);
		I::MatSystemSurface->DrawSetTextPos(x, y);
		I::MatSystemSurface->DrawUnicodeChar(pIcon->cCharacterInFont);
	}
	else if (pIcon->textureId != -1)
	{
		I::MatSystemSurface->DrawSetTexture(pIcon->textureId);
		I::MatSystemSurface->DrawSetColor(clr.r, clr.g, clr.b, clr.a);
		I::MatSystemSurface->DrawTexturedSubRect(x, y, x + pIcon->Width() * s, y + pIcon->Height() * s, pIcon->texCoords[0], pIcon->texCoords[1], pIcon->texCoords[2], pIcon->texCoords[3]);
	}
}

void CDraw::Avatar(int x, int y, int w, int h, const uint32 nFriendID, const EAlign& eAlign)
{
	// Basic validation
	if (!nFriendID || !IsSteamInterfacesValid())
		return;

	// Periodic cleanup to prevent memory issues
	PeriodicAvatarCleanup();

	// Alignment calculation
	switch (eAlign)
	{
	case ALIGN_TOPLEFT: break;
	case ALIGN_TOP: x -= w / 2; break;
	case ALIGN_TOPRIGHT: x -= w; break;
	case ALIGN_LEFT: y -= h / 2; break;
	case ALIGN_CENTER: x -= w / 2; y -= h / 2; break;
	case ALIGN_RIGHT: x -= w; y -= h / 2; break;
	case ALIGN_BOTTOMLEFT: y -= h; break;
	case ALIGN_BOTTOM: x -= w / 2; y -= h; break;
	case ALIGN_BOTTOMRIGHT: x -= w; y -= h; break;
	}

	// Thread-safe avatar cache access
	std::lock_guard<std::mutex> lock(m_mxAvatarMutex);

	// Check if avatar is already cached
	if (m_mAvatars.contains(nFriendID))
	{
		const int textureID = m_mAvatars[nFriendID];
		if (I::MatSystemSurface && I::MatSystemSurface->IsTextureIDValid(textureID))
		{
			I::MatSystemSurface->DrawSetColor(255, 255, 255, 255);
			I::MatSystemSurface->DrawSetTexture(textureID);
			I::MatSystemSurface->DrawTexturedRect(x, y, x + w, y + h);
		}
		else
		{
			// Texture became invalid, remove from cache
			m_mAvatars.erase(nFriendID);
		}
		return;
	}

	// Check if this avatar failed before and shouldn't be retried yet
	if (!ShouldRetryFailedAvatar(nFriendID))
		return;

	// Attempt to load avatar safely
	int newTextureID = -1;
	if (LoadAvatarSafely(nFriendID, newTextureID))
	{
		// Successfully loaded, cache and draw
		m_mAvatars[nFriendID] = newTextureID;

		if (I::MatSystemSurface)
		{
			I::MatSystemSurface->DrawSetColor(255, 255, 255, 255);
			I::MatSystemSurface->DrawSetTexture(newTextureID);
			I::MatSystemSurface->DrawTexturedRect(x, y, x + w, y + h);
		}
	}
	else
	{
		// Failed to load, mark as failed to avoid immediate retries
		MarkAvatarAsFailed(nFriendID);
	}
}
void CDraw::ClearAvatarCache()
{
	std::lock_guard<std::mutex> lock(m_mxAvatarMutex);

	// Safely delete all cached textures
	if (I::MatSystemSurface)
	{
		for (int iID : m_mAvatars | std::views::values)
		{
			if (I::MatSystemSurface->IsTextureIDValid(iID))
			{
				I::MatSystemSurface->DeleteTextureByID(iID);
				I::MatSystemSurface->DestroyTextureID(iID);
			}
		}
	}

	m_mAvatars.clear();
	m_sFailedAvatars.clear();
	m_lastCleanupTime = std::chrono::steady_clock::now();
}

void CDraw::RenderLine(const Vec3& vStart, const Vec3& vEnd, Color_t tColor, bool bZBuffer)
{
	if (!tColor.a)
		return;

	S::RenderLine.Call<void>(std::ref(vStart), std::ref(vEnd), tColor, bZBuffer);
}

void CDraw::RenderPath(const std::vector<Vec3>& vPath, Color_t tColor, bool bZBuffer, int iStyle, float flTime, int iSeparatorSpacing, float flSeparatorLength)
{
	if (!tColor.a || iStyle == Vars::Visuals::Simulation::StyleEnum::Off)
		return;

	for (size_t i = 1; i < vPath.size(); i++)
	{
		if (flTime < 0.f && vPath.size() - i > -flTime)
			continue;

		switch (iStyle)
		{
		case Vars::Visuals::Simulation::StyleEnum::Line:
		{
			RenderLine(vPath[i - 1], vPath[i], tColor, bZBuffer);
			break;
		}
		case Vars::Visuals::Simulation::StyleEnum::Separators:
		{
			RenderLine(vPath[i - 1], vPath[i], tColor, bZBuffer);
			if (!(i % iSeparatorSpacing))
			{
				const Vec3& vStart = vPath[i - 1];
				const Vec3& vEnd = vPath[i];

				Vec3 vDir = (vEnd - vStart).Normalized2D();
				vDir = Math::RotatePoint(vDir * flSeparatorLength, {}, { 0, 90, 0 });
				RenderLine(vEnd, vEnd + vDir, tColor, bZBuffer);
			}
			break;
		}
		case Vars::Visuals::Simulation::StyleEnum::Spaced:
		{
			if (!(i % 2))
				RenderLine(vPath[i - 1], vPath[i], tColor, bZBuffer);
			break;
		}
		case Vars::Visuals::Simulation::StyleEnum::Arrows:
		{
			if (!(i % 3))
			{
				const Vec3& vStart = vPath[i - 1];
				const Vec3& vEnd = vPath[i];

				if (!(vStart - vEnd).IsZero())
				{
					Vec3 vAngles = Math::VectorAngles(vEnd - vStart);
					Vec3 vForward, vRight, vUp; Math::AngleVectors(vAngles, &vForward, &vRight, &vUp);
					RenderLine(vEnd, vEnd - vForward * 5 + vRight * 5, tColor, bZBuffer);
					RenderLine(vEnd, vEnd - vForward * 5 - vRight * 5, tColor, bZBuffer);
				}
			}
			break;
		}
		case Vars::Visuals::Simulation::StyleEnum::Boxes:
		{
			RenderLine(vPath[i - 1], vPath[i], tColor, bZBuffer);
			if (!(i % iSeparatorSpacing))
				RenderWireframeBox(vPath[i], { -1, -1, -1 }, { 1, 1, 1 }, {}, tColor, bZBuffer);
			break;
		}
		}
	}
}

void CDraw::RenderBox(const Vec3& vOrigin, const Vec3& vMins, const Vec3& vMaxs, const Vec3& vAngles, Color_t tColor, bool bZBuffer, bool bInsideOut)
{
	if (!tColor.a)
		return;

	S::RenderBox.Call<void>(std::ref(vOrigin), std::ref(vAngles), std::ref(vMins), std::ref(vMaxs), tColor, bZBuffer, bInsideOut);
}

void CDraw::RenderWireframeBox(const Vec3& vOrigin, const Vec3& vMins, const Vec3& vMaxs, const Vec3& vAngles, Color_t tColor, bool bZBuffer)
{
	if (!tColor.a)
		return;

	S::RenderWireframeBox.Call<void>(std::ref(vOrigin), std::ref(vAngles), std::ref(vMins), std::ref(vMaxs), tColor, bZBuffer);
}

void CDraw::RenderWireframeSweptBox(const Vector& vStart, const Vector& vEnd, const Vec3& vMins, const Vec3& vMaxs, const Vec3& vAngles, Color_t tColor, bool bZBuffer)
{
	if (!tColor.a)
		return;

	S::RenderWireframeSweptBox.Call<void>(std::ref(vStart), std::ref(vEnd), std::ref(vAngles), std::ref(vMins), std::ref(vMaxs), tColor, bZBuffer);
}

void CDraw::RenderTriangle(const Vector& vPoint1, const Vector& vPoint2, const Vector& vPoint3, Color_t tColor, bool bZBuffer)
{
	if (!tColor.a)
		return;

	S::RenderTriangle.Call<void>(std::ref(vPoint1), std::ref(vPoint2), std::ref(vPoint3), tColor, bZBuffer);
}

void CDraw::RenderSphere(const Vector& vCenter, float flRadius, int nTheta, int nPhi, Color_t tColor, IMaterial* pMaterial)
{
	if (!tColor.a)
		return;

	S::RenderSphere.Call<void>(std::ref(vCenter), flRadius, nTheta, nPhi, tColor, pMaterial);
}

void CDraw::RenderSphere(const Vector& vCenter, float flRadius, int nTheta, int nPhi, Color_t tColor, bool bZBuffer)
{
	if (!tColor.a)
		return;

	static auto pVertexColor = *reinterpret_cast<IMaterial**>(U::Memory.RelToAbs(S::VertexColor()));
	static auto pVertexColorIgnoreZ = *reinterpret_cast<IMaterial**>(U::Memory.RelToAbs(S::VertexColorIgnoreZ()));
	auto pMaterial = bZBuffer ? pVertexColor : pVertexColorIgnoreZ;

	RenderSphere(vCenter, flRadius, nTheta, nPhi, tColor, pMaterial);
}

void CDraw::RenderWireframeSphere(const Vector& vCenter, float flRadius, int nTheta, int nPhi, Color_t tColor, bool bZBuffer)
{
	if (!tColor.a)
		return;

	static auto pWireframe = *reinterpret_cast<IMaterial**>(U::Memory.RelToAbs(S::Wireframe()));
	static auto pWireframeIgnoreZ = *reinterpret_cast<IMaterial**>(U::Memory.RelToAbs(S::WireframeIgnoreZ()));
	auto pMaterial = bZBuffer ? pWireframe : pWireframeIgnoreZ;

	RenderSphere(vCenter, flRadius, nTheta, nPhi, tColor, pMaterial);
}

// Avatar Safety Helper Methods Implementation

bool CDraw::IsSteamInterfacesValid() const
{
	return I::SteamFriends && I::SteamUtils && I::MatSystemSurface;
}

bool CDraw::IsValidAvatarHandle(int nAvatar) const
{
	// Steam returns 0 for invalid avatar handles
	return nAvatar > 0;
}

bool CDraw::ShouldRetryFailedAvatar(uint32 nFriendID) const
{
	if (!m_sFailedAvatars.contains(nFriendID))
		return true; // Never failed before, allow attempt

	// Only retry failed avatars after the retry interval
	auto now = std::chrono::steady_clock::now();
	return (now - m_lastCleanupTime) >= FAILED_RETRY_INTERVAL;
}

void CDraw::MarkAvatarAsFailed(uint32 nFriendID)
{
	m_sFailedAvatars.insert(nFriendID);

	// Prevent failed cache from growing too large
	if (m_sFailedAvatars.size() > MAX_FAILED_CACHE_SIZE)
	{
		// Remove oldest entries (simple approach: clear half the cache)
		auto it = m_sFailedAvatars.begin();
		std::advance(it, m_sFailedAvatars.size() / 2);
		m_sFailedAvatars.erase(m_sFailedAvatars.begin(), it);
	}
}

void CDraw::CleanupOldestAvatars(size_t maxSize)
{
	if (m_mAvatars.size() <= maxSize)
		return;

	// Simple approach: remove first half of cache
	// In a real implementation, you'd want LRU (Least Recently Used)
	auto it = m_mAvatars.begin();
	size_t toRemove = m_mAvatars.size() - maxSize;

	for (size_t i = 0; i < toRemove && it != m_mAvatars.end(); ++i)
	{
		// Safely delete texture
		if (I::MatSystemSurface && I::MatSystemSurface->IsTextureIDValid(it->second))
		{
			I::MatSystemSurface->DeleteTextureByID(it->second);
			I::MatSystemSurface->DestroyTextureID(it->second);
		}
		it = m_mAvatars.erase(it);
	}
}

bool CDraw::LoadAvatarSafely(uint32 nFriendID, int& outTextureID)
{
	outTextureID = -1;

	// Double-check Steam interfaces are valid
	if (!IsSteamInterfacesValid())
		return false;

	try
	{
		// Get avatar handle from Steam
		const int nAvatar = I::SteamFriends->GetMediumFriendAvatar(CSteamID(nFriendID, k_EUniversePublic, k_EAccountTypeIndividual));

		// Validate avatar handle
		if (!IsValidAvatarHandle(nAvatar))
			return false;

		// Get image dimensions
		uint32 newW = 0, newH = 0;
		if (!I::SteamUtils->GetImageSize(nAvatar, &newW, &newH))
			return false;

		// Validate dimensions to prevent excessive memory allocation
		if (newW == 0 || newH == 0 || newW > 512 || newH > 512)
			return false;

		// Calculate memory size needed
		const uint32 nSize = newW * newH * 4; // RGBA
		if (nSize == 0 || nSize > (512 * 512 * 4)) // Safety check
			return false;

		// Allocate memory with proper error handling
		auto* pData = static_cast<uint8*>(std::malloc(nSize));
		if (!pData)
			return false;

		// Initialize memory to prevent undefined behavior
		std::memset(pData, 0, nSize);

		bool success = false;

		// Get avatar image data
		if (I::SteamUtils->GetImageRGBA(nAvatar, pData, nSize))
		{
			// Create texture with validation
			const int nTextureID = I::MatSystemSurface->CreateNewTextureID(true);
			if (I::MatSystemSurface->IsTextureIDValid(nTextureID))
			{
				// Set texture data
				I::MatSystemSurface->DrawSetTextureRGBA(nTextureID, pData, newW, newH, 0, false);
				outTextureID = nTextureID;
				success = true;
			}
		}

		// Always free allocated memory
		std::free(pData);
		return success;
	}
	catch (...)
	{
		// Catch any unexpected exceptions from Steam API calls
		return false;
	}
}

void CDraw::PeriodicAvatarCleanup()
{
	auto now = std::chrono::steady_clock::now();

	// Only run cleanup periodically to avoid performance impact
	if ((now - m_lastCleanupTime) < CLEANUP_INTERVAL)
		return;

	// Cleanup oversized caches
	if (m_mAvatars.size() > MAX_AVATAR_CACHE_SIZE)
	{
		CleanupOldestAvatars(MAX_AVATAR_CACHE_SIZE);
	}

	// Clear failed avatars cache periodically to allow retries
	if ((now - m_lastCleanupTime) >= FAILED_RETRY_INTERVAL)
	{
		m_sFailedAvatars.clear();
	}

	m_lastCleanupTime = now;
}

void CDraw::OnMapChange()
{
	// Clear all caches when map changes
	ClearAvatarCache();
}

void CDraw::OnDisconnect()
{
	// Clear all caches when disconnecting
	ClearAvatarCache();
}