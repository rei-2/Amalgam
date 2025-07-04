#pragma once
#include "../../../SDK/SDK.h"
#include <unordered_map>
#include <vector>

struct TargetInfo
{
    std::unordered_map<int, float> Attackers;
    bool IsMultiTargeted = false;
    int AttackerCount = 0;
};

class CFocusFire
{
private:
    // Configuration values are now read from Vars::Competitive::FocusFire::* variables
    static constexpr float CLEANUP_INTERVAL = 0.5f;
    
    // State tracking
    std::unordered_map<int, TargetInfo> m_TargetData;
    float m_NextCleanupTime = 0.0f;
    std::unordered_map<int, float> m_NextVisCheck;
    std::unordered_map<int, bool> m_VisibilityCache;
    
    // Helper functions
    bool IsPlayerVisible(CTFPlayer* pPlayer);
    void DrawCorners(int x1, int y1, int x2, int y2);
    void CleanExpiredAttackers(float currentTime, TargetInfo& targetInfo);
    void CleanInvalidTargets();
    bool ShouldVisualizePlayer(CTFPlayer* pPlayer);
    void DrawBox();
    
public:
    // Chams tracking for focused players
    std::unordered_map<int, bool> m_mEntities;
    
    void Draw();
    void OnPlayerHurt(int victimIndex, int attackerIndex);
    void Reset();
    void UpdateChamsEntities(); // Separate function to populate chams entities
};

ADD_FEATURE(CFocusFire, FocusFire)