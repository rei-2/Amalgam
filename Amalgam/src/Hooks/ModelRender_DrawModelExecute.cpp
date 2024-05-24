#include "../SDK/SDK.h"

#include "../Features/Visuals/Chams/Chams.h"
#include "../Features/Visuals/Glow/Glow.h"

MAKE_SIGNATURE(CBaseAnimating_DrawModel, "client.dll", "4C 8B DC 49 89 5B ? 89 54 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 83 EC ? 48 8B 05 ? ? ? ? 48 8D 3D", 0x0);
MAKE_SIGNATURE(ViewmodelAttachment_DrawModel, "client.dll", "41 8B D5 FF 50 ? 8B 97", 0x6);

MAKE_HOOK(ModelRender_DrawModelExecute, U::Memory.GetVFunc(I::ModelRender, 19), void, __fastcall,
	void* ecx, const DrawModelState_t& pState, const ModelRenderInfo_t& pInfo, matrix3x4* pBoneToWorld)
{
	/*
	if (!F::Chams.iRendering && !F::Glow.bRendering && !I::EngineVGui->IsGameUIVisible())
	{
		if (const auto& pEntity = I::ClientEntityList->GetClientEntity(pInfo.m_nEntIndex))
			Utils::ConLog("Entity", std::format("{}, {}, {}", pInfo.m_nEntIndex, int(pEntity->GetClassID()), I::ModelInfoClient->GetModelName(pInfo.m_pModel)).c_str());
		else
			Utils::ConLog("Model", std::format("{}, {}", pInfo.m_nEntIndex, I::ModelInfoClient->GetModelName(pInfo.m_pModel)).c_str());
	}
	*/

	if (Vars::Visuals::UI::CleanScreenshots.Value && I::EngineClient->IsTakingScreenshot() || G::Unload)
		return CALL_ORIGINAL(ecx, pState, pInfo, pBoneToWorld);

	if (F::Chams.bRendering)
		return F::Chams.RenderHandler(pState, pInfo, pBoneToWorld);
	if (F::Glow.bRendering)
		return F::Glow.RenderHandler(pState, pInfo, pBoneToWorld);

	if (F::Chams.mEntities[pInfo.entity_index])
		return;

	const auto& pEntity = I::ClientEntityList->GetClientEntity(pInfo.entity_index);
	if (pEntity && pEntity->GetClassID() == ETFClassID::CTFViewModel)
	{
		F::Glow.RenderViewmodel(pState, pInfo, pBoneToWorld);
		if (F::Chams.RenderViewmodel(pState, pInfo, pBoneToWorld))
			return;
	}

	CALL_ORIGINAL(ecx, pState, pInfo, pBoneToWorld);
}

MAKE_HOOK(CBaseAnimating_DrawModel, S::CBaseAnimating_DrawModel(), int, __fastcall,
	void* ecx, int flags)
{
	static const auto dwDrawModel = S::ViewmodelAttachment_DrawModel();
	const auto dwRetAddr = std::uintptr_t(_ReturnAddress());

	if (Vars::Visuals::UI::CleanScreenshots.Value && I::EngineClient->IsTakingScreenshot() || G::Unload)
		return CALL_ORIGINAL(ecx, flags);

	if (dwRetAddr == dwDrawModel && flags & STUDIO_RENDER)
	{
		int iReturn;
		F::Glow.RenderViewmodel(ecx, flags);
		if (F::Chams.RenderViewmodel(ecx, flags, &iReturn))
			return iReturn;
	}

	return CALL_ORIGINAL(ecx, flags);
}