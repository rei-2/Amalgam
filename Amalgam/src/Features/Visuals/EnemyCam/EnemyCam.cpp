#include "EnemyCam.h"
#include "../Materials/Materials.h"

void CEnemyCam::Initialize()
{
    if (!InitializeMaterials())
        return;
    
    m_bInitialized = true;
}

void CEnemyCam::Unload()
{
    CleanupMaterials();
    m_bInitialized = false;
}

bool CEnemyCam::InitializeMaterials()
{
    if (m_pCameraMaterial && m_pCameraTexture)
        return true;
    
    if (!m_pCameraMaterial)
    {
        KeyValues* kv = new KeyValues("UnlitGeneric");
        kv->SetString("$basetexture", "m_pEnemyCameraTexture");
        m_pCameraMaterial = F::Materials.Create("EnemyCameraMaterial", kv);
    }
    
    if (!m_pCameraTexture && I::MaterialSystem)
    {
        try
        {
            m_pCameraTexture = I::MaterialSystem->CreateNamedRenderTargetTextureEx(
                "m_pEnemyCameraTexture",
                GetCameraWidth(),
                GetCameraHeight(),
                RT_SIZE_NO_CHANGE,
                IMAGE_FORMAT_RGB888,
                MATERIAL_RT_DEPTH_SHARED,
                TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT,
                CREATERENDERTARGETFLAGS_HDR
            );
            
            if (m_pCameraTexture)
                m_pCameraTexture->IncrementReferenceCount();
        }
        catch (...)
        {
            m_pCameraTexture = nullptr;
        }
    }
    
    // Update tracked dimensions after successful initialization
    if (m_pCameraMaterial && m_pCameraTexture)
    {
        m_iLastWidth = GetCameraWidth();
        m_iLastHeight = GetCameraHeight();
    }
    
    return m_pCameraMaterial && m_pCameraTexture;
}

void CEnemyCam::CleanupMaterials()
{
    if (m_pCameraMaterial)
    {
        m_pCameraMaterial->DecrementReferenceCount();
        m_pCameraMaterial->DeleteIfUnreferenced();
        m_pCameraMaterial = nullptr;
    }
    
    if (m_pCameraTexture)
    {
        m_pCameraTexture->DecrementReferenceCount();
        m_pCameraTexture->DeleteIfUnreferenced();
        m_pCameraTexture = nullptr;
    }
}

void CEnemyCam::Draw()
{
    if (!Vars::Competitive::Features::EnemyCam.Value || !m_bEnabled || !m_bInitialized || !I::EngineClient->IsInGame() || !I::EngineClient->IsConnected())
        return;
    
    if (I::EngineVGui->IsGameUIVisible())
        return;
    
    // Check if materials need reloading due to size changes
    if (CheckMaterialsNeedReload())
    {
        CleanupMaterials();
        InitializeMaterials();
    }
    
    if (!m_pCameraMaterial || !m_pCameraTexture)
        return;
    
    // Update target
    UpdateTargetPlayer();
    
    if (!m_pTargetPlayer)
        return;
    
    // Final validation before rendering
    try {
        int entityIndex = m_pTargetPlayer->entindex();
        if (entityIndex <= 0 || entityIndex > 64)
        {
            m_pTargetPlayer = nullptr;
            return;
        }
        
        auto pEntityFromList = I::ClientEntityList->GetClientEntity(entityIndex);
        if (!pEntityFromList || pEntityFromList != m_pTargetPlayer)
        {
            m_pTargetPlayer = nullptr;
            return;
        }
    }
    catch (...) {
        m_pTargetPlayer = nullptr;
        return;
    }
    
    // Get configured camera position
    int x, y;
    GetCameraPosition(x, y);
    
    // Draw camera to screen
    if (!I::MaterialSystem)
        return;
        
    auto renderCtx = I::MaterialSystem->GetRenderContext();
    if (!renderCtx)
        return;
        
    try
    {
        if (m_pCameraTexture && m_pCameraMaterial)
        {
            renderCtx->DrawScreenSpaceRectangle(
                m_pCameraMaterial,
                x, y, GetCameraWidth(), GetCameraHeight(),
                0, 0, GetCameraWidth(), GetCameraHeight(),
                m_pCameraTexture->GetActualWidth(), m_pCameraTexture->GetActualHeight(),
                nullptr, 1, 1
            );
        }
        renderCtx->Release();
    }
    catch (...)
    {
        if (renderCtx)
            renderCtx->Release();
    }
    
    // Draw overlay
    DrawOverlay();
}

