#include "HiderESP.h"

float CHiderESP::DistanceBetweenVectors(const Vec3& v1, const Vec3& v2)
{
    float dx = v1.x - v2.x;
    float dy = v1.y - v2.y;
    float dz = v1.z - v2.z;
    return sqrtf(dx * dx + dy * dy + dz * dz);
}

void CHiderESP::ResetPlayerData()
{
    m_PlayerData.clear();
}

void CHiderESP::CleanInvalidTargets()
{
    float currentTime = I::GlobalVars->curtime;
    if (currentTime < m_flNextCleanupTime)
        return;
    
    m_flNextCleanupTime = currentTime + CLEANUP_INTERVAL;
    
    // Only clean up if we have data to clean
    if (m_PlayerData.empty())
        return;
    
    auto pLocal = H::Entities.GetLocal();
    if (!pLocal)
        return;
    
    auto it = m_PlayerData.begin();
    while (it != m_PlayerData.end())
    {
        int playerIndex = it->first;
        auto pEntity = I::ClientEntityList->GetClientEntity(playerIndex);
        auto pPlayer = pEntity ? pEntity->As<CTFPlayer>() : nullptr;
        
        // Remove data if entity is invalid or not in play
        if (!pPlayer || 
            !pPlayer->IsAlive() || 
            pPlayer->m_iTeamNum() == pLocal->m_iTeamNum())
        {
            it = m_PlayerData.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

bool CHiderESP::IsPlayerInRange(CTFPlayer* pPlayer, CTFPlayer* pLocal)
{
    if (!pPlayer || !pLocal)
        return false;
    
    Vec3 playerPos = pPlayer->m_vecOrigin();
    Vec3 localPos = pLocal->m_vecOrigin();
    
    return DistanceBetweenVectors(playerPos, localPos) <= Vars::Competitive::HiderESP::MaxDistance.Value;
}

void CHiderESP::UpdatePlayerData(CTFPlayer* pPlayer)
{
    if (!pPlayer || !pPlayer->IsAlive())
        return;
    
    float currentTime = I::GlobalVars->curtime;
    Vec3 currentPos = pPlayer->m_vecOrigin();
    int playerIndex = pPlayer->entindex();
    
    // Initialize new player data
    if (m_PlayerData.find(playerIndex) == m_PlayerData.end())
    {
        HiderData data;
        data.LastPosition = currentPos;
        data.LastMoveTime = currentTime;
        data.IsHider = false;
        data.LastUpdateTime = currentTime;
        m_PlayerData[playerIndex] = data;
        return;
    }
    
    HiderData& data = m_PlayerData[playerIndex];
    data.LastUpdateTime = currentTime;
    
    float distance = DistanceBetweenVectors(currentPos, data.LastPosition);
    
    if (distance > Vars::Competitive::HiderESP::MovementThreshold.Value)
    {
        data.LastPosition = currentPos;
        data.LastMoveTime = currentTime;
        data.IsHider = false;
    }
    else if (currentTime - data.LastMoveTime > Vars::Competitive::HiderESP::TimeToMark.Value)
    {
        data.IsHider = true;
    }
}

void CHiderESP::DrawPlayerESP(CTFPlayer* pPlayer, const HiderData& data)
{
    Vec3 playerPos = pPlayer->m_vecOrigin();
    int screenW, screenH;
    I::MatSystemSurface->GetScreenSize(screenW, screenH);
    int centerX = screenW / 2;
    int centerY = screenH / 2;
    
    // Draw box
    if (Vars::Competitive::HiderESP::ShowBoxes.Value)
    {
        Vec3 mins = pPlayer->m_vecMins();
        Vec3 maxs = pPlayer->m_vecMaxs();
        
        Vec3 bottomPos = Vec3(playerPos.x, playerPos.y, playerPos.z + mins.z);
        Vec3 topPos = Vec3(playerPos.x, playerPos.y, playerPos.z + maxs.z);
        
        Vec3 screenBottom, screenTop;
        if (SDK::W2S(bottomPos, screenBottom) && SDK::W2S(topPos, screenTop))
        {
            int height = static_cast<int>(screenBottom.y - screenTop.y);
            int width = static_cast<int>(height * 0.75f);
            
            int x1 = static_cast<int>(screenBottom.x - width / 2);
            int y1 = static_cast<int>(screenTop.y);
            int x2 = static_cast<int>(screenBottom.x + width / 2);
            int y2 = static_cast<int>(screenBottom.y);
            
            H::Draw.LineRect(x1, y1, x2 - x1, y2 - y1, Vars::Competitive::HiderESP::BoxColor.Value);
        }
    }
    
    // Draw tracer
    if (Vars::Competitive::HiderESP::ShowTracers.Value)
    {
        Vec3 screenPos;
        if (SDK::W2S(playerPos, screenPos))
        {
            H::Draw.Line(centerX, centerY, static_cast<int>(screenPos.x), static_cast<int>(screenPos.y), Vars::Competitive::HiderESP::TracerColor.Value);
        }
    }
}

void CHiderESP::Draw()
{
    // Early exits
    if (!Vars::Competitive::Features::HiderESP.Value || I::EngineVGui->IsGameUIVisible())
        return;
    
    auto pLocal = H::Entities.GetLocal();
    if (!pLocal || !pLocal->IsAlive())
        return;
    
    // Check for respawn
    int currentLifeState = pLocal->m_lifeState();
    if (m_iLastLifeState == 2 && currentLifeState == 0) // If we went from dead to alive
    {
        ResetPlayerData();
    }
    m_iLastLifeState = currentLifeState;
    
    // Clean up invalid targets periodically
    CleanInvalidTargets();
    
    // Process players
    for (auto pEntity : H::Entities.GetGroup(EGroupType::PLAYERS_ALL))
    {
        auto pPlayer = pEntity->As<CTFPlayer>();
        if (!pPlayer || 
            !pPlayer->IsAlive() || 
            pPlayer->m_iTeamNum() == pLocal->m_iTeamNum() ||
            pPlayer->IsDormant() ||
            !IsPlayerInRange(pPlayer, pLocal))
            continue;
        
        UpdatePlayerData(pPlayer);
        
        auto it = m_PlayerData.find(pPlayer->entindex());
        if (it != m_PlayerData.end() && it->second.IsHider)
        {
            DrawPlayerESP(pPlayer, it->second);
        }
    }
}