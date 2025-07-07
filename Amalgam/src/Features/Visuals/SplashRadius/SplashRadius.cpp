#include "SplashRadius.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// LOD distance constants (matching Lua module)
static constexpr float FULL_RES = 500.0f;
static constexpr float HALF_RES = 900.0f;
static constexpr float EVEN_LOWER_RES = 1600.0f;
static constexpr int MINIMUM_SEGMENTS = 4;

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
    
    Vec3 playerPos = pLocal->GetAbsOrigin();
    
    // Collect all splash circles
    std::vector<SplashCircle> allCircles;
    
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
        
        // Add to circles collection
        SplashCircle circle;
        circle.Position = pEntity->GetAbsOrigin();
        circle.Radius = radius;
        circle.Team = pEntity->m_iTeamNum();
        circle.Entity = pEntity;
        
        allCircles.push_back(circle);
    }
    
    if (allCircles.empty())
        return;
    
    // Group circles by team and merge if enabled
    if (Vars::Competitive::SplashRadius::MergeOverlapping.Value)
    {
        // Separate by team
        std::vector<SplashCircle> redCircles, bluCircles;
        for (const auto& circle : allCircles)
        {
            if (circle.Team == 2) // RED
                redCircles.push_back(circle);
            else if (circle.Team == 3) // BLU
                bluCircles.push_back(circle);
        }
        
        // Create merged groups for each team
        auto redGroups = CreateMergedGroups(redCircles);
        auto bluGroups = CreateMergedGroups(bluCircles);
        
        // Draw red team groups
        for (const auto& group : redGroups)
        {
            int segments = Vars::Competitive::SplashRadius::Segments.Value;
            if (Vars::Competitive::SplashRadius::UseLOD.Value && !group.Circles.empty())
            {
                segments = ComputeLODSegments(group.Circles[0].Position, playerPos);
            }
            
            DrawMergedGroup(group, segments);
        }
        
        // Draw blue team groups
        for (const auto& group : bluGroups)
        {
            int segments = Vars::Competitive::SplashRadius::Segments.Value;
            if (Vars::Competitive::SplashRadius::UseLOD.Value && !group.Circles.empty())
            {
                segments = ComputeLODSegments(group.Circles[0].Position, playerPos);
            }
            
            DrawMergedGroup(group, segments);
        }
    }
    else
    {
        // Draw individual circles without merging
        for (const auto& circle : allCircles)
        {
            int segments = Vars::Competitive::SplashRadius::Segments.Value;
            if (Vars::Competitive::SplashRadius::UseLOD.Value)
            {
                segments = ComputeLODSegments(circle.Position, playerPos);
            }
            
            Vec3 normal = {0.0f, 0.0f, 1.0f};
            Draw3DPolygon(circle.Position, circle.Radius, normal, segments);
        }
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

float CSplashRadius::Distance2D(const Vec3& pos1, const Vec3& pos2)
{
    float dx = pos1.x - pos2.x;
    float dy = pos1.y - pos2.y;
    return std::sqrt(dx * dx + dy * dy);
}

bool CSplashRadius::IsPointInsideOtherCircles(const Vec3& point, const std::vector<SplashCircle>& circles, const SplashCircle* exclude)
{
    for (const auto& circle : circles)
    {
        if (&circle != exclude)
        {
            float dist = Distance2D(point, circle.Position);
            if (dist < circle.Radius * 0.95f) // 0.95 threshold like Lua
            {
                return true;
            }
        }
    }
    return false;
}

std::vector<CircleGroup> CSplashRadius::CreateMergedGroups(const std::vector<SplashCircle>& circles)
{
    std::vector<CircleGroup> groups;
    std::vector<bool> used(circles.size(), false);
    
    for (size_t i = 0; i < circles.size(); ++i)
    {
        if (used[i])
            continue;
        
        CircleGroup group;
        group.Circles.push_back(circles[i]);
        group.Team = circles[i].Team;
        used[i] = true;
        
        // Find all overlapping circles
        for (size_t j = i + 1; j < circles.size(); ++j)
        {
            if (used[j])
                continue;
            
            const auto& circle1 = circles[i];
            const auto& circle2 = circles[j];
            
            float dist = Distance2D(circle1.Position, circle2.Position);
            float combinedRadius = circle1.Radius + circle2.Radius;
            
            // Check if circles overlap
            if (dist <= combinedRadius)
            {
                group.Circles.push_back(circle2);
                used[j] = true;
            }
        }
        
        groups.push_back(group);
    }
    
    return groups;
}

int CSplashRadius::ComputeLODSegments(const Vec3& position, const Vec3& playerPos)
{
    float distance = Distance2D(position, playerPos);
    int maxSegments = Vars::Competitive::SplashRadius::Segments.Value;
    
    if (distance <= FULL_RES)
        return maxSegments;
    else if (distance <= HALF_RES)
        return static_cast<int>(maxSegments * 0.5f);
    else if (distance <= EVEN_LOWER_RES)
        return static_cast<int>(maxSegments * 0.25f);
    else
        return MINIMUM_SEGMENTS;
}

void CSplashRadius::GenerateTrigCache(int segments)
{
    if (m_LastSegmentCount == segments)
        return; // Cache still valid
    
    m_CosCache.clear();
    m_SinCache.clear();
    m_CosCache.reserve(segments + 1);
    m_SinCache.reserve(segments + 1);
    
    for (int i = 0; i <= segments; ++i)
    {
        float angle = (static_cast<float>(i) / segments) * 2.0f * M_PI;
        m_CosCache.push_back(std::cos(angle));
        m_SinCache.push_back(std::sin(angle));
    }
    
    m_LastSegmentCount = segments;
}

void CSplashRadius::DrawMergedGroup(const CircleGroup& group, int segments)
{
    if (group.Circles.empty())
        return;
    
    // Generate trig cache for performance
    GenerateTrigCache(segments);
    
    // Get colors based on team or configuration
    Color_t fillColor = Vars::Competitive::SplashRadius::FillColor.Value;
    Color_t edgeColor = Vars::Competitive::SplashRadius::EdgeColor.Value;
    
    if (Vars::Competitive::SplashRadius::TeamColors.Value)
    {
        if (group.Team == 2) // RED
        {
            fillColor = {255, 0, 0, static_cast<byte>(fillColor.a)};
            edgeColor = {255, 0, 0, static_cast<byte>(edgeColor.a)};
        }
        else if (group.Team == 3) // BLU
        {
            fillColor = {3, 219, 252, static_cast<byte>(fillColor.a)};
            edgeColor = {3, 219, 252, static_cast<byte>(edgeColor.a)};
        }
    }
    
    // Draw each circle in the group, but only visible portions
    for (const auto& circle : group.Circles)
    {
        std::vector<Vertex_t> fillVertices;
        std::vector<Vec3> edgePoints;
        
        // Add center point for filled polygon if enabled
        if (Vars::Competitive::SplashRadius::ShowFill.Value)
        {
            Vec3 centerScreen;
            if (SDK::W2S(circle.Position, centerScreen))
            {
                fillVertices.emplace_back(Vertex_t({ { centerScreen.x, centerScreen.y } }));
            }
        }
        
        // Generate edge points, but only add visible ones
        for (int i = 0; i <= segments; ++i)
        {
            float worldX = circle.Position.x + m_CosCache[i] * circle.Radius;
            float worldY = circle.Position.y + m_SinCache[i] * circle.Radius;
            float worldZ = circle.Position.z;
            Vec3 worldPoint = {worldX, worldY, worldZ};
            
            // Check if this point is visible (not inside other circles)
            bool isVisible = !IsPointInsideOtherCircles(worldPoint, group.Circles, &circle);
            
            Vec3 screenPos;
            if (isVisible && SDK::W2S(worldPoint, screenPos))
            {
                edgePoints.push_back(screenPos);
                
                if (Vars::Competitive::SplashRadius::ShowFill.Value)
                {
                    fillVertices.emplace_back(Vertex_t({ { screenPos.x, screenPos.y } }));
                }
            }
        }
        
        // Draw filled polygon if enabled
        if (Vars::Competitive::SplashRadius::ShowFill.Value && fillVertices.size() >= 3)
        {
            H::Draw.FillPolygon(fillVertices, fillColor);
        }
        
        // Draw edge lines if enabled
        if (Vars::Competitive::SplashRadius::ShowEdge.Value && edgePoints.size() >= 2)
        {
            int edgeWidth = Vars::Competitive::SplashRadius::EdgeWidth.Value;
            
            for (size_t i = 0; i < edgePoints.size() - 1; ++i)
            {
                Vec3 start = edgePoints[i];
                Vec3 end = edgePoints[i + 1];
                
                // Draw line with configurable width
                int halfWidth = edgeWidth / 2;
                for (int w = -halfWidth; w <= halfWidth; w++)
                {
                    for (int h = -halfWidth; h <= halfWidth; h++)
                    {
                        if (w == 0 && h == 0)
                        {
                            H::Draw.Line(static_cast<int>(start.x), static_cast<int>(start.y),
                                       static_cast<int>(end.x), static_cast<int>(end.y), edgeColor);
                        }
                        else if (edgeWidth > 1)
                        {
                            H::Draw.Line(static_cast<int>(start.x + w), static_cast<int>(start.y + h),
                                       static_cast<int>(end.x + w), static_cast<int>(end.y + h), edgeColor);
                        }
                    }
                }
            }
        }
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

void CSplashRadius::Draw3DPolygon(const Vec3& center, float radius, const Vec3& normal, int segments)
{
    // Get colors
    Color_t fillColor = Vars::Competitive::SplashRadius::FillColor.Value;
    Color_t edgeColor = Vars::Competitive::SplashRadius::EdgeColor.Value;
    
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
        H::Draw.FillPolygon(vertices, fillColor);
    }
    
    // Draw edge lines if enabled
    if (Vars::Competitive::SplashRadius::ShowEdge.Value && edgeScreenPoints.size() >= 2)
    {
        int edgeWidth = Vars::Competitive::SplashRadius::EdgeWidth.Value;
        
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
                        H::Draw.Line(static_cast<int>(start.x), static_cast<int>(start.y),
                                   static_cast<int>(end.x), static_cast<int>(end.y), edgeColor);
                    }
                    else if (edgeWidth > 1)
                    {
                        H::Draw.Line(static_cast<int>(start.x + w), static_cast<int>(start.y + h),
                                   static_cast<int>(end.x + w), static_cast<int>(end.y + h), edgeColor);
                    }
                }
            }
        }
    }
}