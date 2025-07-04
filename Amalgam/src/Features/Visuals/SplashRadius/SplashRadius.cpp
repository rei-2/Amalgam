#include "SplashRadius.h"

void CSplashRadius::Draw()
{
    // Check if feature is enabled
    if (!Vars::Competitive::Features::SplashRadius.Value)
        return;
    
    // Early exits (following existing patterns)
    if (I::EngineVGui->IsGameUIVisible())
        return;
    
    auto pLocal = H::Entities.GetLocal();
    if (!pLocal)
        return;
    
    // Find all stickybombs (CTFGrenadePipebombProjectile)
    for (auto pEntity : H::Entities.GetGroup(EGroupType::WORLD_PROJECTILES))
    {
        if (!pEntity || pEntity->GetClassID() != ETFClassID::CTFGrenadePipebombProjectile)
            continue;
        
        if (pEntity->IsDormant())
            continue;
        
        auto pSticky = pEntity->As<CTFGrenadePipebombProjectile>();
        if (!pSticky)
            continue;
        
        // Get the damage radius from the sticky bomb
        float radius = pSticky->m_DmgRadius();
        if (radius <= 0.0f)
            continue;
        
        // Enemy filter (only show enemy stickies if enabled)
        if (Vars::Competitive::SplashRadius::EnemyOnly.Value && pSticky->m_iTeamNum() == pLocal->m_iTeamNum())
            continue;
        
        // Get the position of the sticky bomb
        Vec3 pos = pSticky->GetAbsOrigin();
        
        // Draw 3D circle on the ground (normal vector pointing up)
        Vec3 normal = {0.0f, 0.0f, 1.0f};
        Draw3DCircle(pos, radius, normal, Vars::Competitive::SplashRadius::CircleSegments.Value);
    }
}

Vec3 CSplashRadius::CrossProduct(const Vec3& a, const Vec3& b)
{
    return Vec3(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

void CSplashRadius::Draw3DCircle(const Vec3& center, float radius, const Vec3& normal, int segments)
{
    // Normalize the normal vector
    float length = std::sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
    if (length == 0.0f)
        return;
    
    Vec3 normalizedNormal = {normal.x / length, normal.y / length, normal.z / length};
    
    // Create two perpendicular vectors in the plane of the circle
    // First, find a vector not parallel to the normal
    Vec3 v1;
    if (std::abs(normalizedNormal.x) < std::abs(normalizedNormal.y) && std::abs(normalizedNormal.x) < std::abs(normalizedNormal.z))
    {
        v1 = {1.0f, 0.0f, 0.0f};
    }
    else if (std::abs(normalizedNormal.y) < std::abs(normalizedNormal.z))
    {
        v1 = {0.0f, 1.0f, 0.0f};
    }
    else
    {
        v1 = {0.0f, 0.0f, 1.0f};
    }
    
    // Create first basis vector using cross product
    Vec3 u = CrossProduct(normalizedNormal, v1);
    float uLength = std::sqrt(u.x * u.x + u.y * u.y + u.z * u.z);
    if (uLength == 0.0f)
        return;
    
    u = {u.x / uLength, u.y / uLength, u.z / uLength};
    
    // Create second basis vector using cross product
    Vec3 v = CrossProduct(normalizedNormal, u);
    
    // Generate points around the circle
    float angleStep = 2.0f * PI / segments;
    
    Vec3 prevPoint;
    Vec3 prevScreen;
    bool prevValid = false;
    
    for (int i = 0; i <= segments; ++i)
    {
        float angle = i * angleStep;
        
        // Calculate the point on the circle
        Vec3 point = {
            center.x + radius * (u.x * std::cos(angle) + v.x * std::sin(angle)),
            center.y + radius * (u.y * std::cos(angle) + v.y * std::sin(angle)),
            center.z + radius * (u.z * std::cos(angle) + v.z * std::sin(angle))
        };
        
        // Project to 2D screen coordinates
        Vec3 screenPos;
        if (SDK::W2S(point, screenPos))
        {
            if (prevValid)
            {
                // Draw line segment from previous point to current point
                H::Draw.Line(
                    static_cast<int>(prevScreen.x), static_cast<int>(prevScreen.y),
                    static_cast<int>(screenPos.x), static_cast<int>(screenPos.y),
                    Vars::Competitive::SplashRadius::CircleColor.Value
                );
            }
            
            prevPoint = point;
            prevScreen = screenPos;
            prevValid = true;
        }
        else
        {
            prevValid = false;
        }
    }
}