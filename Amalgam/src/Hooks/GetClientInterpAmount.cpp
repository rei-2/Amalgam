#include "../SDK/SDK.h"

MAKE_SIGNATURE(GetClientInterpAmount, "client.dll", "40 53 48 83 EC ? 8B 05 ? ? ? ? A8 ? 75 ? 48 8B 0D ? ? ? ? 48 8D 15", 0x0);
MAKE_SIGNATURE(CNetGraphPanel_DrawTextFields_Call, "client.dll", "F3 41 0F 59 C1 4C 8D 05", 0x0);

MAKE_HOOK(GetClientInterpAmount, S::GetClientInterpAmount(), float, __cdecl,
	)
{
	static auto dwDesired = S::CNetGraphPanel_DrawTextFields_Call();
	const auto dwRetAddr = std::uintptr_t(_ReturnAddress());
	
	return dwRetAddr == dwDesired ? CALL_ORIGINAL() : 0.f;
}