#pragma once
#include "IMaterialSystem.h"
#include "Interface.h"
#include "../Misc/BaseTypes.h"
#include "../Misc/IRefCounted.h"
#include "../Misc/VPlane.h"
#include "../Definitions.h"

class IClientRenderable;
class IClientEntity;
class CViewSetup;
struct model_t;

enum
{
	FRUSTUM_RIGHT = 0,
	FRUSTUM_LEFT = 1,
	FRUSTUM_TOP = 2,
	FRUSTUM_BOTTOM = 3,
	FRUSTUM_NEARZ = 4,
	FRUSTUM_FARZ = 5,
	FRUSTUM_NUMPLANES = 6
};

enum
{
	DRAWWORLDLISTS_DRAW_STRICTLYABOVEWATER = 0x001,
	DRAWWORLDLISTS_DRAW_STRICTLYUNDERWATER = 0x002,
	DRAWWORLDLISTS_DRAW_INTERSECTSWATER = 0x004,
	DRAWWORLDLISTS_DRAW_WATERSURFACE = 0x008,
	DRAWWORLDLISTS_DRAW_SKYBOX = 0x010,
	DRAWWORLDLISTS_DRAW_CLIPSKYBOX = 0x020,
	DRAWWORLDLISTS_DRAW_SHADOWDEPTH = 0x040,
	DRAWWORLDLISTS_DRAW_REFRACTION = 0x080,
	DRAWWORLDLISTS_DRAW_REFLECTION = 0x100,
	DRAWWORLDLISTS_DRAW_SSAO = 0x800
};

enum
{
	MAT_SORT_GROUP_STRICTLY_ABOVEWATER = 0,
	MAT_SORT_GROUP_STRICTLY_UNDERWATER,
	MAT_SORT_GROUP_INTERSECTS_WATER_SURFACE,
	MAT_SORT_GROUP_WATERSURFACE,
	MAX_MAT_SORT_GROUPS
};

enum ERenderDepthMode
{
	DEPTH_MODE_NORMAL = 0,
	DEPTH_MODE_SHADOW = 1,
	DEPTH_MODE_SSA0 = 2,
	DEPTH_MODE_OVERRIDE = 3,
	DEPTH_MODE_MAX
};

typedef VPlane Frustum[FRUSTUM_NUMPLANES];

typedef unsigned short LeafIndex_t;
typedef short LeafFogVolume_t;

enum
{
	INVALID_LEAF_INDEX = (LeafIndex_t)~0
};

struct WorldListInfo_t
{
	int m_ViewFogVolume;
	int m_LeafCount;
	LeafIndex_t* m_pLeafList;
	LeafFogVolume_t* m_pLeafFogVolume;
};

class IWorldRenderList : public IRefCounted
{
};

struct VisibleFogVolumeInfo_t
{
	int m_nVisibleFogVolume;
	int m_nVisibleFogVolumeLeaf;
	bool m_bEyeInFogVolume;
	float m_flDistanceToWater;
	float m_flWaterHeight;
	IMaterial* m_pFogVolumeMaterial;
};

struct BrushVertex_t
{
	Vector m_Pos;
	Vector m_Normal;
	Vector m_TangentS;
	Vector m_TangentT;
	Vector2D m_TexCoord;
	Vector2D m_LightmapCoord;
};

struct VisOverrideData_t
{
	Vector m_vecVisOrigin;
	float m_fDistToAreaPortalTolerance;
};

class IBrushSurface
{
public:
	virtual void ComputeTextureCoordinate(Vector const& worldPos, Vector2D& texCoord) = 0;
	virtual void ComputeLightmapCoordinate(Vector const& worldPos, Vector2D& lightmapCoord) = 0;
	virtual int  GetVertexCount() const = 0;
	virtual void GetVertexData(BrushVertex_t* pVerts) = 0;
	virtual IMaterial* GetMaterial() = 0;
};

class IBrushRenderer
{
public:
	virtual bool RenderBrushModelSurface(IClientEntity* pBaseEntity, IBrushSurface* pBrushSurface) = 0;
};

#define MAX_VIS_LEAVES	32

enum DrawBrushModelMode_t
{
	DBM_DRAW_ALL = 0,
	DBM_DRAW_OPAQUE_ONLY,
	DBM_DRAW_TRANSLUCENT_ONLY
};

enum
{
	VIEW_SETUP_VIS_EX_RETURN_FLAGS_USES_RADIAL_VIS = 0x00000001
};

