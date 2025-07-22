#pragma once
#include "../../../SDK/SDK.h"
#include "../../Spectate/Spectate.h"

enum class SpectateMode
{
    NORMAL,      // Default TF2 spectating
    ENEMY,       // Spectate enemy players (with automatic third person)
    FREE_CAMERA  // Free camera movement
};

class CSpectateAll
{
private:
    // Configuration constants
    static constexpr float SPEED_MODIFIER_SLOW = 0.45f;
    static constexpr float SPEED_MODIFIER_FAST = 1.65f;
    
    // State tracking variables
    Vector m_vStoredOrigin;
    QAngle m_vStoredAngles;
    QAngle m_vThirdPersonAngles;
    bool m_bIsSpectating = false;
    bool m_bWasAlive = true;
    bool m_bThirdPersonMode = true;  // Default to third person when spectating enemies
    bool m_bInFreeCam = false;       // Track if currently in freecam mode
    bool m_bLastInFreeCam = false;   // Track previous freecam state
    bool m_bLastThirdPersonMode = true; // Track previous third person state
    bool m_bForceAngleReset = false; // Force reset of mouse tracking variables
    int m_iLastObserverMode = OBS_MODE_NONE; // Track last observer mode before freezecam
    int m_iCurrentEnemyIndex = 0;
    CTFPlayer* m_pCurrentSpectatedPlayer = nullptr;
    
    // Mouse lock and killer tracking
    float m_flMouseLockEndTime = 0.0f;
    CTFPlayer* m_pLastKiller = nullptr;
    bool m_bJustRespawned = false;
    
    // Taunt freecam tracking
    bool m_bFreecamStartedDuringTaunt = false;
    
    // Helper functions
    void HandleMouseInput();
    CTFPlayer* GetNextEnemyPlayer();
    void HandleFreeCamera(CViewSetup* pView);
    void HandleEnemySpectate(CViewSetup* pView);
    void ApplyThirdPersonView(CViewSetup* pView);
    bool IsValidAngle(const QAngle& angle);
    
    // New functionality helpers
    void CheckForPlayerDeath();
    void SwitchToNextAvailablePlayer();
    void HandleRespawnMouseLock();
    CTFPlayer* FindPlayerKiller(CTFPlayer* pDeadPlayer);
    
public:
    void OverrideView(CViewSetup* pView);
    bool ShouldSpectate();
    bool ShouldHidePlayer(CTFPlayer* pPlayer);
    bool ShouldHideEntity(CBaseEntity* pEntity);
    
    // Health override system for spectator UI
    CTFPlayer* GetCurrentSpectatedPlayer() const { return m_pCurrentSpectatedPlayer; }
    int GetSpectatedPlayerHealth();
    
    // Freecam functionality
    SpectateMode GetCurrentMode();
    Vector GetFreecamPosition() const { return m_vStoredOrigin; }
    bool IsInFreecam() const { return m_bInFreeCam; }
    
    // Event handling
    void OnPlayerDeath(IGameEvent* pEvent);
    
    // Mouse lock functionality
    bool ShouldLockMouse();
};

ADD_FEATURE(CSpectateAll, SpectateAll)