#include "../SDK/SDK.h"

MAKE_SIGNATURE(KeyValues_SetInt, "client.dll", "40 53 48 83 EC ? 41 8B D8 41 B0", 0x0);
MAKE_SIGNATURE(KeyValues_SetInt_Desired, "client.dll", "49 8B 06 4C 8B C3 8B D7", 0x0);
MAKE_SIGNATURE(KeyValues_SetInt_Jump, "client.dll", "8B F0 E8 ? ? ? ? 3B C7", 0x0);

MAKE_HOOK(KeyValues_SetInt, S::KeyValues_SetInt(), void, __fastcall,
	void* ecx, const char* keyName, int value)
{
	static auto dwDesired = S::KeyValues_SetInt_Desired();
	static auto dwJump = S::KeyValues_SetInt_Jump();
	const auto dwRetAddr = std::uintptr_t(_ReturnAddress());

	CALL_ORIGINAL(ecx, keyName, value);

	if (!Vars::Visuals::UI::RevealScoreboard.Value || !keyName)
		return;

	/* Scoreboard class reveal */
	if (dwRetAddr == dwDesired && FNV1A::Hash(keyName) == FNV1A::HashConst("nemesis"))
		*static_cast<uintptr_t*>(_AddressOfReturnAddress()) = dwJump;
}