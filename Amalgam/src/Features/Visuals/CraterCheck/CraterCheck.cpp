#include "CraterCheck.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void CCraterCheck::Draw()
{
    // Check if feature is enabled
    if (!Vars::Competitive::Features::CraterCheck.Value)
        return;
    
    // Early exits
    if (I::EngineVGui->IsGameUIVisible())
        return;
    
    auto pLocal = H::Entities.GetLocal();
    if (!pLocal || !pLocal->IsAlive())
        return;
    
    // Only show when airborne
    if (!IsPlayerAirborne(pLocal))
        return;
    
    // Get aim trace result
    Vec3 hitPos, hitNormal;
    if (!GetAimTraceResult(pLocal, hitPos, hitNormal))
        return;
    
    // Only show on surfaces that are relatively horizontal (landable)
    // Check if the surface normal is pointing mostly upward
    float dotProduct = hitNormal.z; // Dot product with up vector (0,0,1)
    
    // Only show on surfaces with at least 60 degrees from vertical (cos(60°) = 0.5)
    // This filters out walls and steep slopes
    if (dotProduct < 0.5f)
        return;
    
    // Calculate actual landing velocity by predicting fall distance
    Vec3 playerPos = pLocal->GetAbsOrigin();
    float currentFallVelocity = std::abs(pLocal->m_vecVelocity().z);
    
    // Calculate distance to landing spot
    float distanceToLanding = playerPos.z - hitPos.z;
    
    // Use physics formula to calculate final velocity: v² = u² + 2as
    // where: v = final velocity, u = initial velocity, a = gravity (800 units/s²), s = distance
    float gravity = 800.0f; // TF2's gravity constant
    float finalVelocitySquared = (currentFallVelocity * currentFallVelocity) + (2 * gravity * distanceToLanding);
    float predictedLandingVelocity = std::sqrt(std::max(0.0f, finalVelocitySquared));
    
    // Calculate potential fall damage at landing spot using predicted velocity
    float fallDamage = CalculateFallDamage(pLocal, predictedLandingVelocity);
    bool willDie = (fallDamage >= pLocal->m_iHealth());
    
    // If "only show lethal" is enabled, skip safe landings
    if (Vars::Competitive::CraterCheck::OnlyShowLethal.Value && !willDie)
        return;
    
    // Draw the crater polygon at the aim point
    float radius = Vars::Competitive::CraterCheck::Radius.Value;
    int segments = Vars::Competitive::CraterCheck::Segments.Value;
    
    Draw3DPolygon(hitPos, radius, hitNormal, segments, willDie);
}

bool CCraterCheck::IsPlayerAirborne(CTFPlayer* pLocal)
{
    if (!pLocal)
        return false;
    
    // Check if player has ground entity
    CBaseEntity* pGroundEntity = pLocal->m_hGroundEntity().Get();
    if (pGroundEntity)
        return false;
    
    // Get fall velocity (absolute value of downward velocity)
    float zVelocity = pLocal->m_vecVelocity().z;
    float fallVelocity = std::abs(zVelocity);
    
    // Only filter out tiny jumps - regular jump velocity is around 270 units/sec
    // We want to show for rocket jumps and meaningful falls, but not small hops
    if (zVelocity >= -100.0f) // Not falling at all or very slow
        return false;
    
    // Additional height check: trace downward to see how high we are
    Vec3 playerPos = pLocal->GetAbsOrigin();
    Vec3 downPos = playerPos + Vec3(0, 0, -2000.0f); // Trace 2000 units down
    
    CGameTrace trace = {};
    CTraceFilterWorldAndPropsOnly filter = {};
    SDK::Trace(playerPos, downPos, MASK_SOLID, &filter, &trace);
    
    if (trace.DidHit())
    {
        float heightAboveGround = playerPos.z - trace.endpos.z;
        
        // Only filter out being very close to ground (small steps/jumps)
        // 72 units is roughly a player's height, so filter anything lower
        if (heightAboveGround < 72.0f)
            return false;
    }
    
    return true;
}

bool CCraterCheck::GetAimTraceResult(CTFPlayer* pLocal, Vec3& hitPos, Vec3& hitNormal)
{
    if (!pLocal)
        return false;
    
    // Get view angles and position
    Vec3 vForward;
    Math::AngleVectors(I::EngineClient->GetViewAngles(), &vForward);
    
    Vec3 vStart = pLocal->GetEyePosition();
    Vec3 vEnd = vStart + (vForward * 8192.0f);
    
    // Trace from eye position along view direction
    CGameTrace trace = {};
    CTraceFilterWorldAndPropsOnly filter = {};
    SDK::Trace(vStart, vEnd, MASK_SOLID, &filter, &trace);
    
    if (!trace.DidHit())
        return false;
    
    hitPos = trace.endpos;
    hitNormal = trace.plane.normal;
    
    return true;
}

float CCraterCheck::CalculateFallDamage(CTFPlayer* pLocal, float fallVelocity)
{
    if (!pLocal || fallVelocity <= 650.0f)
        return 0.0f;
    
    // Get max health
    int maxHealth = pLocal->GetMaxHealth();
    
    // Calculate fall damage using TF2's formula
    // 5 * (fall_velocity / 300) * (max_health / 100)
    float fallDamage = fallVelocity * float(maxHealth) / 6000.0f;
    
    return fallDamage;
}

void CCraterCheck::Draw3DPolygon(const Vec3& center, float radius, const Vec3& normal, int segments, bool willDie)
{
    // Get colors based on lethality
    Color_t fillColor, edgeColor;
    
    if (willDie)
    {
        fillColor = Vars::Competitive::CraterCheck::LethalFillColor.Value;
        edgeColor = Vars::Competitive::CraterCheck::LethalEdgeColor.Value;
    }
    else
    {
        fillColor = Vars::Competitive::CraterCheck::SafeFillColor.Value;
        edgeColor = Vars::Competitive::CraterCheck::SafeEdgeColor.Value;
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
    if (Vars::Competitive::CraterCheck::ShowFill.Value)
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
            
            if (Vars::Competitive::CraterCheck::ShowFill.Value)
            {
                vertices.emplace_back(Vertex_t({ { screenPos.x, screenPos.y } }));
            }
        }
    }
    
    // Draw filled polygon if enabled
    if (Vars::Competitive::CraterCheck::ShowFill.Value && vertices.size() >= 3)
    {
        H::Draw.FillPolygon(vertices, fillColor);
    }
    
    // Draw edge lines if enabled
    if (Vars::Competitive::CraterCheck::ShowEdge.Value && edgeScreenPoints.size() >= 2)
    {
        int edgeWidth = Vars::Competitive::CraterCheck::EdgeWidth.Value;
        
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
                // Thick line drawing
                for (int w = 0; w < edgeWidth; ++w)
                {
                    for (int h = 0; h < edgeWidth; ++h)
                    {
                        H::Draw.Line(static_cast<int>(start.x) + w, static_cast<int>(start.y) + h,
                                   static_cast<int>(end.x) + w, static_cast<int>(end.y) + h, edgeColor);
                    }
                }
            }
        }
    }
}

Vec3 CCraterCheck::CrossProduct(const Vec3& a, const Vec3& b)
{
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}