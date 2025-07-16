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
    int m_iCurrentEnemyIndex = 0;
    CTFPlayer* m_pCurrentSpectatedPlayer = nullptr;
    
    // Helper functions
    void HandleMouseInput();
    CTFPlayer* GetNextEnemyPlayer();
    void HandleFreeCamera(CViewSetup* pView);
    void HandleEnemySpectate(CViewSetup* pView);
    void ApplyThirdPersonView(CViewSetup* pView);
    SpectateMode GetCurrentMode();
    
public:
    void OverrideView(CViewSetup* pView);
    bool ShouldSpectate();
    bool ShouldHidePlayer(CTFPlayer* pPlayer);
    bool ShouldHideEntity(CBaseEntity* pEntity);
};

ADD_FEATURE(CSpectateAll, SpectateAll)