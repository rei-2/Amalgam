#pragma once
#include "../../SDK/SDK.h"

//#define TICKBASE_DEBUG

class CTicks
{
	void CLMoveFunc(float accumulated_extra_samples, bool bFinalTick);

	void Recharge(CTFPlayer* pLocal);
	void Warp();
	void Doubletap(CTFPlayer* pLocal, CUserCmd* pCmd);
	void Speedhack();
	bool ValidWeapon(CTFWeaponBase* pWeapon);

	void ManagePacket(CUserCmd* pCmd, bool* pSendPacket);

	bool m_bGoalReached = true;
	Vec3 m_vShootPos = {};

	bool m_bShootAngle = false;
	Vec3 m_vShootAngle = {};

public:
	void Run(float accumulated_extra_samples, bool bFinalTick, CTFPlayer* pLocal);
	void Draw(CTFPlayer* pLocal);
	void Reset();

	void CLMove(float accumulated_extra_samples, bool bFinalTick);
	void CLMoveManage(CTFPlayer* pLocal);

	void CreateMove(CTFPlayer* pLocal, CUserCmd* pCmd, bool* pSendPacket);
	void AntiWarp(CTFPlayer* pLocal, CUserCmd* pCmd);

	int GetTicks(CTFWeaponBase* pWeapon = nullptr);
	int GetShotsWithinPacket(CTFWeaponBase* pWeapon, int iTicks = Vars::Doubletap::TickLimit.Value);
	int GetMinimumTicksNeeded(CTFWeaponBase* pWeapon);
	bool CanChoke();

	void SaveShootPos(CTFPlayer* pLocal);
	Vec3 GetShootPos();
	void SaveShootAngle(CUserCmd* pCmd, bool bSendPacket);
	Vec3* GetShootAngle();

	bool m_bDoubletap = false;
	bool m_bWarp = false;
	bool m_bRecharge = false;
	bool m_bAntiWarp = false;
	bool m_bSpeedhack = false;

	int m_iShiftedTicks = 0;
	int m_iShiftedGoal = 0;
	int m_iShiftStart = 0;
	bool m_bShifting = false;
	bool m_bShifted = false;

	int m_iWait = 0;
	int m_iMaxShift = 24;
	int m_iDeficit = 0;
};

ADD_FEATURE(CTicks, Ticks);