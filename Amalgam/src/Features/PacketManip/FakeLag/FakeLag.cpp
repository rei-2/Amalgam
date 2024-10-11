#include "FakeLag.h"

#include "../../Aimbot/AutoRocketJump/AutoRocketJump.h"

bool CFakeLag::IsAllowed(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd)
{
	if (!(Vars::CL_Move::Fakelag::Fakelag.Value || bPreservingBlast || bUnducking)
		|| I::ClientState->chokedcommands >= std::min(24 - G::ShiftedTicks, 21)
		|| G::ShiftedGoal != G::ShiftedTicks || G::Recharge)
		return false;

	if (bPreservingBlast)
	{
		G::PSilentAngles = true; // prevent unchoking while grounded
		return true;
	}

	if (G::IsAttacking && Vars::CL_Move::Fakelag::UnchokeOnAttack.Value || F::AutoRocketJump.IsRunning()
		|| Vars::CL_Move::Fakelag::Options.Value & (1 << 2) && !pLocal->m_hGroundEntity()
		|| pWeapon && pWeapon->m_iItemDefinitionIndex() == Soldier_m_TheBeggarsBazooka && pCmd->buttons & IN_ATTACK && !(G::LastUserCmd->buttons & IN_ATTACK)) // try to prevent issues
		return false;

	if (bUnducking)
		return true;
	
	const bool bMoving = !(Vars::CL_Move::Fakelag::Options.Value & (1 << 0)) || pLocal->m_vecVelocity().Length2D() > 10.f;
	if (!bMoving)
		return false;

	switch (Vars::CL_Move::Fakelag::Fakelag.Value) 
	{
	case 1:
	case 2:
		return G::ChokeAmount < G::ChokeGoal;
	case 3:
	{
		const Vec3 vDelta = vLastPosition - pLocal->m_vecOrigin();
		return vDelta.Length2DSqr() < 4096.f;
	}
	}

	return false;
}

void CFakeLag::PreserveBlastJump(CTFPlayer* pLocal)
{
	bPreservingBlast = false;

	if (!Vars::CL_Move::Fakelag::RetainBlastJump.Value || !Vars::Misc::Movement::Bunnyhop.Value || !(G::Buttons & IN_JUMP)
		|| !pLocal->IsAlive() || pLocal->IsDucking() || !pLocal->m_hGroundEntity() || pLocal->m_iClass() != TF_CLASS_SOLDIER || !pLocal->InCond(TF_COND_BLASTJUMPING))
		return;

	bPreservingBlast = true;
}

void CFakeLag::Unduck(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	bUnducking = false;

	if (!(Vars::CL_Move::Fakelag::Options.Value & (1 << 1)) || !pLocal->IsAlive()
		|| !(pLocal->m_hGroundEntity() && pLocal->IsDucking() && !(pCmd->buttons & IN_DUCK)))
		return;

	bUnducking = true;
}

void CFakeLag::Prediction(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	PreserveBlastJump(pLocal);
	Unduck(pLocal, pCmd);
}

void CFakeLag::Run(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd, bool* pSendPacket)
{
	if (!pLocal)
		return;

	Prediction(pLocal, pCmd);

	// Set the selected choke amount (if not random)
	switch (Vars::CL_Move::Fakelag::Fakelag.Value)
	{
	case 1: G::ChokeGoal = Vars::CL_Move::Fakelag::PlainTicks.Value; break;
	case 2: if (!G::ChokeGoal) G::ChokeGoal = SDK::StdRandomInt(Vars::CL_Move::Fakelag::RandomTicks.Value.Min, Vars::CL_Move::Fakelag::RandomTicks.Value.Max); break;
	case 3: G::ChokeGoal = 22; break;
	}

	// Are we even allowed to choke?
	if (!IsAllowed(pLocal, pWeapon, pCmd))
	{
		vLastPosition = pLocal->m_vecOrigin();
		G::ChokeAmount = G::ChokeGoal = 0;
		return;
	}

	*pSendPacket = false;
	G::ChokeAmount++;
}