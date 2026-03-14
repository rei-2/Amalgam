#pragma once
#include "../Main/CModel.h"

class SurfInfo;
struct decal_t;
typedef unsigned short DispDecalHandle_t;
typedef unsigned short DispShadowHandle_t;
typedef unsigned short ShadowHandle_t;

class GetIntersectingSurfaces_Struct
{
public:
	model_t* m_pModel;
	const Vector* m_pCenter;
	const byte* m_pCenterPVS;
	float m_Radius;
	bool m_bOnlyVisible;
	SurfInfo* m_pInfos;
	int m_nMaxInfos;
	int m_nSetInfos;
};

struct RayDispOutput_t
{
	short ndxVerts[4];
	float u, v;
	float dist;
};

class IDispInfo
{
public:
	virtual ~IDispInfo() = 0;
	virtual void GetIntersectingSurfaces(GetIntersectingSurfaces_Struct* pStruct) = 0;
	virtual void RenderWireframeInLightmapPage(int pageId) = 0;
	virtual void GetBoundingBox(Vector& bbMin, Vector& bbMax) = 0;
	virtual void SetParent(SurfaceHandle_t surfID) = 0;
	virtual SurfaceHandle_t GetParent() = 0;
	virtual void AddDynamicLights(struct dlight_t* pLights, unsigned int lightMask) = 0;
	virtual unsigned int ComputeDynamicLightMask(struct dlight_t* pLights) = 0;
	virtual DispDecalHandle_t NotifyAddDecal(decal_t* pDecal, float flSize) = 0;
	virtual void NotifyRemoveDecal(DispDecalHandle_t h) = 0;
	virtual DispShadowHandle_t AddShadowDecal(ShadowHandle_t shadowHandle) = 0;
	virtual void RemoveShadowDecal(DispShadowHandle_t handle) = 0;
	virtual bool ComputeShadowFragments(DispShadowHandle_t h, int& vertexCount, int& indexCount) = 0;
	virtual bool GetTag() = 0;
	virtual void SetTag() = 0;
	virtual	bool TestRay(Ray_t const& ray, float start, float end, float& dist, Vector2D* lightmapUV, Vector2D* textureUV) = 0;
	virtual void ComputeLightmapAndTextureCoordinate(RayDispOutput_t const& uv, Vector2D* luv, Vector2D* tuv) = 0;
};