#include "SplashRadius.h"
#include <algorithm>

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
        
        // Check visibility if "Show only visible" is enabled
        if (Vars::Competitive::SplashRadius::ShowOnlyVisible.Value)
        {
            Vec3 entityPos = pEntity->GetAbsOrigin();
            Vec3 localPos = pLocal->GetAbsOrigin();
            Vec3 eyePos = localPos + pLocal->m_vecViewOffset();
            
            // Perform line of sight check
            CGameTrace trace = {};
            CTraceFilterHitscan filter = {};
            filter.pSkip = pLocal;
            SDK::Trace(eyePos, entityPos, MASK_VISIBLE, &filter, &trace);
            
            // More lenient visibility check (like HealthBarESP)
            bool isVisible = (trace.fraction > 0.90f || trace.m_pEnt == pEntity);
            if (!isVisible)
                continue;
        }
        
        // Get the damage radius and position
        float radius = 0.0f;
        Vec3 circlePosition = pEntity->GetAbsOrigin();
        Vec3 surfaceNormal = {0.0f, 0.0f, 1.0f}; // Default to horizontal
        ETFClassID classID = pEntity->GetClassID();
        
        if (classID == ETFClassID::CTFGrenadePipebombProjectile)
        {
            auto pSticky = pEntity->As<CTFGrenadePipebombProjectile>();
            if (pSticky)
                radius = pSticky->m_DmgRadius();
        }
        else if (classID == ETFClassID::CTFProjectile_Rocket)
        {
            // Dynamic rocket radius that grows as rocket approaches impact point
            auto pRocket = pEntity->As<CTFBaseRocket>();
            if (pRocket)
            {
                Vec3 rocketPos = pEntity->GetAbsOrigin();
                Vec3 rocketVel = pRocket->m_vInitialVelocity();
                
                // Validate rocket velocity
                float velLength = rocketVel.Length();
                if (velLength < 1.0f) // Invalid or zero velocity
                {
                    radius = 146.0f; // Use standard radius without prediction
                    // Continue with default positioning and normal
                }
                else
                {
                    // Calculate impact point by tracing rocket's path
                CGameTrace trace = {};
                CTraceFilterWorldAndPropsOnly filter = {};
                
                // Calculate end position far along the rocket's trajectory
                Vec3 endPos = rocketPos + (rocketVel.Normalized() * 4000.0f); // Trace far ahead
                SDK::Trace(rocketPos, endPos, MASK_SOLID, &filter, &trace);
                
                if (trace.fraction < 1.0f)
                {
                    Vec3 impactPos = trace.endpos;
                    
                    // Calculate distance from rocket to impact point
                    float distanceToImpact = (impactPos - rocketPos).Length();
                    
                    // Base radius is 146 units, scale from 20% to 100% based on proximity to impact
                    float maxRadius = 146.0f;
                    float minRadius = maxRadius * 0.2f; // Start at 20% size
                    
                    // As rocket gets closer to impact, radius grows
                    // Use a reasonable max distance for scaling (e.g., 2000 units)
                    float maxDistance = 2000.0f;
                    float normalizedDistance = std::min(distanceToImpact / maxDistance, 1.0f);
                    
                    // Invert so close distance = large radius
                    float radiusScale = 1.0f - normalizedDistance;
                    radius = minRadius + (maxRadius - minRadius) * radiusScale;
                    
                    // Use impact position for circle instead of rocket position
                    circlePosition = impactPos;
                    // Store the surface normal for proper circle orientation
                    Vector traceNormal = trace.plane.normal;
                    // Convert from Vector to Vec3 and ensure it's valid
                    surfaceNormal = {traceNormal.x, traceNormal.y, traceNormal.z};
                    
                    // Validate normal - if invalid, use default horizontal
                    float normalLength = sqrt(surfaceNormal.x * surfaceNormal.x + 
                                            surfaceNormal.y * surfaceNormal.y + 
                                            surfaceNormal.z * surfaceNormal.z);
                    if (normalLength < 0.1f) // Invalid normal
                    {
                        surfaceNormal = {0.0f, 0.0f, 1.0f}; // Default horizontal
                    }
                    else
                    {
                        // Normalize the surface normal
                        surfaceNormal.x /= normalLength;
                        surfaceNormal.y /= normalLength;
                        surfaceNormal.z /= normalLength;
                        
                    }
                }
                else
                {
                    // No impact found, use standard radius
                    radius = 146.0f;
                }
                } // End velocity validation else block
            }
        }
        
        if (radius <= 0.0f)
            continue;
        
        // Enemy filter (only show enemy projectiles if enabled)
        if (Vars::Competitive::SplashRadius::EnemyOnly.Value && pEntity->m_iTeamNum() == pLocal->m_iTeamNum())
            continue;
        
        // Add to circles collection
        SplashCircle circle;
        circle.Position = circlePosition;
        circle.Radius = radius;
        circle.Team = pEntity->m_iTeamNum();
        circle.Entity = pEntity;
        circle.Normal = surfaceNormal;
        
        allCircles.push_back(circle);
    }
    
    if (allCircles.empty())
        return;
    
    // Separate circles into mergeable (grenades) and individual (rockets)
    std::vector<SplashCircle> mergeableCircles, individualCircles;
    
    for (const auto& circle : allCircles)
    {
        // Rockets should always be drawn individually to respect surface normals
        if (circle.Entity && circle.Entity->GetClassID() == ETFClassID::CTFProjectile_Rocket)
        {
            individualCircles.push_back(circle);
        }
        else
        {
            mergeableCircles.push_back(circle);
        }
    }
    
    // Group mergeable circles by team and merge if enabled
    if (Vars::Competitive::SplashRadius::MergeOverlapping.Value && !mergeableCircles.empty())
    {
        // Separate by team
        std::vector<SplashCircle> redCircles, bluCircles;
        for (const auto& circle : mergeableCircles)
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
    
    // Always draw individual circles (rockets and unmerged grenades)
    std::vector<SplashCircle> circlesToDrawIndividually = individualCircles;
    
    // If merging is disabled, draw all mergeable circles individually too
    if (!Vars::Competitive::SplashRadius::MergeOverlapping.Value)
    {
        circlesToDrawIndividually.insert(circlesToDrawIndividually.end(), 
                                       mergeableCircles.begin(), mergeableCircles.end());
    }
    
    // Draw individual circles (rockets always, grenades if merging disabled)
    for (const auto& circle : circlesToDrawIndividually)
    {
        int segments = Vars::Competitive::SplashRadius::Segments.Value;
        if (Vars::Competitive::SplashRadius::UseLOD.Value)
        {
            segments = ComputeLODSegments(circle.Position, playerPos);
        }
        
        Draw3DPolygon(circle.Position, circle.Radius, circle.Normal, segments, circle.Team);
    }
}

