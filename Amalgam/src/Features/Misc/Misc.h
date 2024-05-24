#pragma once
#include "../../SDK/SDK.h"

#ifdef DEBUG
#include <iostream>
#include <fstream>
#endif

class CMisc
{
	void AutoJump(CTFPlayer* pLocal, CUserCmd* pCmd);
	void AutoJumpbug(CTFPlayer* pLocal, CUserCmd* pCmd);
	void AutoStrafe(CTFPlayer* pLocal, CUserCmd* pCmd);
	void AntiBackstab(CTFPlayer* pLocal, CUserCmd* pCmd);
	void AutoPeek(CTFPlayer* pLocal, CUserCmd* pCmd);
	void AntiAFK(CTFPlayer* pLocal, CUserCmd* pCmd);
	void InstantRespawnMVM(CTFPlayer* pLocal);

	void CheatsBypass();
	void PingReducer();
	void WeaponSway();

	void TauntKartControl(CTFPlayer* pLocal, CUserCmd* pCmd);
	void FastMovement(CTFPlayer* pLocal, CUserCmd* pCmd);
	void AntiWarp(CTFPlayer* pLocal, CUserCmd* pCmd);
	void LegJitter(CTFPlayer* pLocal, CUserCmd* pCmd, bool pSendPacket);

	int iLastCmdrate = -1;
	Vec3 vPeekReturnPos = {};
	//bool bSteamCleared = false;

public:
	void RunPre(CTFPlayer* pLocal, CUserCmd* pCmd);
	void RunPost(CTFPlayer* pLocal, CUserCmd* pCmd, bool pSendPacket);
	void Event(IGameEvent* pEvent, FNV1A_t uNameHash);
	void DoubletapPacket(CUserCmd* pCmd, bool* pSendPacket);
	void DetectChoke();

	void UnlockAchievements();
	void LockAchievements();
	bool SteamRPC();

#ifdef DEBUG
	void DumpClassIDS();
#endif
};

ADD_FEATURE(CMisc, Misc)
