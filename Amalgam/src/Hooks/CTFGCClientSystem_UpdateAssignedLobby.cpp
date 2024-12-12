#include "../SDK/SDK.h"


MAKE_HOOK(CTFGCClientSystem_UpdateAssignedLobby, S::CTFGCClientSystem_UpdateAssignedLobby(), char,
	void* rcx)
{
	if (rcx && Vars::Misc::Game::F2PChatBypass.Value)
		I::TFGCClientSystem->SetNonPremiumAccount(false);

	return CALL_ORIGINAL(rcx);
}