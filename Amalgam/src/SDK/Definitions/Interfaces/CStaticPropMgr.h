#pragma once
#include "../Main/CStaticProp.h"
#include "../Main/CGameTrace.h"
#include "../Misc/CUtlVector.h"

class IPhysicsEnvironment;
class IVPhysicsKeyHandler;
typedef unsigned short MDLHandle_t;
typedef struct LightCacheHandle_t__* LightCacheHandle_t;

struct StaticPropLeafLump_t
{
	unsigned short m_Leaf;
};

class IStaticPropMgrEngine
{
public:
	virtual bool Init() = 0;
	virtual void Shutdown() = 0;
	virtual void LevelInit() = 0;
	virtual void LevelInitClient() = 0;
	virtual void LevelShutdownClient() = 0;
	virtual void LevelShutdown() = 0;
	virtual void RecomputeStaticLighting() = 0;
	virtual bool IsPropInPVS(IHandleEntity* pHandleEntity, const byte* pVis) const = 0;
	virtual ICollideable* GetStaticProp(IHandleEntity* pHandleEntity) = 0;
	virtual LightCacheHandle_t GetLightCacheHandleForStaticProp(IHandleEntity* pHandleEntity) = 0;
	virtual bool IsStaticProp(IHandleEntity* pHandleEntity) const = 0;
	virtual bool IsStaticProp(CBaseHandle handle) const = 0;
	virtual int GetStaticPropIndex(IHandleEntity* pHandleEntity) const = 0;
	virtual bool PropHasBakedLightingDisabled(IHandleEntity* pHandleEntity) const = 0;
};

class IStaticPropMgr
{
public:
	virtual void CreateVPhysicsRepresentations(IPhysicsEnvironment* physenv, IVPhysicsKeyHandler* pDefaults, void* pGameData) = 0;
	virtual void TraceRayAgainstStaticProp(const Ray_t& ray, int staticPropIndex, trace_t& tr) = 0;
	virtual bool IsStaticProp(IHandleEntity* pHandleEntity) const = 0;
	virtual bool IsStaticProp(CBaseHandle handle) const = 0;
	virtual ICollideable* GetStaticPropByIndex(int propIndex) = 0;
};

class IStaticPropMgrClient : public IStaticPropMgr
{
public:
	virtual void ComputePropOpacity(const Vector & viewOrigin, float factor) = 0;
	virtual void AddDecalToStaticProp(const Vector& rayStart, const Vector& rayEnd, int staticPropIndex, int decalIndex, bool doTrace, trace_t& tr) = 0;
	virtual void AddShadowToStaticProp(unsigned short shadowHandle, IClientRenderable* pRenderable) = 0;
	virtual void RemoveAllShadowsFromStaticProp(IClientRenderable* pRenderable) = 0;
	virtual void GetStaticPropMaterialColorAndLighting(trace_t* pTrace, int staticPropIndex, Vector& lighting, Vector& matColor) = 0;
	virtual void GetAllStaticProps(CUtlVector<ICollideable*>* pOutput) = 0;
	virtual void GetAllStaticPropsInAABB(const Vector& vMins, const Vector& vMaxs, CUtlVector<ICollideable*>* pOutput) = 0;
	virtual void GetAllStaticPropsInOBB(const Vector& ptOrigin, const Vector& vExtent1, const Vector& vExtent2, const Vector& vExtent3, CUtlVector<ICollideable*>* pOutput) = 0;
	virtual void DrawStaticProps(IClientRenderable** pProps, int count, bool bShadowDepth, bool drawVCollideWireframe) = 0;
	virtual void AddColorDecalToStaticProp(Vector const& rayStart, Vector const& rayEnd, int staticPropIndex, int decalIndex, bool doTrace, trace_t& tr, bool bUseColor, Color_t cColor) = 0;
};

class IStaticPropMgrServer : public IStaticPropMgr
{
public:
	virtual void GetAllStaticProps(CUtlVector<ICollideable*>* pOutput) = 0;
	virtual void GetAllStaticPropsInAABB(const Vector& vMins, const Vector& vMaxs, CUtlVector<ICollideable*>* pOutput) = 0;
	virtual void GetAllStaticPropsInOBB(const Vector& ptOrigin, const Vector& vExtent1, const Vector& vExtent2, const Vector& vExtent3, CUtlVector<ICollideable*>* pOutput) = 0;
};

