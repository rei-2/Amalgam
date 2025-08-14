#include "Glow.h"

#include "../Groups/Groups.h"
#include "../Materials/Materials.h"
#include "../FakeAngle/FakeAngle.h"
#include "../../Backtrack/Backtrack.h"

void CGlow::SetupBegin(IMatRenderContext* pRenderContext)
{
	m_tOriginalColor = I::RenderView->GetColorModulation();
	m_flOriginalBlend = I::RenderView->GetBlend();
	I::ModelRender->GetMaterialOverride(&m_pOriginalMaterial, &m_iOriginalOverride);

	I::RenderView->SetBlend(0.f);
	I::RenderView->SetColorModulation(1.f, 1.f, 1.f);
	I::ModelRender->ForcedMaterialOverride(m_pMatGlowColor);

	pRenderContext->SetStencilEnable(true);
	pRenderContext->SetStencilCompareFunction(STENCILCOMPARISONFUNCTION_ALWAYS);
	pRenderContext->SetStencilPassOperation(STENCILOPERATION_REPLACE);
	pRenderContext->SetStencilFailOperation(STENCILOPERATION_KEEP);
	pRenderContext->SetStencilZFailOperation(STENCILOPERATION_REPLACE);
	pRenderContext->SetStencilReferenceValue(1);
	pRenderContext->SetStencilWriteMask(0xFF);
	pRenderContext->SetStencilTestMask(0x0);
}
void CGlow::SetupMid(IMatRenderContext* pRenderContext, int w, int h)
{
	pRenderContext->SetStencilEnable(false);

	pRenderContext->PushRenderTargetAndViewport();
	pRenderContext->SetRenderTarget(m_pRenderBuffer1);
	pRenderContext->Viewport(0, 0, w, h);
	pRenderContext->ClearColor4ub(0, 0, 0, 0);
	pRenderContext->ClearBuffers(true, false, false);
}
void CGlow::SetupEnd(Glow_t tGlow, IMatRenderContext* pRenderContext, int w, int h)
{
	pRenderContext->PopRenderTargetAndViewport();

	if (tGlow.Blur)
	{
		m_pBloomAmount->SetFloatValue(tGlow.Blur);

		pRenderContext->PushRenderTargetAndViewport();
		{
			pRenderContext->Viewport(0, 0, w, h);
			pRenderContext->SetRenderTarget(m_pRenderBuffer2);
			pRenderContext->DrawScreenSpaceRectangle(m_pMatBlurX, 0, 0, w, h, 0.f, 0.f, w - 1, h - 1, w, h);
			pRenderContext->SetRenderTarget(m_pRenderBuffer1);
			pRenderContext->DrawScreenSpaceRectangle(m_pMatBlurY, 0, 0, w, h, 0.f, 0.f, w - 1, h - 1, w, h);
		}
		pRenderContext->PopRenderTargetAndViewport();
	}

	pRenderContext->SetStencilEnable(true);
	pRenderContext->SetStencilCompareFunction(STENCILCOMPARISONFUNCTION_EQUAL);
	pRenderContext->SetStencilPassOperation(STENCILOPERATION_KEEP);
	pRenderContext->SetStencilFailOperation(STENCILOPERATION_KEEP);
	pRenderContext->SetStencilZFailOperation(STENCILOPERATION_KEEP);
	pRenderContext->SetStencilReferenceValue(0);
	pRenderContext->SetStencilWriteMask(0x0);
	pRenderContext->SetStencilTestMask(0xFF);

	if (tGlow.Stencil)
	{
		int iSide = (tGlow.Stencil + 1) / 2.f;
		pRenderContext->DrawScreenSpaceRectangle(m_pMatHaloAddToScreen, -iSide, 0, w, h, 0.f, 0.f, w - 1, h - 1, w, h);
		pRenderContext->DrawScreenSpaceRectangle(m_pMatHaloAddToScreen, 0, -iSide, w, h, 0.f, 0.f, w - 1, h - 1, w, h);
		pRenderContext->DrawScreenSpaceRectangle(m_pMatHaloAddToScreen, iSide, 0, w, h, 0.f, 0.f, w - 1, h - 1, w, h);
		pRenderContext->DrawScreenSpaceRectangle(m_pMatHaloAddToScreen, 0, iSide, w, h, 0.f, 0.f, w - 1, h - 1, w, h);
		if (int iCorner = tGlow.Stencil / 2.f)
		{
			pRenderContext->DrawScreenSpaceRectangle(m_pMatHaloAddToScreen, -iCorner, -iCorner, w, h, 0.f, 0.f, w - 1, h - 1, w, h);
			pRenderContext->DrawScreenSpaceRectangle(m_pMatHaloAddToScreen, iCorner, iCorner, w, h, 0.f, 0.f, w - 1, h - 1, w, h);
			pRenderContext->DrawScreenSpaceRectangle(m_pMatHaloAddToScreen, iCorner, -iCorner, w, h, 0.f, 0.f, w - 1, h - 1, w, h);
			pRenderContext->DrawScreenSpaceRectangle(m_pMatHaloAddToScreen, -iCorner, iCorner, w, h, 0.f, 0.f, w - 1, h - 1, w, h);
		}
	}
	if (tGlow.Blur)
		pRenderContext->DrawScreenSpaceRectangle(m_pMatHaloAddToScreen, 0, 0, w, h, 0.f, 0.f, w - 1, h - 1, w, h);

	pRenderContext->SetStencilEnable(false);

	I::RenderView->SetColorModulation(m_tOriginalColor);
	I::RenderView->SetBlend(m_flOriginalBlend);
	I::ModelRender->ForcedMaterialOverride(m_pOriginalMaterial, m_iOriginalOverride);
}

