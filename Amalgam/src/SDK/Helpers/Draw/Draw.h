#pragma once
#include "../Fonts/Fonts.h"
#include "../../Definitions/Definitions.h"

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

class CDraw
{
	std::unordered_map<uint64, int> m_Avatars = {};

public:
	void UpdateScreenSize();
	void UpdateW2SMatrix();

	void String(const Font_t& font, int x, int y, const Color_t& clr, const EAlign& align, const char* str, ...);
	void String(const Font_t& font, int x, int y, const Color_t& clr, const EAlign& align, const wchar_t* str, ...);

	void Line(int x, int y, int x1, int y1, const Color_t& clr);
	void Polygon(int count, Vertex_t* vertices, const Color_t& clr);

	void DrawFillTriangle(const std::array<Vec2, 3>& points, const Color_t& clr);
	void DrawLineTriangle(const std::array<Vec2, 3>& points, const Color_t& clr);

	void FillRect(int x, int y, int w, int h, const Color_t& clr);
	void LineRect(int x, int y, int w, int h, const Color_t& clr);
	void GradientRect(int x, int y, int w, int h, const Color_t& top_clr, const Color_t& bottom_clr, bool horizontal);
	void FillRectOutline(int x, int y, int w, int h, const Color_t& clr, const Color_t& out = { 0, 0, 0, 255 });
	void LineRectOutline(int x, int y, int w, int h, const Color_t& clr, const Color_t& out = { 0, 0, 0, 255 }, bool inside = true);
	void FillRectPercent(int x, int y, int w, int h, float t, const Color_t& clr, const Color_t& out = { 0, 0, 0, 255 }, const EAlign& align = ALIGN_LEFT, bool adjust = false);
	void RoundRect(int x, int y, int w, int h, int radius, const Color_t& clr);

	void FillCircle(int x, int y, float radius, int segments, Color_t clr);
	void LineCircle(int x, int y, float radius, int segments, const Color_t& clr);

	void Texture(int x, int y, int w, int h, int id, const EAlign& align = ALIGN_CENTER);
	CHudTexture* GetIcon(const char* szIcon, int eIconFormat = 0);
	int CreateTextureFromArray(const unsigned char* rgba, int w, int h);
	void DrawHudTexture(float x, float y, float s, const CHudTexture* texture, Color_t clr = { 255, 255, 255, 255 });
	void DrawHudTextureByName(float x, float y, float s, const char* textureName, Color_t clr = { 255, 255, 255, 255 });
	void Avatar(int x, int y, int w, int h, const uint32 nFriendID, const EAlign& align = ALIGN_CENTER);
	void ClearAvatarCache();

	void StartClipping(int x, int y, int w, int h);
	void EndClipping();

	int m_nScreenW = 0, m_nScreenH = 0;
	VMatrix m_WorldToProjection = {};
};

ADD_FEATURE_CUSTOM(CDraw, Draw, H);