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
    
    // Handle special observer modes
    if (shouldActivate)
    {
        int observerMode = pLocal->m_iObserverMode();
        
        // Track and handle freezecam transitions with audio-safe delays
        if (observerMode == OBS_MODE_FREEZECAM)
        {
            // Remember what mode we should return to (if we haven't already stored it)
            if (m_iLastObserverMode == OBS_MODE_NONE)
            {
                // Default to third person if no previous mode
                m_iLastObserverMode = OBS_MODE_THIRDPERSON;
            }
            
            // Add audio-safe delay to prevent audio system conflicts
            static float lastModeChange = 0;
            float currentTime = I::GlobalVars->realtime;
            if (currentTime - lastModeChange > 0.15f) // 150ms delay for audio stability
            {
                // Force out of freezecam using the remembered mode
                pLocal->m_iObserverMode() = m_iLastObserverMode;
                pLocal->m_hObserverTarget().Set(nullptr);
                
                // Clear freezecam viewangles to prevent inheritance
                QAngle neutralAngles(0.0f, 0.0f, 0.0f);
                I::EngineClient->SetViewAngles(neutralAngles);
                
                // Reset our stored angles to prevent contamination
                m_vThirdPersonAngles = neutralAngles;
                m_bForceAngleReset = true;
                
                lastModeChange = currentTime;
            }
        }
        else if (observerMode != OBS_MODE_FREEZECAM && observerMode != OBS_MODE_NONE)
        {
            // Remember non-freezecam modes for later restoration
            m_iLastObserverMode = observerMode;
        }
        
        // If enabled, skip map cameras by sending next spectate command
        if (Vars::Competitive::SpectateAll::ExcludeMapCameras.Value && observerMode == OBS_MODE_FIXED)
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
        
        // If enabled, skip objective cameras by detecting objective entities
        if (Vars::Competitive::SpectateAll::ExcludeObjectiveCameras.Value)
        {
            auto pTarget = pLocal->m_hObserverTarget().Get();
            if (pTarget && !pTarget->IsPlayer())
            {
                // Additional safety check - ensure we can get the class ID safely
                ETFClassID classID;
                bool canGetClassID = false;
                try 
                {
                    classID = pTarget->GetClassID();
                    canGetClassID = true;
                }
                catch (...)
                {
                    // If we can't get class ID safely, skip this check
                    canGetClassID = false;
                }
                
                if (canGetClassID)
                {
                    // Check if spectating objective entities
                    bool isObjectiveEntity = (classID == ETFClassID::CCaptureFlag ||     // Intelligence/briefcase
                                            classID == ETFClassID::CTeamTrainWatcher ||   // Payload cart watcher
                                            classID == ETFClassID::CCaptureZone);         // Capture point zones
                    
                    if (isObjectiveEntity)
                    {
                        // TF2 is showing an objective camera, skip to next target
                        static float lastObjectiveSkipTime = 0.0f;
                        float currentTime = I::GlobalVars->curtime;
                        
                        // Prevent spam by limiting skip frequency
                        if (currentTime - lastObjectiveSkipTime > 0.1f)
                        {
                            // Send spec_next command to skip to next target
                            I::EngineClient->ClientCmd_Unrestricted("spec_next");
                            lastObjectiveSkipTime = currentTime;
                        }
                        
                        return false; // Don't override while skipping
                    }
                }
            }
        }
    }
    
    // Store origin when transitioning from alive to dead
    if (m_bWasAlive && !isAlive)
    {
        m_vStoredOrigin = pLocal->m_vecOrigin();
        m_bIsSpectating = false;
        m_bInFreeCam = false;
        m_iCurrentEnemyIndex = 0;
        m_iLastObserverMode = OBS_MODE_NONE; // Reset observer mode tracking on death
    }
    // Reset when respawning
    else if (!m_bWasAlive && isAlive)
    {
        m_bIsSpectating = false;
        m_bInFreeCam = false;
        m_iLastObserverMode = OBS_MODE_NONE; // Reset observer mode tracking
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
    static bool mouse3Pressed = false;
    bool mouse1Down = GetAsyncKeyState(VK_LBUTTON) & 0x8000;
    bool mouse2Down = GetAsyncKeyState(VK_RBUTTON) & 0x8000;
    bool mouse3Down = GetAsyncKeyState(VK_MBUTTON) & 0x8000;
    
    auto pLocal = H::Entities.GetLocal();
    bool isAlive = pLocal && pLocal->IsAlive();
    bool isTaunting = pLocal && pLocal->IsTaunting();
    
    // Original Mouse2 (right click) behavior for when dead (spectating)
    if (!isAlive && (mouse2Down && !mouse2Pressed))
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
        
        // Reset regular spectate system when toggling freecam
        F::Spectate.m_iTarget = F::Spectate.m_iIntendedTarget = -1;
    }
    
    // New functionality: Allow freecam activation when alive and taunting
    if (isAlive && isTaunting)
    {
        bool freecamActivate = false;
        if (mouse1Down && !mouse1Pressed)
            freecamActivate = true;
        if (mouse2Down && !mouse2Pressed)
            freecamActivate = true;
        if (mouse3Down && !mouse3Pressed)
            freecamActivate = true;
            
        if (freecamActivate && !m_bInFreeCam)
        {
            // Start freecam instantly when taunting
            m_bInFreeCam = true;
            m_bFreecamStartedDuringTaunt = true;
            
            // Start freecam from player's current position
            if (pLocal)
            {
                m_vStoredOrigin = pLocal->GetEyePosition();
            }
            
            // Reset regular spectate system when toggling freecam
            F::Spectate.m_iTarget = F::Spectate.m_iIntendedTarget = -1;
        }
    }
    
    // Auto-exit freecam when taunt ends (if freecam was started during taunt)
    if (isAlive && !isTaunting && m_bFreecamStartedDuringTaunt)
    {
        m_bInFreeCam = false;
        m_bFreecamStartedDuringTaunt = false;
    }
    
    // Clear taunt freecam flag when player dies
    if (!isAlive && m_bFreecamStartedDuringTaunt)
    {
        m_bFreecamStartedDuringTaunt = false;
    }
    
    // Mouse1 (left click) - cycle players (and exit freecam if active)
    // Only when not alive and taunting (since Mouse1 is used for freecam in that case)
    if (mouse1Down && !mouse1Pressed && !(isAlive && isTaunting))
    {
        // If in freecam, exit freecam and cycle to next player
        if (m_bInFreeCam)
        {
            m_bInFreeCam = false;
        }
        
        // Manual cycle to next enemy player (clear killer preference since user is manually cycling)
        m_pLastKiller = nullptr; // Clear killer preference when manually cycling
        
        auto enemies = H::Entities.GetGroup(EGroupType::PLAYERS_ENEMIES);
        if (!enemies.empty())
        {
            // Find next alive enemy player
            int startIndex = m_iCurrentEnemyIndex;
            int attempts = 0;
            
            do 
            {
                m_iCurrentEnemyIndex = (m_iCurrentEnemyIndex + 1) % enemies.size();
                attempts++;
                
                if (m_iCurrentEnemyIndex < enemies.size())
                {
                    auto pEnemy = enemies[m_iCurrentEnemyIndex]->As<CTFPlayer>();
                    if (pEnemy && pEnemy->IsAlive() && !pEnemy->IsDormant())
                    {
                        // Found next alive player
                        break;
                    }
                }
            } 
            while (m_iCurrentEnemyIndex != startIndex && attempts < enemies.size());
        }
    }
    
    mouse1Pressed = mouse1Down;
    mouse2Pressed = mouse2Down;
    mouse3Pressed = mouse3Down;
}