class IVRenderView
{
public:
	virtual void DrawBrushModel(IClientEntity* baseentity, model_t* model, const Vector& origin, const QAngle& angles, bool bUnused) = 0;
	virtual void DrawIdentityBrushModel(IWorldRenderList* pList, model_t* model) = 0;
	virtual void TouchLight(struct dlight_t* light) = 0;
	virtual void Draw3DDebugOverlays(void) = 0;
	virtual void SetBlend(float blend) = 0;
	virtual float GetBlend(void) = 0;
	virtual void SetColorModulation(float const* blend) = 0;
	virtual void GetColorModulation(float* blend) = 0;
	virtual void SceneBegin(void) = 0;
	virtual void SceneEnd(void) = 0;
	virtual void GetVisibleFogVolume(const Vector& eyePoint, VisibleFogVolumeInfo_t* pInfo) = 0;
	virtual IWorldRenderList* CreateWorldList() = 0;
	virtual void BuildWorldLists(IWorldRenderList* pList, WorldListInfo_t* pInfo, int iForceFViewLeaf, const VisOverrideData_t* pVisData = NULL, bool bShadowDepth = false, float* pReflectionWaterHeight = NULL) = 0;
	virtual void DrawWorldLists(IWorldRenderList* pList, unsigned long flags, float waterZAdjust) = 0;
	virtual void DrawTopView(bool enable) = 0;
	virtual void TopViewBounds(Vector2D const& mins, Vector2D const& maxs) = 0;
	virtual void DrawLights(void) = 0;
	virtual void DrawMaskEntities(void) = 0;
	virtual void DrawTranslucentSurfaces(IWorldRenderList* pList, int sortIndex, unsigned long flags, bool bShadowDepth) = 0;
	virtual void DrawLineFile(void) = 0;
	virtual void DrawLightmaps(IWorldRenderList* pList, int pageId) = 0;
	virtual void ViewSetupVis(bool novis, int numorigins, const Vector origin[]) = 0;
	virtual bool AreAnyLeavesVisible(int* leafList, int nLeaves) = 0;
	virtual	void VguiPaint(void) = 0;
	virtual void ViewDrawFade(byte* color, IMaterial* pMaterial) = 0;
	virtual void OLD_SetProjectionMatrix(float fov, float zNear, float zFar) = 0;
	virtual colorVec GetLightAtPoint(Vector& pos) = 0;
	virtual int GetViewEntity(void) = 0;
	virtual float GetFieldOfView(void) = 0;
	virtual unsigned char** GetAreaBits(void) = 0;
	virtual void SetFogVolumeState(int nVisibleFogVolume, bool bUseHeightFog) = 0;
	virtual void InstallBrushSurfaceRenderer(IBrushRenderer* pBrushRenderer) = 0;
	virtual void DrawBrushModelShadow(IClientRenderable* pRenderable) = 0;
	virtual	bool LeafContainsTranslucentSurfaces(IWorldRenderList* pList, int sortIndex, unsigned long flags) = 0;
	virtual bool DoesBoxIntersectWaterVolume(const Vector& mins, const Vector& maxs, int leafWaterDataID) = 0;
	virtual void SetAreaState(unsigned char chAreaBits[MAX_AREA_STATE_BYTES], unsigned char chAreaPortalBits[MAX_AREA_PORTAL_STATE_BYTES]) = 0;
	virtual void VGui_Paint(int mode) = 0;
	virtual void Push3DView(const CViewSetup& view, int nFlags, ITexture* pRenderTarget, Frustum frustumPlanes) = 0;
	virtual void Push2DView(const CViewSetup& view, int nFlags, ITexture* pRenderTarget, Frustum frustumPlanes) = 0;
	virtual void PopView(Frustum frustumPlanes) = 0;
	virtual void SetMainView(const Vector& vecOrigin, const QAngle& angles) = 0;
	virtual void ViewSetupVisEx(bool novis, int numorigins, const Vector origin[], unsigned int& returnFlags) = 0;
	virtual void OverrideViewFrustum(Frustum custom) = 0;
	virtual void DrawBrushModelShadowDepth(IClientEntity* baseentity, model_t* model, const Vector& origin, const QAngle& angles, ERenderDepthMode DepthMode) = 0;
	virtual void UpdateBrushModelLightmap(model_t* model, IClientRenderable* pRenderable) = 0;
	virtual void BeginUpdateLightmaps(void) = 0;
	virtual void EndUpdateLightmaps(void) = 0;
	virtual void OLD_SetOffCenterProjectionMatrix(float fov, float zNear, float zFar, float flAspectRatio, float flBottom, float flTop, float flLeft, float flRight) = 0;
	virtual void OLD_SetProjectionMatrixOrtho(float left, float top, float right, float bottom, float zNear, float zFar) = 0;
	virtual void Push3DView(const CViewSetup& view, int nFlags, ITexture* pRenderTarget, Frustum frustumPlanes, ITexture* pDepthTexture) = 0;
	virtual void GetMatricesForView(const CViewSetup& view, VMatrix* pWorldToView, VMatrix* pViewToProjection, VMatrix* pWorldToProjection, VMatrix* pWorldToPixels) = 0;
	virtual void DrawBrushModelEx(IClientEntity* baseentity, model_t* model, const Vector& origin, const QAngle& angles, DrawBrushModelMode_t mode) = 0;

	void SetColorModulation(float r, float g, float b)
	{
		float blend[3] = { r, g, b };
		SetColorModulation(blend);
	}

	void SetColorModulation(Color_t clr)
	{
		float blend[3] = { clr.r / 255.f, clr.g / 255.f, clr.b / 255.f };
		SetColorModulation(blend);
	}
};

MAKE_INTERFACE_VERSION(IVRenderView, RenderView, "engine.dll", "VEngineRenderView014");