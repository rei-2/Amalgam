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
	if (bGoalReached)
		G::ShiftedGoal = G::ShiftedTicks + 1;
}

void CTickshiftHandler::Teleport()
{
	G::Warp = false;
	if (!G::ShiftedTicks || G::DoubleTap || G::Recharge || bSpeedhack)
		return;

	G::Warp = Vars::CL_Move::Doubletap::Warp.Value;
	if (G::Warp && bGoalReached)
		G::ShiftedGoal = std::max(G::ShiftedTicks - Vars::CL_Move::Doubletap::WarpRate.Value + 1, 0);
}

void CTickshiftHandler::Doubletap(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	bool bAttacking = G::PrimaryWeaponType == EWeaponType::MELEE ? pCmd->buttons & IN_ATTACK : G::Attacking;
	if (G::ShiftedTicks < std::min(Vars::CL_Move::Doubletap::TickLimit.Value - 1, G::MaxShift)
		|| G::WaitForShift || G::Warp || G::Recharge || bSpeedhack
		|| !G::CanPrimaryAttack && !G::Reloading || !bAttacking && !G::DoubleTap || F::AutoRocketJump.IsRunning())
		return;

	G::DoubleTap = Vars::CL_Move::Doubletap::Doubletap.Value;
	if (G::DoubleTap && Vars::CL_Move::Doubletap::AntiWarp.Value)
		G::AntiWarp = pLocal->m_hGroundEntity();
	if (G::DoubleTap && bGoalReached)
		G::ShiftedGoal = G::ShiftedTicks - std::min(Vars::CL_Move::Doubletap::TickLimit.Value - 1, G::MaxShift);
}

int CTickshiftHandler::GetTicks()
{
	if (G::DoubleTap && G::ShiftedGoal < G::ShiftedTicks)
		return G::ShiftedTicks - G::ShiftedGoal;

	if (!Vars::CL_Move::Doubletap::Doubletap.Value
		|| G::ShiftedTicks < std::min(Vars::CL_Move::Doubletap::TickLimit.Value - 1, G::MaxShift)
		|| G::WaitForShift || G::Warp || G::Recharge || bSpeedhack || F::AutoRocketJump.IsRunning())
		return 0;

	return std::min(Vars::CL_Move::Doubletap::TickLimit.Value - 1, G::MaxShift);
}

void CTickshiftHandler::Speedhack()
{
	bSpeedhack = Vars::CL_Move::SpeedEnabled.Value;
	if (!bSpeedhack)
		return;

	G::DoubleTap = G::Warp = G::Recharge = false;
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

	if (G::ShiftedTicks < std::min(Vars::CL_Move::Doubletap::TickLimit.Value - 1, G::MaxShift))
		G::WaitForShift = 1;

	bGoalReached = bFinalTick && G::ShiftedTicks == G::ShiftedGoal;

	if (CL_Move)
		CL_Move->Original<void(__fastcall*)(float, bool)>()(accumulated_extra_samples, bFinalTick);
}

void CTickshiftHandler::MoveMain(float accumulated_extra_samples, bool bFinalTick)
{
	if (auto pWeapon = H::Entities.GetWeapon())
	{
		const int iWeaponID = pWeapon->GetWeaponID();
		if (iWeaponID != TF_WEAPON_PIPEBOMBLAUNCHER && iWeaponID != TF_WEAPON_CANNON)
		{
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
		G::MaxShift -= 3;

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
	Teleport();
	Speedhack();
}

void CTickshiftHandler::MovePost(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	if (!pLocal)
		return;

	Doubletap(pLocal, pCmd);
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