CTFPlayer* CSpectateAll::GetNextEnemyPlayer()
{
    auto pLocal = H::Entities.GetLocal();
    if (!pLocal)
        return nullptr;
        
    auto enemies = H::Entities.GetGroup(EGroupType::PLAYERS_ENEMIES);
    if (enemies.empty())
        return nullptr;
    
    // If we have a killer stored and prefer killer spectating is enabled, try to spectate them first
    if (Vars::Competitive::SpectateAll::PreferKillerSpectate.Value && m_pLastKiller && m_pLastKiller->IsAlive())
    {
        // Find our killer in the enemies list
        for (size_t i = 0; i < enemies.size(); i++)
        {
            auto pEnemy = enemies[i]->As<CTFPlayer>();
            if (pEnemy == m_pLastKiller)
            {
                m_iCurrentEnemyIndex = i;
                return m_pLastKiller;
            }
        }
        // Killer not found in enemies list (maybe disconnected), clear it
        m_pLastKiller = nullptr;
    }
    
    // Normal enemy selection logic
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
    
    // Apply the new camera position and let engine handle mouse input
    pView->origin = m_vStoredOrigin;
    // Don't override view angles - let the engine handle mouse input naturally
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
            // Initialize third person angles when switching players or modes
            static CTFPlayer* lastTargetPlayer = nullptr;
            if (lastTargetPlayer != pTarget)
            {
                // New player - use neutral angles to prevent inheriting corrupted viewangles
                // Only inherit from player if we don't have valid stored angles
                if (m_vThirdPersonAngles.Length() < 0.1f || !IsValidAngle(m_vThirdPersonAngles))
                {
                    // Use a neutral behind-the-player view instead of player's potentially corrupted eye angles
                    QAngle playerAngles = pTarget->GetEyeAngles();
                    
                    // Validate player angles before using them
                    if (IsValidAngle(playerAngles))
                    {
                        m_vThirdPersonAngles.x = 0.0f; // Level pitch
                        m_vThirdPersonAngles.y = playerAngles.y + 180.0f; // Face opposite direction
                        m_vThirdPersonAngles.z = 0.0f; // No roll
                    }
                    else
                    {
                        // Player angles are corrupted, use completely neutral angles
                        m_vThirdPersonAngles = QAngle(0.0f, 0.0f, 0.0f);
                    }
                    
                    // Normalize yaw
                    while (m_vThirdPersonAngles.y > 180.0f)
                        m_vThirdPersonAngles.y -= 360.0f;
                    while (m_vThirdPersonAngles.y < -180.0f)
                        m_vThirdPersonAngles.y += 360.0f;
                }
                lastTargetPlayer = pTarget;
                m_bForceAngleReset = true;
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
    
    auto pLocal = H::Entities.GetLocal();
    if (!pLocal)
        return false;
        
    // Hide when using SpectateAll custom enemy spectate mode
    SpectateMode mode = GetCurrentMode();
    if (mode == SpectateMode::ENEMY && !m_bThirdPersonMode && m_pCurrentSpectatedPlayer == pPlayer)
        return true;
    
    // Also hide when using regular TF2 first-person spectating
    if (!pLocal->IsAlive() && pLocal->m_iObserverMode() == OBS_MODE_FIRSTPERSON)
    {
        auto pTarget = pLocal->m_hObserverTarget().Get();
        if (pTarget && pTarget->As<CTFPlayer>() == pPlayer)
            return true;
    }
        
    return false;
}

bool CSpectateAll::ShouldHideEntity(CBaseEntity* pEntity)
{
    if (!Vars::Competitive::Features::SpectateAll.Value)
        return false;
    
    auto pLocal = H::Entities.GetLocal();
    if (!pLocal)
        return false;
    
    CTFPlayer* pTargetPlayer = nullptr;
    
    // Get target player from either SpectateAll or regular TF2 spectating
    SpectateMode mode = GetCurrentMode();
    if (mode == SpectateMode::ENEMY && m_pCurrentSpectatedPlayer)
    {
        pTargetPlayer = m_pCurrentSpectatedPlayer;
    }
    else if (!pLocal->IsAlive() && pLocal->m_iObserverMode() == OBS_MODE_FIRSTPERSON)
    {
        auto pTarget = pLocal->m_hObserverTarget().Get();
        if (pTarget)
            pTargetPlayer = pTarget->As<CTFPlayer>();
    }
    
    if (!pTargetPlayer)
        return false;
        
    // Hide weapons and cosmetics belonging to the spectated player
    auto classID = pEntity->GetClassID();
    
    // Check for weapons - hide if weapon hiding is enabled AND in first person mode
    bool isFirstPerson = (mode == SpectateMode::ENEMY && !m_bThirdPersonMode) || 
                        (pLocal->m_iObserverMode() == OBS_MODE_FIRSTPERSON);
    
    if (Vars::Competitive::SpectateAll::HideSpectatedWeapons.Value && isFirstPerson)
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
                
                if (pOwner == pTargetPlayer || pParent == pTargetPlayer)
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
                    
                    if (pOwner == pTargetPlayer)
                        return true;
                        
                    // For weapons without clear ownership, check proximity
                    if (!pOwner && pTargetPlayer)
                    {
                        Vector weaponPos = pEntity->m_vecOrigin();
                        Vector playerPos = pTargetPlayer->m_vecOrigin();
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
            if (pOwner == pTargetPlayer)
                return true;
        }
    }
    
    return false;
}

