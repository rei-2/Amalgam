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
    
    if (!m_pCameraTexture)
    {
        m_pCameraTexture = I::MaterialSystem->CreateNamedRenderTargetTextureEx(
            "m_pEnemyCameraTexture",
            CAMERA_WIDTH,
            CAMERA_HEIGHT,
            RT_SIZE_NO_CHANGE,
            IMAGE_FORMAT_RGB888,
            MATERIAL_RT_DEPTH_SHARED,
            TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT,
            CREATERENDERTARGETFLAGS_HDR
        );
        
        if (m_pCameraTexture)
            m_pCameraTexture->IncrementReferenceCount();
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
    if (!Vars::Competitive::Features::EnemyCam.Value || !m_bEnabled || !m_bInitialized || !I::EngineClient->IsInGame())
        return;
    
    if (I::EngineVGui->IsGameUIVisible())
        return;
    
    if (!m_pCameraMaterial || !m_pCameraTexture)
        return;
    
    // Update target
    UpdateTargetPlayer();
    
    if (!m_pTargetPlayer)
        return;
    
    // Get screen size for positioning
    int screenW, screenH;
    I::MatSystemSurface->GetScreenSize(screenW, screenH);
    
    // Position camera in upper right corner
    int x = screenW - CAMERA_WIDTH - BORDER_OFFSET;
    int y = BORDER_OFFSET + 20; // Leave space for title
    
    // Draw camera to screen
    auto renderCtx = I::MaterialSystem->GetRenderContext();
    if (!renderCtx)
        return;
        
    renderCtx->DrawScreenSpaceRectangle(
        m_pCameraMaterial,
        x, y, CAMERA_WIDTH, CAMERA_HEIGHT,
        0, 0, CAMERA_WIDTH, CAMERA_HEIGHT,
        m_pCameraTexture->GetActualWidth(), m_pCameraTexture->GetActualHeight(),
        nullptr, 1, 1
    );
    renderCtx->Release();
    
    // Draw overlay
    DrawOverlay();
}

void CEnemyCam::RenderView(void* ecx, const CViewSetup& view)
{
    if (!m_bEnabled || !m_bInitialized || !m_pTargetPlayer || !m_pCameraTexture)
        return;
    
    // Get camera view
    Vec3 origin, angles;
    GetCameraView(origin, angles);
    
    // Setup view
    CViewSetup enemyView = view;
    enemyView.x = 0;
    enemyView.y = 0;
    enemyView.width = CAMERA_WIDTH;
    enemyView.height = CAMERA_HEIGHT;
    enemyView.m_flAspectRatio = static_cast<float>(CAMERA_WIDTH) / static_cast<float>(CAMERA_HEIGHT);
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
    switch (m_eMode)
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
    
    if (!m_pTargetPlayer || !m_pTargetPlayer->IsAlive() || 
        m_pTargetPlayer->IsDormant() || m_pTargetPlayer->InCond(TF_COND_STEALTHED))
    {
        needNewTarget = true;
    }
    
    // Auto-switch after track time
    if (m_pTargetPlayer && TRACK_TIME > 0 && 
        currentTime - m_flTargetSwitchTime >= TRACK_TIME)
    {
        needNewTarget = true;
    }
    
    if (needNewTarget)
    {
        m_pTargetPlayer = FindTargetPlayer();
        m_flTargetSwitchTime = currentTime;
    }
}

void CEnemyCam::GetCameraView(Vec3& origin, Vec3& angles)
{
    if (!m_pTargetPlayer)
        return;
    
    // Additional validation to ensure player is still valid
    if (m_pTargetPlayer->IsDormant() || !m_pTargetPlayer->IsAlive())
        return;
    
    // Get player eye position and angles
    try
    {
        origin = m_pTargetPlayer->GetEyePosition();
        angles = m_pTargetPlayer->GetEyeAngles();
    }
    catch (...)
    {
        // If position/angle access fails, use defaults or return
        return;
    }
    
    // Apply offset if configured
    if (m_eViewMode == EViewMode::OFFSET)
    {
        Vec3 forward, up;
        Math::AngleVectors(angles, &forward, nullptr, &up);
        
        // Apply forward offset with pitch adjustment
        float pitchFactor = 1.0f;
        if (angles.x > 60.0f)
            pitchFactor = 1.0f + ((angles.x - 60.0f) / 30.0f) * 7.5f;
        
        origin += forward * (FORWARD_OFFSET * pitchFactor);
        origin += up * UPWARD_OFFSET;
    }
}

void CEnemyCam::DrawOverlay()
{
    if (!m_pTargetPlayer)
        return;
    
    // Get screen size
    int screenW, screenH;
    I::MatSystemSurface->GetScreenSize(screenW, screenH);
    
    // Calculate positions
    int x = screenW - CAMERA_WIDTH - BORDER_OFFSET;
    int y = BORDER_OFFSET;
    
    // Draw title bar background
    H::Draw.FillRect(x, y, CAMERA_WIDTH, 20, {130, 26, 17, 255});
    H::Draw.LineRect(x, y, CAMERA_WIDTH, 20, {235, 64, 52, 255});
    
    // Draw camera border
    H::Draw.LineRect(x, y + 20, CAMERA_WIDTH, CAMERA_HEIGHT, {235, 64, 52, 255});
    
    // Draw player info
    if (!m_pTargetPlayer || !I::EngineClient)
        return;
        
    PlayerInfo_t pi{};
    if (I::EngineClient->GetPlayerInfo(m_pTargetPlayer->entindex(), &pi))
    {
        std::string playerName = pi.name;
        std::string className = GetClassName(m_pTargetPlayer->m_iClass());
        std::string title = playerName + " (" + className + ")";
        
        if (m_pTargetMedic)
        {
            PlayerInfo_t medicPi{};
            if (I::EngineClient->GetPlayerInfo(m_pTargetMedic->entindex(), &medicPi))
            {
                title += " + Medic: " + std::string(medicPi.name);
            }
        }
        
        H::Draw.String(H::Fonts.GetFont(FONT_ESP), x + CAMERA_WIDTH/2, y + 10, 
                      {255, 255, 255, 255}, ALIGN_CENTER, title.c_str());
    }
    
    // Draw health info
    if (!m_pTargetPlayer)
        return;
        
    // Additional validation to ensure player is still valid
    if (m_pTargetPlayer->IsDormant() || !m_pTargetPlayer->IsAlive())
        return;
        
    int health = 100;
    int maxHealth = 100;
    
    try
    {
        // Double-check pointer validity before dereferencing
        if (!m_pTargetPlayer)
            return;
            
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