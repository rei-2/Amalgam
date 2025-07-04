#include "HealthBarESP.h"

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
        
        // Visibility check - more lenient to reduce flickering
        Vec3 eyePos = localPos + pLocal->m_vecViewOffset();
        Vec3 targetPos = playerPos;
        targetPos.z += 40.0f; // Chest height
        
        CGameTrace trace = {};
        CTraceFilterHitscan filter = {}; 
        filter.pSkip = pLocal;
        
        SDK::Trace(eyePos, targetPos, MASK_VISIBLE, &filter, &trace);
        
        bool isVisible;
        if (isFriendly)
            isVisible = (trace.fraction > 0.90f); // More lenient for teammates to reduce flicker
        else
            isVisible = (trace.fraction > 0.90f || trace.m_pEnt == pPlayer); // More lenient to reduce flicker
        
        if (!isVisible)
            continue;
        
        int health = pPlayer->m_iHealth();
        int maxHealth = pPlayer->GetMaxHealth();
        
        // Draw health bar (fixed position and width, no wobble)
        Vec3 screenPos;
        if (SDK::W2S(playerPos, screenPos))
        {
            int x = (int)(screenPos.x - Vars::Competitive::HealthBarESP::BarWidth.Value / 2);
            int y = (int)(screenPos.y + 30);
            DrawHealthBar(x, y, Vars::Competitive::HealthBarESP::BarWidth.Value, health, maxHealth);
        }
    }
}

Color_t CHealthBarESP::GetHealthBarColor(int health, int maxHealth)
{
    if (health > maxHealth)
    {
        // Overheal color (blue) - always full brightness
        Color_t result = Vars::Competitive::HealthBarESP::OverhealColor.Value;
        result.a = 255;
        return result;
    }
    else
    {
        // Red to green gradient with health-based alpha (from lua script)
        float ratio = (float)health / (float)maxHealth;
        
        // More dramatic alpha: lower health = much more visible  
        // Full health = 100 alpha, critical health = 255 alpha
        int currentAlpha = (int)(255 - (ratio * 155)); // As health goes down, alpha goes up
        currentAlpha = std::max(100, std::min(255, currentAlpha));
        
        Color_t result;
        result.r = (int)(255 * (1.0f - ratio));
        result.g = (int)(255 * ratio);
        result.b = 0;
        result.a = currentAlpha;
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