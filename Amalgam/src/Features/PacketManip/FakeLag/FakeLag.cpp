#include "FakeLag.h"

bool CFakeLag::IsAllowed(CTFPlayer* pLocal)
{
	const int iMaxSend = std::min(24 - G::ShiftedTicks, 22);
	const bool bVar = Vars::CL_Move::Fakelag::Fakelag.Value || bPreservingBlast || bUnducking;
	const bool bChargePrio = (iMaxSend > 0 && G::ChokeAmount < iMaxSend) || !G::ShiftedTicks;
	const bool bAttacking = G::IsAttacking && Vars::CL_Move::Fakelag::UnchokeOnAttack.Value;
	const bool bNotAir = Vars::CL_Move::Fakelag::Options.Value & (1 << 2) && !pLocal->OnSolid();

	if (!bVar || !bChargePrio || bAttacking || bNotAir)
		return false;

	if (bPreservingBlast || bUnducking)
		return true;

	if (G::ShiftedGoal != G::ShiftedTicks)
		return false;
	
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
	if (!pLocal || !pLocal->IsAlive() || pLocal->IsAGhost() || !pLocal->IsPlayer() || G::ShiftedTicks == G::MaxShift)
		return;

	const bool bVar = Vars::CL_Move::Fakelag::RetainBlastJump.Value && Vars::Misc::Movement::Bunnyhop.Value;
	static bool bOldSolid = false; const bool bPlayerReady = pLocal->OnSolid() || bOldSolid; bOldSolid = pLocal->OnSolid();
	const bool bCanPreserve = pLocal->m_iClass() == TF_CLASS_SOLDIER && pLocal->InCond(TF_COND_BLASTJUMPING);
	const bool bValid = G::Buttons & IN_JUMP && !pLocal->IsDucking();

	bPreservingBlast = bVar && bPlayerReady && bCanPreserve && bValid;
}

void CFakeLag::Unduck(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	if (!pLocal || !pLocal->IsAlive() || pLocal->IsAGhost())
		return;

	const bool bVar = Vars::CL_Move::Fakelag::Options.Value & (1 << 1);
	const bool bPlayerReady = pLocal->IsPlayer() && pLocal->OnSolid() && pLocal->IsDucking() && !(pCmd->buttons & IN_DUCK);

	bUnducking = bVar && bPlayerReady;
}

void CFakeLag::Prediction(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	PreserveBlastJump(pLocal);
	Unduck(pLocal, pCmd);
}

void CFakeLag::Run(CTFPlayer* pLocal, CUserCmd* pCmd, bool* pSendPacket)
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
	if (!IsAllowed(pLocal))
	{
		vLastPosition = pLocal->m_vecOrigin();
		G::ChokeAmount = G::ChokeGoal = 0;
		bUnducking = false;
		return;
	}

	*pSendPacket = false;
	G::ChokeAmount++;
}