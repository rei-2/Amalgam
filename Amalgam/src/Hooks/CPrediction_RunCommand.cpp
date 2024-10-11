#include "../SDK/SDK.h"

#include "../Features/EnginePrediction/EnginePrediction.h"

std::vector<Vec3> vAngles;

MAKE_HOOK(CPrediction_RunCommand, U::Memory.GetVFunc(I::Prediction, 17), void, __fastcall,
	void* rcx, CTFPlayer* pPlayer, CUserCmd* pCmd, IMoveHelper* pMoveHelper)
{
	F::EnginePrediction.ScalePlayers(H::Entities.GetLocal());
	CALL_ORIGINAL(rcx, pPlayer, pCmd, pMoveHelper);
	F::EnginePrediction.RestorePlayers();

	// credits: KGB
	if (pCmd->hasbeenpredicted || G::Recharge)
		return;

	auto pAnimState = pPlayer->GetAnimState();
	vAngles.push_back(G::ViewAngles);
	if (!pAnimState || G::Choking)
		return;

	for (auto& vAngle : vAngles)
	{
		pAnimState->Update(vAngle.y, vAngle.x);
		pPlayer->FrameAdvance(TICK_INTERVAL);
	}
	vAngles.clear();
}