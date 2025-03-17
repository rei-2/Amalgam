#include "Draw.h"

#include "Icons.h"
#include "../../Definitions/Interfaces.h"
#include "../../../Utils/Math/Math.h"
#include <array>
#include <ranges>

MAKE_SIGNATURE(CHudBaseDeathNotice_GetIcon, "client.dll", "40 53 48 81 EC ? ? ? ? 48 8B DA", 0x0);

void CDraw::UpdateScreenSize()
{
	m_nScreenW = I::BaseClientDLL->GetScreenWidth();
	m_nScreenH = I::BaseClientDLL->GetScreenHeight();
}

void CDraw::UpdateW2SMatrix()
{
	CViewSetup ViewSetup = {};
	if (I::BaseClientDLL->GetPlayerView(ViewSetup))
	{
		static VMatrix WorldToView = {};
		static VMatrix ViewToProjection = {};
		static VMatrix WorldToPixels = {};

		I::RenderView->GetMatricesForView(ViewSetup, &WorldToView, &ViewToProjection, &m_WorldToProjection, &WorldToPixels);
	}
}

void CDraw::String(const Font_t& tFont, int x, int y, const Color_t& tColor, const EAlign& eAlign, const char* str, ...)
{
	if (str == nullptr)
		return;

	va_list va_alist;
	char cbuffer[1024] = { '\0' };
	wchar_t wstr[1024] = { '\0' };

	va_start(va_alist, str);
	vsprintf_s(cbuffer, str, va_alist);
	va_end(va_alist);

	wsprintfW(wstr, L"%hs", cbuffer);

	const auto dwFont = tFont.m_dwFont;

	int w = 0, h = 0; I::MatSystemSurface->GetTextSize(dwFont, wstr, w, h);
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

	I::MatSystemSurface->DrawSetTextPos(x, y);
	I::MatSystemSurface->DrawSetTextFont(dwFont);
	I::MatSystemSurface->DrawSetTextColor(tColor.r, tColor.g, tColor.b, tColor.a);
	I::MatSystemSurface->DrawPrintText(wstr, int(wcslen(wstr)));
}
void CDraw::String(const Font_t& tFont, int x, int y, const Color_t& tColor, const EAlign& eAlign, const wchar_t* str, ...)
{
	if (str == nullptr)
		return;

	va_list va_alist;
	wchar_t wstr[1024] = { '\0' };

	va_start(va_alist, str);
	vswprintf_s(wstr, str, va_alist);
	va_end(va_alist);

	const auto dwFont = tFont.m_dwFont;

	int w = 0, h = 0; I::MatSystemSurface->GetTextSize(dwFont, wstr, w, h);
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

	I::MatSystemSurface->DrawSetTextPos(x, y);
	I::MatSystemSurface->DrawSetTextFont(dwFont);
	I::MatSystemSurface->DrawSetTextColor(tColor.r, tColor.g, tColor.b, tColor.a);
	I::MatSystemSurface->DrawPrintText(wstr, int(wcslen(wstr)));
}

void CDraw::StringOutlined(const Font_t& tFont, int x, int y, const Color_t& tColor, const Color_t& tColorOut, const EAlign& eAlign, const char* str, ...)
{
	if (str == nullptr)
		return;

	va_list va_alist;
	char cbuffer[1024] = { '\0' };
	wchar_t wstr[1024] = { '\0' };

	va_start(va_alist, str);
	vsprintf_s(cbuffer, str, va_alist);
	va_end(va_alist);

	wsprintfW(wstr, L"%hs", cbuffer);

	const auto dwFont = tFont.m_dwFont;

	int w = 0, h = 0; I::MatSystemSurface->GetTextSize(dwFont, wstr, w, h);
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
void CDraw::StringOutlined(const Font_t& tFont, int x, int y, const Color_t& tColor, const Color_t& tColorOut, const EAlign& eAlign, const wchar_t* str, ...)
{
	if (str == nullptr)
		return;

	va_list va_alist;
	wchar_t wstr[1024] = { '\0' };

	va_start(va_alist, str);
	vswprintf_s(wstr, str, va_alist);
	va_end(va_alist);

	const auto dwFont = tFont.m_dwFont;

	int w = 0, h = 0; I::MatSystemSurface->GetTextSize(dwFont, wstr, w, h);
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
	if (const auto nID = static_cast<uint64>(nFriendID + 0x0110000100000000))
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

		if (m_mAvatars.contains(nID))
		{	// The avatar has been cached
			I::MatSystemSurface->DrawSetColor(255, 255, 255, 255);
			I::MatSystemSurface->DrawSetTexture(m_mAvatars[nID]);
			I::MatSystemSurface->DrawTexturedRect(x, y, x + w, y + h);
		}
		else
		{	// Retrieve the avatar
			const int nAvatar = I::SteamFriends->GetMediumFriendAvatar(CSteamID(nID));

			uint32 newW = 0, newH = 0;
			if (I::SteamUtils->GetImageSize(nAvatar, &newW, &newH))
			{
				const uint32 nSize = newW * newH * uint32(sizeof(uint8) * 4);
				auto* pData = static_cast<uint8*>(std::malloc(nSize));
				if (!pData)
					return;

				if (I::SteamUtils->GetImageRGBA(nAvatar, pData, nSize))
				{
					const int nTextureID = I::MatSystemSurface->CreateNewTextureID(true);
					if (I::MatSystemSurface->IsTextureIDValid(nTextureID))
					{
						I::MatSystemSurface->DrawSetTextureRGBA(nTextureID, pData, newW, newH, 0, false);
						m_mAvatars[nID] = nTextureID;
					}
				}

				std::free(pData);
			}
		}
	}
}
void CDraw::ClearAvatarCache()
{
	for (int iID : m_mAvatars | std::views::values)
	{
		I::MatSystemSurface->DeleteTextureByID(iID);
		I::MatSystemSurface->DestroyTextureID(iID);
	}

	m_mAvatars.clear();
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