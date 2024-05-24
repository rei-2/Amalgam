// what is this for?

#include "../SDK/SDK.h"

MAKE_SIGNATURE(CTFGCClientSystem_UpdateAssignedLobby, "client.dll", "55 8B EC 83 EC 28 53 8B D9 56 57 89 5D F0 8B 8B ? ? ? ? 85 C9 0F 84 ? ? ? ? 68 ? ? ? ? E8 ? ? ? ? 85 C0 74 77", 0x0);

MAKE_HOOK(CTFGCClientSystem_UpdateAssignedLobby, S::CTFGCClientSystem_UpdateAssignedLobby(), char /*?*/, __fastcall,
	void* ecx)
{
	char cReturn = CALL_ORIGINAL(ecx);
	if (ecx)
		*((bool*)ecx + 1440) = false;
	return cReturn;
}