int CSpectateAll::GetSpectatedPlayerHealth()
{
    if (m_pCurrentSpectatedPlayer && m_pCurrentSpectatedPlayer->IsAlive())
    {
        return m_pCurrentSpectatedPlayer->m_iHealth();
    }
    return 100; // Default fallback health
}

void CSpectateAll::OverrideView(CViewSetup* pView)
{
    // Always handle respawn mouse lock regardless of spectate state
    HandleRespawnMouseLock();
    
    // Always handle mouse input to allow freecam activation when alive/taunting
    HandleMouseInput();
    
    // Check if we're in freecam mode while alive
    auto pLocal = H::Entities.GetLocal();
    bool isAlive = pLocal && pLocal->IsAlive();
    bool inFreecamWhileAlive = isAlive && m_bInFreeCam;
    
    if ((!ShouldSpectate() && !inFreecamWhileAlive) || !pView)
    {
        // Clear spectated player when not spectating and not in freecam while alive
        m_pCurrentSpectatedPlayer = nullptr;
        // Reset spectate state to prevent viewangle conflicts
        m_bIsSpectating = false;
        if (!inFreecamWhileAlive)
            m_bInFreeCam = false;
        m_bThirdPersonMode = true;
        
        // If we're in freecam while alive, handle the freecam view
        if (inFreecamWhileAlive)
        {
            HandleFreeCamera(pView);
        }
        return;
    }
    
    if (!pLocal)
        return;
    
    // Handle mouse input for freecam and player cycling (already called above)
    // HandleMouseInput();
    
    // Check if current spectated player died and auto-switch if enabled
    CheckForPlayerDeath();
    
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
                // Don't inherit from pView->angles as they might be corrupted
                // Instead, initialize with a clean neutral view behind the player
                QAngle playerAngles = m_pCurrentSpectatedPlayer->GetEyeAngles();
                m_vThirdPersonAngles.x = 0.0f; // Level pitch
                m_vThirdPersonAngles.y = playerAngles.y + 180.0f; // Face opposite direction
                m_vThirdPersonAngles.z = 0.0f; // No roll
                
                // Normalize yaw
                while (m_vThirdPersonAngles.y > 180.0f)
                    m_vThirdPersonAngles.y -= 360.0f;
                while (m_vThirdPersonAngles.y < -180.0f)
                    m_vThirdPersonAngles.y += 360.0f;
                
                // Force reset of mouse tracking static variables to prevent interference
                m_bForceAngleReset = true;
            }
            // When switching to first person, disable regular spectate viewangle override
            else if (!m_bThirdPersonMode)
            {
                // Reset the regular spectate system to prevent viewangle conflicts
                F::Spectate.m_iTarget = F::Spectate.m_iIntendedTarget = -1;
            }
        }
        spacePressed = spaceDown;
        
        // Handle mouse input for third person mode
        if (m_bThirdPersonMode)
        {
            // Get current user command to track mouse input
            auto pCmd = !I::EngineClient->IsPlayingDemo() ? G::LastUserCmd : I::Input->GetUserCmd(I::ClientState->lastoutgoingcommand);
            if (!pCmd)
                return;
                
            // Use command viewangles for mouse input instead of engine angles
            // This prevents interference from spectated player's movements
            static QAngle lastCmdAngles = pCmd->viewangles;
            static bool initialized = false;
            
            // Handle forced reset or initialization
            if (!initialized || m_bForceAngleReset)
            {
                lastCmdAngles = pCmd->viewangles;
                initialized = true;
                m_bForceAngleReset = false;
            }
            
            QAngle deltaAngles = pCmd->viewangles - lastCmdAngles;
            
            // Only update if there's actually mouse movement and we're not switching players
            static CTFPlayer* lastMousePlayer = m_pCurrentSpectatedPlayer;
            bool playerSwitched = (lastMousePlayer != m_pCurrentSpectatedPlayer);
            
            if (!playerSwitched && deltaAngles.Length() > 0.01f)
            {
                m_vThirdPersonAngles += deltaAngles;
                
                // Clamp pitch
                if (m_vThirdPersonAngles.x > 89.0f)
                    m_vThirdPersonAngles.x = 89.0f;
                if (m_vThirdPersonAngles.x < -89.0f)
                    m_vThirdPersonAngles.x = -89.0f;
                    
                // Normalize yaw
                while (m_vThirdPersonAngles.y > 180.0f)
                    m_vThirdPersonAngles.y -= 360.0f;
                while (m_vThirdPersonAngles.y < -180.0f)
                    m_vThirdPersonAngles.y += 360.0f;
            }
            
            lastCmdAngles = pCmd->viewangles;
            lastMousePlayer = m_pCurrentSpectatedPlayer;
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
        // Reset regular spectate system to prevent conflicts
        F::Spectate.m_iTarget = F::Spectate.m_iIntendedTarget = -1;
    }
    
    // Detect mode transitions and handle viewangle preservation
    bool modeChanged = false;
    if (m_bLastInFreeCam != m_bInFreeCam || m_bLastThirdPersonMode != m_bThirdPersonMode)
    {
        modeChanged = true;
        
        // When transitioning out of free camera, preserve current viewangles
        if (m_bLastInFreeCam && !m_bInFreeCam)
        {
            // Reset regular spectate system to prevent old viewangle restoration
            F::Spectate.m_iTarget = F::Spectate.m_iIntendedTarget = -1;
        }
        
        // When transitioning from first person to third person, use clean neutral angles
        if (!m_bLastThirdPersonMode && m_bThirdPersonMode && m_pCurrentSpectatedPlayer)
        {
            // Don't inherit potentially corrupted pView->angles
            // Use neutral angles behind the player instead
            QAngle playerAngles = m_pCurrentSpectatedPlayer->GetEyeAngles();
            m_vThirdPersonAngles.x = 0.0f; // Level pitch
            m_vThirdPersonAngles.y = playerAngles.y + 180.0f; // Face opposite direction
            m_vThirdPersonAngles.z = 0.0f; // No roll
            
            // Normalize yaw
            while (m_vThirdPersonAngles.y > 180.0f)
                m_vThirdPersonAngles.y -= 360.0f;
            while (m_vThirdPersonAngles.y < -180.0f)
                m_vThirdPersonAngles.y += 360.0f;
                
            m_bForceAngleReset = true;
        }
        
        // When transitioning to first person, reset regular spectate system
        if (m_bLastThirdPersonMode && !m_bThirdPersonMode)
        {
            F::Spectate.m_iTarget = F::Spectate.m_iIntendedTarget = -1;
        }
        
        // When entering freecam, reset regular spectate system
        if (!m_bLastInFreeCam && m_bInFreeCam)
        {
            F::Spectate.m_iTarget = F::Spectate.m_iIntendedTarget = -1;
        }
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
    
    // Store current state for next frame
    m_bLastInFreeCam = m_bInFreeCam;
    m_bLastThirdPersonMode = m_bThirdPersonMode;
}

