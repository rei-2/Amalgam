#pragma once
#include "../../SDK/SDK.h"

#include <map>

struct Sightline_t
{
	Vec3 m_vStart = {};
	Vec3 m_vEnd = {};
	Color_t m_Color = {};
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

public:
	void DrawAimbotFOV(CTFPlayer* pLocal);
	void DrawTickbaseText(CTFPlayer* pLocal);
	void DrawOnScreenPing(CTFPlayer* pLocal);
	void DrawOnScreenConditions(CTFPlayer* pLocal);
	void DrawSeedPrediction(CTFPlayer* pLocal);
	void ProjectileTrace(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, const bool bQuick = true);
	void SplashRadius(CTFPlayer* pLocal);
	void DrawAntiAim(CTFPlayer* pLocal);
	void DrawDebugInfo(CTFPlayer* pLocal);

	std::vector<DrawBox> GetHitboxes(matrix3x4 aBones[MAXSTUDIOBONES], CBaseAnimating* pEntity, const int iHitbox = -1);
	void DrawBulletLines();
	void DrawSimLine(std::deque<Vec3>& Line, Color_t Color, int iStyle = 0, bool bZBuffer = false, float flTime = 0.f);
	void DrawSimLines();
	void DrawBoxes();
	void RevealSimLines();
	void RevealBulletLines();
	void RevealBoxes();
	void DrawServerHitboxes(CTFPlayer* pLocal);
	void RenderLine(const Vec3& vStart, const Vec3& vEnd, Color_t cLine, bool bZBuffer = false);
	void RenderBox(const Vec3& vPos, const Vec3& vMins, const Vec3& vMaxs, const Vec3& vOrientation, Color_t cEdge, Color_t cFace, bool bZBuffer = false);

	void FOV(CTFPlayer* pLocal, CViewSetup* pView);
	void ThirdPerson(CTFPlayer* pLocal, CViewSetup* pView);
	void DrawSightlines();
	void Store();
	void PickupTimers();

	std::vector<PickupData> m_vPickupDatas;

	void OverrideWorldTextures();
	void Modulate();
	void RestoreWorldModulation();

	void CreateMove(CTFPlayer* pLocal, CTFWeaponBase* pWeapon);
};

ADD_FEATURE(CVisuals, Visuals)