void CEnemyCam::RenderView(void* ecx, const CViewSetup& view)
{
    if (!m_bEnabled || !m_bInitialized || !m_pTargetPlayer || !m_pCameraTexture)
        return;
    
    // Validate target before rendering
    try {
        int entityIndex = m_pTargetPlayer->entindex();
        if (entityIndex <= 0 || entityIndex > 64)
        {
            m_pTargetPlayer = nullptr;
            return;
        }
        
        auto pEntityFromList = I::ClientEntityList->GetClientEntity(entityIndex);
        if (!pEntityFromList || pEntityFromList != m_pTargetPlayer)
        {
            m_pTargetPlayer = nullptr;
            return;
        }
    }
    catch (...) {
        m_pTargetPlayer = nullptr;
        return;
    }
    
    // Get camera view
    Vec3 origin, angles;
    GetCameraView(origin, angles);
    
    // Setup view
    CViewSetup enemyView = view;
    enemyView.x = 0;
    enemyView.y = 0;
    enemyView.width = GetCameraWidth();
    enemyView.height = GetCameraHeight();
    enemyView.m_flAspectRatio = static_cast<float>(GetCameraWidth()) / static_cast<float>(GetCameraHeight());
    enemyView.fov = 90;
    enemyView.origin = origin;
    enemyView.angles = angles;
    
    // Render to texture
    auto renderCtx = I::MaterialSystem->GetRenderContext();
    if (!renderCtx)
        return;
        
    renderCtx->PushRenderTargetAndViewport();
    renderCtx->SetRenderTarget(m_pCameraTexture);
    
    static auto ViewRender_RenderView = U::Hooks.m_mHooks["CViewRender_RenderView"];
    if (ViewRender_RenderView)
        ViewRender_RenderView->Call<void>(ecx, enemyView, VIEW_CLEAR_COLOR | VIEW_CLEAR_DEPTH, RENDERVIEW_UNSPECIFIED);
    
    renderCtx->PopRenderTargetAndViewport();
    renderCtx->Release();
}

std::vector<CTFPlayer*> CEnemyCam::GetEnemyPlayers()
{
    std::vector<CTFPlayer*> enemies;
    
    auto pLocal = H::Entities.GetLocal();
    if (!pLocal)
        return enemies;
    
    int localTeam = pLocal->m_iTeamNum();
    
    for (auto pEntity : H::Entities.GetGroup(EGroupType::PLAYERS_ALL))
    {
        auto pPlayer = pEntity->As<CTFPlayer>();
        if (!pPlayer || !pPlayer->IsAlive() || pPlayer->IsDormant())
            continue;
        
        if (pPlayer->m_iTeamNum() == localTeam)
            continue;
        
        // Skip cloaked spies
        if (pPlayer->InCond(TF_COND_STEALTHED))
            continue;
        
        enemies.push_back(pPlayer);
    }
    
    return enemies;
}

CTFPlayer* CEnemyCam::FindTargetPlayer()
{
    switch (static_cast<ECameraMode>(Vars::Competitive::EnemyCam::Mode.Value))
    {
        case ECameraMode::CLOSEST:
            return FindClosestEnemy();
        case ECameraMode::HEALED:
            return FindHealedPlayer(m_pTargetMedic);
        case ECameraMode::MEDIC:
            return FindEnemyMedic(m_pTargetMedic);
        case ECameraMode::TOP_SCORE:
            return FindTopScorePlayer();
        case ECameraMode::RANDOM:
        {
            auto enemies = GetEnemyPlayers();
            if (enemies.empty())
                return nullptr;
            return enemies[rand() % enemies.size()];
        }
    }
    
    return nullptr;
}

