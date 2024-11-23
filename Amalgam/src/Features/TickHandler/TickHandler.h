#pragma once
#include "../../SDK/SDK.h"

class CTickshiftHandler
{
	void Recharge(CTFPlayer* pLocal);
	void Warp();
	void Doubletap(CTFPlayer* pLocal, CUserCmd* pCmd);
	void Speedhack();
	void AntiWarp(CTFPlayer* pLocal, CUserCmd* pCmd);
	bool ValidWeapon(CTFWeaponBase* pWeapon);

	void CLMoveFunc(float accumulated_extra_samples, bool bFinalTick);

	bool m_bSpeedhack = false;
	bool m_bGoalReached = true;
	Vec3 m_vShootPos = {};

public:
	void Run(float accumulated_extra_samples, bool bFinalTick, CTFPlayer* pLocal);
	void Reset();

	void MovePre(CTFPlayer* pLocal);
	void MoveMain(float accumulated_extra_samples, bool bFinalTick);
	void MovePost(CTFPlayer* pLocal, CUserCmd* pCmd);

	void ManagePacket(CUserCmd* pCmd, bool* pSendPacket);
	int GetTicks(CTFWeaponBase* pWeapon = nullptr);
	int GetShotsWithinPacket(CTFWeaponBase* pWeapon, int iTicks = Vars::CL_Move::Doubletap::TickLimit.Value);
	int GetMinimumTicksNeeded(CTFWeaponBase* pWeapon);

	void SaveShootPos(CTFPlayer* pLocal);
	Vec3 GetShootPos();

	int m_iDeficit = 0;
};

ADD_FEATURE(CTickshiftHandler, Ticks)