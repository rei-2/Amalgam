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
    // Fixed constants
    static constexpr int BORDER_OFFSET = 5;
    static constexpr float FORWARD_OFFSET = 16.5f;
    static constexpr float UPWARD_OFFSET = 12.0f;
    static constexpr float SEARCH_INTERVAL = 0.5f;
    
    // Window size and position are now configurable via Vars::Competitive::EnemyCam::*
    
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
    
    // Size tracking for material reloading
    int m_iLastWidth = 0;
    int m_iLastHeight = 0;
    
    // Camera settings are now read from Vars::Competitive::EnemyCam::* variables
    
    // Helper functions
    bool InitializeMaterials();
    void CleanupMaterials();
    bool CheckMaterialsNeedReload();
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
    
    // Configuration helpers
    int GetCameraWidth() const { return Vars::Competitive::EnemyCam::WindowWidth.Value; }
    int GetCameraHeight() const { return Vars::Competitive::EnemyCam::WindowHeight.Value; }
    void GetCameraPosition(int& x, int& y) const;
    
public:
    void Initialize();
    void Unload();
    void Draw();
    void RenderView(void* ecx, const CViewSetup& view);
    void Reset();
    
    bool IsEnabled() const { return m_bEnabled; }
    void SetEnabled(bool enabled) { m_bEnabled = enabled; }
    // Mode and ViewMode are now controlled via Vars::Competitive::EnemyCam::* variables
};

ADD_FEATURE(CEnemyCam, EnemyCam)