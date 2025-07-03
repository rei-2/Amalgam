#pragma once
#include "../../../SDK/SDK.h"
#include <unordered_map>
#include <vector>

struct MedicPosition
{
    Vec3 Position;
    float LastUpdate = 0.0f;
};

struct VisibilityCache
{
    bool IsVisible = false;
    float LastCheck = 0.0f;
};

class CPylonESP
{
private:
    // Configuration constants (from Lua script)
    static constexpr float UPDATE_INTERVAL = 0.5f;
    static constexpr int PYLON_WIDTH = 6;
    static constexpr float PYLON_HEIGHT = 350.0f;
    static constexpr float PYLON_OFFSET = 35.0f;
    static constexpr Color_t PYLON_COLOR = {255, 0, 0, 255};
    static constexpr int PYLON_START_ALPHA = 200;
    static constexpr int PYLON_END_ALPHA = 25;
    static constexpr int SEGMENTS = 10;
    static constexpr float MIN_DISTANCE = 800.0f;
    static constexpr float VISIBILITY_PERSISTENCE = 0.2f;
    static constexpr float CLEANUP_INTERVAL = 1.0f;
    
    // Calculated constants
    static constexpr float MIN_DISTANCE_SQR = MIN_DISTANCE * MIN_DISTANCE;
    static constexpr float SEGMENT_HEIGHT = PYLON_HEIGHT / SEGMENTS;
    
    // State tracking
    std::unordered_map<int, MedicPosition> m_MedicPositions;
    std::unordered_map<std::string, VisibilityCache> m_VisibilityCache;
    float m_LastCleanup = 0.0f;
    
    // Pre-calculated alpha steps
    std::vector<int> m_AlphaSteps;
    
    // Helper functions
    void InitializeAlphaSteps();
    bool IsVisibleCached(const Vec3& fromPos, const Vec3& targetPos, const std::string& identifier);
    float GetDistanceSqr(const Vec3& pos1, const Vec3& pos2);
    bool ShouldUpdatePosition(int medicIndex);
    void CleanupStaleEntries();
    bool IsPlayerDirectlyVisible(CTFPlayer* pPlayer, const Vec3& eyePos);
    void DrawPylon(int medicIndex, const Vec3& basePosition, const Vec3& eyePos);
    
public:
    CPylonESP();
    void Draw();
    void Reset();
};

ADD_FEATURE(CPylonESP, PylonESP)