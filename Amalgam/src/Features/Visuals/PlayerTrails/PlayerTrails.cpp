#include "PlayerTrails.h"

CPlayerTrails::CPlayerTrails()
    : m_LastUpdateTick(0)
    , m_NextCleanupTime(0.0f)
    , m_LastLifeState(false) // Dead state
    , m_TrailHeightUnits(std::min(5.0f, std::max(0.0f, Vars::Competitive::PlayerTrails::TrailHeight.Value)) * 20.0f)
    , m_MaxDistanceSqr(Vars::Competitive::PlayerTrails::MaxDistance.Value * Vars::Competitive::PlayerTrails::MaxDistance.Value)
{
}

void CPlayerTrails::Draw()
{
    if (!Vars::Competitive::Features::PlayerTrails.Value)
        return;
    
    // Early exits
    if (I::EngineVGui->IsGameUIVisible())
        return;
    
    auto pLocal = H::Entities.GetLocal();
    if (!pLocal)
        return;
    
    // Check for map change
    std::string newMap = I::EngineClient->GetLevelName();
    if (newMap != m_CurrentMap)
    {
        m_CurrentMap = newMap;
        Reset();
        return;
    }
    
    // Check for respawn (simplified)
    bool currentAlive = pLocal->IsAlive();
    if (!m_LastLifeState && currentAlive) // Player respawned
    {
        ClearData();
    }
    m_LastLifeState = currentAlive;
    
    // Clean up invalid targets periodically
    CleanInvalidTargets();
    
    Vec3 localPos = pLocal->GetAbsOrigin();
    int currentTick = I::GlobalVars->tickcount;
    float currentTime = I::GlobalVars->realtime;
    
    // Update player positions
    if (currentTick - m_LastUpdateTick >= Vars::Competitive::PlayerTrails::UpdateInterval.Value)
    {
        for (auto pEntity : H::Entities.GetGroup(EGroupType::PLAYERS_ALL))
        {
            auto pPlayer = pEntity->As<CTFPlayer>();
            if (!IsValidTrailTarget(pPlayer, pLocal))
                continue;
            
            Vec3 playerPos = pPlayer->GetAbsOrigin();
            
            // Distance check
            Vec3 delta = playerPos - localPos;
            float distanceSqr = delta.x * delta.x + delta.y * delta.y + delta.z * delta.z;
            if (distanceSqr > m_MaxDistanceSqr)
                continue;
            
            bool isVisible = IsVisible(pPlayer, pLocal);
            
            // Adjust height
            playerPos.z += m_TrailHeightUnits;
            
            std::string steamID = GetSteamID(pPlayer);
            if (steamID.empty())
                continue;
            
            // Initialize player data if needed
            if (m_PlayerData.find(steamID) == m_PlayerData.end())
            {
                PlayerTrailData data;
                data.Color = GenerateColor(steamID);
                data.VisibilityState = static_cast<int>(EVisibilityState::UNSEEN);
                data.VisibleStartTime = 0.0f;
                data.LastSeenTime = 0.0f;
                data.LastInvisibleTime = 0.0f;
                m_PlayerData[steamID] = data;
            }
            
            PlayerTrailData& data = m_PlayerData[steamID];
            UpdatePlayerState(data, isVisible, currentTime);
            
            if (ShouldShowTrail(data, currentTime))
            {
                // Only add position if moved enough or first position
                bool shouldAddPosition = data.Positions.empty();
                if (!shouldAddPosition && !data.Positions.empty())
                {
                    Vec3 lastPos = data.Positions[0].Position;
                    Vec3 posDelta = playerPos - lastPos;
                    float movementSqr = posDelta.x * posDelta.x + posDelta.y * posDelta.y + posDelta.z * posDelta.z;
                    shouldAddPosition = movementSqr > (Vars::Competitive::PlayerTrails::MinMovementDistance.Value * Vars::Competitive::PlayerTrails::MinMovementDistance.Value);
                }
                
                if (shouldAddPosition)
                {
                    TrailPosition newPos;
                    newPos.Position = playerPos;
                    newPos.Time = currentTime;
                    
                    data.Positions.insert(data.Positions.begin(), newPos);
                    
                    if (data.Positions.size() > static_cast<size_t>(Vars::Competitive::PlayerTrails::MaxTrailLength.Value))
                        data.Positions.pop_back();
                }
            }
            else if (!data.Positions.empty())
            {
                data.Positions.clear();
            }
        }
        
        m_LastUpdateTick = currentTick;
    }
    
    // Draw trails
    for (const auto& pair : m_PlayerData)
    {
        const PlayerTrailData& data = pair.second;
        
        if (data.Positions.size() > 1 && ShouldShowTrail(data, currentTime))
        {
            int step = (data.VisibilityState == static_cast<int>(EVisibilityState::VISIBLE)) ? 1 : 2;
            
            for (size_t i = 0; i < data.Positions.size() - 1; i += step)
            {
                float timeDiff = currentTime - data.Positions[i].Time;
                int alpha = static_cast<int>(std::max(0.0f, 255.0f * (1.0f - timeDiff / Vars::Competitive::PlayerTrails::TrailLifetime.Value)));
                
                if (alpha <= 0)
                    break;
                
                Vec3 startPos3D = data.Positions[i].Position;
                Vec3 endPos3D = data.Positions[i + 1].Position;
                
                Vec3 startPos, endPos;
                if (SDK::W2S(startPos3D, startPos) && SDK::W2S(endPos3D, endPos))
                {
                    Color_t color = data.Color;
                    color.a = alpha;
                    H::Draw.Line(static_cast<int>(startPos.x), static_cast<int>(startPos.y),
                               static_cast<int>(endPos.x), static_cast<int>(endPos.y), color);
                }
            }
        }
    }
}

