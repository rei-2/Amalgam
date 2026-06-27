#pragma once
#include "../../SDK/SDK.h"

#define DEBUG_INFO
//#define DEBUG_TEXT

#ifdef DEBUG_INFO

#ifdef DEBUG_TEXT
struct DebugText_t
{
	std::string m_sText = "";
	float m_flTime = 0.f;
	std::optional<Vec2> vPosition2D = std::nullopt;
	std::optional<Vec3> vPosition3D = std::nullopt;
	Color_t m_tColor = {};
};
#endif

class CDebug
{
private:
#ifdef DEBUG_TEXT
	std::map<int, std::vector<DebugText_t>> m_mDebugText = {};
#endif

public:
	void Draw(CTFPlayer* pLocal);

#ifdef DEBUG_TEXT
	const std::map<int, std::vector<DebugText_t>>& GetText();
	const std::vector<DebugText_t>& GetText(uint32_t uType);
	void ClearText(uint32_t uType = 0);
	void PopText(uint32_t uType = 0);
	void AddText(const DebugText_t& sText);
	void AddText(const DebugText_t& sText, uint32_t uType);
	void AddText(const std::string& sString, Color_t tColor = Vars::Menu::Theme::Active.Value);
	void AddText(const std::string& sString, const Vec2& vPosition, Color_t tColor = Vars::Menu::Theme::Active.Value);
	void AddText(const std::string& sString, const Vec3& vPosition, Color_t tColor = Vars::Menu::Theme::Active.Value);
	void AddText(const std::string& sString, uint32_t uType, Color_t tColor = Vars::Menu::Theme::Active.Value);
	void AddText(const std::string& sString, uint32_t uType, const Vec2& vPosition, Color_t tColor = Vars::Menu::Theme::Active.Value);
	void AddText(const std::string& sString, uint32_t uType, const Vec3& vPosition, Color_t tColor = Vars::Menu::Theme::Active.Value);
	void AddText(const std::string& sString, float flTime, Color_t tColor = Vars::Menu::Theme::Active.Value);
	void AddText(const std::string& sString, float flTime, const Vec2& vPosition, Color_t tColor = Vars::Menu::Theme::Active.Value);
	void AddText(const std::string& sString, float flTime, const Vec3& vPosition, Color_t tColor = Vars::Menu::Theme::Active.Value);
	void AddText(const std::string& sString, float flTime, uint32_t uType, Color_t tColor = Vars::Menu::Theme::Active.Value);
	void AddText(const std::string& sString, float flTime, uint32_t uType, const Vec2& vPosition, Color_t tColor = Vars::Menu::Theme::Active.Value);
	void AddText(const std::string& sString, float flTime, uint32_t uType, const Vec3& vPosition, Color_t tColor = Vars::Menu::Theme::Active.Value);
#endif
};

ADD_FEATURE(CDebug, Debug);

#endif