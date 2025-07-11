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
    
    // Store origin when transitioning from alive to dead
    if (m_bWasAlive && !isAlive)
    {
        m_vStoredOrigin = pLocal->m_vecOrigin();
        m_bIsSpectating = false;
        m_iCurrentEnemyIndex = 0;
    }
    // Reset when respawning
    else if (!m_bWasAlive && isAlive)
    {
        m_bIsSpectating = false;
    }
    
    m_bWasAlive = isAlive;
    return shouldActivate;
}

SpectateMode CSpectateAll::GetCurrentMode()
{
    if (Vars::Competitive::SpectateAll::FreeCamera.Value)
        return SpectateMode::FREE_CAMERA;
    else if (Vars::Competitive::SpectateAll::EnemySpectate.Value)
        return SpectateMode::ENEMY;
    else
        return SpectateMode::NORMAL;
}

CTFPlayer* CSpectateAll::GetNextEnemyPlayer()
{
    auto pLocal = H::Entities.GetLocal();
    if (!pLocal)
        return nullptr;
        
    auto enemies = H::Entities.GetGroup(EGroupType::PLAYERS_ENEMIES);
    if (enemies.empty())
        return nullptr;
        
    // Manual switching with Mouse1 (left click) - only in enemy spectate mode
    SpectateMode currentMode = GetCurrentMode();
    if (currentMode == SpectateMode::ENEMY)
    {
        static bool mousePressed = false;
        bool mouseDown = GetAsyncKeyState(VK_LBUTTON) & 0x8000;
        
        if (mouseDown && !mousePressed)
        {
            m_iCurrentEnemyIndex = (m_iCurrentEnemyIndex + 1) % enemies.size();
        }
        mousePressed = mouseDown;
    }
    
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
        pView->angles = pTarget->GetEyeAngles();
        
        // Apply third person view if enabled
        if (m_bThirdPersonMode)
        {
            ApplyThirdPersonView(pView);
        }
        // If first person mode, keep the player's eye position and angles
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
    pView->origin -= forward * 150.0f;
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
    
    // Handle different spectate modes
    SpectateMode mode = GetCurrentMode();
    
    // Handle Space key toggle for first/third person (only in Enemy Spectate mode)
    if (mode == SpectateMode::ENEMY)
    {
        static bool spacePressed = false;
        bool spaceDown = GetAsyncKeyState(VK_SPACE) & 0x8000;
        
        if (spaceDown && !spacePressed)
        {
            m_bThirdPersonMode = !m_bThirdPersonMode;
        }
        spacePressed = spaceDown;
    }
    else
    {
        // Clear spectated player when not in enemy spectate mode
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