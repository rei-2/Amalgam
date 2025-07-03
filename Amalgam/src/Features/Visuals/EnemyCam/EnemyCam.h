#pragma once
#include "../../../SDK/SDK.h"

enum class ECameraMode
{
    CLOSEST = 0,
    HEALED = 1,
    MEDIC = 2,
    TOP_SCORE = 3,
    RANDOM = 4
};

enum class EViewMode
{
    RAW = 0,    // Exact player view
    OFFSET = 1  // Offset view for better visibility
};

class CEnemyCam
{
private:
    // Configuration constants
    static constexpr int CAMERA_WIDTH = 320;
    static constexpr int CAMERA_HEIGHT = 240;
    static constexpr int BORDER_OFFSET = 5;
    static constexpr float FORWARD_OFFSET = 16.5f;
    static constexpr float UPWARD_OFFSET = 12.0f;
    static constexpr float TRACK_TIME = 3.0f;
    static constexpr float SEARCH_INTERVAL = 0.5f;
    
    // Camera state
    CTFPlayer* m_pTargetPlayer = nullptr;
    CTFPlayer* m_pTargetMedic = nullptr;
    float m_flTargetSwitchTime = 0.0f;
    float m_flLastSearchTime = 0.0f;
    
    // Camera materials
    IMaterial* m_pCameraMaterial = nullptr;
    ITexture* m_pCameraTexture = nullptr;
    bool m_bInitialized = false;
    bool m_bEnabled = true;
    
    // Camera settings
    ECameraMode m_eMode = ECameraMode::CLOSEST;
    EViewMode m_eViewMode = EViewMode::OFFSET;
    
    // Helper functions
    bool InitializeMaterials();
    void CleanupMaterials();
    std::vector<CTFPlayer*> GetEnemyPlayers();
    CTFPlayer* FindTargetPlayer();
    CTFPlayer* FindClosestEnemy();
    CTFPlayer* FindHealedPlayer(CTFPlayer*& outMedic);
    CTFPlayer* FindEnemyMedic(CTFPlayer*& outTarget);
    CTFPlayer* FindTopScorePlayer();
    void UpdateTargetPlayer();
    void GetCameraView(Vec3& origin, Vec3& angles);
    void DrawOverlay();
    const char* GetClassName(int classId);
    
public:
    void Initialize();
    void Unload();
    void Draw();
    void RenderView(void* ecx, const CViewSetup& view);
    
    bool IsEnabled() const { return m_bEnabled; }
    void SetEnabled(bool enabled) { m_bEnabled = enabled; }
    void SetMode(ECameraMode mode) { m_eMode = mode; }
    void SetViewMode(EViewMode mode) { m_eViewMode = mode; }
};

ADD_FEATURE(CEnemyCam, EnemyCam)