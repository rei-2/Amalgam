#include "../SDK/SDK.h"

// FarESP fix: Prevent players from disappearing when off-screen by bypassing PVS optimization
// This hook addresses the Player Visible Set optimization that causes players to disappear
// when they're off-screen by returning true during specific contexts (like AccumulateLayers)

MAKE_HOOK(IVEngineClient_IsHLTV, U::Memory.GetVirtual(I::EngineClient, 87), bool,
	void* rcx)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::IVEngineClient_IsHLTV[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx);
#endif

	if (G::Unload)
		return CALL_ORIGINAL(rcx);

	// Check if ESP is enabled and we want to bypass PVS optimization
	if (Vars::ESP::Draw.Value & (Vars::ESP::DrawEnum::Players | Vars::ESP::DrawEnum::NPCs))
	{
		// Get the return address to determine the calling context
		void* pReturnAddress = _ReturnAddress();

		// Check if the call is coming from AccumulateLayers or similar bone setup functions
		// This is where the PVS optimization typically causes issues
		static uintptr_t pAccumulateLayersReturnAddress = 0;

		// Find the AccumulateLayers return address pattern if not cached
		if (!pAccumulateLayersReturnAddress)
		{
			// Pattern for AccumulateLayers context: test al, al; jnz short
			pAccumulateLayersReturnAddress = U::Memory.FindSignature("client.dll", "\x84\xC0\x75");
		}

		// If we're being called from AccumulateLayers context, return true to bypass PVS
		if (pAccumulateLayersReturnAddress &&
			((uintptr_t)pReturnAddress >= pAccumulateLayersReturnAddress - 0x100 &&
			 (uintptr_t)pReturnAddress <= pAccumulateLayersReturnAddress + 0x100))
		{
			return true;
		}

		// Alternative approach: Check for specific return address patterns
		// Pattern for SetupVelocity context as mentioned in the reference
		static uintptr_t pSetupVelocityReturnAddress = 0;
		if (!pSetupVelocityReturnAddress)
		{
			pSetupVelocityReturnAddress = U::Memory.FindSignature("client.dll",
				"\x84\xC0\x75\x38\x8B\x0D\x00\x00\x00\x00\x8B\x01\x8B\x80");
		}

		// If called from SetupVelocity context, return true
		if (pSetupVelocityReturnAddress && (uintptr_t)pReturnAddress == pSetupVelocityReturnAddress)
		{
			return true;
		}
	}

	return CALL_ORIGINAL(rcx);
}