#include "StickyESP.h"

CStickyESP::CStickyESP()
    : m_NextCleanupTime(0.0f)
    , m_LastStickyUpdate(0.0f)
    , m_MaxDistanceSqr(MAX_DISTANCE * MAX_DISTANCE)
{
}

void CStickyESP::Draw()
{
    // Early exits
    if (I::EngineVGui->IsGameUIVisible())
        return;
    
    auto pLocal = H::Entities.GetLocal();
    if (!pLocal)
        return;
    
    float currentTime = I::GlobalVars->realtime;
    Vec3 localPos = pLocal->GetAbsOrigin();
    
    // Update sticky cache periodically
    if (currentTime - m_LastStickyUpdate > STICKY_UPDATE_INTERVAL)
    {
        m_LastStickyUpdate = currentTime;
        
        // Clear old cache entries first
        CleanCaches();
        
        // Process all projectiles looking for stickybombs
        for (auto pEntity : H::Entities.GetGroup(EGroupType::WORLD_PROJECTILES))
        {
            if (!IsValidStickyTarget(pEntity, pLocal))
                continue;
            
            auto pSticky = pEntity->As<CTFGrenadePipebombProjectile>();
            if (!pSticky || pSticky->m_iType() != 1) // Only actual stickybombs (not pipes)
                continue;
            
            Vec3 stickyPos = pSticky->GetAbsOrigin();
            
            // Distance check
            Vec3 delta = stickyPos - localPos;
            float distanceSqr = delta.x * delta.x + delta.y * delta.y + delta.z * delta.z;
            if (distanceSqr > m_MaxDistanceSqr)
                continue;
            
            // Enemy filter
            if (ENEMY_ONLY && pSticky->m_iTeamNum() == pLocal->m_iTeamNum())
                continue;
            
            int entityIndex = pSticky->entindex();
            
            // Update sticky data
            StickyData& data = m_StickyCache[entityIndex];
            data.Position = stickyPos;
            data.LastPosUpdate = currentTime;
            
            // Check visibility
            bool isVisible = IsVisible(pSticky, pLocal);
            data.IsVisible = isVisible;
            
            // Skip if only showing when visible and not visible
            if (BOX_ONLY_WHEN_VISIBLE && !isVisible)
                continue;
            
            // Convert to screen coordinates
            Vec3 screenPos;
            if (!SDK::W2S(stickyPos, screenPos))
                continue;
            
            // Choose color based on visibility
            Color_t color = isVisible ? BOX_COLOR_VISIBLE : BOX_COLOR_INVISIBLE;
            
            // Draw 2D box
            if (BOX_2D)
            {
                Draw2DBox(static_cast<int>(screenPos.x), static_cast<int>(screenPos.y), BOX_2D_SIZE, color);
            }
            
            // Draw 3D box
            if (BOX_3D)
            {
                auto hitbox = GetHitboxWithCache(pSticky);
                if (hitbox.first != Vec3() && hitbox.second != Vec3())
                {
                    Vec3 min = hitbox.first;
                    Vec3 max = hitbox.second;
                    
                    // Create 8 vertices of the bounding box
                    std::vector<Vec3> vertices = {
                        Vec3(min.x, min.y, min.z), Vec3(min.x, max.y, min.z),
                        Vec3(max.x, max.y, min.z), Vec3(max.x, min.y, min.z),
                        Vec3(min.x, min.y, max.z), Vec3(min.x, max.y, max.z),
                        Vec3(max.x, max.y, max.z), Vec3(max.x, min.y, max.z)
                    };
                    
                    Draw3DBox(vertices, color);
                }
            }
        }
    }
}

void CStickyESP::CleanCaches()
{
    float currentTime = I::GlobalVars->realtime;
    if (currentTime < m_NextCleanupTime)
        return;
    
    m_NextCleanupTime = currentTime + CACHE_CLEANUP_INTERVAL;
    
    // Clean visibility cache
    auto visIt = m_VisibilityCacheTime.begin();
    while (visIt != m_VisibilityCacheTime.end())
    {
        if (currentTime - visIt->second > VISIBILITY_CHECK_INTERVAL * 2.0f)
        {
            m_VisibilityCache.erase(visIt->first);
            visIt = m_VisibilityCacheTime.erase(visIt);
        }
        else
        {
            ++visIt;
        }
    }
    
    // Clean hitbox cache
    auto hitboxIt = m_HitboxCacheTime.begin();
    while (hitboxIt != m_HitboxCacheTime.end())
    {
        if (currentTime - hitboxIt->second > HITBOX_CACHE_LIFETIME * 2.0f)
        {
            m_HitboxCache.erase(hitboxIt->first);
            hitboxIt = m_HitboxCacheTime.erase(hitboxIt);
        }
        else
        {
            ++hitboxIt;
        }
    }
    
    // Clean sticky cache for entities that no longer exist
    auto stickyIt = m_StickyCache.begin();
    while (stickyIt != m_StickyCache.end())
    {
        if (currentTime - stickyIt->second.LastPosUpdate > STICKY_UPDATE_INTERVAL * 3.0f)
        {
            stickyIt = m_StickyCache.erase(stickyIt);
        }
        else
        {
            ++stickyIt;
        }
    }
}

