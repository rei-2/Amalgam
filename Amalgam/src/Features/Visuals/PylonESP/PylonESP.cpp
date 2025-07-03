#include "PylonESP.h"
#include "../../../SDK/SDK.h"

CPylonESP::CPylonESP()
{
    InitializeAlphaSteps();
}

void CPylonESP::InitializeAlphaSteps()
{
    m_AlphaSteps.reserve(SEGMENTS + 1);
    for (int i = 0; i <= SEGMENTS; i++)
    {
        float progress = static_cast<float>(i) / SEGMENTS;
        int alpha = static_cast<int>(PYLON_START_ALPHA - (progress * (PYLON_START_ALPHA - PYLON_END_ALPHA)));
        m_AlphaSteps.push_back(alpha);
    }
}

void CPylonESP::Draw()
{
    // Early exits
    if (I::EngineVGui->IsGameUIVisible())
        return;
    
    auto pLocal = H::Entities.GetLocal();
    if (!pLocal)
        return;
    
    // Cache frequently used values
    Vec3 localPos = pLocal->GetAbsOrigin();
    Vec3 eyePos = pLocal->GetEyePosition();
    int localTeam = pLocal->m_iTeamNum();
    
    // Clean up stale entries periodically
    CleanupStaleEntries();
    
    // Process all players
    for (auto pEntity : H::Entities.GetGroup(EGroupType::PLAYERS_ALL))
    {
        auto pPlayer = pEntity->As<CTFPlayer>();
        if (!pPlayer || !pPlayer->IsAlive() || pPlayer->IsDormant())
            continue;
        
        // Only track enemy medics
        if (pPlayer->m_iTeamNum() == localTeam || pPlayer->m_iClass() != TF_CLASS_MEDIC)
            continue;
        
        int playerIndex = pPlayer->entindex();
        Vec3 playerPos = pPlayer->GetAbsOrigin();
        
        // Skip if we can see the medic directly
        if (IsPlayerDirectlyVisible(pPlayer, eyePos))
            continue;
        
        // Skip if we're too close to the medic
        float distanceSqr = GetDistanceSqr(localPos, playerPos);
        if (distanceSqr < MIN_DISTANCE_SQR)
            continue;
        
        // Calculate pylon base position
        Vec3 playerMaxs = pPlayer->m_vecMaxs();
        Vec3 currentPos = Vec3(playerPos.x, playerPos.y, playerPos.z + playerMaxs.z + PYLON_OFFSET);
        
        // Update or create position data
        if (ShouldUpdatePosition(playerIndex))
        {
            m_MedicPositions[playerIndex] = {currentPos, I::GlobalVars->curtime};
        }
        
        // If we don't have a position yet, create one
        if (m_MedicPositions.find(playerIndex) == m_MedicPositions.end())
        {
            m_MedicPositions[playerIndex] = {currentPos, I::GlobalVars->curtime};
        }
        
        // Draw pylon
        DrawPylon(playerIndex, m_MedicPositions[playerIndex].Position, eyePos);
    }
}

bool CPylonESP::IsVisibleCached(const Vec3& fromPos, const Vec3& targetPos, const std::string& identifier)
{
    float currentTime = I::GlobalVars->curtime;
    
    // Check if we need to update the cache
    auto it = m_VisibilityCache.find(identifier);
    if (it == m_VisibilityCache.end() || (currentTime - it->second.LastCheck > VISIBILITY_PERSISTENCE))
    {
        // Update the cache
        CGameTrace trace;
        CTraceFilterHitscan filter;
        filter.pSkip = H::Entities.GetLocal();
        SDK::Trace(fromPos, targetPos, MASK_VISIBLE, &filter, &trace);
        
        bool isVisible = trace.fraction >= 0.99f;
        m_VisibilityCache[identifier] = {isVisible, currentTime};
        return isVisible;
    }
    
    // Return cached value
    return it->second.IsVisible;
}

float CPylonESP::GetDistanceSqr(const Vec3& pos1, const Vec3& pos2)
{
    Vec3 delta = pos2 - pos1;
    return delta.x * delta.x + delta.y * delta.y + delta.z * delta.z;
}

