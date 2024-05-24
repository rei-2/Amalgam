#include "../SDK/SDK.h"

MAKE_SIGNATURE(CTFPlayerInventory_GetMaxItemCount, "client.dll", "40 53 48 83 EC ? 48 8B 89 ? ? ? ? BB", 0x0);

MAKE_HOOK(CTFPlayerInventory_GetMaxItemCount, S::CTFPlayerInventory_GetMaxItemCount(), int, __fastcall,
	void* ecx)
{
	return !Vars::Misc::Automation::BackpackExpander.Value ? CALL_ORIGINAL(ecx) : 3000;
}