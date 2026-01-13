#pragma once
#include "../../SDK/SDK.h"

class CTicks
{
private:
	void MoveFunc(float accumulated_extra_samples, bool bFinalTick);
	void MoveManage();

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

	bool m_bPredictAntiwarp = false;
	bool m_bTimingUnsure = false; // we aren't sure when we'll actually fire, hold aim

public:
	void Move(float accumulated_extra_samples, bool bFinalTick);
	void CreateMove(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd, bool* pSendPacket);
	void Draw(CTFPlayer* pLocal);
	void Reset();

	void Start(CTFPlayer* pLocal, CUserCmd* pCmd);
	void End(CTFPlayer* pLocal, CUserCmd* pCmd);

	void AntiWarp(CTFPlayer* pLocal, float flYaw, float& flForwardMove, float& flSideMove, int iTicks);
	void AntiWarp(CTFPlayer* pLocal, float flYaw, float& flForwardMove, float& flSideMove);
	void AntiWarp(CTFPlayer* pLocal, CUserCmd* pCmd);

	bool CanChoke(bool bCanShift, int iMaxTicks);
	bool CanChoke(bool bCanShift = false);
	int GetTicks(CTFWeaponBase* pWeapon = nullptr);
	int GetShotsWithinPacket(CTFWeaponBase* pWeapon, int iTicks = Vars::Doubletap::TickLimit.Value);
	int GetMinimumTicksNeeded(CTFWeaponBase* pWeapon);

	void SaveShootPos(CTFPlayer* pLocal);
	Vec3 GetShootPos();
	void SaveShootAngle(CUserCmd* pCmd, bool bSendPacket);
	Vec3* GetShootAngle();
	bool IsTimingUnsure();

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
	int m_iMaxUsrCmdProcessTicks = 24;
	int m_iMaxShift = 24;
	int m_iDeficit = 0;
};

ADD_FEATURE(CTicks, Ticks);