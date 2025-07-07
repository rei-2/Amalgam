#include "SplashRadius.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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
    
    // Find all projectiles
    for (auto pEntity : H::Entities.GetGroup(EGroupType::WORLD_PROJECTILES))
    {
        if (!pEntity || pEntity->IsDormant())
            continue;
            
        if (!ShouldShowProjectile(pEntity))
            continue;
        
        // Get the damage radius
        float radius = 0.0f;
        ETFClassID classID = pEntity->GetClassID();
        
        if (classID == ETFClassID::CTFGrenadePipebombProjectile)
        {
            auto pSticky = pEntity->As<CTFGrenadePipebombProjectile>();
            if (pSticky)
                radius = pSticky->m_DmgRadius();
        }
        else if (classID == ETFClassID::CTFProjectile_Rocket)
        {
            // Standard rocket radius (approximately 146 units)
            radius = 146.0f;
        }
        
        if (radius <= 0.0f)
            continue;
        
        // Enemy filter (only show enemy projectiles if enabled)
        if (Vars::Competitive::SplashRadius::EnemyOnly.Value && pEntity->m_iTeamNum() == pLocal->m_iTeamNum())
            continue;
        
        // Get the position of the projectile
        Vec3 pos = pEntity->GetAbsOrigin();
        
        // Draw 3D polygon on the ground (normal vector pointing up)
        Vec3 normal = {0.0f, 0.0f, 1.0f};
        Draw3DPolygon(pos, radius, normal, Vars::Competitive::SplashRadius::Segments.Value);
    }
}

bool CSplashRadius::ShouldShowProjectile(CBaseEntity* pEntity)
{
    ETFClassID classID = pEntity->GetClassID();
    
    if (classID == ETFClassID::CTFGrenadePipebombProjectile)
        return Vars::Competitive::SplashRadius::ShowPipebombs.Value;
    else if (classID == ETFClassID::CTFProjectile_Rocket)
        return Vars::Competitive::SplashRadius::ShowRockets.Value;
    
    return false;
}

Vec3 CSplashRadius::CrossProduct(const Vec3& a, const Vec3& b)
{
    return Vec3(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

void CSplashRadius::Draw3DPolygon(const Vec3& center, float radius, const Vec3& normal, int segments)
{
    // Normalize the normal vector
    float length = std::sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
    if (length == 0.0f)
        return;
    
    Vec3 normalizedNormal = {normal.x / length, normal.y / length, normal.z / length};
    
    // Create two perpendicular vectors in the plane of the circle
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
    float angleStep = 2.0f * M_PI / segments;
    std::vector<Vertex_t> vertices;
    
    // Add center point for filled polygon
    if (Vars::Competitive::SplashRadius::ShowFill.Value)
    {
        Vec3 centerScreen;
        if (SDK::W2S(center, centerScreen))
        {
            vertices.emplace_back(Vertex_t({ { centerScreen.x, centerScreen.y } }));
        }
    }
    
    // Generate edge points
    std::vector<Vec3> edgePoints;
    std::vector<Vec3> edgeScreenPoints;
    
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
            edgePoints.push_back(point);
            edgeScreenPoints.push_back(screenPos);
            
            if (Vars::Competitive::SplashRadius::ShowFill.Value)
            {
                vertices.emplace_back(Vertex_t({ { screenPos.x, screenPos.y } }));
            }
        }
    }
    
    // Draw filled polygon if enabled
    if (Vars::Competitive::SplashRadius::ShowFill.Value && vertices.size() >= 3)
    {
        H::Draw.FillPolygon(vertices, Vars::Competitive::SplashRadius::FillColor.Value);
    }
    
    // Draw edge lines if enabled
    if (Vars::Competitive::SplashRadius::ShowEdge.Value && edgeScreenPoints.size() >= 2)
    {
        int edgeWidth = Vars::Competitive::SplashRadius::EdgeWidth.Value;
        Color_t edgeColor = Vars::Competitive::SplashRadius::EdgeColor.Value;
        
        for (size_t i = 0; i < edgeScreenPoints.size() - 1; ++i)
        {
            Vec3 start = edgeScreenPoints[i];
            Vec3 end = edgeScreenPoints[i + 1];
            
            // Draw line with configurable width
            int halfWidth = edgeWidth / 2;
            for (int w = -halfWidth; w <= halfWidth; w++)
            {
                for (int h = -halfWidth; h <= halfWidth; h++)
                {
                    if (w == 0 && h == 0)
                    {
                        // Main line
                        H::Draw.Line(static_cast<int>(start.x), static_cast<int>(start.y),
                                   static_cast<int>(end.x), static_cast<int>(end.y), edgeColor);
                    }
                    else if (edgeWidth > 1)
                    {
                        // Offset lines for thickness
                        H::Draw.Line(static_cast<int>(start.x + w), static_cast<int>(start.y + h),
                                   static_cast<int>(end.x + w), static_cast<int>(end.y + h), edgeColor);
                    }
                }
            }
        }
    }
}