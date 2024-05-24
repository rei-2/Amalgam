#include "CameraWindow.h"

void CCameraWindow::Init()
{
	// Create camera texture
	CameraTex = I::MaterialSystem->CreateNamedRenderTargetTextureEx("mirrorcam_rt", 1, 1, RT_SIZE_FULL_FRAME_BUFFER, IMAGE_FORMAT_RGB888, MATERIAL_RT_DEPTH_SHARED, TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT, CREATERENDERTARGETFLAGS_HDR);

	// Create camera material
	static auto* kv = new KeyValues("UnlitGeneric");
	kv->SetString("$basetexture", "mirrorcam_rt");
	CameraMat = I::MaterialSystem->CreateMaterial("m_cameraMat", kv);
}

// Draws camera to the screen
void CCameraWindow::Draw()
{
	if (!ShouldDraw || !CameraMat || !I::EngineClient->IsInGame())
		return;

	const WindowBox_t& info = Vars::Visuals::Simulation::ProjectileWindow.Value;

	// Draw to screen
	const auto renderCtx = I::MaterialSystem->GetRenderContext();
	renderCtx->DrawScreenSpaceRectangle(
		CameraMat,
		info.x, info.y, info.w, info.h,
		0, 0, info.w, info.h,
		CameraTex->GetActualWidth(), CameraTex->GetActualHeight(),
		nullptr, 1, 1
	);
	renderCtx->Release();
}

// Renders another view onto a texture
void CCameraWindow::RenderView(void* ecx, const CViewSetup& pViewSetup)
{
	if (!ShouldDraw || !CameraTex)
		return;

	const WindowBox_t& info = Vars::Visuals::Simulation::ProjectileWindow.Value;

	CViewSetup viewSetup = pViewSetup;
	viewSetup.x = 0;
	viewSetup.y = 0;

	viewSetup.origin = CameraOrigin;
	viewSetup.angles = CameraAngles;

	viewSetup.width = info.w + 1;
	viewSetup.height = info.h + 1;
	viewSetup.m_flAspectRatio = static_cast<float>(viewSetup.width) / static_cast<float>(viewSetup.height);
	viewSetup.fov = 90;

	RenderCustomView(ecx, viewSetup, CameraTex);
}

void CCameraWindow::RenderCustomView(void* ecx, const CViewSetup& pViewSetup, ITexture* pTexture)
{
	const auto renderCtx = I::MaterialSystem->GetRenderContext();

	renderCtx->PushRenderTargetAndViewport();
	renderCtx->SetRenderTarget(pTexture);

	static auto ViewRender_RenderView = U::Hooks.m_mHooks["ViewRender_RenderView"];
	if (ViewRender_RenderView)
		ViewRender_RenderView->Original<void(__fastcall*)(void*, const CViewSetup&, int, int)>()(ecx, pViewSetup, VIEW_CLEAR_COLOR | VIEW_CLEAR_DEPTH, RENDERVIEW_UNSPECIFIED);

	renderCtx->PopRenderTargetAndViewport();
	renderCtx->Release();
}