CTFPlayer* CEnemyCam::FindClosestEnemy()
{
    auto pLocal = H::Entities.GetLocal();
    if (!pLocal)
        return nullptr;
    
    auto enemies = GetEnemyPlayers();
    if (enemies.empty())
        return nullptr;
    
    CTFPlayer* closest = nullptr;
    float minDist = FLT_MAX;
    Vec3 localPos = pLocal->GetAbsOrigin();
    
    for (auto pEnemy : enemies)
    {
        Vec3 enemyPos = pEnemy->GetAbsOrigin();
        float dist = (enemyPos - localPos).Length();
        
        if (dist < minDist)
        {
            minDist = dist;
            closest = pEnemy;
        }
    }
    
    return closest;
}

CTFPlayer* CEnemyCam::FindHealedPlayer(CTFPlayer*& outMedic)
{
    auto enemies = GetEnemyPlayers();
    
    for (auto pPlayer : enemies)
    {
        if (pPlayer->InCond(TF_COND_HEALTH_OVERHEALED))
        {
            // Try to find the medic healing this player
            for (auto pMedic : enemies)
            {
                if (pMedic->m_iClass() == TF_CLASS_MEDIC)
                {
                    auto pMedigun = pMedic->GetWeaponFromSlot(SLOT_SECONDARY);
                    if (pMedigun)
                    {
                        auto pHealTarget = pMedigun->As<CWeaponMedigun>()->m_hHealingTarget().Get();
                        if (pHealTarget == pPlayer)
                        {
                            outMedic = pMedic;
                            return pPlayer;
                        }
                    }
                }
            }
            
            // Found healed player but no medic
            outMedic = nullptr;
            return pPlayer;
        }
    }
    
    // No healed players, fall back to closest
    outMedic = nullptr;
    return FindClosestEnemy();
}

CTFPlayer* CEnemyCam::FindEnemyMedic(CTFPlayer*& outTarget)
{
    auto enemies = GetEnemyPlayers();
    
    for (auto pPlayer : enemies)
    {
        if (pPlayer->m_iClass() == TF_CLASS_MEDIC)
        {
            // Try to find healing target
            auto pMedigun = pPlayer->GetWeaponFromSlot(SLOT_SECONDARY);
            if (pMedigun)
            {
                auto pHealTarget = pMedigun->As<CWeaponMedigun>()->m_hHealingTarget().Get();
                if (pHealTarget && pHealTarget->As<CTFPlayer>())
                    outTarget = pHealTarget->As<CTFPlayer>();
                else
                    outTarget = nullptr;
            }
            else
            {
                outTarget = nullptr;
            }
            
            return pPlayer;
        }
    }
    
    outTarget = nullptr;
    return nullptr;
}

CTFPlayer* CEnemyCam::FindTopScorePlayer()
{
    auto enemies = GetEnemyPlayers();
    if (enemies.empty())
        return nullptr;
    
    CTFPlayer* topPlayer = nullptr;
    int maxScore = -1;
    
    auto pResource = H::Entities.GetPR();
    if (!pResource)
        return enemies[0]; // Fallback to first enemy
    
    for (auto pEnemy : enemies)
    {
        int score = pResource->m_iTotalScore(pEnemy->entindex());
        if (score > maxScore)
        {
            maxScore = score;
            topPlayer = pEnemy;
        }
    }
    
    return topPlayer ? topPlayer : enemies[0];
}

