#include "TickHandler.h"

#include "../NetworkFix/NetworkFix.h"
#include "../PacketManip/AntiAim/AntiAim.h"
#include "../Aimbot/AutoRocketJump/AutoRocketJump.h"
#include "../Backtrack/Backtrack.h"

void CTickshiftHandler::Reset()
{
	m_bSpeedhack = m_bDoubletap = m_bRecharge = m_bWarp = false;
	m_iShiftedTicks = m_iShiftedGoal = 0;
}

void CTickshiftHandler::Recharge(CTFPlayer* pLocal)
{
	if (!m_bGoalReached)
		return;

	bool bPassive = m_bRecharge = false;

	static float flPassiveTime = 0.f;
	flPassiveTime = std::max(flPassiveTime - TICK_INTERVAL, -TICK_INTERVAL);
	if (Vars::CL_Move::Doubletap::PassiveRecharge.Value && 0.f >= flPassiveTime)
	{
		bPassive = true;
		flPassiveTime += 1.f / Vars::CL_Move::Doubletap::PassiveRecharge.Value;
	}

	if (m_iDeficit)
	{
		bPassive = true;
		m_iDeficit--, m_iShiftedTicks--;
	}

	if (!Vars::CL_Move::Doubletap::RechargeTicks.Value && !bPassive
		|| m_bDoubletap || m_bWarp || m_iShiftedTicks == m_iMaxShift || m_bSpeedhack)
		return;

	m_bRecharge = true;
	m_iShiftedGoal = m_iShiftedTicks + 1;
}

void CTickshiftHandler::Warp()
{
	if (!m_bGoalReached)
		return;

	m_bWarp = false;
	if (!Vars::CL_Move::Doubletap::Warp.Value
		|| !m_iShiftedTicks || m_bDoubletap || m_bRecharge || m_bSpeedhack)
		return;

	m_bWarp = true;
	m_iShiftedGoal = std::max(m_iShiftedTicks - Vars::CL_Move::Doubletap::WarpRate.Value + 1, 0);
}

void CTickshiftHandler::Doubletap(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	if (!m_bGoalReached)
		return;

	if (!Vars::CL_Move::Doubletap::Doubletap.Value
		|| m_iWait || m_bWarp || m_bRecharge || m_bSpeedhack)
		return;

	int iTicks = std::min(m_iShiftedTicks + 1, 22);
	auto pWeapon = H::Entities.GetWeapon();
	if (!(iTicks >= Vars::CL_Move::Doubletap::TickLimit.Value || pWeapon && GetShotsWithinPacket(pWeapon, iTicks) > 1))
		return;

	bool bAttacking = G::PrimaryWeaponType == EWeaponType::MELEE ? pCmd->buttons & IN_ATTACK : G::Attacking;
	if (!G::CanPrimaryAttack && !G::Reloading || !bAttacking && !m_bDoubletap || F::AutoRocketJump.IsRunning())
		return;

	m_bDoubletap = true;
	m_iShiftedGoal = std::max(m_iShiftedTicks - Vars::CL_Move::Doubletap::TickLimit.Value + 1, 0);
	if (Vars::CL_Move::Doubletap::AntiWarp.Value)
		m_bAntiWarp = pLocal->m_hGroundEntity();
}

void CTickshiftHandler::Speedhack()
{
	m_bSpeedhack = Vars::CL_Move::SpeedEnabled.Value;
	if (!m_bSpeedhack)
		return;

	m_bDoubletap = m_bWarp = m_bRecharge = false;
}

void CTickshiftHandler::SaveShootPos(CTFPlayer* pLocal)
{
	if (!m_bDoubletap && !m_bWarp)
		m_vShootPos = pLocal->GetShootPos();
}
Vec3 CTickshiftHandler::GetShootPos()
{
	return m_vShootPos;
}

bool CTickshiftHandler::CanChoke()
{
	static auto sv_maxusrcmdprocessticks = U::ConVars.FindVar("sv_maxusrcmdprocessticks");
	const int iMaxTicks = sv_maxusrcmdprocessticks ? sv_maxusrcmdprocessticks->GetInt() : 24;

	return I::ClientState->chokedcommands < 21 && F::Ticks.m_iShiftedTicks + I::ClientState->chokedcommands < iMaxTicks;
}

