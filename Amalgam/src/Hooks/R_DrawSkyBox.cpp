#include "../SDK/SDK.h"

MAKE_SIGNATURE(R_DrawSkyBox, "engine.dll", "48 8B C4 55 53 41 54 41 55", 0x0);

MAKE_HOOK(R_DrawSkyBox, S::R_DrawSkyBox(), void,
	float zFar, int nDrawFlags)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::R_DrawSkyBox[DEFAULT_BIND])
		return CALL_ORIGINAL(zFar, nDrawFlags);
#endif

	if (FNV1A::Hash32(Vars::Visuals::World::SkyboxChanger.Value.c_str()) == FNV1A::Hash32Const("Off") || Vars::Visuals::UI::CleanScreenshots.Value && I::EngineClient->IsTakingScreenshot())
		return CALL_ORIGINAL(zFar, nDrawFlags);

	static auto sv_skyname = U::ConVars.FindVar("sv_skyname");
	std::string sOriginal = sv_skyname->GetString();
	sv_skyname->SetValue(Vars::Visuals::World::SkyboxChanger.Value.c_str());
	CALL_ORIGINAL(zFar, nDrawFlags);
	sv_skyname->SetValue(sOriginal.c_str());
}