void CEnemyCam::UpdateTargetPlayer()
{
    if (!I::GlobalVars)
        return;
        
    float currentTime = I::GlobalVars->curtime;
    
    // Only search periodically
    if (currentTime - m_flLastSearchTime < SEARCH_INTERVAL)
        return;
    
    m_flLastSearchTime = currentTime;
    
    // Check if we need a new target
    bool needNewTarget = false;
    
    // Validate current target with comprehensive checks
    if (!m_pTargetPlayer)
    {
        needNewTarget = true;
    }
    else
    {
        try
        {
            // Validate entity is still in the entity list
            bool bValidEntity = false;
            for (int i = 1; i <= 64; i++)
            {
                auto pEntity = I::ClientEntityList->GetClientEntity(i);
                if (pEntity == m_pTargetPlayer)
                {
                    bValidEntity = true;
                    break;
                }
            }
            
            if (!bValidEntity || !m_pTargetPlayer->IsAlive() || 
                m_pTargetPlayer->IsDormant() || m_pTargetPlayer->InCond(TF_COND_STEALTHED))
            {
                needNewTarget = true;
            }
        }
        catch (...)
        {
            // If any validation fails, need new target and clear current one
            m_pTargetPlayer = nullptr;
            needNewTarget = true;
        }
    }
    
    // Auto-switch after track time (only if we still have a valid target)
    if (m_pTargetPlayer && !needNewTarget && Vars::Competitive::EnemyCam::TrackTime.Value > 0 && 
        currentTime - m_flTargetSwitchTime >= Vars::Competitive::EnemyCam::TrackTime.Value)
    {
        needNewTarget = true;
    }
    
    if (needNewTarget)
    {
        CTFPlayer* pNewTarget = FindTargetPlayer();
        
        // Validate the new target before assigning it
        if (pNewTarget)
        {
            try
            {
                // Test if the entity is valid by calling basic methods
                if (!pNewTarget->IsDormant() && pNewTarget->IsAlive())
                {
                    int testIndex = pNewTarget->entindex();
                    if (testIndex > 0 && testIndex <= 64)
                    {
                        m_pTargetPlayer = pNewTarget;
                        m_flTargetSwitchTime = currentTime;
                    }
                }
            }
            catch (...)
            {
                // If validation fails, don't set the target
                m_pTargetPlayer = nullptr;
            }
        }
        else
        {
            m_pTargetPlayer = nullptr;
        }
    }
}

void CEnemyCam::GetCameraView(Vec3& origin, Vec3& angles)
{
    if (!m_pTargetPlayer)
        return;
    
    // Robust entity validation before accessing any methods
    try
    {
        // First validate that the entity is still in the entity list
        bool bValidEntity = false;
        int targetIndex = 0;
        
        // Try to get entindex safely
        targetIndex = m_pTargetPlayer->entindex();
        if (targetIndex <= 0 || targetIndex > 64)
        {
            m_pTargetPlayer = nullptr;
            return;
        }
        
        // Check if entity is still in the entity list
        auto pEntityFromList = I::ClientEntityList->GetClientEntity(targetIndex);
        if (pEntityFromList != m_pTargetPlayer)
        {
            m_pTargetPlayer = nullptr;
            return;
        }
        
        // Entity is valid, proceed with getting camera data
        origin = m_pTargetPlayer->GetEyePosition();
        angles = m_pTargetPlayer->GetEyeAngles();
    }
    catch (...)
    {
        // If any access fails, clear target and return
        m_pTargetPlayer = nullptr;
        return;
    }
    
    // Apply offset if configured
    if (static_cast<EViewMode>(Vars::Competitive::EnemyCam::ViewMode.Value) == EViewMode::OFFSET)
    {
        Vec3 forward, up;
        Math::AngleVectors(angles, &forward, nullptr, &up);
        
        // Apply configurable X offset (forward/back) with pitch adjustment
        float pitchFactor = 1.0f;
        if (angles.x > 60.0f)
            pitchFactor = 1.0f + ((angles.x - 60.0f) / 30.0f) * 7.5f;
        
        float offsetX = Vars::Competitive::EnemyCam::OffsetX.Value;
        float offsetY = Vars::Competitive::EnemyCam::OffsetY.Value;
        
        origin += forward * (offsetX * pitchFactor);
        origin += up * offsetY;
    }
}

