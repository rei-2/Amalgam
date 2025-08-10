#include "CameraWindow.h"

#include "../../Visuals/Materials/Materials.h"

// Draws camera to the screen
void CCameraWindow::Draw()
{
	if (!m_pCameraMaterial || !m_bShouldDraw || !I::EngineClient->IsInGame())
		return;

	auto& tWindowBox = Vars::Visuals::Simulation::ProjectileWindow.Value;

	// Draw to screen
	auto pRenderContext = I::MaterialSystem->GetRenderContext();
	pRenderContext->DrawScreenSpaceRectangle(
		m_pCameraMaterial,
		tWindowBox.x - tWindowBox.w / 2, tWindowBox.y, tWindowBox.w, tWindowBox.h,
		0, 0, tWindowBox.w, tWindowBox.h,
		m_pCameraTexture->GetActualWidth(), m_pCameraTexture->GetActualHeight(),
		nullptr, 1, 1
	);
	pRenderContext->Release();
}

// Renders another view onto a texture
void CCameraWindow::RenderView(void* ecx, const CViewSetup& pViewSetup)
{
	if (!m_bShouldDraw || !m_pCameraTexture)
		return;

	m_bDrawing = true;

	auto& tWindowBox = Vars::Visuals::Simulation::ProjectileWindow.Value;

	CViewSetup tViewSetup = pViewSetup;
	tViewSetup.x = 0;
	tViewSetup.y = 0;

	tViewSetup.origin = m_vCameraOrigin;
	tViewSetup.angles = m_vCameraAngles;

	tViewSetup.width = tWindowBox.w + 1;
	tViewSetup.height = tWindowBox.h + 1;
	tViewSetup.m_flAspectRatio = float(tViewSetup.width) / float(tViewSetup.height);
	tViewSetup.fov = 90;

	RenderCustomView(ecx, tViewSetup, m_pCameraTexture);

	m_bDrawing = false;
}

void CCameraWindow::RenderCustomView(void* ecx, const CViewSetup& pViewSetup, ITexture* pTexture)
{
	auto pRenderContext = I::MaterialSystem->GetRenderContext();

	pRenderContext->PushRenderTargetAndViewport();
	pRenderContext->SetRenderTarget(pTexture);

	static auto CViewRender_RenderView = U::Hooks.m_mHooks["CViewRender_RenderView"];
	CViewRender_RenderView->Call<void>(ecx, pViewSetup, VIEW_CLEAR_COLOR | VIEW_CLEAR_DEPTH, RENDERVIEW_UNSPECIFIED);

	pRenderContext->PopRenderTargetAndViewport();
	pRenderContext->Release();
}

void CCameraWindow::Initialize()
{
	if (!m_pCameraMaterial)
	{
		KeyValues* kv = new KeyValues("UnlitGeneric");
		kv->SetString("$basetexture", "m_pCameraTexture");
		m_pCameraMaterial = F::Materials.Create("CameraMaterial", kv);
	}

	if (!m_pCameraTexture)
	{
		m_pCameraTexture = I::MaterialSystem->CreateNamedRenderTargetTextureEx(
			"m_pCameraTexture",
			1,
			1,
			RT_SIZE_FULL_FRAME_BUFFER,
			IMAGE_FORMAT_RGB888,
			MATERIAL_RT_DEPTH_SHARED,
			TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT,
			CREATERENDERTARGETFLAGS_HDR
		);
		m_pCameraTexture->IncrementReferenceCount();
	}
}

void CCameraWindow::Unload()
{
	if (m_pCameraMaterial)
	{
		m_pCameraMaterial->DecrementReferenceCount();
		m_pCameraMaterial->DeleteIfUnreferenced();
		m_pCameraMaterial = nullptr;
	}

	if (m_pCameraTexture)
	{
		m_pCameraTexture->DecrementReferenceCount();
		m_pCameraTexture->DeleteIfUnreferenced();
		m_pCameraTexture = nullptr;
	}
}