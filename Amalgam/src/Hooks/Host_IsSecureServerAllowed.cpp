#include "../SDK/SDK.h"

MAKE_SIGNATURE(g_bAllowSecureServers, "engine.dll", "40 88 35 ? ? ? ? 40 84 FF", 0x0);
MAKE_SIGNATURE(Host_IsSecureServerAllowed, "engine.dll", "48 83 EC ? FF 15 ? ? ? ? 48 8D 15 ? ? ? ? 48 8B C8 4C 8B 00 41 FF 50 ? 85 C0 75", 0x0);

MAKE_HOOK(Host_IsSecureServerAllowed, S::Host_IsSecureServerAllowed(), bool)
{
	if (Vars::Misc::Game::VACBypass.Value)
	{
		if (auto uSig = S::g_bAllowSecureServers())
		{
			__try { *reinterpret_cast<bool*>(U::Memory.RelToAbs(uSig)) = true; }
			__except (EXCEPTION_EXECUTE_HANDLER) {}
		}
		return true;
	}
	return CALL_ORIGINAL();
}