void CGlow::DrawModel(CBaseEntity* pEntity)
{
	m_bRendering = true;

	if (pEntity->IsPlayer())
	{
		auto pPlayer = pEntity->As<CTFPlayer>();
		float flOldInvisibility = pPlayer->m_flInvisibility();
		pPlayer->m_flInvisibility() = 0.f;
		pEntity->DrawModel(STUDIO_RENDER | STUDIO_NOSHADOWS);
		pPlayer->m_flInvisibility() = flOldInvisibility;
	}
	else
		pEntity->DrawModel(STUDIO_RENDER | STUDIO_NOSHADOWS);

	m_bRendering = false;
}



void CGlow::Store(CTFPlayer* pLocal)
{
	m_mEntities.clear();
	if (!pLocal || !F::Groups.GroupsActive())
		return;

	for (auto& [pEntity, pGroup] : F::Groups.GetGroup())
	{
		if (pEntity->IsDormant() || !pEntity->ShouldDraw())
			continue;

		Color_t tColor = F::Groups.GetColor(pEntity, pGroup);
		if (pGroup->m_tGlow()
			&& SDK::IsOnScreen(pEntity, pEntity->IsBaseCombatWeapon() || pEntity->IsWearable()))
			m_mEntities[pGroup->m_tGlow].emplace_back(pEntity, tColor);

		if (pEntity->IsPlayer() && pEntity != pLocal && pGroup->m_bBacktrack && pGroup->m_tBacktrackGlow()
			&& (F::Backtrack.GetFakeLatency() || F::Backtrack.GetFakeInterp() > G::Lerp || F::Backtrack.GetWindow()))
		{
			auto pWeapon = H::Entities.GetWeapon();
			if (pWeapon && (pGroup->m_iBacktrackDraw & BacktrackEnum::Always || G::PrimaryWeaponType != EWeaponType::PROJECTILE))
			{
				bool bShowFriendly = false, bShowEnemy = true;
				if (G::PrimaryWeaponType == EWeaponType::MELEE && SDK::AttribHookValue(0, "speed_buff_ally", pWeapon) > 0)
					bShowFriendly = true;
				else if (pWeapon->GetWeaponID() == TF_WEAPON_MEDIGUN)
					bShowFriendly = true, bShowEnemy = false;

				if (bShowEnemy && pEntity->m_iTeamNum() != pLocal->m_iTeamNum() || bShowFriendly && pEntity->m_iTeamNum() == pLocal->m_iTeamNum())
					m_mEntities[pGroup->m_tBacktrackGlow].emplace_back(pEntity, tColor, 1 | (pGroup->m_iBacktrackDraw << 1));
			}
		}
	}

	Group_t* pGroup = nullptr;
	if (F::FakeAngle.bDrawChams && F::FakeAngle.bBonesSetup
		&& F::Groups.GetGroup(TargetsEnum::FakeAngle, pGroup) && pGroup->m_tGlow())
	{	// fakeangle
		m_mEntities[pGroup->m_tGlow].emplace_back(pLocal, pGroup->m_tColor, 1 | (true << 1));
	}
}

