#include "TickHandler.h"

#include "../NetworkFix/NetworkFix.h"
#include "../PacketManip/AntiAim/AntiAim.h"
#include "../Aimbot/AutoRocketJump/AutoRocketJump.h"

void CTickshiftHandler::Reset()
{
	bSpeedhack = G::DoubleTap = G::Recharge = G::Warp = false;
	G::ShiftedTicks = G::ShiftedGoal = 0;
}

void CTickshiftHandler::Recharge(CTFPlayer* pLocal)
{
	if (!bGoalReached)
		return;

	bool bPassive = G::Recharge = false;

	static float flPassiveTime = 0.f;
	flPassiveTime = std::max(flPassiveTime - TICK_INTERVAL, -TICK_INTERVAL);
	if (Vars::CL_Move::Doubletap::PassiveRecharge.Value && 0.f >= flPassiveTime)
	{
		bPassive = true;
		flPassiveTime += 1.f / Vars::CL_Move::Doubletap::PassiveRecharge.Value;
	}

	if (iDeficit && G::ShiftedTicks < G::MaxShift)
	{
		bPassive = true;
		iDeficit--;
	}
	else if (iDeficit)
		iDeficit = 0;

	if (!Vars::CL_Move::Doubletap::RechargeTicks.Value && !bPassive
		|| G::DoubleTap || G::Warp || G::ShiftedTicks == G::MaxShift || bSpeedhack)
		return;

	G::Recharge = true;
	G::ShiftedGoal = G::ShiftedTicks + 1;
}

void CTickshiftHandler::Warp()
{
	if (!bGoalReached)
		return;

	G::Warp = false;
	if (!Vars::CL_Move::Doubletap::Warp.Value
		|| !G::ShiftedTicks || G::DoubleTap || G::Recharge || bSpeedhack)
		return;

	G::Warp = true;
	G::ShiftedGoal = std::max(G::ShiftedTicks - Vars::CL_Move::Doubletap::WarpRate.Value + 1, 0);
}

void CTickshiftHandler::Doubletap(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	if (!bGoalReached)
		return;

	G::DoubleTap = false;
	if (!Vars::CL_Move::Doubletap::Doubletap.Value
		|| G::WaitForShift || G::Warp || G::Recharge || bSpeedhack)
		return;

	int iTicks = std::min(G::ShiftedTicks + 1, 22);
	auto pWeapon = H::Entities.GetWeapon();
	if (!(iTicks >= Vars::CL_Move::Doubletap::TickLimit.Value || pWeapon && GetShotsWithinPacket(pWeapon, iTicks) > 1))
		return;

	bool bAttacking = G::PrimaryWeaponType == EWeaponType::MELEE ? pCmd->buttons & IN_ATTACK : G::Attacking;
	if (!G::CanPrimaryAttack && !G::Reloading || !bAttacking && !G::DoubleTap || F::AutoRocketJump.IsRunning())
		return;

	G::DoubleTap = true;
	G::ShiftedGoal = std::max(G::ShiftedTicks - Vars::CL_Move::Doubletap::TickLimit.Value + 1, 0);
	if (Vars::CL_Move::Doubletap::AntiWarp.Value)
		G::AntiWarp = pLocal->m_hGroundEntity();
}

void CTickshiftHandler::Speedhack()
{
	bSpeedhack = Vars::CL_Move::SpeedEnabled.Value;
	if (!bSpeedhack)
		return;

	G::DoubleTap = G::Warp = G::Recharge = false;
}

void CTickshiftHandler::AntiWarp(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	static Vec3 vVelocity = {};
	static int iMaxTicks = 0;
	if (G::AntiWarp)
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

	if (!G::AntiWarp)
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

	G::ShiftedTicks--;
	if (G::ShiftedTicks < 0)
		return;
	if (G::WaitForShift > 0)
		G::WaitForShift--;

	int iTicks = std::min(G::ShiftedTicks + 1, 22);
	auto pWeapon = H::Entities.GetWeapon();
	if (!(iTicks >= Vars::CL_Move::Doubletap::TickLimit.Value || pWeapon && GetShotsWithinPacket(pWeapon, iTicks) > 1))
		G::WaitForShift = 1;

	bGoalReached = bFinalTick && G::ShiftedTicks == G::ShiftedGoal;

	if (CL_Move)
		CL_Move->Original<void(__fastcall*)(float, bool)>()(accumulated_extra_samples, bFinalTick);
}

