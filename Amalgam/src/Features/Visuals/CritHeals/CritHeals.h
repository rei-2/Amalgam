#pragma once
#include "../../../SDK/SDK.h"
#include <unordered_map>

class CCritHeals
{
private:
    // Configuration constants from lua script
    static constexpr float CRIT_HEAL_TIME = 10.0f;
    static constexpr float MAX_DISTANCE = 800.0f;
    static constexpr float MAX_OVERHEAL_MULTIPLIER = 1.5f;
    static constexpr float UBER_PENALTY_THRESHOLD = 1.425f; // 142.5% health for uber build penalty
    static constexpr float BUFF_THRESHOLD = 0.9f;
    static constexpr float BUFF_FADE_THRESHOLD = 0.33f;
    
    // Visual configuration
    static constexpr int TRIANGLE_SIZE = 12;
    static constexpr int TRIANGLE_VERTICAL_OFFSET = 100;
    static constexpr int OUTLINE_THICKNESS = 2;
    
    // Colors
    static constexpr Color_t FRIEND_COLOR = {50, 255, 50, 255};  // Green
    static constexpr Color_t ENEMY_COLOR = {255, 50, 50, 255};   // Red
    static constexpr Color_t OUTLINE_COLOR = {0, 0, 0, 255};     // Black
    static constexpr Color_t WARNING_COLOR = {255, 200, 0, 255}; // Yellow
    
    // Feature toggles
    static constexpr bool UBER_BUILD_WARNING = true;
    static constexpr bool SHOW_ON_ENEMIES = false;
    
    // Tracking data
    std::unordered_map<int, float> m_LastDamageTimes;
    std::unordered_map<int, float> m_LastBuffTimes;
    
    // Helper functions
    void DrawTriangle(int x, int y, bool isEnemy);
    void DrawUberBuildWarning();
    bool IsPlayerVisible(CTFPlayer* pPlayer, CTFPlayer* pLocal);

public:
    void Draw();
    void OnPlayerHurt(int victimIndex);
};

ADD_FEATURE(CCritHeals, CritHeals);