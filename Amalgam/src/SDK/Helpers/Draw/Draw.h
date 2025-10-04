#pragma once
#include "../Fonts/Fonts.h"
#include "../../Definitions/Definitions.h"
#include "../../Vars.h"
#include <mutex>
#include <chrono>
#include <unordered_set>

MAKE_SIGNATURE(InitializeStandardMaterials, "engine.dll", "48 83 EC ? 80 3D ? ? ? ? ? 0F 85 ? ? ? ? 48 89 5C 24 ? B9 ? ? ? ? 48 89 7C 24 ? C6 05", 0x0);
MAKE_SIGNATURE(Wireframe, "engine.dll", "48 89 05 ? ? ? ? E8 ? ? ? ? 48 85 C0 74 ? 48 8D 15 ? ? ? ? 48 8B C8 E8 ? ? ? ? 48 8B F8 EB ? 48 8B FB 41 B8 ? ? ? ? 48 8D 15 ? ? ? ? 48 8B CF E8 ? ? ? ? 41 B8 ? ? ? ? 48 8D 15 ? ? ? ? 48 8B CF E8 ? ? ? ? 41 B8", 0x0);
MAKE_SIGNATURE(WireframeIgnoreZ, "engine.dll", "48 89 05 ? ? ? ? E8 ? ? ? ? 48 85 C0 74 ? 48 8D 15 ? ? ? ? 48 8B C8 E8 ? ? ? ? 48 8B F8 EB ? 48 8B FB 41 B8 ? ? ? ? 48 8D 15 ? ? ? ? 48 8B CF E8 ? ? ? ? 41 B8 ? ? ? ? 48 8D 15 ? ? ? ? 48 8B CF E8 ? ? ? ? 48 8B 0D", 0x0);
MAKE_SIGNATURE(VertexColor, "engine.dll", "48 89 05 ? ? ? ? E8 ? ? ? ? 48 8B 7C 24", 0x0);
MAKE_SIGNATURE(VertexColorIgnoreZ, "engine.dll", "48 89 05 ? ? ? ? 48 83 C4 ? C3 CC CC CC CC CC CC CC CC CC 48 8B C4", 0x0);

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
	// Avatar cache with safety infrastructure
	std::unordered_map<uint32, int> m_mAvatars = {};
	std::unordered_set<uint32> m_sFailedAvatars = {}; // Track failed Steam IDs to avoid retries
	std::mutex m_mxAvatarMutex; // Thread safety for avatar operations
	std::chrono::steady_clock::time_point m_lastCleanupTime = std::chrono::steady_clock::now();

	// Avatar cache limits and constants
	static constexpr size_t MAX_AVATAR_CACHE_SIZE = 256;
	static constexpr size_t MAX_FAILED_CACHE_SIZE = 512;
	static constexpr std::chrono::minutes CLEANUP_INTERVAL{5}; // Clean cache every 5 minutes
	static constexpr std::chrono::minutes FAILED_RETRY_INTERVAL{30}; // Retry failed avatars after 30 minutes

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

	void Start(bool bBadFontCheck = false);
	void End();
	void StartClipping(int x, int y, int w, int h);
	void EndClipping();

	void UpdateScreenSize();
	void UpdateW2SMatrix();

	Vec2 GetTextSize(const char* text, const Font_t& tFont);
	Vec2 GetTextSize(const wchar_t* text, const Font_t& tFont);

	void String(const Font_t& tFont, int x, int y, const Color_t& tColor, const EAlign& eAlign, const char* str);
	void String(const Font_t& tFont, int x, int y, const Color_t& tColor, const EAlign& eAlign, const wchar_t* wstr);
	void StringOutlined(const Font_t& tFont, int x, int y, const Color_t& tColor, const Color_t& tColorOut, const EAlign& eAlign, const char* str);
	void StringOutlined(const Font_t& tFont, int x, int y, const Color_t& tColor, const Color_t& tColorOut, const EAlign& eAlign, const wchar_t* wstr);

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
	void PeriodicAvatarCleanup(); // Automatic cache maintenance
	void OnMapChange(); // Call when map changes to clean cache
	void OnDisconnect(); // Call when disconnecting to clean cache

	void RenderLine(const Vec3& vStart, const Vec3& vEnd, Color_t tColor, bool bZBuffer = false);
	void RenderPath(const std::vector<Vec3>& vPath, Color_t tColor, bool bZBuffer = false,
		int iStyle = Vars::Visuals::Simulation::StyleEnum::Line, float flTime = 0.f,
		int iSeparatorSpacing = Vars::Visuals::Simulation::SeparatorSpacing.Value,
		float flSeparatorLength = Vars::Visuals::Simulation::SeparatorLength.Value);
	void RenderBox(const Vec3& vOrigin, const Vec3& vMins, const Vec3& vMaxs, const Vec3& vAngles, Color_t tColor, bool bZBuffer = false, bool bInsideOut = false);
	void RenderWireframeBox(const Vec3& vOrigin, const Vec3& vMins, const Vec3& vMaxs, const Vec3& vAngles, Color_t tColor, bool bZBuffer = false);
	void RenderWireframeSweptBox(const Vector& vStart, const Vector& vEnd, const Vec3& vMins, const Vec3& vMaxs, const Vec3& vAngles, Color_t tColor, bool bZBuffer = false);
	void RenderTriangle(const Vector& vPoint1, const Vector& vPoint2, const Vector& vPoint3, Color_t tColor, bool bZBuffer = false);
	void RenderSphere(const Vector& vCenter, float flRadius, int nTheta, int nPhi, Color_t tColor, IMaterial* pMaterial);
	void RenderSphere(const Vector& vCenter, float flRadius, int nTheta, int nPhi, Color_t tColor, bool bZBuffer = false);
	void RenderWireframeSphere(const Vector& vCenter, float flRadius, int nTheta, int nPhi, Color_t tColor, bool bZBuffer = false);

	int m_nScreenW = 0, m_nScreenH = 0;
	VMatrix m_mWorldToProjection = {};

private:
	// Safety helper methods
	bool IsSteamInterfacesValid() const;
	bool IsValidAvatarHandle(int nAvatar) const;
	bool ShouldRetryFailedAvatar(uint32 nFriendID) const;
	void MarkAvatarAsFailed(uint32 nFriendID);
	void CleanupOldestAvatars(size_t maxSize);
	bool LoadAvatarSafely(uint32 nFriendID, int& outTextureID);
};

ADD_FEATURE_CUSTOM(CDraw, Draw, H);