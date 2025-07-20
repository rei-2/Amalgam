#include "HealthBarESP.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void CHealthBarESP::Draw()
{
    if (!Vars::Competitive::Features::HealthBarESP.Value)
        return;
    
    // Early exits (match native ESP pattern)
    if (I::EngineVGui->IsGameUIVisible())
        return;
    
    auto pLocal = H::Entities.GetLocal();
    if (!pLocal)
        return;
    
    Vec3 localPos = pLocal->GetAbsOrigin();
    bool isLocalPlayerMedic = (pLocal->m_iClass() == TF_CLASS_MEDIC);
    bool showTeammates = Vars::Competitive::HealthBarESP::MedicMode.Value && isLocalPlayerMedic;
    
    // Process all players (follow native ESP pattern)
    for (auto pEntity : H::Entities.GetGroup(EGroupType::PLAYERS_ALL))
    {
        auto pPlayer = pEntity->As<CTFPlayer>();
        if (!pPlayer || !pPlayer->IsAlive() || pPlayer->IsDormant() || pPlayer == pLocal)
            continue;
        
        // Skip if cloaked or disguised (from lua script logic)
        if (pPlayer->InCond(TF_COND_STEALTHED) || pPlayer->InCond(TF_COND_DISGUISED))
            continue;
        
        bool isFriendly = (pPlayer->m_iTeamNum() == pLocal->m_iTeamNum());
        
        // Apply medic mode logic: show teammates if medic, enemies otherwise
        if (!((showTeammates && isFriendly) || (!showTeammates && !isFriendly)))
            continue;
        
        Vec3 playerPos = pPlayer->GetAbsOrigin();
        
        // Distance check (from lua script)
        Vec3 delta = playerPos - localPos;
        float distSqr = delta.x * delta.x + delta.y * delta.y + delta.z * delta.z;
        if (distSqr > (Vars::Competitive::HealthBarESP::MaxDistance.Value * Vars::Competitive::HealthBarESP::MaxDistance.Value))
            continue;
        
        int health = pPlayer->m_iHealth();
        int maxHealth = pPlayer->GetMaxHealth();
        
        // Check visibility for bars and polygons separately
        bool isVisible = IsVisible(playerPos);
        
        // Draw health polygon under player if enabled and visible (or through walls enabled)
        if (Vars::Competitive::HealthBarESP::ShowPolygons.Value)
        {
            if (Vars::Competitive::HealthBarESP::PolygonThroughWalls.Value || isVisible)
            {
                DrawHealthPolygon(playerPos, health, maxHealth);
            }
        }
        
        // Draw health bar if enabled and visible (or through walls enabled for bars)
        if (Vars::Competitive::HealthBarESP::ShowHealthBars.Value)
        {
            if (Vars::Competitive::HealthBarESP::ShowThroughWalls.Value || isVisible)
            {
                Vec3 screenPos;
                // Use chest height position for better close-range projection
                Vec3 chestPos = playerPos;
                chestPos.z += 40.0f;
                
                if (SDK::W2S(chestPos, screenPos))
                {
                    // Allow health bars slightly off-screen for close range players
                    if (screenPos.x >= -100 && screenPos.x <= H::Draw.m_nScreenW + 100 &&
                        screenPos.y >= -100 && screenPos.y <= H::Draw.m_nScreenH + 100)
                    {
                        int x = (int)(screenPos.x - Vars::Competitive::HealthBarESP::BarWidth.Value / 2);
                        int y = (int)(screenPos.y + 30);
                        DrawHealthBar(x, y, Vars::Competitive::HealthBarESP::BarWidth.Value, health, maxHealth);
                    }
                }
            }
        }
    }
}

Color_t CHealthBarESP::GetHealthBarColor(int health, int maxHealth)
{
    if (health > maxHealth)
    {
        // Overheal color (blue) - use user's alpha setting
        Color_t result = Vars::Competitive::HealthBarESP::OverhealColor.Value;
        result.a = static_cast<byte>(Vars::Competitive::HealthBarESP::Alpha.Value);
        return result;
    }
    else
    {
        // Red to green gradient using user's alpha setting
        float ratio = (float)health / (float)maxHealth;
        
        Color_t result;
        result.r = (int)(255 * (1.0f - ratio));
        result.g = (int)(255 * ratio);
        result.b = 0;
        result.a = static_cast<byte>(Vars::Competitive::HealthBarESP::Alpha.Value);
        return result;
    }
}

