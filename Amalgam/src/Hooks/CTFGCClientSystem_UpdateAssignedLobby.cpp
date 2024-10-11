#include "../SDK/SDK.h"

MAKE_SIGNATURE(CTFGCClientSystem_UpdateAssignedLobby, "client.dll", "40 55 53 41 54 41 56 41 57 48 8B EC", 0x0);

MAKE_HOOK(CTFGCClientSystem_UpdateAssignedLobby, S::CTFGCClientSystem_UpdateAssignedLobby(), char, __fastcall,
	void* rcx)
{
	if (rcx && Vars::Misc::Game::F2PChatBypass.Value)
		*reinterpret_cast<bool*>(uintptr_t(rcx) + 1888) = false;

	return CALL_ORIGINAL(rcx);
}