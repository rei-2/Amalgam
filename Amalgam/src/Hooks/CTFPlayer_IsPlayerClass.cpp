#include "../SDK/SDK.h"
#include "../Features/Misc/SpectateAll/SpectateAll.h"
#include "../Features/TempShowHealth/TempShowHealth.h"

MAKE_SIGNATURE(CTFPlayer_IsPlayerClass, "client.dll", "48 81 C1 ? ? ? ? 75 ? 32 C0", 0x0);
MAKE_SIGNATURE(CDamageAccountPanel_DisplayDamageFeedback_IsPlayerClass_Call, "client.dll", "84 C0 0F 84 ? ? ? ? 48 8B 06 48 8B CE FF 90 ? ? ? ? 49 8B 16", 0x0);

// Specific TargetID caller signatures (based on disassembly analysis)
MAKE_SIGNATURE(UpdateIDCaller, "client.dll", "84 C0 0F 84 ? ? ? ? 48 8B 06 48 8B CE FF 90 ? ? ? ? 45 0F AF F7", 0x0);
MAKE_SIGNATURE(ShouldHealthBarBeVisibleCaller, "client.dll", "84 C0 0F 84 ? ? ? ? 48 8B 06 48 8B CE", 0x0);
MAKE_SIGNATURE(IsValidIDTargetCaller, "client.dll", "84 C0 0F 84 ? ? ? ? 48 8B 06 48 8B CE FF 90 ? ? ? ? 48 85 C0", 0x0);

MAKE_HOOK(CTFPlayer_IsPlayerClass, S::CTFPlayer_IsPlayerClass(), bool,
	void* rcx, int iClass)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::CTFPlayer_IsPlayerClass[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx, iClass);
#endif

	static const auto dwUpdateIDCaller = S::UpdateIDCaller();
	static const auto dwShouldHealthBarBeVisibleCaller = S::ShouldHealthBarBeVisibleCaller();
	static const auto dwIsValidIDTargetCaller = S::IsValidIDTargetCaller();
	
	const auto dwRetAddr = uintptr_t(_ReturnAddress());
	
	// TempShowHealth: Always enable enemy health display by pretending to be spy
	if (F::TempShowHealth.m_bEnabled && 
		(dwRetAddr == dwUpdateIDCaller || dwRetAddr == dwShouldHealthBarBeVisibleCaller || dwRetAddr == dwIsValidIDTargetCaller))
	{
		// Return true when checking for spy class (class 8) to enable enemy health/name display
		if (iClass == 8) // TF_CLASS_SPY
			return true;
	}
	
	// SpectateAll enemy health/name display functionality
	if (Vars::Competitive::Features::SpectateAll.Value && 
		Vars::Competitive::SpectateAll::SpyVision.Value &&
		F::SpectateAll.ShouldSpectate() &&
		(dwRetAddr == dwUpdateIDCaller || dwRetAddr == dwShouldHealthBarBeVisibleCaller || dwRetAddr == dwIsValidIDTargetCaller))
	{
		// Return true only when checking for spy class (class 8) to enable enemy health/name display
		if (iClass == 8) // TF_CLASS_SPY
			return true;
	}

	// Hitsound functionality
	static const auto dwDesired = S::CDamageAccountPanel_DisplayDamageFeedback_IsPlayerClass_Call();
	if (Vars::Misc::Sound::HitsoundAlways.Value && dwRetAddr == dwDesired)
		return false;

	return CALL_ORIGINAL(rcx, iClass);
}