bool CPylonESP::ShouldUpdatePosition(int medicIndex)
{
    if (UPDATE_INTERVAL == 0.0f)
        return true;
    
    auto it = m_MedicPositions.find(medicIndex);
    if (it == m_MedicPositions.end())
        return true;
    
    float lastUpdate = it->second.LastUpdate;
    return (I::GlobalVars->curtime - lastUpdate) >= UPDATE_INTERVAL;
}

void CPylonESP::CleanupStaleEntries()
{
    float currentTime = I::GlobalVars->curtime;
    
    // Only clean up periodically
    if (currentTime - m_LastCleanup < CLEANUP_INTERVAL)
        return;
    
    m_LastCleanup = currentTime;
    
    // Remove stale medic positions
    auto it = m_MedicPositions.begin();
    while (it != m_MedicPositions.end())
    {
        if (currentTime - it->second.LastUpdate > UPDATE_INTERVAL * 3.0f)
        {
            int medicIdx = it->first;
            it = m_MedicPositions.erase(it);
            
            // Clean up associated visibility cache entries
            auto cacheIt = m_VisibilityCache.begin();
            while (cacheIt != m_VisibilityCache.end())
            {
                if (cacheIt->first.find(std::to_string(medicIdx) + "_") == 0)
                {
                    cacheIt = m_VisibilityCache.erase(cacheIt);
                }
                else
                {
                    ++cacheIt;
                }
            }
        }
        else
        {
            ++it;
        }
    }
}

bool CPylonESP::IsPlayerDirectlyVisible(CTFPlayer* pPlayer, const Vec3& eyePos)
{
    Vec3 playerPos = pPlayer->GetAbsOrigin();
    std::string identifier = std::to_string(pPlayer->entindex()) + "_base";
    return IsVisibleCached(eyePos, playerPos, identifier);
}

void CPylonESP::DrawPylon(int medicIndex, const Vec3& basePosition, const Vec3& eyePos)
{
    // First pass: check if any segment is visible
    bool anySegmentVisible = false;
    for (int i = 0; i <= SEGMENTS; i++)
    {
        Vec3 segmentPos = Vec3(basePosition.x, basePosition.y, basePosition.z + (i * SEGMENT_HEIGHT));
        std::string segmentKey = std::to_string(medicIndex) + "_segment_" + std::to_string(i);
        
        if (IsVisibleCached(eyePos, segmentPos, segmentKey))
        {
            anySegmentVisible = true;
            break;
        }
    }
    
    // Skip drawing if no segments are visible
    if (!anySegmentVisible)
        return;
    
    // Second pass: draw visible segments
    Vec3 lastScreenPos;
    bool hasLastScreenPos = false;
    
    for (int i = 0; i <= SEGMENTS; i++)
    {
        Vec3 segmentPos = Vec3(basePosition.x, basePosition.y, basePosition.z + (i * SEGMENT_HEIGHT));
        std::string segmentKey = std::to_string(medicIndex) + "_segment_" + std::to_string(i);
        
        bool visible = IsVisibleCached(eyePos, segmentPos, segmentKey);
        
        if (!visible)
        {
            hasLastScreenPos = false;
            continue;
        }
        
        Vec3 screenPos;
        if (!SDK::W2S(segmentPos, screenPos))
        {
            hasLastScreenPos = false;
            continue;
        }
        
        if (hasLastScreenPos)
        {
            // Draw lines with fading alpha
            Color_t segmentColor = PYLON_COLOR;
            segmentColor.a = static_cast<byte>(m_AlphaSteps[i]);
            
            for (int w = 0; w < PYLON_WIDTH; w++)
            {
                H::Draw.Line(static_cast<int>(lastScreenPos.x + w), static_cast<int>(lastScreenPos.y),
                           static_cast<int>(screenPos.x + w), static_cast<int>(screenPos.y), segmentColor);
            }
        }
        
        lastScreenPos = screenPos;
        hasLastScreenPos = true;
    }
}

void CPylonESP::Reset()
{
    m_MedicPositions.clear();
    m_VisibilityCache.clear();
    m_LastCleanup = 0.0f;
}