bool CSpectateAll::IsValidAngle(const QAngle& angle)
{
    // Check for NaN values
    if (isnan(angle.x) || isnan(angle.y) || isnan(angle.z))
        return false;
        
    // Check for extreme values that could indicate corruption
    if (abs(angle.x) > 360.0f || abs(angle.y) > 360.0f || abs(angle.z) > 360.0f)
        return false;
        
    return true;
}

void CSpectateAll::CheckForPlayerDeath()
{
    if (!Vars::Competitive::SpectateAll::AutoSwitchOnDeath.Value)
        return;
    
    // Only check when we're actually spectating someone in enemy mode
    if (GetCurrentMode() != SpectateMode::ENEMY || !m_pCurrentSpectatedPlayer)
        return;
    
    // Check if the current spectated player is dead or invalid
    if (!m_pCurrentSpectatedPlayer->IsAlive() || m_pCurrentSpectatedPlayer->IsDormant())
    {
        // Player died, switch to next available player
        SwitchToNextAvailablePlayer();
    }
}

void CSpectateAll::SwitchToNextAvailablePlayer()
{
    auto pLocal = H::Entities.GetLocal();
    if (!pLocal)
        return;
    
    // Simple auto-switch logic: just go to next available enemy player
    auto enemies = H::Entities.GetGroup(EGroupType::PLAYERS_ENEMIES);
    if (enemies.empty())
    {
        m_pCurrentSpectatedPlayer = nullptr;
        return;
    }
    
    // Find next alive enemy player
    int startIndex = m_iCurrentEnemyIndex;
    int attempts = 0;
    
    do 
    {
        m_iCurrentEnemyIndex = (m_iCurrentEnemyIndex + 1) % enemies.size();
        attempts++;
        
        if (m_iCurrentEnemyIndex < enemies.size())
        {
            auto pEnemy = enemies[m_iCurrentEnemyIndex]->As<CTFPlayer>();
            if (pEnemy && pEnemy->IsAlive() && !pEnemy->IsDormant())
            {
                m_pCurrentSpectatedPlayer = pEnemy;
                return;
            }
        }
    } 
    while (m_iCurrentEnemyIndex != startIndex && attempts < enemies.size());
    
    // No alive players found
    m_pCurrentSpectatedPlayer = nullptr;
}

