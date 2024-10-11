#pragma once
#include "../../../SDK/SDK.h"

enum TextMode
{
	TextTop,
	TextBottom,
	TextRight,
	TextHealth,
	TextUber
};

struct PlayerCache
{
	float m_flAlpha = 1.f;
	std::vector<std::tuple<TextMode, std::string, Color_t>> m_vText = {};
	Color_t m_tColor = {};
	bool m_bBox = false;
	bool m_bBones = false;

	bool m_bHealthBar = false;
	bool m_bUberBar = false;
	int m_iClassIcon = 0;
	CHudTexture* m_pWeaponIcon = nullptr;
	float m_flHealth = 0.f;
	float m_flUber = 0.f;
};

struct BuildingCache
{
	float m_flAlpha = 1.f;
	std::vector<std::tuple<TextMode, std::string, Color_t>> m_vText = {};
	Color_t m_tColor = {};
	bool m_bBox = false;

	bool m_bHealthBar = false;
	float m_flHealth = 0.f;
};

struct WorldCache
{
	float m_flAlpha = 1.f;
	std::vector<std::tuple<TextMode, std::string, Color_t>> m_vText = {};
	Color_t m_tColor = {};
	bool m_bBox = false;
};

class CESP
{
private:
	void StorePlayers(CTFPlayer* pLocal);
	void StoreBuildings(CTFPlayer* pLocal);
	void StoreProjectiles(CTFPlayer* pLocal);
	void StoreObjective(CTFPlayer* pLocal);
	void StoreWorld();

	void DrawPlayers();
	void DrawBuildings();
	void DrawWorld();

	bool GetDrawBounds(CBaseEntity* pEntity, int& x, int& y, int& w, int& h);
	const char* GetPlayerClass(int nClassNum);
	void DrawBones(CTFPlayer* pPlayer, std::vector<int> vecBones, Color_t clr);

	std::unordered_map<CBaseEntity*, PlayerCache> m_mPlayerCache = {};
	std::unordered_map<CBaseEntity*, BuildingCache> m_mBuildingCache = {};
	std::unordered_map<CBaseEntity*, WorldCache> m_mWorldCache = {};

public:
	void Store(CTFPlayer* pLocal);
	void Draw();
};

ADD_FEATURE(CESP, ESP)