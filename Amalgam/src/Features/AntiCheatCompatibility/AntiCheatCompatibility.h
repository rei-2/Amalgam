#pragma once
#include "../../SDK/SDK.h"

#define MATH_EPSILON (1.f / 16)
#define PSILENT_EPSILON (1.f - MATH_EPSILON)
#define REAL_EPSILON (0.1f + MATH_EPSILON)
#define SNAP_SIZE_EPSILON (10.f - MATH_EPSILON)
#define SNAP_NOISE_EPSILON (0.5f + MATH_EPSILON)

struct CmdHistory_t
{
	Vec3 m_vAngle;
	bool m_bAttack1;
	bool m_bAttack2;
	bool m_bSendingPacket;
};

class CAntiCheatCompatibility
{
private:
	std::deque<CmdHistory_t> m_vHistory;
	std::string m_sValue = "";
	int m_iJumps = 0;

public:
    void CreateMove(CUserCmd* pCmd, bool* pSendPacket);
    void BunnyHop(CUserCmd* pCmd, bool bCurrValid, bool bLastValid);
    void RespondCvarValue(INetMessage& msg);

    inline bool Active() { return Vars::Misc::Game::AntiCheatCompatibility.Value; }
};

ADD_FEATURE(CAntiCheatCompatibility, AntiCheatCompatibility);