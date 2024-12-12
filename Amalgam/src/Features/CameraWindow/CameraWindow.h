#pragma once
#include "../../SDK/SDK.h"

class CCameraWindow
{
public:
	void Initialize();
	void Unload();

	void Draw();
	void RenderView(void* ecx, const CViewSetup& pViewSetup);
	void RenderCustomView(void* ecx, const CViewSetup& pViewSetup, ITexture* pTexture);

	IMaterial* m_pCameraMaterial;
	ITexture* m_pCameraTexture;
	Vec3 m_vCameraOrigin;
	Vec3 m_vCameraAngles;
	bool m_bShouldDraw = false;
	bool m_bDrawing = false;
};

ADD_FEATURE(CCameraWindow, CameraWindow)