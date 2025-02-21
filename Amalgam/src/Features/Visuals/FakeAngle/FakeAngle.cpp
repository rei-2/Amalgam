#include "FakeAngle.h"

#include "../../PacketManip/AntiAim/AntiAim.h"
#include "../../TickHandler/TickHandler.h"

void CFakeAngle::Run(CTFPlayer* pLocal)
{
	if (!pLocal || !pLocal->IsAlive() || pLocal->IsAGhost()
		|| !F::AntiAim.AntiAimOn() && (!Vars::CL_Move::Fakelag::Fakelag.Value || F::Ticks.m_iShiftedTicks == F::Ticks.m_iMaxShift)
		|| !Vars::Chams::FakeAngle::Enabled.Value && !Vars::Glow::FakeAngle::Enabled.Value)
	{
		bBonesSetup = false;
		return;
	}

	auto pAnimState = pLocal->GetAnimState();
	if (!pAnimState)
		return;

	float flOldFrameTime = I::GlobalVars->frametime;
	int nOldSequence = pLocal->m_nSequence();
	float flOldCycle = pLocal->m_flCycle();
	auto pOldPoseParams = pLocal->m_flPoseParameter();
	char pOldAnimState[sizeof(CTFPlayerAnimState)] = {};
	memcpy(pOldAnimState, pAnimState, sizeof(CTFPlayerAnimState));

	I::GlobalVars->frametime = 0.f;
	Vec2 vAngle = { std::clamp(F::AntiAim.vFakeAngles.x, -89.f, 89.f), F::AntiAim.vFakeAngles.y };
	if (pLocal->IsTaunting() && pLocal->m_bAllowMoveDuringTaunt())
		pLocal->m_flTauntYaw() = vAngle.y;
	pAnimState->Update(pAnimState->m_flCurrentFeetYaw = /*pAnimState->m_flEyeYaw =*/ vAngle.y, vAngle.x);
	pLocal->InvalidateBoneCache(); // fix issue with certain cosmetics
	bBonesSetup = pLocal->SetupBones(aBones, MAXSTUDIOBONES, BONE_USED_BY_ANYTHING, I::GlobalVars->curtime);

	I::GlobalVars->frametime = flOldFrameTime;
	pLocal->m_nSequence() = nOldSequence;
	pLocal->m_flCycle() = flOldCycle;
	pLocal->m_flPoseParameter() = pOldPoseParams;
	memcpy(pAnimState, pOldAnimState, sizeof(CTFPlayerAnimState));
}