void CEnemyCam::DrawOverlay()
{
    if (!m_pTargetPlayer)
        return;
    
    // Get configured camera position
    int x, y;
    GetCameraPosition(x, y);
    y -= 20; // Adjust for title bar
    
    // Get team color for frame
    auto pLocal = H::Entities.GetLocal();
    Color_t frameColor = {235, 64, 52, 255}; // Default red
    Color_t fillColor = {130, 26, 17, 255}; // Default dark red
    
    if (pLocal)
    {
        frameColor = H::Color.GetTeamColor(pLocal->m_iTeamNum(), pLocal->m_iTeamNum(), false);
        // Make fill color darker by reducing RGB values
        fillColor = {
            static_cast<byte>(frameColor.r * 0.55f),
            static_cast<byte>(frameColor.g * 0.55f), 
            static_cast<byte>(frameColor.b * 0.55f),
            255
        };
    }
    
    // Draw team-colored title bar background
    H::Draw.FillRect(x, y, GetCameraWidth(), 20, fillColor);
    H::Draw.LineRect(x, y, GetCameraWidth(), 20, frameColor);
    
    // Draw team-colored camera border
    H::Draw.LineRect(x, y + 20, GetCameraWidth(), GetCameraHeight(), frameColor);
    
    // Draw player info
    if (!m_pTargetPlayer || !I::EngineClient)
        return;
        
    // Basic crash protection - validate through entity list first
    try
    {
        // Instead of calling methods on potentially invalid pointer,
        // check if this entity still exists in the entity list
        bool entityFound = false;
        for (int i = 1; i <= 64; ++i)
        {
            auto pEntity = I::ClientEntityList->GetClientEntity(i);
            if (pEntity == m_pTargetPlayer)
            {
                // Entity still exists at this index
                if (!pEntity->IsDormant())
                {
                    entityFound = true;
                    break;
                }
            }
        }
        
        if (!entityFound)
        {
            m_pTargetPlayer = nullptr;
            return;
        }
        
        // Now safe to access methods since we confirmed entity exists
        int entIndex = m_pTargetPlayer->entindex();
        if (entIndex <= 0 || entIndex > 64)
        {
            m_pTargetPlayer = nullptr;
            return;
        }
    }
    catch (...)
    {
        // If any access fails, clear it and skip drawing
        m_pTargetPlayer = nullptr;
        return;
    }
        
    PlayerInfo_t pi{};
    if (I::EngineClient->GetPlayerInfo(m_pTargetPlayer->entindex(), &pi))
    {
        std::string playerName = pi.name;
        std::string className = GetClassName(m_pTargetPlayer->m_iClass());
        std::string title = playerName + " (" + className + ")";
        
        if (m_pTargetMedic)
        {
            // Basic crash protection for medic info
            try
            {
                PlayerInfo_t medicPi{};
                if (I::EngineClient->GetPlayerInfo(m_pTargetMedic->entindex(), &medicPi))
                {
                    title += " + Medic: " + std::string(medicPi.name);
                }
            }
            catch (...)
            {
                // If medic info fails, just skip it (don't clear the pointer)
            }
        }
        
        H::Draw.String(H::Fonts.GetFont(FONT_ESP), x + GetCameraWidth()/2, y + 10, 
                      {255, 255, 255, 255}, ALIGN_CENTER, title.c_str());
    }
    
    // Draw health info - player already validated above
    if (!m_pTargetPlayer)
        return;
        
    int health = 100;
    int maxHealth = 100;
    
    try
    {
        // Additional validation before accessing methods
        if (m_pTargetPlayer->IsDormant())
        {
            m_pTargetPlayer = nullptr;
            return;
        }
        
        // Verify entity is still in entity list at same index
        int targetIndex = m_pTargetPlayer->entindex();
        if (targetIndex <= 0 || targetIndex > 64)
        {
            m_pTargetPlayer = nullptr;
            return;
        }
        
        auto pEntityFromList = I::ClientEntityList->GetClientEntity(targetIndex);
        if (pEntityFromList != m_pTargetPlayer)
        {
            m_pTargetPlayer = nullptr;
            return;
        }
            
        health = m_pTargetPlayer->m_iHealth();
        maxHealth = m_pTargetPlayer->GetMaxHealth();
        
        // Prevent division by zero
        if (maxHealth <= 0)
            maxHealth = 1;
    }
    catch (...)
    {
        // If health access fails, use defaults
        health = 100;
        maxHealth = 100;
    }
        
    float healthPercent = static_cast<float>(health) / static_cast<float>(maxHealth);
    
    Color_t healthColor = {
        static_cast<byte>(255 * (1.0f - healthPercent)),
        static_cast<byte>(255 * healthPercent),
        0, 255
    };
    
    H::Draw.FillRect(x + 5, y + 25, 100, 15, {0, 0, 0, 180});
    
    std::string healthText = std::format("HP: {}/{}", health, maxHealth);
    H::Draw.String(H::Fonts.GetFont(FONT_ESP), x + 10, y + 28, 
                  healthColor, ALIGN_TOPLEFT, healthText.c_str());
}

