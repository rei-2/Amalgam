#pragma once
#include "../../SDK/SDK.h"

#include <map>

struct Sightline_t
{
	Vec3 m_vStart = { 0,0,0 };
	Vec3 m_vEnd = { 0,0,0 };
	Color_t m_Color = { 0,0,0,0 };
	bool m_bDraw = false;
};

class CVisuals
{
private:
	int m_nHudZoom = 0;

public:
	void DrawAimbotFOV(CTFPlayer* pLocal);
	void DrawTickbaseText(CTFPlayer* pLocal);
	void DrawTickbaseBars();
	void DrawOnScreenPing(CTFPlayer* pLocal);
	void DrawOnScreenConditions(CTFPlayer* pLocal);
	void DrawSeedPrediction(CTFPlayer* pLocal);
	void ProjectileTrace(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, const bool bQuick = true);
	void DrawAntiAim(CTFPlayer* pLocal);
	void DrawDebugInfo(CTFPlayer* pLocal);

	std::vector<DrawBox> GetHitboxes(matrix3x4 bones[128], CBaseAnimating* pEntity, const int iHitbox = -1);
	void DrawBulletLines();
	void DrawSimLine(std::deque<std::pair<Vec3, Vec3>>& Line, Color_t Color, bool bSeparators = false, bool bZBuffer = false, float flTime = 0.f);
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
	bool RemoveScope(int nPanel);
	void DrawSightlines();
	void StoreSightlines();
	void PickupTimers();
	void ManualNetwork(const StartSoundParams_t& params); // Credits: reestart

	std::array<Sightline_t, 64> m_SightLines;

	struct PickupData
	{
		int Type = 0;
		float Time = 0.f;
		Vec3 Location;
	};
	std::vector<PickupData> PickupDatas;

	void OverrideWorldTextures();
	void Modulate();
	void SkyboxChanger();
	void RestoreWorldModulation();

	struct MaterialHandleData
	{
		enum class EMatGroupType
		{
			GROUP_OTHER,
			GROUP_WORLD,
			GROUP_SKY
		};

		MaterialHandle_t Handle;
		IMaterial* Material;
		std::string_view Group;
		std::string_view Name;
		EMatGroupType	 GroupType;
		bool			 ShouldOverrideTextures;
	};
};

ADD_FEATURE(CVisuals, Visuals)