#include "../SDK/SDK.h"

#include "../Features/EnginePrediction/EnginePrediction.h"
#include "../Features/Visuals/FakeAngle/FakeAngle.h"

MAKE_HOOK(CPrediction_RunCommand, U::Memory.GetVFunc(I::Prediction, 17), void,
	void* rcx, CTFPlayer* pPlayer, CUserCmd* pCmd, IMoveHelper* pMoveHelper)
{
	F::EnginePrediction.ScalePlayers(H::Entities.GetLocal());
	CALL_ORIGINAL(rcx, pPlayer, pCmd, pMoveHelper);
	F::EnginePrediction.RestorePlayers();
}