#pragma once
#include "../Fonts/Fonts.h"
#include "../../Definitions/Definitions.h"
#include "../../Vars.h"

enum EAlign
{
	ALIGN_TOPLEFT,
	ALIGN_TOP,
	ALIGN_TOPRIGHT,
	ALIGN_LEFT,
	ALIGN_CENTER,
	ALIGN_RIGHT,
	ALIGN_BOTTOMLEFT,
	ALIGN_BOTTOM,
	ALIGN_BOTTOMRIGHT
};

enum Scale_
{
	Scale_None = 0,
	Scale_Round = 1,
	Scale_Floor = 2,
	Scale_Ceil = 3
};

class CDraw
{
	std::unordered_map<uint64, int> m_mAvatars = {};

public:
	inline bool IsColorBright(const Color_t& clr)
	{
		return clr.r + clr.g + clr.b > 510;
	}
	inline bool IsColorDark(const Color_t& clr)
	{
		return clr.r + clr.g + clr.b < 201;
	}

	inline float Scale(float flN = 1.f, int iFlags = Scale_None, float flScale = Vars::Menu::Scale.Value)
	{
		flN *= flScale;
		switch (iFlags)
		{
		case Scale_Round: flN = roundf(flN); break;
		case Scale_Floor: flN = floorf(flN); break;
		case Scale_Ceil: flN = ceilf(flN); break;
		}
		return flN;
	}

	void UpdateScreenSize();
	void UpdateW2SMatrix();

	void String(const Font_t& tFont, int x, int y, const Color_t& tColor, const EAlign& eAlign, const char* str, ...);
	void String(const Font_t& tFont, int x, int y, const Color_t& tColor, const EAlign& eAlign, const wchar_t* str, ...);
	void StringOutlined(const Font_t& tFont, int x, int y, const Color_t& tColor, const Color_t& tColorOut, const EAlign& eAlign, const char* str, ...);
	void StringOutlined(const Font_t& tFont, int x, int y, const Color_t& tColor, const Color_t& tColorOut, const EAlign& eAlign, const wchar_t* str, ...);

	void Line(int x1, int y1, int x2, int y2, const Color_t& tColor);
	void FillPolygon(std::vector<Vertex_t> vVertices, const Color_t& tColor);
	void LinePolygon(std::vector<Vertex_t> vVertices, const Color_t& tColor);

	void FillRect(int x, int y, int w, int h, const Color_t& tColor);
	void LineRect(int x, int y, int w, int h, const Color_t& tColor);
	void GradientRect(int x, int y, int w, int h, const Color_t& tColorTop, const Color_t& tColorBottom, bool bHorizontal);
	void FillRectOutline(int x, int y, int w, int h, const Color_t& tColor, const Color_t& tColorOut = { 0, 0, 0, 255 });
	void LineRectOutline(int x, int y, int w, int h, const Color_t& tColor, const Color_t& tColorOut = { 0, 0, 0, 255 }, bool bInside = true);
	void FillRectPercent(int x, int y, int w, int h, float t, const Color_t& tColor, const Color_t& tColorOut = { 0, 0, 0, 255 }, const EAlign& eAlign = ALIGN_LEFT, bool bAdjust = false);
	void FillRoundRect(int x, int y, int w, int h, int iRadius, const Color_t& tColor, int iCount = 64);
	void LineRoundRect(int x, int y, int w, int h, int iRadius, const Color_t& tColor, int iCount = 64);

	void FillCircle(int x, int y, float iRadius, int iSegments, Color_t clr);
	void LineCircle(int x, int y, float iRadius, int iSegments, const Color_t& clr);

	void Texture(int x, int y, int w, int h, int iId, const EAlign& eAlign = ALIGN_CENTER);
	CHudTexture* GetIcon(const char* szIcon, int eIconFormat = 0);
	int CreateTextureFromArray(const unsigned char* rgba, int w, int h);
	void DrawHudTexture(float x, float y, float s, const CHudTexture* pTexture, Color_t tColor = { 255, 255, 255, 255 });
	void DrawHudTextureByName(float x, float y, float s, const char* sTexture, Color_t tColor = { 255, 255, 255, 255 });
	void Avatar(int x, int y, int w, int h, const uint32 nFriendID, const EAlign& eAlign = ALIGN_CENTER);
	void ClearAvatarCache();

	void StartClipping(int x, int y, int w, int h);
	void EndClipping();

	int m_nScreenW = 0, m_nScreenH = 0;
	VMatrix m_WorldToProjection = {};
};

ADD_FEATURE_CUSTOM(CDraw, Draw, H);