void CGlow::RenderMain()
{
	const int w = H::Draw.m_nScreenW, h = H::Draw.m_nScreenH;
	if (w < 1 || h < 1 || w > 4096 || h > 2160)
		return;

	auto pRenderContext = I::MaterialSystem->GetRenderContext();
	if (!pRenderContext || !m_pMatGlowColor || !m_pMatBlurX || !m_pMatBlurY || !m_pMatHaloAddToScreen)
		return F::Materials.ReloadMaterials();

	for (auto& [tGlow, vInfo] : m_mEntities)
	{
		SetupBegin(pRenderContext);
		for (auto& tInfo : vInfo)
		{
			m_iFlags = tInfo.m_iFlags;
			DrawModel(tInfo.m_pEntity);
			m_iFlags = false;
		}
		SetupMid(pRenderContext, w, h);
		for (auto& tInfo : vInfo)
		{
			I::RenderView->SetColorModulation(tInfo.m_cColor);
			I::RenderView->SetBlend(tInfo.m_cColor.a / 255.f);

			m_iFlags = tInfo.m_iFlags;
			DrawModel(tInfo.m_pEntity);
			m_iFlags = false;
		}
		SetupEnd(tGlow, pRenderContext, w, h);
	}
}

void CGlow::RenderBacktrack(const DrawModelState_t& pState, const ModelRenderInfo_t& pInfo)
{
	auto pEntity = I::ClientEntityList->GetClientEntity(pInfo.entity_index)->As<CTFPlayer>();
	if (!pEntity || !pEntity->IsPlayer())
		return;



	std::vector<TickRecord*> vRecords = {};
	if (!F::Backtrack.GetRecords(pEntity, vRecords))
		return;
	vRecords = F::Backtrack.GetValidRecords(vRecords);
	if (!vRecords.size())
		return;

	int iFlags = (~1 & m_iFlags) >> 1;
	bool bDrawLast = iFlags & BacktrackEnum::Last;
	bool bDrawFirst = iFlags & BacktrackEnum::First;

	auto drawModel = [&](Vec3& vOrigin, const DrawModelState_t& pState, const ModelRenderInfo_t& pInfo, matrix3x4* pBoneToWorld, float flBlend)
		{
			if (!SDK::IsOnScreen(pEntity, vOrigin))
				return;

			//float flOriginalBlend = I::RenderView->GetBlend();
			//I::RenderView->SetBlend(flBlend * flOriginalBlend);
			static auto IVModelRender_DrawModelExecute = U::Hooks.m_mHooks["IVModelRender_DrawModelExecute"];
			IVModelRender_DrawModelExecute->Call<void>(I::ModelRender, pState, pInfo, pBoneToWorld);
			//I::RenderView->SetBlend(flOriginalBlend);
		};
	if (!bDrawLast && !bDrawFirst)
	{
		for (auto pRecord : vRecords)
		{
			if (float flBlend = Math::RemapVal(pEntity->GetAbsOrigin().DistTo(pRecord->m_vOrigin), 1.f, 24.f, 0.f, 1.f))
				drawModel(pRecord->m_vOrigin, pState, pInfo, pRecord->m_aBones, flBlend);
		}
	}
	else
	{
		if (bDrawLast)
		{
			auto pRecord = vRecords.back();
			if (float flBlend = Math::RemapVal(pEntity->GetAbsOrigin().DistTo(pRecord->m_vOrigin), 1.f, 24.f, 0.f, 1.f))
				drawModel(pRecord->m_vOrigin, pState, pInfo, pRecord->m_aBones, flBlend);
		}
		if (bDrawFirst)
		{
			auto pRecord = vRecords.front();
			if (float flBlend = Math::RemapVal(pEntity->GetAbsOrigin().DistTo(pRecord->m_vOrigin), 1.f, 24.f, 0.f, 1.f))
				drawModel(pRecord->m_vOrigin, pState, pInfo, pRecord->m_aBones, flBlend);
		}
	}
}
void CGlow::RenderFakeAngle(const DrawModelState_t& pState, const ModelRenderInfo_t& pInfo)
{
	static auto IVModelRender_DrawModelExecute = U::Hooks.m_mHooks["IVModelRender_DrawModelExecute"];
	IVModelRender_DrawModelExecute->Call<void>(I::ModelRender, pState, pInfo, F::FakeAngle.aBones);
}
void CGlow::RenderHandler(const DrawModelState_t& pState, const ModelRenderInfo_t& pInfo, matrix3x4* pBoneToWorld)
{
	if (!m_iFlags)
	{
		static auto IVModelRender_DrawModelExecute = U::Hooks.m_mHooks["IVModelRender_DrawModelExecute"];
		IVModelRender_DrawModelExecute->Call<void>(I::ModelRender, pState, pInfo, pBoneToWorld);
	}
	else
	{
		if (pInfo.entity_index != I::EngineClient->GetLocalPlayer())
			RenderBacktrack(pState, pInfo);
		else
			RenderFakeAngle(pState, pInfo);
	}
}

