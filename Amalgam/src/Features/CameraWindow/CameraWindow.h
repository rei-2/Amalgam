#pragma once
#include "../../SDK/SDK.h"

using RenderViewFN = void(__fastcall*)(void* ecx, const CViewSetup& view, ClearFlags_t nClearFlags, RenderViewInfo_t whatToDraw);

class CCameraWindow
{
public:
	IMaterial* CameraMat;
	ITexture* CameraTex;
	Vec3 CameraOrigin;
	Vec3 CameraAngles;
	bool ShouldDraw = false;

	void Init();
	void Draw();
	void RenderView(void* ecx, const CViewSetup& pViewSetup);
	void RenderCustomView(void* ecx, const CViewSetup& pViewSetup, ITexture* pTexture);
};

ADD_FEATURE(CCameraWindow, CameraWindow)