void CHealthBarESP::DrawHealthBar(int x, int y, int width, int health, int maxHealth)
{
    // Calculate bar sizes (from lua script logic)
    int healthBarSize = (int)(width * ((float)std::min(health, maxHealth) / (float)maxHealth));
    int overhealSize = (health > maxHealth) ? (int)(width * ((float)(health - maxHealth) / (float)maxHealth)) : 0;
    
    // Background (black with alpha)
    H::Draw.FillRect(x, y, width, Vars::Competitive::HealthBarESP::BarHeight.Value, {0, 0, 0, static_cast<byte>(Vars::Competitive::HealthBarESP::Alpha.Value)});
    
    // Main health bar
    Color_t healthColor = GetHealthBarColor(std::min(health, maxHealth), maxHealth);
    if (healthBarSize > 2)
        H::Draw.FillRect(x + 1, y + 1, healthBarSize - 2, Vars::Competitive::HealthBarESP::BarHeight.Value - 2, healthColor);
    
    // Overheal bar
    if (overhealSize > 0)
    {
        Color_t overhealColor = Vars::Competitive::HealthBarESP::OverhealColor.Value;
        overhealColor.a = static_cast<byte>(Vars::Competitive::HealthBarESP::Alpha.Value);
        H::Draw.FillRect(x + healthBarSize, y + 1, overhealSize - 1, Vars::Competitive::HealthBarESP::BarHeight.Value - 2, overhealColor);
    }
}

bool CHealthBarESP::IsVisible(const Vec3& vPos)
{
    auto pLocal = H::Entities.GetLocal();
    if (!pLocal)
        return false;
    
    Vec3 vEyePos = pLocal->GetAbsOrigin() + pLocal->m_vecViewOffset();
    Vec3 targetPos = vPos;
    targetPos.z += 40.0f; // Chest height
    
    // Check distance first - if very close, be more lenient with visibility
    float distance = (targetPos - vEyePos).Length();
    if (distance < 150.0f)
    {
        // At very close range, skip trace check to prevent disappearing
        return true;
    }
    
    CGameTrace trace = {};
    CTraceFilterHitscan filter = {};
    filter.pSkip = pLocal;
    
    SDK::Trace(vEyePos, targetPos, MASK_VISIBLE, &filter, &trace);
    
    // Use more lenient trace fraction for close range
    float requiredFraction = (distance < 300.0f) ? 0.75f : 0.90f;
    
    return trace.fraction > requiredFraction;
}

void CHealthBarESP::DrawHealthPolygon(const Vec3& playerPos, int health, int maxHealth)
{
    // Get health color based on current health
    Color_t healthColor = GetHealthBarColor(health, maxHealth);
    
    // Calculate health percentage for polygon fill
    float healthRatio = (float)std::min(health, maxHealth) / (float)maxHealth;
    float overhealRatio = (health > maxHealth) ? (float)(health - maxHealth) / (float)maxHealth : 0.0f;
    
    // Create polygon points around player position
    const int segments = Vars::Competitive::HealthBarESP::PolygonSegments.Value;
    const float radius = Vars::Competitive::HealthBarESP::PolygonRadius.Value;
    
    std::vector<Vertex_t> vertices;
    
    // Generate circle points on the ground
    for (int i = 0; i < segments; i++)
    {
        float angle = (2.0f * M_PI * i) / segments;
        Vec3 worldPoint = playerPos;
        worldPoint.x += cos(angle) * radius;
        worldPoint.y += sin(angle) * radius;
        worldPoint.z -= 10.0f; // Slightly below player feet
        
        Vec3 screenPoint;
        if (SDK::W2S(worldPoint, screenPoint))
        {
            // Allow some off-screen coordinates for close-range visibility
            if (screenPoint.x >= -200 && screenPoint.x <= H::Draw.m_nScreenW + 200 &&
                screenPoint.y >= -200 && screenPoint.y <= H::Draw.m_nScreenH + 200)
            {
                vertices.emplace_back(Vertex_t(Vector2D(screenPoint.x, screenPoint.y)));
            }
        }
        // Don't return early if one point fails - continue with other points
    }
    
    // Only draw if we have enough vertices
    if (vertices.size() < 3)
        return;
    
    // Draw filled polygon if enabled
    if (Vars::Competitive::HealthBarESP::ShowPolygonFill.Value)
    {
        // Draw main health portion
        if (healthRatio > 0.0f)
        {
            H::Draw.FillPolygon(vertices, healthColor);
        }
        
        // Draw overheal portion with different color if present
        if (overhealRatio > 0.0f)
        {
            Color_t overhealColor = Vars::Competitive::HealthBarESP::OverhealColor.Value;
            overhealColor.a = static_cast<byte>(Vars::Competitive::HealthBarESP::Alpha.Value);
            H::Draw.FillPolygon(vertices, overhealColor);
        }
    }
    
    // Draw polygon edge if enabled
    if (Vars::Competitive::HealthBarESP::ShowPolygonEdge.Value)
    {
        H::Draw.LinePolygon(vertices, healthColor);
    }
}