Color_t CPlayerTrails::GenerateColor(const std::string& steamID)
{
    if (m_ColorCache.find(steamID) != m_ColorCache.end())
        return m_ColorCache[steamID];
    
    // Simple hash function for color generation
    uint32_t hash = 0;
    for (char c : steamID)
    {
        hash += static_cast<uint32_t>(c);
    }
    
    int r = static_cast<int>((hash * 11) % 200 + 55);
    int g = static_cast<int>((hash * 23) % 200 + 55);
    int b = static_cast<int>((hash * 37) % 200 + 55);
    
    Color_t color = {byte(r), byte(g), byte(b), 255};
    m_ColorCache[steamID] = color;
    return color;
}

void CPlayerTrails::ClearData()
{
    m_PlayerData.clear();
    m_NextVisCheck.clear();
    m_VisibilityCache.clear();
    m_LastUpdateTick = 0;
    m_NextCleanupTime = 0.0f;
}

void CPlayerTrails::Reset()
{
    ClearData();
}

void CPlayerTrails::CleanInvalidTargets()
{
    float currentTime = I::GlobalVars->realtime;
    if (currentTime < m_NextCleanupTime)
        return;
    
    m_NextCleanupTime = currentTime + CLEANUP_INTERVAL;
    
    auto pLocal = H::Entities.GetLocal();
    if (!pLocal)
        return;
    
    auto it = m_PlayerData.begin();
    while (it != m_PlayerData.end())
    {
        const std::string& steamID = it->first;
        const PlayerTrailData& data = it->second;
        
        // Find the player entity
        bool playerFound = false;
        bool playerValid = false;
        
        for (auto pEntity : H::Entities.GetGroup(EGroupType::PLAYERS_ALL))
        {
            auto pPlayer = pEntity->As<CTFPlayer>();
            if (pPlayer && GetSteamID(pPlayer) == steamID)
            {
                playerFound = true;
                playerValid = pPlayer->IsAlive() && !pPlayer->IsDormant();
                break;
            }
        }
        
        // Clean if player not found, invalid, or data is too old
        bool shouldClean = !playerFound || 
                          !playerValid ||
                          (!data.Positions.empty() && currentTime - data.Positions[0].Time > Vars::Competitive::PlayerTrails::TrailLifetime.Value) ||
                          (data.LastSeenTime > 0.0f && currentTime - data.LastSeenTime > Vars::Competitive::PlayerTrails::VisibilityTimeout.Value);
        
        if (shouldClean)
        {
            it = m_PlayerData.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

bool CPlayerTrails::IsVisible(CBaseEntity* pEntity, CBaseEntity* pLocal)
{
    if (!pEntity || !pLocal)
        return false;
    
    int id = pEntity->entindex();
    int curTick = I::GlobalVars->tickcount;
    
    // Check cache
    if (m_NextVisCheck.find(id) != m_NextVisCheck.end() && curTick < m_NextVisCheck[id])
    {
        return m_VisibilityCache.find(id) != m_VisibilityCache.end() ? m_VisibilityCache[id] : false;
    }
    
    // Perform visibility check
    Vec3 source = pLocal->GetAbsOrigin() + pLocal->As<CTFPlayer>()->m_vecViewOffset();
    Vec3 target = pEntity->GetAbsOrigin();
    
    CGameTrace trace = {};
    CTraceFilterHitscan filter = {};
    filter.pSkip = pLocal;
    
    SDK::Trace(source, target, MASK_VISIBLE, &filter, &trace);
    
    bool isVisible = trace.m_pEnt == pEntity || trace.fraction > 0.99f;
    
    // Cache result
    m_VisibilityCache[id] = isVisible;
    m_NextVisCheck[id] = curTick + 2; // Check every 2 ticks
    
    return isVisible;
}

void CPlayerTrails::UpdatePlayerState(PlayerTrailData& data, bool isVisible, float currentTime)
{
    if (isVisible)
    {
        data.VisibilityState = static_cast<int>(EVisibilityState::VISIBLE);
        if (data.VisibleStartTime == 0.0f)
            data.VisibleStartTime = currentTime;
        data.LastSeenTime = currentTime;
    }
    else
    {
        if (data.VisibilityState == static_cast<int>(EVisibilityState::VISIBLE))
        {
            data.VisibilityState = static_cast<int>(EVisibilityState::RECENTLY_INVISIBLE);
            data.LastInvisibleTime = currentTime;
        }
        else if (data.VisibilityState == static_cast<int>(EVisibilityState::RECENTLY_INVISIBLE) &&
                 currentTime - data.LastInvisibleTime > Vars::Competitive::PlayerTrails::VisibilityTimeout.Value)
        {
            data.VisibilityState = static_cast<int>(EVisibilityState::UNSEEN);
            data.VisibleStartTime = 0.0f;
        }
    }
}

bool CPlayerTrails::ShouldShowTrail(const PlayerTrailData& data, float currentTime)
{
    if (data.VisibilityState == static_cast<int>(EVisibilityState::VISIBLE))
    {
        return data.VisibleStartTime > 0.0f && (currentTime - data.VisibleStartTime <= Vars::Competitive::PlayerTrails::MaxVisibleDuration.Value);
    }
    else if (data.VisibilityState == static_cast<int>(EVisibilityState::RECENTLY_INVISIBLE))
    {
        return currentTime - data.LastInvisibleTime <= Vars::Competitive::PlayerTrails::VisibilityTimeout.Value;
    }
    return false;
}

std::string CPlayerTrails::GetSteamID(CBaseEntity* pPlayer)
{
    if (!pPlayer)
        return "";
    
    PlayerInfo_t playerInfo;
    if (I::EngineClient->GetPlayerInfo(pPlayer->entindex(), &playerInfo))
    {
        if (std::string(playerInfo.guid) == "BOT")
            return "";
        return std::string(playerInfo.guid);
    }
    
    return "";
}

bool CPlayerTrails::IsValidTrailTarget(CBaseEntity* pPlayer, CBaseEntity* pLocal)
{
    if (!pPlayer || !pLocal)
        return false;
    
    auto pTFPlayer = pPlayer->As<CTFPlayer>();
    if (!pTFPlayer || !pTFPlayer->IsAlive() || pTFPlayer->IsDormant() || pTFPlayer == pLocal)
        return false;
    
    // Only show trails for enemies
    if (pTFPlayer->m_iTeamNum() == pLocal->As<CTFPlayer>()->m_iTeamNum())
        return false;
    
    // Skip invisible or disguised spies
    if (pTFPlayer->InCond(TF_COND_STEALTHED) || pTFPlayer->InCond(TF_COND_DISGUISED))
        return false;
    
    return true;
}