void CGlow::RenderViewmodel(void* ecx, int flags)
{
	if (!F::Groups.GroupsActive())
		return;

	const int w = H::Draw.m_nScreenW, h = H::Draw.m_nScreenH;
	if (w < 1 || h < 1 || w > 4096 || h > 2160)
		return;

	auto pRenderContext = I::MaterialSystem->GetRenderContext();
	if (!pRenderContext || !m_pMatGlowColor || !m_pMatBlurX || !m_pMatBlurY || !m_pMatHaloAddToScreen)
		return F::Materials.ReloadMaterials();



	Group_t* pGroup = nullptr;
	if (!F::Groups.GetGroup(TargetsEnum::ViewmodelWeapon, pGroup) || !pGroup->m_tGlow())
		return;

	static auto CBaseAnimating_InternalDrawModel = U::Hooks.m_mHooks["CBaseAnimating_InternalDrawModel"];

	pRenderContext->CullMode(MATERIAL_CULLMODE_CCW); // glow won't work properly with MATERIAL_CULLMODE_CW
	SetupBegin(pRenderContext);
	CBaseAnimating_InternalDrawModel->Call<int>(ecx, flags);
	SetupMid(pRenderContext, w, h);
	I::RenderView->SetColorModulation(pGroup->m_tColor);
	I::RenderView->SetBlend(pGroup->m_tColor.a / 255.f);
	CBaseAnimating_InternalDrawModel->Call<int>(ecx, flags);
	SetupEnd(pGroup->m_tGlow, pRenderContext, w, h);

	pRenderContext->CullMode(G::FlipViewmodels ? MATERIAL_CULLMODE_CW : MATERIAL_CULLMODE_CCW);
}
void CGlow::RenderViewmodel(const DrawModelState_t& pState, const ModelRenderInfo_t& pInfo, matrix3x4* pBoneToWorld)
{
	if (!F::Groups.GroupsActive())
		return;

	const int w = H::Draw.m_nScreenW, h = H::Draw.m_nScreenH;
	if (w < 1 || h < 1 || w > 4096 || h > 2160)
		return;

	auto pRenderContext = I::MaterialSystem->GetRenderContext();
	if (!pRenderContext || !m_pMatGlowColor || !m_pMatBlurX || !m_pMatBlurY || !m_pMatHaloAddToScreen)
		return F::Materials.ReloadMaterials();



	Group_t* pGroup = nullptr;
	if (!F::Groups.GetGroup(TargetsEnum::ViewmodelHands, pGroup) || !pGroup->m_tGlow())
		return;

	static auto IVModelRender_DrawModelExecute = U::Hooks.m_mHooks["IVModelRender_DrawModelExecute"];

	pRenderContext->CullMode(MATERIAL_CULLMODE_CCW); // glow won't work properly with MATERIAL_CULLMODE_CW
	SetupBegin(pRenderContext);
	IVModelRender_DrawModelExecute->Call<void>(I::ModelRender, pState, pInfo, pBoneToWorld);
	SetupMid(pRenderContext, w, h);
	I::RenderView->SetColorModulation(pGroup->m_tColor);
	I::RenderView->SetBlend(pGroup->m_tColor.a / 255.f);
	IVModelRender_DrawModelExecute->Call<void>(I::ModelRender, pState, pInfo, pBoneToWorld);
	SetupEnd(pGroup->m_tGlow, pRenderContext, w, h);

	pRenderContext->CullMode(G::FlipViewmodels ? MATERIAL_CULLMODE_CW : MATERIAL_CULLMODE_CCW);
}



