#pragma once
#include "../../../SDK/SDK.h"
#include <vector>

class CPylonESP
{
private:
    // Configuration constants
    static constexpr int PYLON_WIDTH = 6;
    static constexpr float PYLON_HEIGHT = 350.0f;
    static constexpr float PYLON_OFFSET = 35.0f;
    static constexpr Color_t PYLON_COLOR = {255, 0, 0, 255};
    static constexpr int PYLON_START_ALPHA = 200;
    static constexpr int PYLON_END_ALPHA = 25;
    static constexpr int SEGMENTS = 10;
    static constexpr float MIN_DISTANCE = 800.0f;
    
    // Calculated constants
    static constexpr float MIN_DISTANCE_SQR = MIN_DISTANCE * MIN_DISTANCE;
    static constexpr float SEGMENT_HEIGHT = PYLON_HEIGHT / SEGMENTS;
    
    // Pre-calculated alpha steps
    std::vector<int> m_AlphaSteps;
    
    // Helper functions
    void InitializeAlphaSteps();
    bool IsVisible(const Vec3& fromPos, const Vec3& targetPos);
    float GetDistanceSqr(const Vec3& pos1, const Vec3& pos2);
    bool IsPlayerDirectlyVisible(CTFPlayer* pPlayer, const Vec3& eyePos);
    void DrawPylon(const Vec3& basePosition, const Vec3& eyePos);
    
public:
    CPylonESP();
    void Draw();
};

ADD_FEATURE(CPylonESP, PylonESP)