#include "FocusFire.h"
#include "../../../SDK/SDK.h"

void CFocusFire::Draw()
{
    if (!Vars::Competitive::Features::FocusFire.Value)
        return;
    
    // Early exit if game UI is visible
    if (I::EngineVGui->IsGameUIVisible())
        return;
    
    auto pLocal = H::Entities.GetLocal();
    if (!pLocal)
        return;
    
    // Clean up invalid targets periodically
    CleanInvalidTargets();
    
    // Draw box effects
    DrawBox();
}

bool CFocusFire::IsPlayerVisible(CTFPlayer* pPlayer)
{
    if (!Vars::Competitive::FocusFire::VisibleOnly.Value)
        return true;
    
    auto pLocal = H::Entities.GetLocal();
    if (!pPlayer || !pLocal)
        return false;
    
    int id = pPlayer->entindex();
    float curTime = I::GlobalVars->curtime;
    
    // Use cached result if available
    if (m_NextVisCheck.find(id) != m_NextVisCheck.end() && curTime < m_NextVisCheck[id])
        return m_VisibilityCache[id];
    
    // Get eye position for more accurate tracing
    Vec3 localEyePos = pLocal->GetEyePosition();
    Vec3 targetPos = pPlayer->GetAbsOrigin() + Vec3(0, 0, 40); // Aim for chest area
    
    // Use appropriate mask based on team
    bool isTeammate = pPlayer->m_iTeamNum() == pLocal->m_iTeamNum();
    int mask = isTeammate ? MASK_SHOT_HULL : MASK_SHOT;
    
    CGameTrace trace;
    CTraceFilterHitscan filter;
    filter.pSkip = pLocal;
    SDK::Trace(localEyePos, targetPos, mask, &filter, &trace);
    
    // Cache result
    m_VisibilityCache[id] = trace.m_pEnt == pPlayer || trace.fraction > 0.99f;
    m_NextVisCheck[id] = curTime + 0.1f; // Cache for 0.1 seconds
    
    return m_VisibilityCache[id];
}

void CFocusFire::DrawCorners(int x1, int y1, int x2, int y2)
{
    int length = Vars::Competitive::FocusFire::CornerLength.Value;
    
    // Top left corner
    H::Draw.Line(x1, y1, x1 + length, y1, Vars::Competitive::FocusFire::Color.Value);
    H::Draw.Line(x1, y1, x1, y1 + length, Vars::Competitive::FocusFire::Color.Value);
    
    // Top right corner
    H::Draw.Line(x2, y1, x2 - length, y1, Vars::Competitive::FocusFire::Color.Value);
    H::Draw.Line(x2, y1, x2, y1 + length, Vars::Competitive::FocusFire::Color.Value);
    
    // Bottom left corner
    H::Draw.Line(x1, y2, x1 + length, y2, Vars::Competitive::FocusFire::Color.Value);
    H::Draw.Line(x1, y2, x1, y2 - length, Vars::Competitive::FocusFire::Color.Value);
    
    // Bottom right corner
    H::Draw.Line(x2, y2, x2 - length, y2, Vars::Competitive::FocusFire::Color.Value);
    H::Draw.Line(x2, y2, x2, y2 - length, Vars::Competitive::FocusFire::Color.Value);
}

void CFocusFire::CleanExpiredAttackers(float currentTime, TargetInfo& targetInfo)
{
    std::unordered_map<int, float> newAttackers;
    int count = 0;
    
    for (auto& [attackerIndex, timestamp] : targetInfo.Attackers)
    {
        if (currentTime - timestamp <= Vars::Competitive::FocusFire::TrackerTimeWindow.Value)
        {
            newAttackers[attackerIndex] = timestamp;
            count++;
        }
    }
    
    targetInfo.Attackers = newAttackers;
    targetInfo.AttackerCount = count;
    targetInfo.IsMultiTargeted = (count >= Vars::Competitive::FocusFire::MinAttackers.Value);
}

