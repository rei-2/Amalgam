#pragma once
#include "../../SDK/SDK.h"

class CTickshiftHandler
{
	void Recharge(CTFPlayer* pLocal);
	void Teleport();
	void Doubletap(CTFPlayer* pLocal, CUserCmd* pCmd);
	void Speedhack();
	bool ValidWeapon(CTFWeaponBase* pWeapon);
	void AntiWarp(CTFPlayer* pLocal, CUserCmd* pCmd);

	void CLMoveFunc(float accumulated_extra_samples, bool bFinalTick);

	bool bSpeedhack = false;
	bool bGoalReached = true;

public:
	void Run(float accumulated_extra_samples, bool bFinalTick, CTFPlayer* pLocal);
	void Reset();

	void MovePre(CTFPlayer* pLocal);
	void MoveMain(float accumulated_extra_samples, bool bFinalTick);
	void MovePost(CTFPlayer* pLocal, CUserCmd* pCmd);

	int GetTicks();
	void ManagePacket(CUserCmd* pCmd, bool* pSendPacket);

	int iDeficit = 0;
};

ADD_FEATURE(CTickshiftHandler, Ticks)