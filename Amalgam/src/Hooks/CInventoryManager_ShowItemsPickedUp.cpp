#include "../SDK/SDK.h"

MAKE_SIGNATURE(CInventoryManager_ShowItemsPickedUp, "client.dll", "44 88 4C 24 ? 44 88 44 24 ? 53 41 56", 0x0);

MAKE_HOOK(CInventoryManager_ShowItemsPickedUp, S::CInventoryManager_ShowItemsPickedUp(), bool, __fastcall,
	void* ecx, bool bForce, bool bReturnToGame, bool bNoPanel)
{
	if (Vars::Misc::Automation::AcceptItemDrops.Value)
	{
		CALL_ORIGINAL(ecx, true, true, true);
		return false;
	}
	return CALL_ORIGINAL(ecx, bForce, bReturnToGame, bNoPanel);
}