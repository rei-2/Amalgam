#include "CritHeals.h"

void CCritHeals::Draw()
{
    // Early exits
    if (I::EngineVGui->IsGameUIVisible())
        return;
    
    auto pLocal = H::Entities.GetLocal();
    if (!pLocal || !pLocal->IsAlive())
        return;
    
    // Only run for medics
    if (pLocal->m_iClass() != TF_CLASS_MEDIC)
        return;
    
    // Handle uber build warning
    if (UBER_BUILD_WARNING)
        DrawUberBuildWarning();
    
    Vec3 localPos = pLocal->GetAbsOrigin();
    float currentTime = I::GlobalVars->curtime;
    
    // Process all players for crit heal indicators
    for (auto pEntity : H::Entities.GetGroup(EGroupType::PLAYERS_ALL))
    {
        auto pPlayer = pEntity->As<CTFPlayer>();
        if (!pPlayer || !pPlayer->IsAlive() || pPlayer->IsDormant() || pPlayer == pLocal)
            continue;
        
        bool isEnemy = (pPlayer->m_iTeamNum() != pLocal->m_iTeamNum());
        
        // Skip enemies if feature is disabled
        if (isEnemy && !SHOW_ON_ENEMIES)
            continue;
        
        Vec3 playerPos = pPlayer->GetAbsOrigin();
        
        // Distance check only for teammates
        if (!isEnemy)
        {
            Vec3 delta = playerPos - localPos;
            float dist = delta.Length();
            if (dist > MAX_DISTANCE)
                continue;
        }
        
        // Visibility check
        if (!IsPlayerVisible(pPlayer, pLocal))
            continue;
        
        // Get player's health info
        int health = pPlayer->m_iHealth();
        int baseMaxHealth = pPlayer->GetMaxHealth();
        
        if (health <= 0 || baseMaxHealth <= 0)
            continue;
        
        int maxBuffedHealth = (int)(baseMaxHealth * MAX_OVERHEAL_MULTIPLIER);
        int bufferAmount = maxBuffedHealth - baseMaxHealth;
        int currentBuffAmount = health - baseMaxHealth;
        
        // Track when player was last well-buffed
        int playerIndex = pPlayer->entindex();
        if (currentBuffAmount > (bufferAmount * BUFF_THRESHOLD))
            m_LastBuffTimes[playerIndex] = currentTime;
        
        // Determine if player should be considered still buffed
        bool wasRecentlyBuffed = false;
        if (m_LastBuffTimes.find(playerIndex) != m_LastBuffTimes.end())
        {
            float lastBuffTime = m_LastBuffTimes[playerIndex];
            if (lastBuffTime > 0 && currentBuffAmount > (bufferAmount * BUFF_FADE_THRESHOLD))
                wasRecentlyBuffed = true;
        }
        
        // Check crit heal eligibility
        float lastDamageTime = 0.0f;
        if (m_LastDamageTimes.find(playerIndex) != m_LastDamageTimes.end())
            lastDamageTime = m_LastDamageTimes[playerIndex];
        
        float timeSinceLastDamage = currentTime - lastDamageTime;
        
        if (timeSinceLastDamage >= CRIT_HEAL_TIME && 
            health < maxBuffedHealth && 
            !wasRecentlyBuffed)
        {
            // Convert player position to screen coordinates and add vertical offset
            Vec3 headPos = playerPos;
            headPos.z += TRIANGLE_VERTICAL_OFFSET;
            
            Vec3 screenPos;
            if (SDK::W2S(headPos, screenPos))
            {
                DrawTriangle((int)screenPos.x, (int)screenPos.y, isEnemy);
            }
        }
    }
}

void CCritHeals::DrawTriangle(int x, int y, bool isEnemy)
{
    int centerX = x;
    int centerY = y;
    
    // Calculate sizes for outer and inner triangles
    int outerSize = TRIANGLE_SIZE;
    int innerSize = TRIANGLE_SIZE - OUTLINE_THICKNESS;
    
    // Draw the black outline triangle first
    for (int i = 0; i <= outerSize; i++)
    {
        int width = (outerSize - i) * 2;
        int xPos = centerX - width / 2;
        int yPos = centerY - outerSize + i;
        H::Draw.FillRect(xPos, yPos, width, 1, OUTLINE_COLOR);
    }
    
    // Draw the colored inner triangle
    Color_t color = isEnemy ? ENEMY_COLOR : FRIEND_COLOR;
    
    // Calculate offset to center the inner triangle
    int offsetY = (outerSize - innerSize) / 2;
    
    for (int i = 0; i <= innerSize; i++)
    {
        int width = (innerSize - i) * 2;
        int xPos = centerX - width / 2;
        int yPos = centerY - outerSize + offsetY + i;
        H::Draw.FillRect(xPos, yPos, width, 1, color);
    }
}

void CCritHeals::DrawUberBuildWarning()
{
    auto pLocal = H::Entities.GetLocal();
    if (!pLocal)
        return;
    
    auto pWeapon = pLocal->m_hActiveWeapon().Get();
    if (!pWeapon)
        return;
    
    auto pWeaponBase = pWeapon->As<CTFWeaponBase>();
    if (!pWeaponBase || pWeaponBase->GetWeaponID() != TF_WEAPON_MEDIGUN)
        return;
    
    auto pMedigun = pWeapon->As<CWeaponMedigun>();
    if (!pMedigun)
        return;
    
    auto pHealTarget = pMedigun->m_hHealingTarget().Get();
    if (!pHealTarget)
        return;
    
    auto pTargetPlayer = pHealTarget->As<CTFPlayer>();
    if (!pTargetPlayer || !pTargetPlayer->IsAlive())
        return;
    
    int health = pTargetPlayer->m_iHealth();
    int baseMaxHealth = pTargetPlayer->GetMaxHealth();
    
    if (health <= 0 || baseMaxHealth <= 0)
        return;
    
    float healthRatio = (float)health / (float)baseMaxHealth;
    
    if (healthRatio >= UBER_PENALTY_THRESHOLD)
    {
        // Position on same line as KRITZ text but centered (matching UberTracker positioning)
        int screenW, screenH;
        I::MatSystemSurface->GetScreenSize(screenW, screenH);
        
        int x = screenW / 2;  // Center horizontally
        int y = (screenH / 2) + 250 - 25;  // Same Y as KRITZ text
        
        const char* message = "Reduced Uber Build Rate!";
        auto font = H::Fonts.GetFont(FONT_ESP);
        
        H::Draw.String(font, x, y, WARNING_COLOR, ALIGN_CENTER, message);
    }
}

bool CCritHeals::IsPlayerVisible(CTFPlayer* pPlayer, CTFPlayer* pLocal)
{
    if (!pPlayer || !pLocal)
        return false;
    
    Vec3 eyePos = pLocal->GetAbsOrigin() + pLocal->m_vecViewOffset();
    Vec3 targetPos = pPlayer->GetAbsOrigin();
    
    bool isTeammate = (pPlayer->m_iTeamNum() == pLocal->m_iTeamNum());
    
    CGameTrace trace = {};
    CTraceFilterHitscan filter = {};
    filter.pSkip = pLocal;
    
    SDK::Trace(eyePos, targetPos, MASK_VISIBLE, &filter, &trace);
    
    return isTeammate ? (trace.fraction > 0.90f) : (trace.fraction > 0.90f || trace.m_pEnt == pPlayer);
}

void CCritHeals::OnPlayerHurt(int victimIndex)
{
    m_LastDamageTimes[victimIndex] = I::GlobalVars->curtime;
}