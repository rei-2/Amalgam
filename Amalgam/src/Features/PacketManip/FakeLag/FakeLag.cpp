#include "FakeLag.h"

#include "../../Ticks/Ticks.h"
#include "../../Aimbot/AutoRocketJump/AutoRocketJump.h"

bool CFakeLag::IsAllowed(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd)
{
	const bool bCurrentlyGrounded = pLocal->m_hGroundEntity();
	const bool bJustLeftGround = m_bWasGroundedLastTick && !bCurrentlyGrounded;
	m_bWasGroundedLastTick = bCurrentlyGrounded;

	if (m_bPreservingBlast && bJustLeftGround)
	{
		m_bPreservingBlast = false;
		return false;
	}

	if (!(Vars::Fakelag::Fakelag.Value || m_bPreservingBlast || m_bUnducking)
		|| I::ClientState->chokedcommands >= std::min(24 - F::Ticks.m_iShiftedTicks, std::min(21, F::Ticks.m_iMaxShift))
		|| F::Ticks.m_iShiftedGoal != F::Ticks.m_iShiftedTicks || F::Ticks.m_bRecharge
		|| !pLocal->IsAlive() || pLocal->IsAGhost())
		return false;

	if (m_bPreservingBlast)
	{
		G::PSilentAngles = true; // prevent unchoking while grounded
		return true;
	}

	if (G::Attacking == 1 && Vars::Fakelag::UnchokeOnAttack.Value || F::AutoRocketJump.IsRunning()
		|| Vars::Fakelag::Options.Value & Vars::Fakelag::OptionsEnum::NotAirborne && !pLocal->m_hGroundEntity()
		|| pWeapon && pWeapon->m_iItemDefinitionIndex() == Soldier_m_TheBeggarsBazooka && pCmd->buttons & IN_ATTACK && !(G::LastUserCmd->buttons & IN_ATTACK)) // try to prevent issues
		return false;

	if (m_bUnducking)
		return true;
	
	const bool bMoving = !(Vars::Fakelag::Options.Value & Vars::Fakelag::OptionsEnum::OnlyMoving) || pLocal->m_vecVelocity().Length2D() > 10.f;
	if (!bMoving)
		return false;

	switch (Vars::Fakelag::Fakelag.Value) 
	{
	case Vars::Fakelag::FakelagEnum::Plain:
	case Vars::Fakelag::FakelagEnum::Random:
		return I::ClientState->chokedcommands < m_iGoal;
	case Vars::Fakelag::FakelagEnum::Adaptive:
	{
		static auto sv_lagcompensation_teleport_dist = U::ConVars.FindVar("sv_lagcompensation_teleport_dist");
		const float flDist = powf(sv_lagcompensation_teleport_dist->GetFloat(), 2.f);
		return (m_vLastPosition - pLocal->m_vecOrigin()).Length2DSqr() < flDist;
	}
	}

	return false;
}

void CFakeLag::PreserveBlastJump(CTFPlayer* pLocal)
{
	m_bPreservingBlast = false;

	if (!Vars::Fakelag::RetainBlastJump.Value || Vars::Misc::Movement::AutoRocketJump.Value || Vars::Misc::Movement::AutoCTap.Value
		|| !pLocal->IsAlive() || pLocal->IsAGhost() || Vars::Fakelag::RetainSoldierOnly.Value && pLocal->m_iClass() != TF_CLASS_SOLDIER)
		return;

	if (!pLocal->InCond(TF_COND_BLASTJUMPING))
		return;

	const bool bInAir = !pLocal->m_hGroundEntity();
	if (!bInAir)
		return;

	Vec3 vVelocity = pLocal->m_vecVelocity();
	Vec3 vOrigin = pLocal->m_vecOrigin();
	
	CGameTrace trace = {};
	CTraceFilterWorldAndPropsOnly filter = {};
	
	static auto sv_gravity = U::ConVars.FindVar("sv_gravity");
	float flGravity = sv_gravity ? sv_gravity->GetFloat() : 800.f;
	
	vVelocity.z -= flGravity * I::GlobalVars->interval_per_tick;
	Vec3 vNextOrigin = vOrigin + vVelocity * I::GlobalVars->interval_per_tick;
	
	SDK::Trace(vNextOrigin, vNextOrigin - Vec3(0, 0, 2.f), MASK_PLAYERSOLID, &filter, &trace);
	
	const bool bWillLand = trace.DidHit() && vVelocity.z < 0.f;
	if (bWillLand && I::ClientState->chokedcommands < 2)
		m_bPreservingBlast = true;
}

void CFakeLag::Unduck(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	m_bUnducking = false;

	if (!(Vars::Fakelag::Options.Value & Vars::Fakelag::OptionsEnum::OnUnduck)
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
	if (!m_iGoal)
	{
		switch (Vars::Fakelag::Fakelag.Value)
		{
		case Vars::Fakelag::FakelagEnum::Off: m_iGoal = 0; break;
		case Vars::Fakelag::FakelagEnum::Plain: m_iGoal = Vars::Fakelag::PlainTicks.Value; break;
		case Vars::Fakelag::FakelagEnum::Random: m_iGoal = SDK::StdRandomInt(Vars::Fakelag::RandomTicks.Value.Min, Vars::Fakelag::RandomTicks.Value.Max); break;
		case Vars::Fakelag::FakelagEnum::Adaptive: m_iGoal = 22; break;
		}
	}

	Prediction(pLocal, pCmd);
	if (!IsAllowed(pLocal, pWeapon, pCmd))
	{
		m_iGoal = 0;
		m_vLastPosition = pLocal->m_vecOrigin();
		return;
	}

	*pSendPacket = false;
}