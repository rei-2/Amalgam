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
    // Configuration constants
    static constexpr int MIN_ATTACKERS = 2;
    static constexpr float TRACKER_TIME_WINDOW = 4.5f;
    static constexpr float CLEANUP_INTERVAL = 0.5f;
    
    // Visualization settings
    static constexpr bool ENABLE_CHAMS = true;
    static constexpr bool ENABLE_BOX = true;
    static constexpr bool VISIBLE_ONLY = false;
    
    // Box settings
    static constexpr Color_t BOX_COLOR = {255, 0, 0, 190};
    static constexpr int BOX_THICKNESS = 2;
    static constexpr int BOX_PADDING = 3;
    static constexpr bool USE_CORNERS = true;
    static constexpr int CORNER_LENGTH = 10;
    
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
    void Draw();
    void OnPlayerHurt(int victimIndex, int attackerIndex);
    void Reset();
};

ADD_FEATURE(CFocusFire, FocusFire)