#include "../SDK/SDK.h"

MAKE_SIGNATURE(CSkyboxView_Enable3dSkyboxFog, "client.dll", "40 57 48 83 EC ? E8 ? ? ? ? 48 8B F8 48 85 C0 0F 84 ? ? ? ? 48 8B 0D", 0x0);

MAKE_HOOK(CSkyboxView_Enable3dSkyboxFog, S::CSkyboxView_Enable3dSkyboxFog(), void, __fastcall,
	void* ecx)
{
	if (!(Vars::Visuals::World::Modulations.Value & 1 << 4) || I::EngineClient->IsTakingScreenshot() && Vars::Visuals::UI::CleanScreenshots.Value)
		return CALL_ORIGINAL(ecx);

	if (!Vars::Colors::FogModulation.Value.a)
		return;

	CALL_ORIGINAL(ecx);
	if (const auto pRenderContext = I::MaterialSystem->GetRenderContext())
	{
		float blend[3] = { float(Vars::Colors::FogModulation.Value.r) / 255.f, float(Vars::Colors::FogModulation.Value.g) / 255.f, float(Vars::Colors::FogModulation.Value.b) / 255.f };
		pRenderContext->FogColor3fv(blend);
	}
}