void CGlow::Initialize()
{
	int nWidth, nHeight; I::MatSystemSurface->GetScreenSize(nWidth, nHeight);

	if (!m_pMatGlowColor)
	{
		m_pMatGlowColor = I::MaterialSystem->FindMaterial("dev/glow_color", TEXTURE_GROUP_OTHER);
		m_pMatGlowColor->IncrementReferenceCount();
		F::Materials.m_mMatList[m_pMatGlowColor] = true;
	}

	if (!m_pRenderBuffer1)
	{
		m_pRenderBuffer1 = I::MaterialSystem->CreateNamedRenderTargetTextureEx(
			"RenderBuffer1",
			nWidth, nHeight,
			RT_SIZE_LITERAL,
			IMAGE_FORMAT_RGB888,
			MATERIAL_RT_DEPTH_SHARED,
			TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT | TEXTUREFLAGS_EIGHTBITALPHA,
			CREATERENDERTARGETFLAGS_HDR
		);
		m_pRenderBuffer1->IncrementReferenceCount();
	}

	if (!m_pRenderBuffer2)
	{
		m_pRenderBuffer2 = I::MaterialSystem->CreateNamedRenderTargetTextureEx(
			"RenderBuffer2",
			nWidth, nHeight,
			RT_SIZE_LITERAL,
			IMAGE_FORMAT_RGB888,
			MATERIAL_RT_DEPTH_SHARED,
			TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT | TEXTUREFLAGS_EIGHTBITALPHA,
			CREATERENDERTARGETFLAGS_HDR
		);
		m_pRenderBuffer2->IncrementReferenceCount();
	}

	if (!m_pMatHaloAddToScreen)
	{
		KeyValues* kv = new KeyValues("UnlitGeneric");
		kv->SetString("$basetexture", "RenderBuffer1");
		kv->SetString("$additive", "1");
		m_pMatHaloAddToScreen = F::Materials.Create("MatHaloAddToScreen", kv);
	}

	if (!m_pMatBlurX)
	{
		KeyValues* kv = new KeyValues("BlurFilterX");
		kv->SetString("$basetexture", "RenderBuffer1");
		m_pMatBlurX = F::Materials.Create("MatBlurX", kv);
	}

	if (!m_pMatBlurY)
	{
		KeyValues* kv = new KeyValues("BlurFilterY");
		kv->SetString("$basetexture", "RenderBuffer2");
		m_pMatBlurY = F::Materials.Create("MatBlurY", kv);
		m_pBloomAmount = m_pMatBlurY->FindVar("$bloomamount", nullptr);
	}
}

void CGlow::Unload()
{
	if (m_pMatGlowColor)
	{
		m_pMatGlowColor->DecrementReferenceCount();
		m_pMatGlowColor->DeleteIfUnreferenced();
		m_pMatGlowColor = nullptr;
	}

	if (m_pMatBlurX)
	{
		m_pMatBlurX->DecrementReferenceCount();
		m_pMatBlurX->DeleteIfUnreferenced();
		m_pMatBlurX = nullptr;
	}

	if (m_pMatBlurY)
	{
		m_pMatBlurY->DecrementReferenceCount();
		m_pMatBlurY->DeleteIfUnreferenced();
		m_pMatBlurY = nullptr;
	}

	if (m_pMatHaloAddToScreen)
	{
		m_pMatHaloAddToScreen->DecrementReferenceCount();
		m_pMatHaloAddToScreen->DeleteIfUnreferenced();
		m_pMatHaloAddToScreen = nullptr;
	}

	if (m_pRenderBuffer1)
	{
		m_pRenderBuffer1->DecrementReferenceCount();
		m_pRenderBuffer1->DeleteIfUnreferenced();
		m_pRenderBuffer1 = nullptr;
	}

	if (m_pRenderBuffer2)
	{
		m_pRenderBuffer2->DecrementReferenceCount();
		m_pRenderBuffer2->DeleteIfUnreferenced();
		m_pRenderBuffer2 = nullptr;
	}
}