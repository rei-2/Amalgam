#include "../SDK/SDK.h"

std::vector<Vec3> vAngles;

MAKE_HOOK(CPrediction_RunCommand, U::Memory.GetVFunc(I::Prediction, 17), void, __fastcall,
	void* ecx, CTFPlayer* pPlayer, CUserCmd* pCmd, IMoveHelper* moveHelper)
{
	CALL_ORIGINAL(ecx, pPlayer, pCmd, moveHelper);

	// credits: KGB
	if (pPlayer != H::Entities.GetLocal() || G::Recharge || pCmd->hasbeenpredicted)
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