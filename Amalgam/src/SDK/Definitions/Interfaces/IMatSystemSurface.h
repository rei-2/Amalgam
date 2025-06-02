#pragma once
#include "Interface.h"
#include "../Misc/ISurface.h"

MAKE_SIGNATURE(CMatSystemSurface_StartDrawing, "vguimatsurface.dll", "40 53 56 57 48 83 EC ? 48 8B F9 80 3D", 0x0);
MAKE_SIGNATURE(CMatSystemSurface_FinishDrawing, "vguimatsurface.dll", "40 53 48 83 EC ? 33 C9", 0x0);

class VMatrix;
class IMaterial;
class ITexture;
struct InputEvent_t;

typedef void (*GetMouseCallback_t)(int& x, int& y);
typedef void (*SetMouseCallback_t)(int x, int y);
typedef void (*PlaySoundFunc_t)(const char* pFileName);

class IMatSystemSurface : public ISurface
{
public:
	virtual void AttachToWindow(void* hwnd, bool bLetAppDriveInput = false) = 0;
	virtual void EnableWindowsMessages(bool bEnable) = 0;
	virtual void Begin3DPaint(int iLeft, int iTop, int iRight, int iBottom, bool bRenderToTexture = true) = 0;
	virtual void End3DPaint() = 0;
	virtual void DisableClipping(bool bDisable) = 0;
	virtual void GetClippingRect(int& left, int& top, int& right, int& bottom, bool& bClippingDisabled) = 0;
	virtual void SetClippingRect(int left, int top, int right, int bottom) = 0;
	virtual bool IsCursorLocked() const = 0;
	virtual void SetMouseCallbacks(GetMouseCallback_t getFunc, SetMouseCallback_t setFunc) = 0;
	virtual void InstallPlaySoundFunc(PlaySoundFunc_t soundFunc) = 0;
	virtual void DrawColoredCircle(int centerx, int centery, float radius, int r, int g, int b, int a) = 0;
	virtual int DrawColoredText(HFont font, int x, int y, int r, int g, int b, int a, const char* fmt, ...) = 0;
	virtual void DrawColoredTextRect(HFont font, int x, int y, int w, int h, int r, int g, int b, int a, const char* fmt, ...) = 0;
	virtual void DrawTextHeight(HFont font, int w, int& h, const char* fmt, ...) = 0;
	virtual int	DrawTextLen(HFont font, const char* fmt, ...) = 0;
	virtual void DrawPanelIn3DSpace(VPANEL pRootPanel, const VMatrix& panelCenterToWorld, int nPixelWidth, int nPixelHeight, float flWorldWidth, float flWorldHeight) = 0;
	virtual void DrawSetTextureMaterial(int id, IMaterial* pMaterial) = 0;
	virtual bool HandleInputEvent(const InputEvent_t& event) = 0;
	virtual void Set3DPaintTempRenderTarget(const char* pRenderTargetName) = 0;
	virtual void Reset3DPaintTempRenderTarget(void) = 0;
	virtual IMaterial* DrawGetTextureMaterial(int id) = 0;
	virtual void GetFullscreenViewportAndRenderTarget(int& x, int& y, int& w, int& h, ITexture** ppRenderTarget) = 0;
	virtual void SetFullscreenViewportAndRenderTarget(int x, int y, int w, int h, ITexture* pRenderTarget) = 0;
	virtual int  DrawGetTextureId(ITexture* pTexture) = 0;
	virtual void BeginSkinCompositionPainting() = 0;
	virtual void EndSkinCompositionPainting() = 0;

	SIGNATURE(StartDrawing, void, CMatSystemSurface, this);
	SIGNATURE(FinishDrawing, void, CMatSystemSurface, this);
};

MAKE_INTERFACE_VERSION(IMatSystemSurface, MatSystemSurface, "vguimatsurface.dll", "VGUI_Surface030");