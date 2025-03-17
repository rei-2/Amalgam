#include "../SDK/SDK.h"

MAKE_SIGNATURE(CTFPlayer_IsPlayerClass, "client.dll", "48 81 C1 ? ? ? ? 75 ? 32 C0", 0x0);
MAKE_SIGNATURE(CDamageAccountPanel_DisplayDamageFeedback_IsPlayerClass_Call, "client.dll", "84 C0 0F 84 ? ? ? ? 48 8B 06 48 8B CE FF 90 ? ? ? ? 49 8B 16", 0x0);

MAKE_HOOK(CTFPlayer_IsPlayerClass, S::CTFPlayer_IsPlayerClass(), bool,
	void* rcx, int iClass)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::CTFPlayer_IsPlayerClass.Map[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx, iClass);
#endif

	static const auto dwDesired = S::CDamageAccountPanel_DisplayDamageFeedback_IsPlayerClass_Call();
	const auto dwRetAddr = uintptr_t(_ReturnAddress());

	if (Vars::Misc::Sound::HitsoundAlways.Value && dwRetAddr == dwDesired)
		return false;

	return CALL_ORIGINAL(rcx, iClass);
}