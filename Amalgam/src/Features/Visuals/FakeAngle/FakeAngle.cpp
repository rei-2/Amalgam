#include "FakeAngle.h"

#include "../../PacketManip/AntiAim/AntiAim.h"

void CFakeAngle::Run(CTFPlayer* pLocal)
{
	if (!pLocal || !pLocal->IsAlive() || pLocal->IsAGhost())
		return;

	auto pAnimState = pLocal->GetAnimState();
	if (!pAnimState)
		return;

	F::AntiAim.vFakeAngles.x = std::clamp(F::AntiAim.vFakeAngles.x, -89.f, 89.f);

	float flOldFrameTime = I::GlobalVars->frametime;
	int nOldSequence = pLocal->m_nSequence();
	float flOldCycle = pLocal->m_flCycle();
	auto& pOldPoseParams = pLocal->m_flPoseParameter();
	char pOldAnimState[sizeof(CTFPlayerAnimState)] = {};
	memcpy(pOldAnimState, pAnimState, sizeof(CTFPlayerAnimState));

	I::GlobalVars->frametime = 0.f;
	pAnimState->m_flCurrentFeetYaw = F::AntiAim.vFakeAngles.y;
	pAnimState->m_flEyeYaw = F::AntiAim.vFakeAngles.y;
	pAnimState->Update(F::AntiAim.vFakeAngles.y, F::AntiAim.vFakeAngles.x);

	BonesSetup = pLocal->SetupBones(BoneMatrix, 128, BONE_USED_BY_ANYTHING, I::GlobalVars->curtime);

	I::GlobalVars->frametime = flOldFrameTime;
	pLocal->m_nSequence() = nOldSequence;
	pLocal->m_flCycle() = flOldCycle;
	pLocal->m_flPoseParameter() = pOldPoseParams;
	memcpy(pAnimState, pOldAnimState, sizeof(CTFPlayerAnimState));
}