CTFPlayer* CSpectateAll::FindPlayerKiller(CTFPlayer* pDeadPlayer)
{
    if (!pDeadPlayer)
        return nullptr;
    
    // Return the stored killer from death events (much more reliable now)
    if (m_pLastKiller && m_pLastKiller->IsAlive())
    {
        return m_pLastKiller;
    }
    
    return nullptr;
}

void CSpectateAll::HandleRespawnMouseLock()
{
    auto pLocal = H::Entities.GetLocal();
    if (!pLocal)
        return;
    
    // Check if we just respawned (transition from dead to alive)
    bool isAlive = pLocal->IsAlive();
    if (!m_bWasAlive && isAlive)
    {
        // Just respawned
        m_bJustRespawned = true;
        
        if (Vars::Competitive::SpectateAll::MouseLockAfterRespawn.Value)
        {
            // Set mouse lock end time
            m_flMouseLockEndTime = I::GlobalVars->realtime + (Vars::Competitive::SpectateAll::MouseLockDuration.Value / 1000.0f);
        }
        
        // Reset killer tracking
        m_pLastKiller = nullptr;
    }
    
    // Update the alive state for next frame
    m_bWasAlive = isAlive;
}

void CSpectateAll::OnPlayerDeath(IGameEvent* pEvent)
{
    if (!pEvent)
        return;
    
    auto pLocal = H::Entities.GetLocal();
    if (!pLocal)
        return;
    
    // Check if we are the one who died
    int deadPlayerIndex = I::EngineClient->GetPlayerForUserID(pEvent->GetInt("userid"));
    if (deadPlayerIndex != pLocal->entindex())
        return;
    
    // We died! Store our killer for spectating preference
    if (Vars::Competitive::SpectateAll::PreferKillerSpectate.Value)
    {
        int attackerIndex = I::EngineClient->GetPlayerForUserID(pEvent->GetInt("attacker"));
        
        if (attackerIndex > 0 && attackerIndex != deadPlayerIndex)
        {
            // Get the attacker entity
            auto pAttacker = I::ClientEntityList->GetClientEntity(attackerIndex);
            if (pAttacker)
            {
                auto pKiller = pAttacker->As<CTFPlayer>();
                if (pKiller && pKiller->IsAlive() && pKiller->m_iTeamNum() != pLocal->m_iTeamNum())
                {
                    // Store our killer for when spectating starts
                    m_pLastKiller = pKiller;
                }
            }
        }
    }
}

bool CSpectateAll::ShouldLockMouse()
{
    if (!Vars::Competitive::SpectateAll::MouseLockAfterRespawn.Value)
        return false;
    
    auto pLocal = H::Entities.GetLocal();
    if (!pLocal || !pLocal->IsAlive())
        return false;
    
    // Check if we're in the mouse lock period
    if (m_flMouseLockEndTime > 0.0f && I::GlobalVars->realtime < m_flMouseLockEndTime)
    {
        return true;
    }
    else if (m_flMouseLockEndTime > 0.0f)
    {
        // Mouse lock period ended, clear it
        m_flMouseLockEndTime = 0.0f;
        m_bJustRespawned = false;
    }
    
    return false;
}