bool CSplashRadius::ShouldShowProjectile(CBaseEntity* pEntity)
{
    ETFClassID classID = pEntity->GetClassID();
    
    if (classID == ETFClassID::CTFGrenadePipebombProjectile)
    {
        auto pGrenade = pEntity->As<CTFGrenadePipebombProjectile>();
        if (pGrenade)
        {
            // Distinguish between pipebombs and stickybombs using m_iType
            if (pGrenade->HasStickyEffects()) // TF_GL_MODE_REMOTE_DETONATE or TF_GL_MODE_REMOTE_DETONATE_PRACTICE
                return Vars::Competitive::SplashRadius::ShowStickybombs.Value;
            else // TF_GL_MODE_REGULAR (pipebombs)
                return Vars::Competitive::SplashRadius::ShowPipebombs.Value;
        }
    }
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
    
    // Collect all outer boundary points from all circles
    std::vector<Vec3> allPoints;
    
    for (const auto& circle : group.Circles)
    {
        // Generate points for this circle
        for (int i = 0; i <= segments; ++i)
        {
            float worldX = circle.Position.x + m_CosCache[i] * circle.Radius;
            float worldY = circle.Position.y + m_SinCache[i] * circle.Radius;
            float worldZ = circle.Position.z;
            Vec3 worldPoint = {worldX, worldY, worldZ};
            
            // Only include points that are NOT inside other circles (outer boundary)
            bool isOuterBoundary = !IsPointInsideOtherCircles(worldPoint, group.Circles, &circle);
            
            if (isOuterBoundary)
            {
                Vec3 screenPos;
                if (SDK::W2S(worldPoint, screenPos))
                {
                    allPoints.push_back(screenPos);
                }
            }
        }
    }
    
    if (allPoints.empty())
        return;
    
    // Sort points to form a proper perimeter using convex hull approach
    std::vector<Vec3> hull = ComputeConvexHull(allPoints);
    
    if (hull.size() < 3)
        return;
    
    // Draw filled polygon if enabled
    if (Vars::Competitive::SplashRadius::ShowFill.Value)
    {
        std::vector<Vertex_t> fillVertices;
        for (const auto& point : hull)
        {
            fillVertices.emplace_back(Vertex_t({ { point.x, point.y } }));
        }
        H::Draw.FillPolygon(fillVertices, fillColor);
    }
    
    // Draw edge lines if enabled
    if (Vars::Competitive::SplashRadius::ShowEdge.Value)
    {
        int edgeWidth = Vars::Competitive::SplashRadius::EdgeWidth.Value;
        
        // Draw lines between consecutive hull points
        for (size_t i = 0; i < hull.size(); ++i)
        {
            size_t nextIdx = (i + 1) % hull.size(); // Wrap around to close the polygon
            Vec3 start = hull[i];
            Vec3 end = hull[nextIdx];
            
            if (edgeWidth <= 1)
            {
                // Single line for width 1
                H::Draw.Line(static_cast<int>(start.x), static_cast<int>(start.y),
                           static_cast<int>(end.x), static_cast<int>(end.y), edgeColor);
            }
            else
            {
                // Efficient thick line drawing
                Vec3 direction = { end.x - start.x, end.y - start.y, 0 };
                float length = sqrt(direction.x * direction.x + direction.y * direction.y);
                
                if (length > 0)
                {
                    // Normalize and get perpendicular vector
                    Vec3 perpendicular = { -direction.y / length, direction.x / length, 0 };
                    
                    int halfWidth = edgeWidth / 2;
                    
                    // Draw parallel lines for thickness
                    for (int offset = -halfWidth; offset <= halfWidth; offset++)
                    {
                        float offsetX = perpendicular.x * offset;
                        float offsetY = perpendicular.y * offset;
                        
                        H::Draw.Line(static_cast<int>(start.x + offsetX), static_cast<int>(start.y + offsetY),
                                   static_cast<int>(end.x + offsetX), static_cast<int>(end.y + offsetY), edgeColor);
                    }
                }
                else
                {
                    // Fallback for zero-length lines
                    H::Draw.Line(static_cast<int>(start.x), static_cast<int>(start.y),
                               static_cast<int>(end.x), static_cast<int>(end.y), edgeColor);
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

void CSplashRadius::Draw3DPolygon(const Vec3& center, float radius, const Vec3& normal, int segments, int team)
{
    // Get colors
    Color_t fillColor = Vars::Competitive::SplashRadius::FillColor.Value;
    Color_t edgeColor = Vars::Competitive::SplashRadius::EdgeColor.Value;
    
    // Apply team colors if enabled
    if (Vars::Competitive::SplashRadius::TeamColors.Value)
    {
        if (team == 2) // RED
        {
            fillColor = {255, 0, 0, static_cast<byte>(fillColor.a)};
            edgeColor = {255, 0, 0, static_cast<byte>(edgeColor.a)};
        }
        else if (team == 3) // BLU
        {
            fillColor = {3, 219, 252, static_cast<byte>(fillColor.a)};
            edgeColor = {3, 219, 252, static_cast<byte>(edgeColor.a)};
        }
    }
    
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
            
            if (edgeWidth <= 1)
            {
                // Single line for width 1
                H::Draw.Line(static_cast<int>(start.x), static_cast<int>(start.y),
                           static_cast<int>(end.x), static_cast<int>(end.y), edgeColor);
            }
            else
            {
                // Efficient thick line drawing - draw parallel lines instead of nested loops
                Vec3 direction = { end.x - start.x, end.y - start.y, 0 };
                float length = sqrt(direction.x * direction.x + direction.y * direction.y);
                
                if (length > 0)
                {
                    // Normalize and get perpendicular vector
                    Vec3 perpendicular = { -direction.y / length, direction.x / length, 0 };
                    
                    int halfWidth = edgeWidth / 2;
                    
                    // Draw parallel lines for thickness
                    for (int offset = -halfWidth; offset <= halfWidth; offset++)
                    {
                        float offsetX = perpendicular.x * offset;
                        float offsetY = perpendicular.y * offset;
                        
                        H::Draw.Line(static_cast<int>(start.x + offsetX), static_cast<int>(start.y + offsetY),
                                   static_cast<int>(end.x + offsetX), static_cast<int>(end.y + offsetY), edgeColor);
                    }
                }
                else
                {
                    // Fallback for zero-length lines
                    H::Draw.Line(static_cast<int>(start.x), static_cast<int>(start.y),
                               static_cast<int>(end.x), static_cast<int>(end.y), edgeColor);
                }
            }
        }
    }
}

std::vector<Vec3> CSplashRadius::ComputeConvexHull(std::vector<Vec3> points)
{
    if (points.size() < 3)
        return points;
    
    // Sort points lexicographically (by x, then by y)
    std::sort(points.begin(), points.end(), [](const Vec3& a, const Vec3& b) {
        return a.x < b.x || (a.x == b.x && a.y < b.y);
    });
    
    // Remove duplicate points
    points.erase(std::unique(points.begin(), points.end(), [](const Vec3& a, const Vec3& b) {
        return std::abs(a.x - b.x) < 1.0f && std::abs(a.y - b.y) < 1.0f;
    }), points.end());
    
    if (points.size() < 3)
        return points;
    
    // Build lower hull
    std::vector<Vec3> hull;
    for (const auto& p : points)
    {
        while (hull.size() >= 2)
        {
            // Check if we need to remove the last point (cross product test)
            Vec3 a = hull[hull.size() - 2];
            Vec3 b = hull[hull.size() - 1];
            Vec3 c = p;
            
            // Cross product: (b-a) x (c-a)
            float cross = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
            if (cross <= 0)
                hull.pop_back();
            else
                break;
        }
        hull.push_back(p);
    }
    
    // Build upper hull
    int lowerHullSize = static_cast<int>(hull.size());
    for (int i = static_cast<int>(points.size()) - 2; i >= 0; i--)
    {
        const auto& p = points[i];
        while (hull.size() > lowerHullSize)
        {
            // Check if we need to remove the last point
            Vec3 a = hull[hull.size() - 2];
            Vec3 b = hull[hull.size() - 1];
            Vec3 c = p;
            
            // Cross product: (b-a) x (c-a)
            float cross = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
            if (cross <= 0)
                hull.pop_back();
            else
                break;
        }
        hull.push_back(p);
    }
    
    // Remove the last point (it's the same as the first)
    if (hull.size() > 1)
        hull.pop_back();
    
    return hull;
}