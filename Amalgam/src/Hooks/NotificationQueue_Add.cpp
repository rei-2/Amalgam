#include "../SDK/SDK.h"

MAKE_SIGNATURE(NotificationQueue_Add, "client.dll", "48 89 5C 24 ? 57 48 83 EC ? 48 8B F9 48 8B 0D ? ? ? ? 48 8B 01 FF 90 ? ? ? ? 84 C0 75", 0x0);
MAKE_SIGNATURE(CTFPlayer_OnHasNewItems_Call, "client.dll", "E9 ? ? ? ? 48 8B 5C 24 ? 48 83 C4 ? C3 CC CC CC CC CC CC CC CC CC 40 53", 0x5);

MAKE_HOOK(NotificationQueue_Add, S::NotificationQueue_Add(), int, __cdecl,
	CEconNotification* pNotification)
{
	static const auto dwDesired = S::CTFPlayer_OnHasNewItems_Call();
	const auto dwRetAddr = std::uintptr_t(_ReturnAddress());

	if (Vars::Misc::Automation::AcceptItemDrops.Value && dwRetAddr == dwDesired)
	{
		pNotification->Accept();
		pNotification->Trigger();
		pNotification->UpdateTick();
		pNotification->MarkForDeletion();
		return 0;
	}

	return CALL_ORIGINAL(pNotification);
}