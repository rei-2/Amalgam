#pragma once
#include "Interface.h"

class CViewSetup;
class CBaseEntity;
struct vrect_t;
struct WriteReplayScreenshotParams_t;
class IReplayScreenshotSystem;
class ITexture;
class IMaterial;

enum DrawFlags_t
{
	DF_RENDER_REFRACTION = 0x1,
	DF_RENDER_REFLECTION = 0x2,

	DF_CLIP_Z = 0x4,
	DF_CLIP_BELOW = 0x8,

	DF_RENDER_UNDERWATER = 0x10,
	DF_RENDER_ABOVEWATER = 0x20,
	DF_RENDER_WATER = 0x40,

	DF_SSAO_DEPTH_PASS = 0x100,
	DF_WATERHEIGHT = 0x200,
	DF_DRAW_SSAO = 0x400,
	DF_DRAWSKYBOX = 0x800,

	DF_FUDGE_UP = 0x1000,

	DF_DRAW_ENTITITES = 0x2000,
	DF_UNUSED3 = 0x4000,

	DF_UNUSED4 = 0x8000,

	DF_UNUSED5 = 0x10000,
	DF_SAVEGAMESCREENSHOT = 0x20000,
	DF_CLIP_SKYBOX = 0x40000,

	DF_SHADOW_DEPTH_MAP = 0x100000
};

enum ClearFlags_t
{
	VIEW_CLEAR_COLOR = 0x1,
	VIEW_CLEAR_DEPTH = 0x2,
	VIEW_CLEAR_FULL_TARGET = 0x4,
	VIEW_NO_DRAW = 0x8,
	VIEW_CLEAR_OBEY_STENCIL = 0x10, // Draws a quad allowing stencil test to clear through portals
	VIEW_CLEAR_STENCIL = 0x20,
};

enum RenderViewInfo_t
{
	RENDERVIEW_UNSPECIFIED = 0,
	RENDERVIEW_DRAWVIEWMODEL = (1 << 0),
	RENDERVIEW_DRAWHUD = (1 << 1),
	RENDERVIEW_SUPPRESSMONITORRENDERING = (1 << 2),
};

//-----------------------------------------------------------------------------
// Purpose: View setup and rendering
//-----------------------------------------------------------------------------
class IViewRender
{
public:
	// SETUP
	// Initialize view renderer
	virtual void		Init(void) = 0;

	// Clear any systems between levels
	virtual void		LevelInit(void) = 0;
	virtual void		LevelShutdown(void) = 0;

	// Shutdown
	virtual void		Shutdown(void) = 0;

	// RENDERING
	// Called right before simulation. It must setup the view model origins and angles here so 
	// the correct attachment points can be used during simulation.	
	virtual void		OnRenderStart() = 0;

	// Called to render the entire scene
	virtual	void		Render(vrect_t* rect) = 0;

	// Called to render just a particular setup ( for timerefresh and envmap creation )
	virtual void		RenderView(const CViewSetup& view, int nClearFlags, int whatToDraw) = 0;

	// What are we currently rendering? Returns a combination of DF_ flags.
	virtual int GetDrawFlags() = 0;

	// MISC
	// Start and stop pitch drifting logic
	virtual void		StartPitchDrift(void) = 0;
	virtual void		StopPitchDrift(void) = 0;

	// This can only be called during rendering (while within RenderView).
	virtual void* GetFrustum() = 0;

	virtual bool		ShouldDrawBrushModels(void) = 0;

	virtual const CViewSetup* GetPlayerViewSetup(void) const = 0;
	virtual const CViewSetup* GetViewSetup(void) const = 0;

	virtual void		DisableVis(void) = 0;

	virtual int			BuildWorldListsNumber() const = 0;

	virtual void		SetCheapWaterStartDistance(float flCheapWaterStartDistance) = 0;
	virtual void		SetCheapWaterEndDistance(float flCheapWaterEndDistance) = 0;

	virtual void		GetWaterLODParams(float& flCheapWaterStartDistance, float& flCheapWaterEndDistance) = 0;

	virtual void		DriftPitch(void) = 0;

	virtual void		SetScreenOverlayMaterial(IMaterial* pMaterial) = 0;
	virtual IMaterial* GetScreenOverlayMaterial() = 0;

	virtual void		WriteSaveGameScreenshot(const char* pFilename) = 0;
	virtual void		WriteSaveGameScreenshotOfSize(const char* pFilename, int width, int height, bool bCreatePowerOf2Padded = false, bool bWriteVTF = false) = 0;

	virtual void		WriteReplayScreenshot(WriteReplayScreenshotParams_t& params) = 0;
	virtual void		UpdateReplayScreenshotCache() = 0;

	// Draws another rendering over the top of the screen
	virtual void		QueueOverlayRenderView(const CViewSetup& view, int nClearFlags, int whatToDraw) = 0;

	// Returns znear and zfar
	virtual float		GetZNear() = 0;
	virtual float		GetZFar() = 0;

	virtual void		GetScreenFadeDistances(float* min, float* max) = 0;

	virtual CBaseEntity* GetCurrentlyDrawingEntity() = 0;
	virtual void		SetCurrentlyDrawingEntity(CBaseEntity* pEnt) = 0;

	virtual bool		UpdateShadowDepthTexture(ITexture* pRenderTarget, ITexture* pDepthTexture, const CViewSetup& shadowView) = 0;

	virtual void		FreezeFrame(float flFreezeTime) = 0;

	virtual IReplayScreenshotSystem* GetReplayScreenshotSystem() = 0;
};

MAKE_INTERFACE_SIGNATURE(IViewRender, ViewRender, "client.dll", "48 8B 0D ? ? ? ? 48 8B 01 FF 90 ? ? ? ? 48 8B 03 48 8D 54 24", 0x0, 1);