void CTickshiftHandler::AntiWarp(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	static Vec3 vVelocity = {};
	static int iMaxTicks = 0;
	if (m_bAntiWarp)
	{
		int iTicks = GetTicks();
		iMaxTicks = std::max(iTicks + 1, iMaxTicks);

		Vec3 vAngles; Math::VectorAngles(vVelocity, vAngles);
		vAngles.y = pCmd->viewangles.y - vAngles.y;
		Vec3 vForward; Math::AngleVectors(vAngles, &vForward);
		vForward *= vVelocity.Length2D();

		if (iTicks > std::max(iMaxTicks - 8, 3))
			pCmd->forwardmove = -vForward.x, pCmd->sidemove = -vForward.y;
		else if (iTicks > 3)
		{
			pCmd->forwardmove = pCmd->sidemove = 0.f;
			pCmd->buttons &= ~(IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT);
		}
		else
			pCmd->forwardmove = vForward.x, pCmd->sidemove = vForward.y;
	}
	else
	{
		vVelocity = pLocal->m_vecVelocity();
		iMaxTicks = 0;
	}

	/*
	static bool bSet = false;

	if (!m_bAntiWarp)
	{
		bSet = false;
		return;
	}

	if (G::Attacking != 1 && !bSet)
	{
		bSet = true;
		SDK::StopMovement(pLocal, pCmd);
	}
	else
		pCmd->forwardmove = pCmd->sidemove = 0.f;
	*/
}

bool CTickshiftHandler::ValidWeapon(CTFWeaponBase* pWeapon)
{
	switch (pWeapon->GetWeaponID())
	{
	case TF_WEAPON_PDA:
	case TF_WEAPON_PDA_ENGINEER_BUILD:
	case TF_WEAPON_PDA_ENGINEER_DESTROY:
	case TF_WEAPON_PDA_SPY:
	case TF_WEAPON_PDA_SPY_BUILD:
	case TF_WEAPON_BUILDER:
	case TF_WEAPON_INVIS:
	case TF_WEAPON_GRAPPLINGHOOK:
	case TF_WEAPON_JAR_MILK:
	case TF_WEAPON_LUNCHBOX:
	case TF_WEAPON_BUFF_ITEM:
	case TF_WEAPON_ROCKETPACK:
	case TF_WEAPON_JAR_GAS:
	case TF_WEAPON_LASER_POINTER:
	case TF_WEAPON_MEDIGUN:
	case TF_WEAPON_SNIPERRIFLE:
	case TF_WEAPON_SNIPERRIFLE_DECAP:
	case TF_WEAPON_SNIPERRIFLE_CLASSIC:
	case TF_WEAPON_COMPOUND_BOW:
	case TF_WEAPON_JAR:
		return false;
	}

	return true;
}

void CTickshiftHandler::CLMoveFunc(float accumulated_extra_samples, bool bFinalTick)
{
	static auto CL_Move = U::Hooks.m_mHooks["CL_Move"];

	m_iShiftedTicks--;
	if (m_iShiftedTicks < 0)
		return;
	if (m_iWait > 0)
		m_iWait--;

	int iTicks = std::min(m_iShiftedTicks + 1, 22);
	auto pWeapon = H::Entities.GetWeapon();
	if (!(iTicks >= Vars::CL_Move::Doubletap::TickLimit.Value || pWeapon && GetShotsWithinPacket(pWeapon, iTicks) > 1))
		m_iWait = 1;

	m_bGoalReached = bFinalTick && m_iShiftedTicks == m_iShiftedGoal;

	if (CL_Move)
		CL_Move->Call<void>(accumulated_extra_samples, bFinalTick);
}

void CTickshiftHandler::CLMove(float accumulated_extra_samples, bool bFinalTick)
{
	if (auto pWeapon = H::Entities.GetWeapon())
	{
		switch (pWeapon->GetWeaponID())
		{
		case TF_WEAPON_PIPEBOMBLAUNCHER:
		case TF_WEAPON_CANNON:
			if (!G::CanSecondaryAttack)
				m_iWait = Vars::CL_Move::Doubletap::TickLimit.Value;
			break;
		default:
			if (!ValidWeapon(pWeapon))
				m_iWait = 2;
			else if (G::Attacking || !G::CanPrimaryAttack && !G::Reloading)
				m_iWait = Vars::CL_Move::Doubletap::TickLimit.Value;
		}
	}
	else
		m_iWait = 2;

	static auto sv_maxusrcmdprocessticks = U::ConVars.FindVar("sv_maxusrcmdprocessticks");
	m_iMaxShift = sv_maxusrcmdprocessticks ? sv_maxusrcmdprocessticks->GetInt() : 24;
	m_iMaxShift -= std::max(24 - std::clamp(Vars::CL_Move::Doubletap::RechargeLimit.Value, 1, 24), F::AntiAim.YawOn() ? F::AntiAim.AntiAimTicks() : 0);

	while (m_iShiftedTicks > m_iMaxShift)
		CLMoveFunc(accumulated_extra_samples, false); // skim any excess ticks

	m_iShiftedTicks++; // since we now have full control over CL_Move, increment.
	if (m_iShiftedTicks <= 0)
	{
		m_iShiftedTicks = 0;
		return;
	}

	if (m_bSpeedhack)
	{
		m_iShiftedTicks = Vars::CL_Move::SpeedFactor.Value;
		m_iShiftedGoal = 0;
	}

	m_iShiftedGoal = std::clamp(m_iShiftedGoal, 0, m_iMaxShift);
	if (m_iShiftedTicks > m_iShiftedGoal) // normal use/doubletap/teleport
	{
		m_bShifting = m_bShifted = m_iShiftedTicks - 1 != m_iShiftedGoal;
		m_iShiftStart = m_iShiftedTicks;

#ifndef TICKBASE_DEBUG
		while (m_iShiftedTicks > m_iShiftedGoal)
			//CLMoveFunc(accumulated_extra_samples, m_iShiftedTicks - 1 == m_iShiftedGoal);
			CLMoveFunc(accumulated_extra_samples, bFinalTick);
#else
		if (Vars::Debug::Info.Value)
			SDK::Output("Pre loop", "", { 0, 255, 255, 255 });
		while (m_iShiftedTicks > m_iShiftedGoal)
		{
			if (Vars::Debug::Info.Value)
				SDK::Output("Pre move", "", { 0, 127, 255, 255 });
			CLMoveFunc(accumulated_extra_samples, m_iShiftedTicks - 1 == m_iShiftedGoal);
			if (Vars::Debug::Info.Value)
				SDK::Output("Post move", "\n", { 0, 127, 255, 255 });
		}
		if (Vars::Debug::Info.Value)
			SDK::Output("Post loop", "\n", { 0, 0, 255, 255 });
#endif

		m_bShifting = m_bAntiWarp = false;
		if (m_bWarp)
			m_iDeficit = 0;

		m_bDoubletap = m_bWarp = false;
	}
	else // else recharge, run once if we have any choked ticks
	{
		if (I::ClientState->chokedcommands)
			CLMoveFunc(accumulated_extra_samples, bFinalTick);
	}
}