void CFocusFire::CleanInvalidTargets()
{
    float currentTime = I::GlobalVars->curtime;
    if (currentTime < m_NextCleanupTime)
        return;
    
    m_NextCleanupTime = currentTime + CLEANUP_INTERVAL;
    
    auto pLocal = H::Entities.GetLocal();
    if (!pLocal)
        return;
    
    auto it = m_TargetData.begin();
    while (it != m_TargetData.end())
    {
        int entIndex = it->first;
        TargetInfo& targetInfo = it->second;
        
        auto pEntity = I::ClientEntityList->GetClientEntity(entIndex);
        auto pPlayer = pEntity ? pEntity->As<CTFPlayer>() : nullptr;
        
        // Clean if entity is invalid, dead, dormant, or on our team
        if (!pPlayer || 
            !pPlayer->IsAlive() || 
            pPlayer->IsDormant() || 
            pPlayer->m_iTeamNum() == pLocal->m_iTeamNum())
        {
            m_NextVisCheck.erase(entIndex);
            m_VisibilityCache.erase(entIndex);
            it = m_TargetData.erase(it);
            continue;
        }
        
        // Update target status and clean if no longer multi-targeted
        CleanExpiredAttackers(currentTime, targetInfo);
        if (targetInfo.AttackerCount < Vars::Competitive::FocusFire::MinAttackers.Value)
        {
            m_NextVisCheck.erase(entIndex);
            m_VisibilityCache.erase(entIndex);
            it = m_TargetData.erase(it);
            continue;
        }
        
        ++it;
    }
}

bool CFocusFire::ShouldVisualizePlayer(CTFPlayer* pPlayer)
{
    if (!pPlayer || 
        !pPlayer->IsAlive() || 
        pPlayer->IsDormant())
        return false;
    
    auto pLocal = H::Entities.GetLocal();
    if (!pLocal || pPlayer->m_iTeamNum() == pLocal->m_iTeamNum())
        return false;
    
    auto it = m_TargetData.find(pPlayer->entindex());
    return it != m_TargetData.end() && it->second.IsMultiTargeted && IsPlayerVisible(pPlayer);
}

void CFocusFire::DrawBox()
{
    if (!Vars::Competitive::FocusFire::EnableBox.Value)
        return;
    
    
    // Find players being targeted by multiple teammates
    for (auto& [index, targetInfo] : m_TargetData)
    {
        auto pEntity = I::ClientEntityList->GetClientEntity(index);
        auto pPlayer = pEntity ? pEntity->As<CTFPlayer>() : nullptr;
        
        if (pPlayer && ShouldVisualizePlayer(pPlayer))
        {
            // Chams are now handled in UpdateChamsEntities()
            
            // Skip box drawing if disabled
            if (!Vars::Competitive::FocusFire::EnableBox.Value)
                continue;
            // Get player bounds in world space
            Vec3 origin = pPlayer->GetAbsOrigin();
            Vec3 mins = pPlayer->m_vecMins();
            Vec3 maxs = pPlayer->m_vecMaxs();
            
            // Calculate corners in world space
            Vec3 corners[8] = {
                Vec3(origin.x + mins.x - Vars::Competitive::FocusFire::BoxPadding.Value, origin.y + mins.y - Vars::Competitive::FocusFire::BoxPadding.Value, origin.z + mins.z),
                Vec3(origin.x + maxs.x + Vars::Competitive::FocusFire::BoxPadding.Value, origin.y + mins.y - Vars::Competitive::FocusFire::BoxPadding.Value, origin.z + mins.z),
                Vec3(origin.x + maxs.x + Vars::Competitive::FocusFire::BoxPadding.Value, origin.y + maxs.y + Vars::Competitive::FocusFire::BoxPadding.Value, origin.z + mins.z),
                Vec3(origin.x + mins.x - Vars::Competitive::FocusFire::BoxPadding.Value, origin.y + maxs.y + Vars::Competitive::FocusFire::BoxPadding.Value, origin.z + mins.z),
                Vec3(origin.x + mins.x - Vars::Competitive::FocusFire::BoxPadding.Value, origin.y + mins.y - Vars::Competitive::FocusFire::BoxPadding.Value, origin.z + maxs.z + Vars::Competitive::FocusFire::BoxPadding.Value),
                Vec3(origin.x + maxs.x + Vars::Competitive::FocusFire::BoxPadding.Value, origin.y + mins.y - Vars::Competitive::FocusFire::BoxPadding.Value, origin.z + maxs.z + Vars::Competitive::FocusFire::BoxPadding.Value),
                Vec3(origin.x + maxs.x + Vars::Competitive::FocusFire::BoxPadding.Value, origin.y + maxs.y + Vars::Competitive::FocusFire::BoxPadding.Value, origin.z + maxs.z + Vars::Competitive::FocusFire::BoxPadding.Value),
                Vec3(origin.x + mins.x - Vars::Competitive::FocusFire::BoxPadding.Value, origin.y + maxs.y + Vars::Competitive::FocusFire::BoxPadding.Value, origin.z + maxs.z + Vars::Competitive::FocusFire::BoxPadding.Value)
            };
            
            // Convert to screen coordinates
            std::vector<Vec3> screenCorners;
            for (int i = 0; i < 8; i++)
            {
                Vec3 screen;
                if (SDK::W2S(corners[i], screen))
                    screenCorners.push_back(screen);
            }
            
            // Draw the border if we have screen coordinates
            if (!screenCorners.empty())
            {
                // Find bounds
                float minX = screenCorners[0].x, minY = screenCorners[0].y;
                float maxX = screenCorners[0].x, maxY = screenCorners[0].y;
                
                for (const auto& screen : screenCorners)
                {
                    minX = std::min(minX, screen.x);
                    minY = std::min(minY, screen.y);
                    maxX = std::max(maxX, screen.x);
                    maxY = std::max(maxY, screen.y);
                }
                
                // Draw either corners or full box based on config
                if (Vars::Competitive::FocusFire::UseCorners.Value)
                {
                    // Draw corners with thickness
                    for (int i = 0; i < Vars::Competitive::FocusFire::BoxThickness.Value; i++)
                    {
                        DrawCorners(static_cast<int>(minX - i), static_cast<int>(minY - i), 
                                   static_cast<int>(maxX + i), static_cast<int>(maxY + i));
                    }
                }
                else
                {
                    // Draw full box with thickness
                    for (int i = 0; i < Vars::Competitive::FocusFire::BoxThickness.Value; i++)
                    {
                        H::Draw.LineRect(static_cast<int>(minX - i), static_cast<int>(minY - i), 
                                        static_cast<int>(maxX + i - minX + 2*i), static_cast<int>(maxY + i - minY + 2*i), 
                                        Vars::Competitive::FocusFire::Color.Value);
                    }
                }
            }
        }
    }
}

