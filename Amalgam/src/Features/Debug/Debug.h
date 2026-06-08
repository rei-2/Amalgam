#pragma once
#include "../../SDK/SDK.h"

#define DEBUG_INFO
//#define DEBUG_TEXT

#ifdef DEBUG_INFO

#ifdef DEBUG_TEXT
Enum(Text, All, Auto, Manual)

struct DebugText_t
{
	std::string m_sText = "";
	Color_t m_tColor = {};
	float m_flTime = 0.f;
	std::optional<Vec2> vPosition2D = std::nullopt;
	std::optional<Vec3> vPosition3D = std::nullopt;
};
#endif

class CDebug
{
private:
#ifdef DEBUG_TEXT
	std::vector<DebugText_t> m_vDebugText = {};
#endif

public:
	void Draw(CTFPlayer* pLocal);

#ifdef DEBUG_TEXT
	const std::vector<DebugText_t>& GetText();
	void ClearText(byte iType = TextEnum::All);
	void PopText(byte iType = TextEnum::All);
	void AddText(const DebugText_t& sText);
	void AddText(const std::string& sString, Color_t tColor = Vars::Menu::Theme::Active.Value);
	void AddText(const std::string& sString, const Vec2& vPosition, Color_t tColor = Vars::Menu::Theme::Active.Value);
	void AddText(const std::string& sString, const Vec3& vPosition, Color_t tColor = Vars::Menu::Theme::Active.Value);
	void AddText(const std::string& sString, float flTime, Color_t tColor = Vars::Menu::Theme::Active.Value);
	void AddText(const std::string& sString, const Vec2& vPosition, float flTime, Color_t tColor = Vars::Menu::Theme::Active.Value);
	void AddText(const std::string& sString, const Vec3& vPosition, float flTime, Color_t tColor = Vars::Menu::Theme::Active.Value);
#endif
};

ADD_FEATURE(CDebug, Debug);

#endif