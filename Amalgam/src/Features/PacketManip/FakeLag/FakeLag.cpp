#include "FakeLag.h"

#include "../../TickHandler/TickHandler.h"
#include "../../Aimbot/AutoRocketJump/AutoRocketJump.h"

bool CFakeLag::IsAllowed(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd)
{
	if (!(Vars::CL_Move::Fakelag::Fakelag.Value || m_bPreservingBlast || m_bUnducking)
		|| I::ClientState->chokedcommands >= std::min(24 - F::Ticks.m_iShiftedTicks, std::min(21, F::Ticks.m_iMaxShift))
		|| F::Ticks.m_iShiftedGoal != F::Ticks.m_iShiftedTicks || F::Ticks.m_bRecharge
		|| !pLocal->IsAlive() || pLocal->IsAGhost())
		return false;

	if (m_bPreservingBlast)
	{
		G::PSilentAngles = true; // prevent unchoking while grounded
		return true;
	}

	if (G::Attacking == 1 && Vars::CL_Move::Fakelag::UnchokeOnAttack.Value || F::AutoRocketJump.IsRunning()
		|| Vars::CL_Move::Fakelag::Options.Value & Vars::CL_Move::Fakelag::OptionsEnum::WhileAirborne && !pLocal->m_hGroundEntity()
		|| pWeapon && pWeapon->m_iItemDefinitionIndex() == Soldier_m_TheBeggarsBazooka && pCmd->buttons & IN_ATTACK && !(G::LastUserCmd->buttons & IN_ATTACK)) // try to prevent issues
		return false;

	if (m_bUnducking)
		return true;
	
	const bool bMoving = !(Vars::CL_Move::Fakelag::Options.Value & Vars::CL_Move::Fakelag::OptionsEnum::WhileMoving) || pLocal->m_vecVelocity().Length2D() > 10.f;
	if (!bMoving)
		return false;

	switch (Vars::CL_Move::Fakelag::Fakelag.Value) 
	{
	case Vars::CL_Move::Fakelag::FakelagEnum::Plain:
	case Vars::CL_Move::Fakelag::FakelagEnum::Random:
		return I::ClientState->chokedcommands < m_iGoal;
	case Vars::CL_Move::Fakelag::FakelagEnum::Adaptive:
	{
		const Vec3 vDelta = m_vLastPosition - pLocal->m_vecOrigin();
		return vDelta.Length2DSqr() < 4096.f;
	}
	}

	return false;
}

void CFakeLag::PreserveBlastJump(CTFPlayer* pLocal)
{
	m_bPreservingBlast = false;

	if (!Vars::CL_Move::Fakelag::RetainBlastJump.Value || Vars::Misc::Movement::AutoRocketJump.Value || Vars::Misc::Movement::AutoCTap.Value
		|| !pLocal->IsAlive() || pLocal->IsAGhost() || Vars::CL_Move::Fakelag::RetainSoldierOnly.Value && pLocal->m_iClass() != TF_CLASS_SOLDIER)
		return;
	static bool bStaticGround = true;
	const bool bLastGround = bStaticGround;
	const bool bCurrGround = bStaticGround = pLocal->m_hGroundEntity();
	if (!pLocal->InCond(TF_COND_BLASTJUMPING) || bLastGround || !bCurrGround)
		return;

	m_bPreservingBlast = true;
}

void CFakeLag::Unduck(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	m_bUnducking = false;

	if (!(Vars::CL_Move::Fakelag::Options.Value & Vars::CL_Move::Fakelag::OptionsEnum::WhileUnducking)
		|| !pLocal->IsAlive() || pLocal->IsAGhost())
		return;
	if (!(pLocal->m_hGroundEntity() && pLocal->IsDucking() && !(pCmd->buttons & IN_DUCK)))
		return;

	m_bUnducking = true;
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

	switch (Vars::CL_Move::Fakelag::Fakelag.Value)
	{
	case Vars::CL_Move::Fakelag::FakelagEnum::Off: m_iGoal = 0; break;
	case Vars::CL_Move::Fakelag::FakelagEnum::Plain: m_iGoal = Vars::CL_Move::Fakelag::PlainTicks.Value; break;
	case Vars::CL_Move::Fakelag::FakelagEnum::Random: if (!m_iGoal) m_iGoal = SDK::StdRandomInt(Vars::CL_Move::Fakelag::RandomTicks.Value.Min, Vars::CL_Move::Fakelag::RandomTicks.Value.Max); break;
	case Vars::CL_Move::Fakelag::FakelagEnum::Adaptive: m_iGoal = 22; break;
	}

	Prediction(pLocal, pCmd);
	if (!IsAllowed(pLocal, pWeapon, pCmd))
	{
		m_vLastPosition = pLocal->m_vecOrigin();
		return;
	}

	*pSendPacket = false;
}