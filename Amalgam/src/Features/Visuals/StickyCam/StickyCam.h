#pragma once
#include "../../../SDK/SDK.h"
#include <vector>
#include <unordered_map>

enum class EStickyMode
{
	MANUAL = 0,      // Manual cycling with TAB
	FOLLOW_LATEST = 1 // Always follow newest sticky
};

enum class EStickyViewMode
{
	RAW = 0,         // Exact sticky position view
	OFFSET = 1       // Offset view for better visibility
};

struct StickyData
{
	CBaseEntity* pSticky = nullptr;
	float flCreationTime = 0.0f;
	float flLastVisibilityCheck = 0.0f;
	bool bVisible = false;
};

class CStickyCam
{
private:
	// Fixed constants
	static constexpr float CAMERA_OFFSET = 35.0f;
	static constexpr float TARGET_LOCK_DURATION = 5.0f;
	static constexpr float TARGET_SEARCH_RADIUS = 584.0f; // 146 * 4
	static constexpr float OCCLUSION_CHECK_INTERVAL = 0.1f;
	static constexpr float ANGLE_INTERPOLATION_SPEED = 0.1f;
	static constexpr float STICKY_DORMANT_DISTANCE = 1200.0f;
	static constexpr float STICKY_WARNING_THRESHOLD = 0.70f;
	static constexpr float KEY_DELAY = 0.2f;

	// Sticky and target tracking state
	std::vector<StickyData> m_Stickies;
	CBaseEntity* m_pCurrentSticky = nullptr;
	CTFPlayer* m_pCurrentTarget = nullptr;
	Vec3 m_vSmoothedAngles = Vec3(0, 0, 0);
	float m_flTargetLockTime = 0.0f;
	bool m_bTargetVisible = false;
	float m_flLastOcclusionCheck = 0.0f;

	// Sticky cycling state
	float m_flLastKeyPress = 0.0f;
	std::vector<CBaseEntity*> m_VisitedStickies;

	// Camera materials
	IMaterial* m_pCameraMaterial = nullptr;
	ITexture* m_pCameraTexture = nullptr;
	IMaterial* m_pInvisibleMaterial = nullptr;
	IMaterial* m_pChamsMaterial = nullptr;
	bool m_bInitialized = false;

	// Configuration helpers
	int GetCameraWidth() const { return Vars::Competitive::StickyCam::WindowWidth.Value; }
	int GetCameraHeight() const { return Vars::Competitive::StickyCam::WindowHeight.Value; }
	void GetCameraPosition(int& x, int& y) const;
	EStickyMode GetMode() const { return static_cast<EStickyMode>(Vars::Competitive::StickyCam::Mode.Value); }
	EStickyViewMode GetViewMode() const { return static_cast<EStickyViewMode>(Vars::Competitive::StickyCam::ViewMode.Value); }

	// Helper functions
	bool InitializeMaterials();
	void CleanupMaterials();
	bool CheckMaterialsNeedReload();
	bool IsDemoman();
	Vec3 CalculateAngles(const Vec3& source, const Vec3& dest);
	bool CheckPlayerVisibility(const Vec3& startPos, CTFPlayer* pPlayer);
	bool IsStickyOnCeiling(CBaseEntity* pSticky);
	Vec3 AddVectors(const Vec3& vec1, const Vec3& vec2);
	Vec3 SubtractVectors(const Vec3& vec1, const Vec3& vec2);
	Vec3 MultiplyVector(const Vec3& vec, float scalar);
	Vec3 GetStickyNormal(CBaseEntity* pSticky);
	Vec3 CalculateCameraOffset(const Vec3& stickyPos, const Vec3& normal);
	CTFPlayer* FindNearestVisiblePlayer(const Vec3& position);
	Vec3 LerpAngles(const Vec3& start, const Vec3& target, float factor);
	void UpdateStickies();
	bool IsCameraViewClear(const Vec3& origin, const Vec3& angles);
	void CycleNextSticky();
	CBaseEntity* FindLatestSticky();
	bool UpdateUserInput();
	float GetStickyPlayerDistance(CBaseEntity* pSticky);
	void DrawStickyRangeWarning();
	bool IsTargetWithinRange(CTFPlayer* pTarget, const Vec3& stickyPos);
	void DrawOverlay();

	// Size tracking for material reloading
	int m_iLastWidth = 0;
	int m_iLastHeight = 0;

public:
	// Chams tracking for players in sticky radius (public for Chams system access)
	std::unordered_map<int, bool> m_mEntities;

	void Initialize();
	void Unload();
	void Draw();
	void RenderView(void* ecx, const CViewSetup& view);
	void UpdateChamsEntities();
	void Reset();

	bool IsEnabled() const { return Vars::Competitive::Features::StickyCam.Value; }
};

ADD_FEATURE(CStickyCam, StickyCam)