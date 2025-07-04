#pragma once
#include "../../../SDK/SDK.h"

class CSafeBhop
{
private:
    // Safety state tracking
    bool m_bWasOnGround = false;
    bool m_bIsBlocked = false;
    bool m_bShouldBlockOnLand = false;
    int m_iPerfectJumpCount = 0;
    int m_iBlockJumpUntil = 0;
    
    // Constants from Lua module
    static constexpr int BLOCK_DURATION = 60; // ~1 second at 60 fps
    static constexpr int MAX_PERFECT_JUMPS = 10;
    
    // Helper functions
    void ResetState();
    bool IsPlayerValid(CTFPlayer* pPlayer);
    int GetRandomInt(int min, int max);

public:
    void Run(CTFPlayer* pLocal, CUserCmd* pCmd);
    void Reset();
};

ADD_FEATURE(CSafeBhop, SafeBhop)