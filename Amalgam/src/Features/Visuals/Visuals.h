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
	std::vector<PickupData> m_vPickups;

public:
	void DrawFOV(CTFPlayer* pLocal);
	void DrawTicks(CTFPlayer* pLocal);
	void DrawPing(CTFPlayer* pLocal);
	void ProjectileTrace(CTFPlayer* pPlayer, CTFWeaponBase* pWeapon, const bool bQuick = true);
	void SplashRadius(CTFPlayer* pLocal);
	void DrawAntiAim(CTFPlayer* pLocal);
	void DrawDebugInfo(CTFPlayer* pLocal);

	std::vector<DrawBox> GetHitboxes(matrix3x4 aBones[MAXSTUDIOBONES], CBaseAnimating* pEntity, std::vector<int> vHitboxes = {}, int iTarget = -1);
	void DrawPath(std::deque<Vec3>& Line, Color_t Color, int iStyle, bool bZBuffer = false, float flTime = 0.f);
	void DrawLines();
	void DrawPaths();
	void DrawBoxes();
	void RestoreLines();
	void RestorePaths();
	void RestoreBoxes();
	void DrawServerHitboxes(CTFPlayer* pLocal);
	void RenderLine(const Vec3& vStart, const Vec3& vEnd, Color_t cLine, bool bZBuffer = false);
	void RenderBox(const Vec3& vPos, const Vec3& vMins, const Vec3& vMaxs, const Vec3& vOrientation, Color_t cEdge, Color_t cFace, bool bZBuffer = false);

	void FOV(CTFPlayer* pLocal, CViewSetup* pView);
	void ThirdPerson(CTFPlayer* pLocal, CViewSetup* pView);
	void DrawSightlines();
	void Store();
	void DrawPickupTimers();
	void Event(IGameEvent* pEvent, uint32_t uHash);

	void OverrideWorldTextures();
	void Modulate();
	void RestoreWorldModulation();

	void CreateMove(CTFPlayer* pLocal, CTFWeaponBase* pWeapon);
};

ADD_FEATURE(CVisuals, Visuals)