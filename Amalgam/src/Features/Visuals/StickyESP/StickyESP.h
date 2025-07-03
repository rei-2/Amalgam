#pragma once
#include "../../../SDK/SDK.h"
#include <unordered_map>

// Sticky bomb ESP data structure
struct StickyData
{
    Vec3 Position;
    bool IsVisible;
    float LastVisCheck;
    float LastPosUpdate;
};

class CStickyESP
{
private:
    // Configuration constants (from Lua script)
    static constexpr bool ENEMY_ONLY = true;
    static constexpr float MAX_DISTANCE = 2800.0f;
    static constexpr bool BOX_3D = false;
    static constexpr bool BOX_2D = true;
    static constexpr int BOX_2D_SIZE = 20;
    static constexpr bool BOX_ONLY_WHEN_VISIBLE = true;
    static constexpr bool CHAMS_ON = false; // Disabled by default as per Lua
    static constexpr bool CHAMS_ONLY_WHEN_VISIBLE = false;
    
    // Performance optimization constants
    static constexpr float VISIBILITY_CHECK_INTERVAL = 0.1f;
    static constexpr float CACHE_CLEANUP_INTERVAL = 1.0f;
    static constexpr float STICKY_UPDATE_INTERVAL = 0.1f;
    static constexpr float HITBOX_CACHE_LIFETIME = 0.2f;
    
    // Colors
    static constexpr Color_t BOX_COLOR_VISIBLE = {0, 255, 0, 255};   // Green
    static constexpr Color_t BOX_COLOR_INVISIBLE = {255, 0, 0, 255}; // Red
    
    // State tracking
    std::unordered_map<int, StickyData> m_StickyCache;
    std::unordered_map<int, bool> m_VisibilityCache;
    std::unordered_map<int, float> m_VisibilityCacheTime;
    std::unordered_map<int, std::pair<Vec3, Vec3>> m_HitboxCache;
    std::unordered_map<int, float> m_HitboxCacheTime;
    
    float m_NextCleanupTime;
    float m_LastStickyUpdate;
    
    // Pre-calculated values
    float m_MaxDistanceSqr;
    
    // Helper functions
    void CleanCaches();
    bool IsVisible(CBaseEntity* pEntity, CBaseEntity* pLocal);
    std::pair<Vec3, Vec3> GetHitboxWithCache(CBaseEntity* pEntity);
    void Draw3DBox(const std::vector<Vec3>& vertices, const Color_t& color);
    void Draw2DBox(int x, int y, int size, const Color_t& color);
    bool IsValidStickyTarget(CBaseEntity* pEntity, CBaseEntity* pLocal);
    
public:
    CStickyESP();
    void Draw();
};

ADD_FEATURE(CStickyESP, StickyESP)