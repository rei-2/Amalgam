#include "../SDK/SDK.h"

MAKE_SIGNATURE(CSkyboxView_Enable3dSkyboxFog, "client.dll", "40 57 48 83 EC ? E8 ? ? ? ? 48 8B F8 48 85 C0 0F 84 ? ? ? ? 48 8B 0D", 0x0);

MAKE_HOOK(CSkyboxView_Enable3dSkyboxFog, S::CSkyboxView_Enable3dSkyboxFog(), void,
	void* rcx)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::CSkyboxView_Enable3dSkyboxFog[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx);
#endif

	if (!(Vars::Visuals::World::Modulations.Value & Vars::Visuals::World::ModulationsEnum::Fog) || I::EngineClient->IsTakingScreenshot() && Vars::Visuals::UI::CleanScreenshots.Value)
		return CALL_ORIGINAL(rcx);

	CALL_ORIGINAL(rcx);
	if (auto pRenderContext = I::MaterialSystem->GetRenderContext())
	{
		if (Vars::Colors::FogModulation.Value.a)
		{
			pRenderContext->FogColor3ub(Vars::Colors::FogModulation.Value.r, Vars::Colors::FogModulation.Value.g, Vars::Colors::FogModulation.Value.b);

			float flRatio = 255.f / Vars::Colors::FogModulation.Value.a;
			float flStart, flEnd; pRenderContext->GetFogDistances(&flStart, &flEnd, nullptr);
			pRenderContext->FogStart(flStart * flRatio);
			pRenderContext->FogEnd(flEnd * flRatio);
		}
		else
			pRenderContext->FogMode(MATERIAL_FOG_NONE);
	}
}