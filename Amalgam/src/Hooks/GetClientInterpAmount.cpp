#include "../SDK/SDK.h"

MAKE_SIGNATURE(GetClientInterpAmount, "client.dll", "40 53 48 83 EC ? 8B 05 ? ? ? ? A8 ? 75 ? 48 8B 0D ? ? ? ? 48 8D 15", 0x0);
MAKE_SIGNATURE(CNetGraphPanel_DrawTextFields_GetClientInterpAmount_Call1, "client.dll", "F3 41 0F 59 C1 4C 8D 05", 0x0);
MAKE_SIGNATURE(CNetGraphPanel_DrawTextFields_GetClientInterpAmount_Call2, "client.dll", "0F 28 F8 0F 2F 3D ? ? ? ? 76", 0x0);

MAKE_HOOK(GetClientInterpAmount, S::GetClientInterpAmount(), float,
	)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::GetClientInterpAmount[DEFAULT_BIND])
		return CALL_ORIGINAL();
#endif

	if (!Vars::Visuals::Removals::Lerp.Value && !Vars::Visuals::Removals::Interpolation.Value)
		return CALL_ORIGINAL();

	static const auto dwDesired1 = S::CNetGraphPanel_DrawTextFields_GetClientInterpAmount_Call1();
	static const auto dwDesired2 = S::CNetGraphPanel_DrawTextFields_GetClientInterpAmount_Call2();
	const auto dwRetAddr = uintptr_t(_ReturnAddress());

	if (dwRetAddr == dwDesired1 || dwRetAddr == dwDesired2)
		return CALL_ORIGINAL();
	
	return 0.f;
}