void CFocusFire::OnPlayerHurt(int victimIndex, int attackerIndex)
{
    
    auto pVictim = I::ClientEntityList->GetClientEntity(victimIndex);
    auto pAttacker = I::ClientEntityList->GetClientEntity(attackerIndex);
    
    if (!pVictim || !pAttacker)
        return;
    
    auto pVictimPlayer = pVictim->As<CTFPlayer>();
    auto pAttackerPlayer = pAttacker->As<CTFPlayer>();
    
    if (!pVictimPlayer || !pAttackerPlayer)
        return;
    
    auto pLocal = H::Entities.GetLocal();
    if (!pLocal)
        return;
    
    // Skip if victim == attacker (self-damage)
    if (victimIndex == attackerIndex)
        return;
    
    // Skip if attacker is on same team as victim (friendly fire shouldn't count)
    if (pVictimPlayer->m_iTeamNum() == pAttackerPlayer->m_iTeamNum())
        return;
    
    // Skip if attacker is not on our team (we only track our team's attacks)
    if (pAttackerPlayer->m_iTeamNum() != pLocal->m_iTeamNum())
        return;
    
    // Skip dormant entities
    if (pVictimPlayer->IsDormant() || pAttackerPlayer->IsDormant())
        return;
    
    if (m_TargetData.find(victimIndex) == m_TargetData.end())
    {
        m_TargetData[victimIndex] = TargetInfo();
    }
    
    float currentTime = I::GlobalVars->curtime;
    TargetInfo& targetInfo = m_TargetData[victimIndex];
    
    // Update attackers list
    targetInfo.Attackers[attackerIndex] = currentTime;
    CleanExpiredAttackers(currentTime, targetInfo);
    
}

void CFocusFire::Reset()
{
    m_TargetData.clear();
    m_NextVisCheck.clear();
    m_VisibilityCache.clear();
    m_NextCleanupTime = 0.0f;
}

void CFocusFire::UpdateChamsEntities()
{
    // Clear previous chams entries
    m_mEntities.clear();
    
    if (!Vars::Competitive::Features::FocusFire.Value || !Vars::Competitive::FocusFire::EnableChams.Value)
        return;
    
    // Clean up invalid targets first
    CleanInvalidTargets();
    
    // Find players being targeted by multiple teammates
    for (auto& [index, targetInfo] : m_TargetData)
    {
        auto pEntity = I::ClientEntityList->GetClientEntity(index);
        auto pPlayer = pEntity ? pEntity->As<CTFPlayer>() : nullptr;
        
        if (pPlayer && ShouldVisualizePlayer(pPlayer))
        {
            // Add to chams tracking
            m_mEntities[pPlayer->entindex()] = true;
        }
    }
}