void CTickshiftHandler::MoveMain(float accumulated_extra_samples, bool bFinalTick)
{
	if (auto pWeapon = H::Entities.GetWeapon())
	{
		switch (pWeapon->GetWeaponID())
		{
		case TF_WEAPON_PIPEBOMBLAUNCHER:
		case TF_WEAPON_CANNON:
			if (G::Reloading)
				G::WaitForShift = Vars::CL_Move::Doubletap::TickLimit.Value;
			break;
		default:
			if (!ValidWeapon(pWeapon))
				G::WaitForShift = 2;
			else if (G::Attacking || !G::CanPrimaryAttack && !G::Reloading)
				G::WaitForShift = Vars::CL_Move::Doubletap::TickLimit.Value;
		}
	}
	else
		G::WaitForShift = 2;

	static auto sv_maxusrcmdprocessticks = U::ConVars.FindVar("sv_maxusrcmdprocessticks");
	G::MaxShift = sv_maxusrcmdprocessticks ? sv_maxusrcmdprocessticks->GetInt() : 24;
	if (F::AntiAim.YawOn())
		G::MaxShift -= F::AntiAim.AntiAimTicks();

	while (G::ShiftedTicks > G::MaxShift)
		CLMoveFunc(accumulated_extra_samples, false); // skim any excess ticks

	G::ShiftedTicks++; // since we now have full control over CL_Move, increment.
	if (G::ShiftedTicks <= 0)
	{
		G::ShiftedTicks = 0;
		return;
	}

	if (bSpeedhack)
	{
		for (int i = 0; i < Vars::CL_Move::SpeedFactor.Value; i++)
			CLMoveFunc(accumulated_extra_samples, i == Vars::CL_Move::SpeedFactor.Value);
		return;
	}

	G::ShiftedGoal = std::clamp(G::ShiftedGoal, 0, G::MaxShift);
	if (G::ShiftedTicks > G::ShiftedGoal) // normal use/doubletap/teleport
	{
		while (G::ShiftedTicks > G::ShiftedGoal)
			CLMoveFunc(accumulated_extra_samples, G::ShiftedTicks - 1 == G::ShiftedGoal);
		G::AntiWarp = false;
		if (G::Warp)
			iDeficit = 0;

		G::DoubleTap = G::Warp = false;
	}
	// else recharge, run once if we have any choked ticks
	else if (I::ClientState->chokedcommands)
		CLMoveFunc(accumulated_extra_samples, bFinalTick);
}

void CTickshiftHandler::MovePre(CTFPlayer* pLocal)
{
	if (!pLocal)
		return;

	Recharge(pLocal);
	Warp();
	Speedhack();
}

void CTickshiftHandler::MovePost(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	if (!pLocal)
		return;

	Doubletap(pLocal, pCmd);
	AntiWarp(pLocal, pCmd);
}

void CTickshiftHandler::Run(float accumulated_extra_samples, bool bFinalTick, CTFPlayer* pLocal)
{
	F::NetworkFix.FixInputDelay(bFinalTick);

	MovePre(pLocal);
	MoveMain(accumulated_extra_samples, bFinalTick);
}

void CTickshiftHandler::ManagePacket(CUserCmd* pCmd, bool* pSendPacket)
{
	if (!G::DoubleTap /*&& !G::Warp*/)
		return;

	*pSendPacket = G::ShiftedGoal == G::ShiftedTicks;
	if (I::ClientState->chokedcommands >= 21 // prevent overchoking
		|| G::ShiftedTicks == G::ShiftedGoal + Vars::CL_Move::Doubletap::TickLimit.Value - 1 && I::ClientState->chokedcommands) // unchoke if we are choking
		*pSendPacket = true;
}

int CTickshiftHandler::GetTicks(CTFWeaponBase* pWeapon)
{
	if (G::DoubleTap && G::ShiftedGoal < G::ShiftedTicks)
		return G::ShiftedTicks - G::ShiftedGoal;

	if (!Vars::CL_Move::Doubletap::Doubletap.Value
		|| G::WaitForShift || G::Warp || G::Recharge || bSpeedhack || F::AutoRocketJump.IsRunning())
		return 0;

	int iTicks = std::min(G::ShiftedTicks + 1, 22);
	if (!(iTicks >= Vars::CL_Move::Doubletap::TickLimit.Value || pWeapon && GetShotsWithinPacket(pWeapon, iTicks) > 1))
		return 0;
	
	return std::min(Vars::CL_Move::Doubletap::TickLimit.Value - 1, G::MaxShift);
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