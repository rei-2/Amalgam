#pragma once
#include "../../../SDK/SDK.h"
#include <unordered_map>
#include <vector>
#include <string>

// Trail position data structure
struct TrailPosition
{
    Vec3 Position;
    float Time;
};

// Player trail data
struct PlayerTrailData
{
    std::vector<TrailPosition> Positions;
    Color_t Color;
    int VisibilityState;
    float VisibleStartTime;
    float LastSeenTime;
    float LastInvisibleTime;
};

// Visibility states
enum class EVisibilityState
{
    UNSEEN = 1,
    VISIBLE = 2,
    RECENTLY_INVISIBLE = 3
};

class CPlayerTrails
{
private:
    // Configuration constants (from Lua script)
    static constexpr int TRAIL_LENGTH = 19;
    static constexpr int UPDATE_INTERVAL = 6;
    static constexpr float FADE_TIME = 7.0f;
    static constexpr float MAX_TRAIL_DISTANCE = 1850.0f;
    static constexpr float VISIBILITY_TIMEOUT = 4.0f;
    static constexpr float MAX_VISIBLE_DURATION = 2.0f;
    static constexpr float TRAIL_HEIGHT = 1.5f;
    static constexpr float CLEANUP_INTERVAL = 0.5f;
    static constexpr float MIN_MOVEMENT_DISTANCE_SQR = 100.0f;
    
    // State tracking
    std::unordered_map<std::string, PlayerTrailData> m_PlayerData;
    std::unordered_map<std::string, Color_t> m_ColorCache;
    std::unordered_map<int, float> m_NextVisCheck;
    std::unordered_map<int, bool> m_VisibilityCache;
    
    int m_LastUpdateTick;
    float m_NextCleanupTime;
    std::string m_CurrentMap;
    bool m_LastLifeState;
    
    // Pre-calculated values
    float m_TrailHeightUnits;
    float m_MaxDistanceSqr;
    
    // Helper functions
    Color_t GenerateColor(const std::string& steamID);
    void ClearData();
    void CleanInvalidTargets();
    bool IsVisible(CBaseEntity* pEntity, CBaseEntity* pLocal);
    void UpdatePlayerState(PlayerTrailData& data, bool isVisible, float currentTime);
    bool ShouldShowTrail(const PlayerTrailData& data, float currentTime);
    std::string GetSteamID(CBaseEntity* pPlayer);
    bool IsValidTrailTarget(CBaseEntity* pPlayer, CBaseEntity* pLocal);
    
public:
    CPlayerTrails();
    void Draw();
    void Reset();
};

ADD_FEATURE(CPlayerTrails, PlayerTrails)