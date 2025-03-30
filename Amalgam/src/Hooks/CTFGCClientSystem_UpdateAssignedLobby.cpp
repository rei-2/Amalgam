#include "../SDK/SDK.h"

MAKE_HOOK(CTFGCClientSystem_UpdateAssignedLobby, S::CTFGCClientSystem_UpdateAssignedLobby(), bool,
	void* rcx)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::CTFGCClientSystem_UpdateAssignedLobby[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx);
#endif

	bool bReturn = CALL_ORIGINAL(rcx);

	if (rcx && Vars::Misc::Game::F2PChatBypass.Value)
		I::TFGCClientSystem->SetNonPremiumAccount(false);

	return bReturn;
}