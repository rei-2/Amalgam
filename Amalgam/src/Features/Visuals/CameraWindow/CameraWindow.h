#pragma once
#include "../../../SDK/SDK.h"

class CCameraWindow
{
private:
	void RenderCustomView(void* ecx, const CViewSetup& pViewSetup, ITexture* pTexture);

	IMaterial* m_pCameraMaterial;
	ITexture* m_pCameraTexture;

public:
	void Draw();
	void RenderView(void* ecx, const CViewSetup& pViewSetup);

	void Initialize();
	void Unload();

	bool m_bDrawing = false;
	bool m_bShouldDraw = false;
	Vec3 m_vCameraOrigin;
	Vec3 m_vCameraAngles;
};

ADD_FEATURE(CCameraWindow, CameraWindow);