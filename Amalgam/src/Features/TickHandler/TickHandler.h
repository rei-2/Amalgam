#pragma once
#include "../../SDK/SDK.h"

class CTickshiftHandler
{
	// logic
	void Recharge(CUserCmd* pCmd, CTFPlayer* pLocal);
	void Teleport(CUserCmd* pCmd);
	void Doubletap(const CUserCmd* pCmd, CTFPlayer* pLocal);
	void Speedhack(CUserCmd* pCmd);
	bool ValidWeapon(CTFWeaponBase* pWeapon);

	// utils
	void CLMoveFunc(float accumulated_extra_samples, bool bFinalTick);

	void MovePre(CTFPlayer* pLocal);
	void MoveMain(float accumulated_extra_samples, bool bFinalTick);

	bool bSpeedhack = false;
	bool bGoalReached = true;

public:
	int GetTicks(CTFPlayer* pLocal);
	void Run(float accumulated_extra_samples, bool bFinalTick, CTFPlayer* pLocal);
	void MovePost(CTFPlayer* pLocal, CUserCmd* pCmd);
	void Reset();

	int iDeficit = 0;
};

ADD_FEATURE(CTickshiftHandler, Ticks)