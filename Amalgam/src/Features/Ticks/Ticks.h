#pragma once
#include "../../SDK/SDK.h"

class CTicks
{
private:
	void MoveFunc(float accumulated_extra_samples, bool bFinalTick);
	void MoveManage(CTFPlayer* pLocal);

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
	void Move(float accumulated_extra_samples, bool bFinalTick, CTFPlayer* pLocal);
	void CreateMove(CTFPlayer* pLocal, CUserCmd* pCmd, bool* pSendPacket);
	void Draw(CTFPlayer* pLocal);
	void Reset();

	void AntiWarp(CTFPlayer* pLocal, float flYaw, float& flForwardMove, float& flSideMove, int iTicks = -1);
	void AntiWarp(CTFPlayer* pLocal, CUserCmd* pCmd);

	bool CanChoke();
	int GetTicks(CTFWeaponBase* pWeapon = nullptr);
	int GetShotsWithinPacket(CTFWeaponBase* pWeapon, int iTicks = Vars::Doubletap::TickLimit.Value);
	int GetMinimumTicksNeeded(CTFWeaponBase* pWeapon);

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