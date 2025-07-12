#include "SpectateAll.h"
#include "../../../SDK/SDK.h"

bool CSpectateAll::ShouldSpectate()
{
    if (!Vars::Competitive::Features::SpectateAll.Value)
        return false;
        
    auto pLocal = H::Entities.GetLocal();
    if (!pLocal)
        return false;
    
    // Only allow spectating when dead
    bool isAlive = pLocal->IsAlive();
    bool shouldActivate = !isAlive;
    
    // If enabled, skip map cameras by sending next spectate command
    if (shouldActivate && Vars::Competitive::SpectateAll::ExcludeMapCameras.Value)
    {
        int observerMode = pLocal->m_iObserverMode();
        if (observerMode == OBS_MODE_FIXED)
        {
            // TF2 is showing a static map camera, skip to next player
            static float lastSkipTime = 0.0f;
            float currentTime = I::GlobalVars->curtime;
            
            // Prevent spam by limiting skip frequency
            if (currentTime - lastSkipTime > 0.1f)
            {
                // Send spec_next command to skip to next target
                I::EngineClient->ClientCmd_Unrestricted("spec_next");
                lastSkipTime = currentTime;
            }
            
            return false; // Don't override while skipping
        }
    }
    
    // Store origin when transitioning from alive to dead
    if (m_bWasAlive && !isAlive)
    {
        m_vStoredOrigin = pLocal->m_vecOrigin();
        m_bIsSpectating = false;
        m_bInFreeCam = false;
        m_iCurrentEnemyIndex = 0;
    }
    // Reset when respawning
    else if (!m_bWasAlive && isAlive)
    {
        m_bIsSpectating = false;
        m_bInFreeCam = false;
    }
    
    m_bWasAlive = isAlive;
    return shouldActivate;
}

SpectateMode CSpectateAll::GetCurrentMode()
{
    // If manually in freecam mode (via mouse2), override other settings
    if (m_bInFreeCam)
        return SpectateMode::FREE_CAMERA;
    else if (Vars::Competitive::SpectateAll::FreeCamera.Value)
        return SpectateMode::FREE_CAMERA;
    else if (Vars::Competitive::SpectateAll::EnemySpectate.Value)
        return SpectateMode::ENEMY;
    else
        return SpectateMode::NORMAL;
}

void CSpectateAll::HandleMouseInput()
{
    // Handle mouse input for player cycling and freecam
    static bool mouse1Pressed = false;
    static bool mouse2Pressed = false;
    bool mouse1Down = GetAsyncKeyState(VK_LBUTTON) & 0x8000;
    bool mouse2Down = GetAsyncKeyState(VK_RBUTTON) & 0x8000;
    
    // Mouse2 (right click) - toggle freecam mode
    if (mouse2Down && !mouse2Pressed)
    {
        m_bInFreeCam = !m_bInFreeCam;
        
        // When entering freecam, store current camera position
        if (m_bInFreeCam && m_pCurrentSpectatedPlayer)
        {
            // Store the current spectated player's position as starting point
            m_vStoredOrigin = m_pCurrentSpectatedPlayer->GetEyePosition();
        }
        else if (m_bInFreeCam)
        {
            // If no current player, use stored origin or current view
            // m_vStoredOrigin should already be set from initialization
        }
    }
    
    // Mouse1 (left click) - cycle players (and exit freecam if active)
    if (mouse1Down && !mouse1Pressed)
    {
        // If in freecam, exit freecam and cycle to next player
        if (m_bInFreeCam)
        {
            m_bInFreeCam = false;
        }
        
        // Cycle to next enemy player
        auto enemies = H::Entities.GetGroup(EGroupType::PLAYERS_ENEMIES);
        if (!enemies.empty())
        {
            m_iCurrentEnemyIndex = (m_iCurrentEnemyIndex + 1) % enemies.size();
        }
    }
    
    mouse1Pressed = mouse1Down;
    mouse2Pressed = mouse2Down;
}

CTFPlayer* CSpectateAll::GetNextEnemyPlayer()
{
    auto pLocal = H::Entities.GetLocal();
    if (!pLocal)
        return nullptr;
        
    auto enemies = H::Entities.GetGroup(EGroupType::PLAYERS_ENEMIES);
    if (enemies.empty())
        return nullptr;
    
    // Ensure valid index
    if (m_iCurrentEnemyIndex >= enemies.size())
        m_iCurrentEnemyIndex = 0;
    
    if (m_iCurrentEnemyIndex < enemies.size())
    {
        auto pEnemy = enemies[m_iCurrentEnemyIndex]->As<CTFPlayer>();
        if (pEnemy && pEnemy->IsAlive())
            return pEnemy;
    }
    
    return nullptr;
}

