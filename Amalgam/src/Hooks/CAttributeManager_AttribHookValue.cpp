#include "../SDK/SDK.h"

MAKE_SIGNATURE(CAttributeManager_AttribHookInt, "client.dll", "4C 8B DC 49 89 5B ? 49 89 6B ? 49 89 73 ? 57 41 54 41 55 41 56 41 57 48 83 EC ? 48 8B 3D ? ? ? ? 4C 8D 35", 0x0);
MAKE_SIGNATURE(CTFPlayer_FireEvent_AttribHookValue_Call, "client.dll", "8B F8 83 BE", 0x0);

static int ColorToInt(const Color_t& col)
{
    return col.r << 16 | col.g << 8 | col.b;
}

MAKE_HOOK(CAttributeManager_AttribHookInt, S::CAttributeManager_AttribHookInt(), int,
	int value, const char* name, void* econent, void* buffer, bool isGlobalConstString)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::CAttributeManager_AttribHookValue[DEFAULT_BIND])
		return CALL_ORIGINAL(value, name, econent, buffer, isGlobalConstString);
#endif

	static const auto dwDesired = S::CTFPlayer_FireEvent_AttribHookValue_Call();
	const auto dwRetAddr = uintptr_t(_ReturnAddress());

	if (!Vars::Visuals::Effects::SpellFootsteps.Value || econent != H::Entities.GetLocal() || I::EngineClient->IsTakingScreenshot() && Vars::Visuals::UI::CleanScreenshots.Value)
		return CALL_ORIGINAL(value, name, econent, buffer, isGlobalConstString);

	if (dwRetAddr == dwDesired && FNV1A::Hash32(name) == FNV1A::Hash32Const("halloween_footstep_type"))
	{
		switch (Vars::Visuals::Effects::SpellFootsteps.Value)
		{
		case Vars::Visuals::Effects::SpellFootstepsEnum::Color: return ColorToInt(Vars::Colors::SpellFootstep.Value);
		case Vars::Visuals::Effects::SpellFootstepsEnum::Team: return 1;
		case Vars::Visuals::Effects::SpellFootstepsEnum::Halloween: return 2;
		}
	}

	return CALL_ORIGINAL(value, name, econent, buffer, isGlobalConstString);
}