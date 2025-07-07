#pragma once
#include "../../../SDK/SDK.h"
#include <vector>
#include <cmath>

struct SplashCircle
{
    Vec3 Position;
    float Radius;
    int Team;
    CBaseEntity* Entity;
    Vec3 Normal = {0.0f, 0.0f, 1.0f}; // Surface normal for orientation
};

struct CircleGroup
{
    std::vector<SplashCircle> Circles;
    int Team;
};

class CSplashRadius
{
private:
    // Performance optimization
    std::vector<float> m_CosCache;
    std::vector<float> m_SinCache;
    int m_LastSegmentCount = -1;
    
    // Helper functions
    Vec3 CrossProduct(const Vec3& a, const Vec3& b);
    void Draw3DPolygon(const Vec3& center, float radius, const Vec3& normal, int segments, int team = 0);
    void DrawMergedGroup(const CircleGroup& group, int segments);
    bool ShouldShowProjectile(CBaseEntity* pEntity);
    float Distance2D(const Vec3& pos1, const Vec3& pos2);
    bool IsPointInsideOtherCircles(const Vec3& point, const std::vector<SplashCircle>& circles, const SplashCircle* exclude);
    std::vector<CircleGroup> CreateMergedGroups(const std::vector<SplashCircle>& circles);
    int ComputeLODSegments(const Vec3& position, const Vec3& playerPos);
    void GenerateTrigCache(int segments);
    std::vector<Vec3> ComputeConvexHull(std::vector<Vec3> points);
    
public:
    void Draw();
};

ADD_FEATURE(CSplashRadius, SplashRadius)