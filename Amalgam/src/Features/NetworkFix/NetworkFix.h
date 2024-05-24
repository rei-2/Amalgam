#pragma once
#include "../../SDK/SDK.h"

class CReadPacketState
{
private:
    float m_flFrameTimeClientState = 0.f;
    float m_flFrameTime = 0.f;
    float m_flCurTime = 0.f;
    int m_nTickCount = 0;

public:
    void Store();
    void Restore();
};

class CNetworkFix
{
private:
    CReadPacketState m_State = {};

public:
    void FixInputDelay(bool bFinalTick);
    bool ShouldReadPackets();
};

ADD_FEATURE(CNetworkFix, NetworkFix);