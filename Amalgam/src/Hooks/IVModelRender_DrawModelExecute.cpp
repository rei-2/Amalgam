#include "../SDK/SDK.h"

#include "../Features/Visuals/Chams/Chams.h"
#include "../Features/Visuals/Glow/Glow.h"
#include "../Features/Visuals/Materials/Materials.h"
#include "../Features/Visuals/CameraWindow/CameraWindow.h"

MAKE_SIGNATURE(CBaseAnimating_DrawModel, "client.dll", "4C 8B DC 49 89 5B ? 89 54 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 83 EC ? 48 8B 05 ? ? ? ? 48 8D 3D", 0x0);
MAKE_SIGNATURE(CEconEntity_DrawOverriddenViewmodel_DrawModel_Call, "client.dll", "41 8B D5 FF 50 ? 8B 97", 0x6);
MAKE_SIGNATURE(CBaseAnimating_InternalDrawModel, "client.dll", "48 8B C4 55 56 48 8D 6C 24 ? 48 81 EC ? ? ? ? 44 8B 81", 0x0);

MAKE_HOOK(IVModelRender_DrawModelExecute, U::Memory.GetVirtual(I::ModelRender, 19), void,
	void* rcx, const DrawModelState_t& pState, const ModelRenderInfo_t& pInfo, matrix3x4* pBoneToWorld)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::IVModelRender_DrawModelExecute[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx, pState, pInfo, pBoneToWorld);
#endif

	/*
	if (!F::Chams.iRendering && !F::Glow.m_bRendering && !I::EngineVGui->IsGameUIVisible())
	{
		if (const auto& pEntity = I::ClientEntityList->GetClientEntity(pInfo.m_nEntIndex))
			Utils::ConLog("Entity", std::format("{}, {}, {}", pInfo.m_nEntIndex, int(pEntity->GetClassID()), I::ModelInfoClient->GetModelName(pInfo.m_pModel)).c_str());
		else
			Utils::ConLog("Model", std::format("{}, {}", pInfo.m_nEntIndex, I::ModelInfoClient->GetModelName(pInfo.m_pModel)).c_str());
	}
	*/

	if (I::EngineVGui->IsGameUIVisible() || Vars::Visuals::UI::CleanScreenshots.Value && I::EngineClient->IsTakingScreenshot()
		|| F::CameraWindow.m_bDrawing || !F::Materials.m_bLoaded || G::Unload)
		return CALL_ORIGINAL(rcx, pState, pInfo, pBoneToWorld);

	if (F::Chams.m_bRendering)
		return F::Chams.RenderHandler(pState, pInfo, pBoneToWorld);
	if (F::Glow.m_bRendering)
		return F::Glow.RenderHandler(pState, pInfo, pBoneToWorld);

	if (F::Chams.m_mEntities[pInfo.entity_index])
		return;

	auto pEntity = I::ClientEntityList->GetClientEntity(pInfo.entity_index);
	auto pRenderContext = I::MaterialSystem->GetRenderContext();
	if (pEntity && pRenderContext && pEntity->GetClassID() == ETFClassID::CTFViewModel)
	{
		F::Glow.RenderViewmodel(pState, pInfo, pBoneToWorld);
		if (F::Chams.RenderViewmodel(pState, pInfo, pBoneToWorld))
			return;
	}

	CALL_ORIGINAL(rcx, pState, pInfo, pBoneToWorld);
}

static bool s_bDrawingViewmodel = false;

MAKE_HOOK(CBaseAnimating_DrawModel, S::CBaseAnimating_DrawModel(), int,
	void* rcx, int flags)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::IVModelRender_DrawModelExecute[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx, flags);
#endif

	static const auto dwDrawModel = S::CEconEntity_DrawOverriddenViewmodel_DrawModel_Call();
	const auto dwRetAddr = uintptr_t(_ReturnAddress());

	if (dwRetAddr != dwDrawModel || I::EngineVGui->IsGameUIVisible() || Vars::Visuals::UI::CleanScreenshots.Value && I::EngineClient->IsTakingScreenshot()
		|| F::CameraWindow.m_bDrawing || !F::Materials.m_bLoaded || G::Unload)
		return CALL_ORIGINAL(rcx, flags);

	s_bDrawingViewmodel = true;
	int iReturn = CALL_ORIGINAL(rcx, flags);
	s_bDrawingViewmodel = false;
	return iReturn;
}

MAKE_HOOK(CBaseAnimating_InternalDrawModel, S::CBaseAnimating_InternalDrawModel(), int,
	void* rcx, int flags)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::IVModelRender_DrawModelExecute[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx, flags);
#endif

	if (!s_bDrawingViewmodel || !(flags & STUDIO_RENDER))
		return CALL_ORIGINAL(rcx, flags);

	auto pRenderContext = I::MaterialSystem->GetRenderContext();
	if (!pRenderContext)
		return CALL_ORIGINAL(rcx, flags);

	int iReturn;
	F::Glow.RenderViewmodel(rcx, flags);
	if (F::Chams.RenderViewmodel(rcx, flags, &iReturn))
		return iReturn;

	return CALL_ORIGINAL(rcx, 1);
}