void CTickshiftHandler::CLMoveManage(CTFPlayer* pLocal)
{
	if (!pLocal)
		return;

	Recharge(pLocal);
	Warp();
	Speedhack();
}

void CTickshiftHandler::Run(float accumulated_extra_samples, bool bFinalTick, CTFPlayer* pLocal)
{
	F::NetworkFix.FixInputDelay(bFinalTick);

	CLMoveManage(pLocal);
	CLMove(accumulated_extra_samples, bFinalTick);
}

void CTickshiftHandler::CreateMove(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	if (!pLocal)
		return;

	Doubletap(pLocal, pCmd);
	AntiWarp(pLocal, pCmd);
}

void CTickshiftHandler::ManagePacket(CUserCmd* pCmd, bool* pSendPacket)
{
	if (!m_bDoubletap && !m_bWarp && !m_bSpeedhack)
		return;

	if ((m_bSpeedhack || m_bWarp) && G::Attacking == 1)
	{
		*pSendPacket = true;
		return;
	}

	*pSendPacket = m_iShiftedGoal == m_iShiftedTicks;
	if (I::ClientState->chokedcommands >= 21 // prevent overchoking
		|| m_iShiftedTicks == m_iShiftedGoal + Vars::CL_Move::Doubletap::TickLimit.Value - 1 && I::ClientState->chokedcommands) // unchoke if we are choking
		*pSendPacket = true;
}

int CTickshiftHandler::GetTicks(CTFWeaponBase* pWeapon)
{
	if (m_bDoubletap && m_iShiftedGoal < m_iShiftedTicks)
		return m_iShiftedTicks - m_iShiftedGoal;

	if (!Vars::CL_Move::Doubletap::Doubletap.Value
		|| m_iWait || m_bWarp || m_bRecharge || m_bSpeedhack || F::AutoRocketJump.IsRunning())
		return 0;

	int iTicks = std::min(m_iShiftedTicks + 1, 22);
	if (!(iTicks >= Vars::CL_Move::Doubletap::TickLimit.Value || pWeapon && GetShotsWithinPacket(pWeapon, iTicks) > 1))
		return 0;
	
	return std::min(Vars::CL_Move::Doubletap::TickLimit.Value - 1, m_iMaxShift);
}

int CTickshiftHandler::GetShotsWithinPacket(CTFWeaponBase* pWeapon, int iTicks)
{
	int iDelay = 1;
	switch (pWeapon->GetWeaponID())
	{
	case TF_WEAPON_MINIGUN:
	case TF_WEAPON_PIPEBOMBLAUNCHER:
	case TF_WEAPON_CANNON:
		iDelay = 2;
	}

	return 1 + (iTicks - iDelay) / std::ceilf(pWeapon->GetFireRate() / TICK_INTERVAL);
}

int CTickshiftHandler::GetMinimumTicksNeeded(CTFWeaponBase* pWeapon)
{
	int iDelay = 1;
	switch (pWeapon->GetWeaponID())
	{
	case TF_WEAPON_MINIGUN:
	case TF_WEAPON_PIPEBOMBLAUNCHER:
	case TF_WEAPON_CANNON:
		iDelay = 2;
	}

	return (GetShotsWithinPacket(pWeapon) - 1) * std::ceilf(pWeapon->GetFireRate() / TICK_INTERVAL) + iDelay;
}