void CSpectateAll::HandleFreeCamera(CViewSetup* pView)
{
    // Calculate movement speed with modifiers
    float speed = Vars::Competitive::SpectateAll::CameraSpeed.Value;
    if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
        speed *= SPEED_MODIFIER_SLOW;
    if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
        speed *= SPEED_MODIFIER_FAST;
    
    // Calculate movement vectors
    Vector forward, right, up;
    Math::AngleVectors(pView->angles, &forward, &right, &up);
    
    // Handle WASD movement
    if (GetAsyncKeyState('W') & 0x8000)
        m_vStoredOrigin += forward * speed;
    if (GetAsyncKeyState('S') & 0x8000)
        m_vStoredOrigin -= forward * speed;
    if (GetAsyncKeyState('A') & 0x8000)
        m_vStoredOrigin -= right * speed;
    if (GetAsyncKeyState('D') & 0x8000)
        m_vStoredOrigin += right * speed;
    
    // Handle vertical movement (use Q/E instead of Space/C to avoid conflicts)
    if (GetAsyncKeyState('Q') & 0x8000)
        m_vStoredOrigin += up * speed;
    if (GetAsyncKeyState('E') & 0x8000)
        m_vStoredOrigin -= up * speed;
    
    // Apply the new camera position
    pView->origin = m_vStoredOrigin;
}

void CSpectateAll::HandleEnemySpectate(CViewSetup* pView)
{
    auto pTarget = GetNextEnemyPlayer();
    if (pTarget)
    {
        // Update current spectated player
        m_pCurrentSpectatedPlayer = pTarget;
        
        // Set view to player's perspective first
        pView->origin = pTarget->GetEyePosition();
        
        if (m_bThirdPersonMode)
        {
            // Always use mouse look in third person
            // Initialize third person angles if switching
            static bool wasFirstPerson = true;
            if (wasFirstPerson)
            {
                m_vThirdPersonAngles = pTarget->GetEyeAngles();
                wasFirstPerson = false;
            }
            
            // Use stored third person angles for mouse look
            pView->angles = m_vThirdPersonAngles;
            
            ApplyThirdPersonView(pView);
        }
        else
        {
            // First person mode - use player's exact angles
            pView->angles = pTarget->GetEyeAngles();
        }
    }
    else
    {
        m_pCurrentSpectatedPlayer = nullptr;
    }
}

void CSpectateAll::ApplyThirdPersonView(CViewSetup* pView)
{
    // Move camera back from the view position for third person view
    Vector forward, up;
    Math::AngleVectors(pView->angles, &forward, nullptr, &up);
    
    Vector originalOrigin = pView->origin;
    
    // Move back and slightly up for better third person view
    pView->origin -= forward * Vars::Competitive::SpectateAll::CameraDistance.Value;
    pView->origin += up * 50.0f;
    
    // Trace to make sure we don't go through walls
    trace_t trace;
    CTraceFilterWorldAndPropsOnly filter;
    SDK::Trace(originalOrigin, pView->origin, MASK_SOLID, &filter, &trace);
    
    if (trace.fraction < 1.0f)
    {
        // Adjust position if we hit something
        pView->origin = trace.endpos + forward * 10.0f;
    }
}

bool CSpectateAll::ShouldHidePlayer(CTFPlayer* pPlayer)
{
    if (!Vars::Competitive::Features::SpectateAll.Value)
        return false;
        
    if (!Vars::Competitive::SpectateAll::HideSpectatedPlayer.Value)
        return false;
        
    // Only hide when in first person mode and actively spectating this player
    SpectateMode mode = GetCurrentMode();
    if (mode == SpectateMode::ENEMY && !m_bThirdPersonMode && m_pCurrentSpectatedPlayer == pPlayer)
        return true;
        
    return false;
}