const char* CEnemyCam::GetClassName(int classId)
{
    switch (classId)
    {
        case TF_CLASS_SCOUT: return "Scout";
        case TF_CLASS_SNIPER: return "Sniper";
        case TF_CLASS_SOLDIER: return "Soldier";
        case TF_CLASS_DEMOMAN: return "Demoman";
        case TF_CLASS_MEDIC: return "Medic";
        case TF_CLASS_HEAVY: return "Heavy";
        case TF_CLASS_PYRO: return "Pyro";
        case TF_CLASS_SPY: return "Spy";
        case TF_CLASS_ENGINEER: return "Engineer";
        default: return "Unknown";
    }
}

void CEnemyCam::GetCameraPosition(int& x, int& y) const
{
    // Get screen size
    int screenW, screenH;
    I::MatSystemSurface->GetScreenSize(screenW, screenH);
    
    // Check if user has set custom position (WindowX/WindowY >= 0)
    if (Vars::Competitive::EnemyCam::WindowX.Value >= 0 && Vars::Competitive::EnemyCam::WindowY.Value >= 0)
    {
        // Use custom position
        x = Vars::Competitive::EnemyCam::WindowX.Value;
        y = Vars::Competitive::EnemyCam::WindowY.Value;
        
        // Clamp to screen bounds
        x = std::max(0, std::min(x, screenW - GetCameraWidth()));
        y = std::max(20, std::min(y, screenH - GetCameraHeight())); // Leave space for title bar
    }
    else
    {
        // Use default position (upper right corner)
        x = screenW - GetCameraWidth() - BORDER_OFFSET;
        y = BORDER_OFFSET + 20; // Leave space for title bar
    }
}

bool CEnemyCam::CheckMaterialsNeedReload()
{
    int currentWidth = GetCameraWidth();
    int currentHeight = GetCameraHeight();
    
    // Check if size has changed since last initialization
    if (m_iLastWidth != currentWidth || m_iLastHeight != currentHeight)
    {
        // Update tracked dimensions
        m_iLastWidth = currentWidth;
        m_iLastHeight = currentHeight;
        return true;
    }
    
    return false;
}

void CEnemyCam::Reset()
{
    m_pTargetPlayer = nullptr;
    m_pTargetMedic = nullptr;
    m_flTargetSwitchTime = 0.0f;
    m_flLastSearchTime = 0.0f;
    m_bEnabled = true; // Re-enable camera after reset
}