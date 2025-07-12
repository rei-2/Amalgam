#pragma once
#include "../../../SDK/SDK.h"
#include <unordered_map>

struct HiderData
{
    Vec3 LastPosition;
    float LastMoveTime;
    bool IsHider;
    float LastUpdateTime;
};

class CHiderESP
{
private:
    // Configuration constants (mirroring Lua CONFIG)
    static constexpr bool SHOW_BOXES = true;
    static constexpr bool SHOW_TRACERS = false;
    static constexpr Color_t BOX_COLOR = {255, 165, 0, 255};     // Orange for hiders
    static constexpr Color_t TRACER_COLOR = {0, 0, 255, 255};   // Blue for tracers
    static constexpr float TIME_TO_MARK_AS_HIDER = 5.5f;        // Seconds player must remain stationary
    static constexpr float UPDATE_POSITION_THRESHOLD = 1.0f;    // Units player must move to be considered "moving"
    static constexpr float MAX_DISTANCE = 1650.0f;              // Maximum distance to check for hiders
    static constexpr float CLEANUP_INTERVAL = 1.0f;             // Clean invalid players every second

    // Player tracking data
    std::unordered_map<int, HiderData> m_PlayerData;
    int m_iLastLifeState = 2; // Start with LIFE_DEAD (2)
    float m_flNextCleanupTime = 0.0f;

    // Helper functions
    float DistanceBetweenVectors(const Vec3& v1, const Vec3& v2);
    void ResetPlayerData();
    void CleanInvalidTargets();
    bool IsPlayerInRange(CTFPlayer* pPlayer, CTFPlayer* pLocal);
    void UpdatePlayerData(CTFPlayer* pPlayer);
    void DrawPlayerESP(CTFPlayer* pPlayer, const HiderData& data);

public:
    void Draw();
};

ADD_FEATURE(CHiderESP, HiderESP)