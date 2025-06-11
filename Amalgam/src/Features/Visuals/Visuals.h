#pragma once
#include "../../SDK/SDK.h"

#include <map>

struct Sightline_t
{
	Vec3 m_vStart = {};
	Vec3 m_vEnd = {};
	Color_t m_Color = {};
	bool m_bZBuffer = false;
};

struct PickupData
{
	int Type = 0;
	float Time = 0.f;
	Vec3 Location;
};

class CVisuals
{
private:
	int m_nHudZoom = 0;
	std::vector<Sightline_t> m_vSightLines;
	std::vector<PickupData> m_vPickups;

	class CPrecipitation // thanks .pointone_ for the robux
    {
    private:
        int m_iRainEntityIndex = -1;
        ClientClass* GetPrecipitationClass();

    public:
        void Run();
        void Cleanup();
        bool IsRaining() const { return m_iRainEntityIndex != -1; }
        friend class CVisuals; 
    };

    CPrecipitation m_Precipitation;

public:
	void Event(IGameEvent* pEvent, uint32_t uHash);
	void Store(CTFPlayer* pLocal);

	void ProjectileTrace(CTFPlayer* pPlayer, CTFWeaponBase* pWeapon, const bool bQuick = true);
	void SplashRadius(CTFPlayer* pLocal);
	void DrawAntiAim(CTFPlayer* pLocal);
	void DrawPickupTimers();
	void DrawDebugInfo(CTFPlayer* pLocal);

	std::vector<DrawBox_t> GetHitboxes(matrix3x4* aBones, CBaseAnimating* pEntity, std::vector<int> vHitboxes = {}, int iTarget = -1);
	void DrawEffects();
	void DrawServerHitboxes(CTFPlayer* pLocal);

	void FOV(CTFPlayer* pLocal, CViewSetup* pView);
	void ThirdPerson(CTFPlayer* pLocal, CViewSetup* pView);

	void OverrideWorldTextures();
	void Modulate();
	void RestoreWorldModulation();
	void UpdatePrecipitation();

	void CreateMove(CTFPlayer* pLocal, CTFWeaponBase* pWeapon);
};

ADD_FEATURE(CVisuals, Visuals);