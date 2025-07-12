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