class CStaticPropMgr : public IStaticPropMgrEngine, public IStaticPropMgrClient, public IStaticPropMgrServer
{
public:
	virtual ~CStaticPropMgr() = 0;
	virtual bool Init() = 0;
	virtual void Shutdown() = 0;
	virtual void LevelInit() = 0;
	virtual void LevelInitClient() = 0;
	virtual void LevelShutdown() = 0;
	virtual void LevelShutdownClient() = 0;
	virtual bool IsPropInPVS(IHandleEntity* pHandleEntity, const byte* pVis) const = 0;
	virtual ICollideable* GetStaticProp(IHandleEntity* pHandleEntity) = 0;
	virtual void RecomputeStaticLighting() = 0;
	virtual LightCacheHandle_t GetLightCacheHandleForStaticProp(IHandleEntity* pHandleEntity) = 0;
	virtual bool IsStaticProp(IHandleEntity* pHandleEntity) const = 0;
	virtual bool IsStaticProp(CBaseHandle handle) const = 0;
	virtual int GetStaticPropIndex(IHandleEntity* pHandleEntity) const = 0;
	virtual ICollideable* GetStaticPropByIndex(int propIndex) = 0;
	virtual void ComputePropOpacity(const Vector& viewOrigin, float factor) = 0;
	virtual void TraceRayAgainstStaticProp(const Ray_t& ray, int staticPropIndex, trace_t& tr) = 0;
	virtual void AddDecalToStaticProp(Vector const& rayStart, Vector const& rayEnd, int staticPropIndex, int decalIndex, bool doTrace, trace_t& tr) = 0;
	virtual void AddColorDecalToStaticProp(Vector const& rayStart, Vector const& rayEnd, int staticPropIndex, int decalIndex, bool doTrace, trace_t& tr, bool bUseColor, Color_t cColor) = 0;
	virtual void AddShadowToStaticProp(unsigned short shadowHandle, IClientRenderable* pRenderable) = 0;
	virtual void RemoveAllShadowsFromStaticProp(IClientRenderable* pRenderable) = 0;
	virtual void GetStaticPropMaterialColorAndLighting(trace_t* pTrace, int staticPropIndex, Vector& lighting, Vector& matColor) = 0;
	virtual void CreateVPhysicsRepresentations(IPhysicsEnvironment* physenv, IVPhysicsKeyHandler* pDefaults, void* pGameData) = 0;
	virtual void GetAllStaticProps(CUtlVector<ICollideable*>* pOutput) = 0;
	virtual void GetAllStaticPropsInAABB(const Vector& vMins, const Vector& vMaxs, CUtlVector<ICollideable*>* pOutput) = 0;
	virtual void GetAllStaticPropsInOBB(const Vector& ptOrigin, const Vector& vExtent1, const Vector& vExtent2, const Vector& vExtent3, CUtlVector<ICollideable*>* pOutput) = 0;
	virtual bool PropHasBakedLightingDisabled(IHandleEntity* pHandleEntity) const = 0;

	struct StaticPropDict_t
	{
		model_t* m_pModel;
		MDLHandle_t m_hMDL;
	};
	struct StaticPropFade_t
	{
		int		m_Model;
		union
		{
			float	m_MinDistSq;
			float	m_MaxScreenWidth;
		};
		union
		{
			float	m_MaxDistSq;
			float	m_MinScreenWidth;
		};
		float	m_FalloffFactor;
	};

	CUtlVector <StaticPropDict_t> m_StaticPropDict;
	CUtlVector <CStaticProp> m_StaticProps;
	CUtlVector <StaticPropLeafLump_t> m_StaticPropLeaves;
	CUtlVector <StaticPropFade_t> m_StaticPropFade;
	bool m_bLevelInitialized;
	bool m_bClientInitialized;
	Vector m_vecLastViewOrigin;
	float m_flLastViewFactor;
};

MAKE_INTERFACE_SIGNATURE(CStaticPropMgr, StaticPropMgr, "engine.dll", "48 8D 05 ? ? ? ? C3 CC CC CC CC CC CC CC CC 48 89 5C 24 ? 48 89 74 24 ? 57 48 81 EC ? ? ? ? 41 8B C0", 0x0, 0);