bool CSpectateAll::ShouldHideEntity(CBaseEntity* pEntity)
{
    if (!Vars::Competitive::Features::SpectateAll.Value)
        return false;
        
    if (!m_pCurrentSpectatedPlayer)
        return false;
        
    SpectateMode mode = GetCurrentMode();
    if (mode != SpectateMode::ENEMY)
        return false;
        
    // Hide weapons and cosmetics belonging to the spectated player
    auto classID = pEntity->GetClassID();
    
    // Check for weapons - only hide if weapon hiding is enabled AND we're in first person mode
    if (Vars::Competitive::SpectateAll::HideSpectatedWeapons.Value && !m_bThirdPersonMode)
    {
        // Check by class ID first
        if (classID == ETFClassID::CTFWeaponBase || 
            classID == ETFClassID::CTFWeaponBaseGun ||
            classID == ETFClassID::CTFWeaponBaseMelee ||
            classID == ETFClassID::CBaseCombatWeapon)
        {
            auto pWeapon = pEntity->As<CTFWeaponBase>();
            if (pWeapon)
            {
                // Check both owner and parent relationships
                auto pOwner = pWeapon->m_hOwner().Get();
                auto pParent = pEntity->m_hOwnerEntity().Get();
                
                if (pOwner == m_pCurrentSpectatedPlayer || pParent == m_pCurrentSpectatedPlayer)
                    return true;
            }
        }
        
        // Also check by model path for all weapon models
        auto pModel = pEntity->GetModel();
        if (pModel)
        {
            const char* modelName = I::ModelInfoClient->GetModelName(pModel);
            if (modelName)
            {
                std::string modelPath(modelName);
                // Check for both world and view weapon models
                if (modelPath.find("models/weapons/") != std::string::npos ||
                    modelPath.find("models/workshop/weapons/") != std::string::npos ||
                    modelPath.find("/weapons/") != std::string::npos)
                {
                    // Check multiple ownership relationships
                    auto pOwner = pEntity->m_hOwnerEntity().Get();
                    
                    if (pOwner == m_pCurrentSpectatedPlayer)
                        return true;
                        
                    // For weapons without clear ownership, check proximity
                    if (!pOwner && m_pCurrentSpectatedPlayer)
                    {
                        Vector weaponPos = pEntity->m_vecOrigin();
                        Vector playerPos = m_pCurrentSpectatedPlayer->m_vecOrigin();
                        float distance = (weaponPos - playerPos).Length();
                        
                        // If weapon is very close to player (within 100 units), likely belongs to them
                        if (distance < 100.0f)
                            return true;
                    }
                }
            }
        }
    }
    
    // Check for wearables/cosmetics (always hide with player model)
    if (Vars::Competitive::SpectateAll::HideSpectatedPlayer.Value &&
        (classID == ETFClassID::CTFWearable || classID == ETFClassID::CTFWearableDemoShield))
    {
        // Get the owner of the wearable
        auto pWearable = pEntity->As<CBaseEntity>();
        if (pWearable)
        {
            // Check if it belongs to our spectated player
            auto pOwner = pWearable->m_hOwnerEntity().Get();
            if (pOwner == m_pCurrentSpectatedPlayer)
                return true;
        }
    }
    
    return false;
}

void CSpectateAll::OverrideView(CViewSetup* pView)
{
    if (!ShouldSpectate() || !pView)
    {
        // Clear spectated player when not spectating
        m_pCurrentSpectatedPlayer = nullptr;
        return;
    }
    
    auto pLocal = H::Entities.GetLocal();
    if (!pLocal)
        return;
    
    // Handle mouse input for freecam and player cycling
    HandleMouseInput();
    
    // Handle different spectate modes
    SpectateMode mode = GetCurrentMode();
    
    // Handle Space key toggle for first/third person (only in Enemy Spectate mode and not in freecam)
    if (mode == SpectateMode::ENEMY && !m_bInFreeCam)
    {
        static bool spacePressed = false;
        bool spaceDown = GetAsyncKeyState(VK_SPACE) & 0x8000;
        
        if (spaceDown && !spacePressed)
        {
            m_bThirdPersonMode = !m_bThirdPersonMode;
            
            // Initialize third person angles when switching to third person
            if (m_bThirdPersonMode && m_pCurrentSpectatedPlayer)
            {
                m_vThirdPersonAngles = m_pCurrentSpectatedPlayer->GetEyeAngles();
            }
        }
        spacePressed = spaceDown;
        
        // Handle mouse input for third person mode
        if (m_bThirdPersonMode)
        {
            // Get mouse delta (this would need proper mouse input handling)
            // For now, we'll use a simple approach with engine's view angles
            QAngle engineAngles;
            I::EngineClient->GetViewAngles(engineAngles);
            
            // Update third person angles based on mouse movement
            static QAngle lastEngineAngles = engineAngles;
            QAngle deltaAngles = engineAngles - lastEngineAngles;
            
            m_vThirdPersonAngles += deltaAngles;
            
            // Clamp pitch
            if (m_vThirdPersonAngles.x > 89.0f)
                m_vThirdPersonAngles.x = 89.0f;
            if (m_vThirdPersonAngles.x < -89.0f)
                m_vThirdPersonAngles.x = -89.0f;
                
            lastEngineAngles = engineAngles;
        }
    }
    else if (!m_bInFreeCam)
    {
        // Clear spectated player when not in enemy spectate mode and not in freecam
        m_pCurrentSpectatedPlayer = nullptr;
    }
    
    // Initialize spectate mode if not already active
    if (!m_bIsSpectating)
    {
        m_vStoredOrigin = pView->origin;
        m_vStoredAngles = pView->angles;
        m_bIsSpectating = true;
    }
    
    switch (mode)
    {
        case SpectateMode::FREE_CAMERA:
            HandleFreeCamera(pView);
            break;
            
        case SpectateMode::ENEMY:
            HandleEnemySpectate(pView);
            break;
            
        case SpectateMode::NORMAL:
        default:
            // Default TF2 spectating behavior
            break;
    }
}