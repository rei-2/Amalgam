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
	void AutoPeek(CTFPlayer* pLocal, CUserCmd* pCmd);
	void MovementLock(CTFPlayer* pLocal, CUserCmd* pCmd);
	void AntiAFK(CTFPlayer* pLocal, CUserCmd* pCmd);
	void InstantRespawnMVM(CTFPlayer* pLocal);

	void CheatsBypass();
	void PingReducer();
	void WeaponSway();

	void TauntKartControl(CTFPlayer* pLocal, CUserCmd* pCmd);
	void FastMovement(CTFPlayer* pLocal, CUserCmd* pCmd);

	Vec3 vPeekReturnPos = {};
	//bool bSteamCleared = false;

public:
	void RunPre(CTFPlayer* pLocal, CUserCmd* pCmd);
	void RunPost(CTFPlayer* pLocal, CUserCmd* pCmd, bool pSendPacket);

	void Event(IGameEvent* pEvent, uint32_t uNameHash);
	int AntiBackstab(CTFPlayer* pLocal, CUserCmd* pCmd, bool bSendPacket);

	void UnlockAchievements();
	void LockAchievements();
	bool SteamRPC();

	int iWishCmdrate = -1;
	int iWishUpdaterate = -1;

#ifdef DEBUG
	void DumpClassIDS();
#endif
};

ADD_FEATURE(CMisc, Misc)