bool CStickyESP::IsVisible(CBaseEntity* pEntity, CBaseEntity* pLocal)
{
    if (!pEntity || !pLocal)
        return false;
    
    float currentTime = I::GlobalVars->realtime;
    int entityIndex = pEntity->entindex();
    
    // Check cache
    if (m_VisibilityCacheTime.find(entityIndex) != m_VisibilityCacheTime.end() &&
        currentTime - m_VisibilityCacheTime[entityIndex] < VISIBILITY_CHECK_INTERVAL)
    {
        return m_VisibilityCache[entityIndex];
    }
    
    // Perform visibility check
    Vec3 source = pLocal->GetAbsOrigin() + pLocal->As<CTFPlayer>()->m_vecViewOffset();
    Vec3 target = pEntity->GetAbsOrigin();
    
    CGameTrace trace = {};
    CTraceFilterHitscan filter = {};
    filter.pSkip = pLocal;
    
    SDK::Trace(source, target, MASK_SHOT, &filter, &trace);
    
    bool isVisible = trace.fraction > 0.99f || trace.m_pEnt == pEntity;
    
    // Cache result
    m_VisibilityCache[entityIndex] = isVisible;
    m_VisibilityCacheTime[entityIndex] = currentTime;
    
    return isVisible;
}

std::pair<Vec3, Vec3> CStickyESP::GetHitboxWithCache(CBaseEntity* pEntity)
{
    if (!pEntity)
        return {Vec3(), Vec3()};
    
    float currentTime = I::GlobalVars->realtime;
    int entityIndex = pEntity->entindex();
    
    // Check cache
    if (m_HitboxCacheTime.find(entityIndex) != m_HitboxCacheTime.end() &&
        currentTime - m_HitboxCacheTime[entityIndex] < HITBOX_CACHE_LIFETIME)
    {
        return m_HitboxCache[entityIndex];
    }
    
    // Calculate hitbox bounds (simplified for projectiles)
    Vec3 origin = pEntity->GetAbsOrigin();
    Vec3 mins = pEntity->m_vecMins();
    Vec3 maxs = pEntity->m_vecMaxs();
    
    Vec3 worldMins = origin + mins;
    Vec3 worldMaxs = origin + maxs;
    
    std::pair<Vec3, Vec3> hitbox = {worldMins, worldMaxs};
    
    // Cache result
    m_HitboxCache[entityIndex] = hitbox;
    m_HitboxCacheTime[entityIndex] = currentTime;
    
    return hitbox;
}

void CStickyESP::Draw3DBox(const std::vector<Vec3>& vertices, const Color_t& color)
{
    if (vertices.size() < 8)
        return;
    
    // Convert all vertices to screen coordinates
    std::vector<Vec3> screenVertices;
    for (const auto& vertex : vertices)
    {
        Vec3 screenPos;
        if (SDK::W2S(vertex, screenPos))
        {
            screenVertices.push_back(screenPos);
        }
        else
        {
            return; // If any vertex is off-screen, don't draw
        }
    }
    
    // Define the edges of the box
    std::vector<std::pair<int, int>> edges = {
        {0, 1}, {1, 2}, {2, 3}, {3, 0}, // Bottom face
        {4, 5}, {5, 6}, {6, 7}, {7, 4}, // Top face
        {0, 4}, {1, 5}, {2, 6}, {3, 7}  // Vertical edges
    };
    
    // Draw all edges
    for (const auto& edge : edges)
    {
        Vec3 v1 = screenVertices[edge.first];
        Vec3 v2 = screenVertices[edge.second];
        H::Draw.Line(static_cast<int>(v1.x), static_cast<int>(v1.y),
                    static_cast<int>(v2.x), static_cast<int>(v2.y), color);
    }
}

void CStickyESP::Draw2DBox(int x, int y, int size, const Color_t& color)
{
    int halfSize = size / 2;
    int x1 = x - halfSize;
    int y1 = y - halfSize;
    int x2 = x + halfSize;
    int y2 = y + halfSize;
    
    // Draw box outline
    H::Draw.Line(x1, y1, x2, y1, color); // Top
    H::Draw.Line(x1, y2, x2, y2, color); // Bottom
    H::Draw.Line(x1, y1, x1, y2, color); // Left
    H::Draw.Line(x2, y1, x2, y2, color); // Right
}

bool CStickyESP::IsValidStickyTarget(CBaseEntity* pEntity, CBaseEntity* pLocal)
{
    if (!pEntity || !pLocal)
        return false;
    
    // Check if it's a stickybomb projectile
    if (pEntity->GetClassID() != ETFClassID::CTFGrenadePipebombProjectile)
        return false;
    
    // Check if entity is valid and not dormant
    if (pEntity->IsDormant())
        return false;
    
    return true;
}

