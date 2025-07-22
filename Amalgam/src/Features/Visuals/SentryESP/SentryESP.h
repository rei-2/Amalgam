#pragma once
#include "../../../SDK/SDK.h"
#include <unordered_set>
#include <unordered_map>

struct SentryAimData
{
    Vec3 MuzzlePos;
    Vec3 AimAngles;
    CBaseEntity* Target = nullptr;
    bool IsValid = false;
};

struct SentryInterpolationData
{
    Vec3 LastMuzzlePos;
    Vec3 LastAimAngles;
    float LastUpdateTime = 0.0f;
    bool HasValidData = false;
};

class CSentryESP
{
private:
    // State tracking
    std::unordered_set<int> m_SentriesTargetingLocal;
    
    // Interpolation data for smooth sentry lines
    std::unordered_map<int, SentryInterpolationData> m_InterpolationData;
    
    // Helper functions
    bool IsVisible(CBaseEntity* pEntity, CTFPlayer* pLocal);
    SentryAimData GetSentryAimData(CBaseEntity* pSentry);
    Vec3 GetAimLineEndPos(const Vec3& startPos, const Vec3& angles);
    void DrawESPBox(int x1, int y1, int x2, int y2, Color_t color);
    void DrawCornerBox(int x1, int y1, int x2, int y2, Color_t color);
    Vec3 GetPositionFromMatrix(const matrix3x4& matrix);
    Vec3 GetAnglesFromMatrix(const matrix3x4& matrix);
    
    // Interpolation functions
    SentryAimData GetInterpolatedAimData(int sentryIndex, const SentryAimData& currentData);
    Vec3 LerpVec3(const Vec3& from, const Vec3& to, float t);
    Vec3 LerpAngles(const Vec3& from, const Vec3& to, float t);
    
public:
    // Chams tracking for sentries (following StickyESP pattern)
    std::unordered_map<int, bool> m_mEntities;
    
    void Draw();
    void UpdateChamsEntities(); // Separate function to populate chams entities
};

ADD_FEATURE(CSentryESP, SentryESP)