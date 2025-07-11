#pragma once
#include "../../../SDK/SDK.h"

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
    bool m_bIsSpectating = false;
    bool m_bWasAlive = true;
    bool m_bThirdPersonMode = true;  // Default to third person when spectating enemies
    int m_iCurrentEnemyIndex = 0;
    CTFPlayer* m_pCurrentSpectatedPlayer = nullptr;
    
    // Helper functions
    CTFPlayer* GetNextEnemyPlayer();
    void HandleFreeCamera(CViewSetup* pView);
    void HandleEnemySpectate(CViewSetup* pView);
    void ApplyThirdPersonView(CViewSetup* pView);
    SpectateMode GetCurrentMode();
    
public:
    void OverrideView(CViewSetup* pView);
    bool ShouldSpectate();
    bool ShouldHidePlayer(CTFPlayer* pPlayer);
};

ADD_FEATURE(CSpectateAll, SpectateAll)