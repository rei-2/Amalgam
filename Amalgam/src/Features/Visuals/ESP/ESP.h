#pragma once
#include "../../../SDK/SDK.h"

struct Text_t
{
	int m_iMode = ALIGN_TOP;
	std::string m_sText = "";
	Color_t m_tColor = {};
	Color_t m_tOutline = {};
};

struct Bar_t
{
	int m_iMode = ALIGN_TOP;
	float flPercent = 1.f;
	Color_t m_tColor = {};
	Color_t m_tOverfill = {};
	bool m_bAdjust = true;
};

struct EntityCache_t
{
	float m_flAlpha = 1.f;
	std::vector<Text_t> m_vText = {};
	Color_t m_tColor = {};
	bool m_bBox = false;
};

struct BuildingCache_t : EntityCache_t
{
	std::vector<Bar_t> m_vBars = {};
	float m_flHealth = 1.f;
};

struct PlayerCache_t : BuildingCache_t
{
	bool m_bBones = false;
	int m_iClassIcon = 0;
	CHudTexture* m_pWeaponIcon = nullptr;
};

class CESP
{
private:
	void DrawPlayers();
	void DrawBuildings();
	void DrawWorld();
	
	bool GetDrawBounds(CBaseEntity* pEntity, float& x, float& y, float& w, float& h);
	void DrawBones(CTFPlayer* pPlayer, matrix3x4* aBones, std::vector<int> vecBones, Color_t clr);

	std::unordered_map<CBaseEntity*, PlayerCache_t> m_mPlayerCache = {};
	std::unordered_map<CBaseEntity*, BuildingCache_t> m_mBuildingCache = {};
	std::unordered_map<CBaseEntity*, EntityCache_t> m_mEntityCache = {};

public:
	void Store(CTFPlayer* pLocal);
	void Draw();
};

ADD_FEATURE(CESP, ESP);