#include "SafeBhop.h"
#include <random>

void CSafeBhop::ResetState()
{
    m_iPerfectJumpCount = 0;
    m_bIsBlocked = false;
    m_iBlockJumpUntil = 0;
    m_bShouldBlockOnLand = false;
}

bool CSafeBhop::IsPlayerValid(CTFPlayer* pPlayer)
{
    return pPlayer && pPlayer->IsAlive();
}

int CSafeBhop::GetRandomInt(int min, int max)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(min, max);
    return dis(gen);
}

void CSafeBhop::Run(CTFPlayer* pLocal, CUserCmd* pCmd)
{
    if (!Vars::Competitive::Features::SafeBhop.Value)
        return;
        
    if (!IsPlayerValid(pLocal))
        return;
        
    // Early exit if space is not being pressed
    if (!(pCmd->buttons & IN_JUMP))
    {
        ResetState();
        return;
    }
    
    int currentTick = I::GlobalVars->tickcount;
    bool isGrounded = (pLocal->m_fFlags() & FL_ONGROUND) != 0;
    
    // Handle blocking on land
    if (m_bShouldBlockOnLand && isGrounded)
    {
        m_bIsBlocked = true;
        m_iBlockJumpUntil = currentTick + BLOCK_DURATION;
        m_bShouldBlockOnLand = false;
        m_iPerfectJumpCount = 0;
        pCmd->buttons &= ~IN_JUMP;
        return;
    }
    
    // Handle active blocking
    if (m_bIsBlocked)
    {
        pCmd->buttons &= ~IN_JUMP;
        if (currentTick >= m_iBlockJumpUntil)
        {
            m_bIsBlocked = false;
            m_iBlockJumpUntil = 0;
        }
        return;
    }
    
    // Don't jump in air
    if (!isGrounded)
    {
        pCmd->buttons &= ~IN_JUMP;
        m_bWasOnGround = false;
        return;
    }
    
    // Don't jump if still in block period
    if (currentTick < m_iBlockJumpUntil)
    {
        m_bWasOnGround = true;
        return;
    }
    
    // Only jump on first ground frame (like Lua module)
    if (!m_bWasOnGround)
    {
        // Apply success rate randomization
        if (GetRandomInt(1, 100) <= Vars::Competitive::SafeBhop::SuccessRate.Value)
        {
            // Safety check: prevent too many perfect jumps
            if (Vars::Competitive::SafeBhop::SafetyEnabled.Value && 
                m_iPerfectJumpCount >= MAX_PERFECT_JUMPS - 1)
            {
                m_bShouldBlockOnLand = true;
                pCmd->buttons &= ~IN_JUMP;
                return;
            }
            
            // Allow the jump
            pCmd->buttons |= IN_JUMP;
            m_iPerfectJumpCount++;
        }
        else
        {
            // Failed success rate - block this jump and reset counter
            m_bShouldBlockOnLand = true;
            pCmd->buttons &= ~IN_JUMP;
            m_iPerfectJumpCount = 0;
        }
    }
    
    m_bWasOnGround = true;
}

void CSafeBhop::Reset()
{
    ResetState();
    m_bWasOnGround = false;
}