#include "../SDK/SDK.h"

MAKE_SIGNATURE(CStaticPropMgr_ComputePropOpacity, "engine.dll", "48 89 5C 24 ? 48 89 7C 24 ? 41 54 41 56 41 57 48 83 EC ? 48 8B 05", 0x0);

class CStaticProp
{
public:
	byte pad[20];
	Vector						m_Origin;
	QAngle						m_Angles;
	model_t*					m_pModel;
	SpatialPartitionHandle_t	m_Partition;
	ModelInstanceHandle_t		m_ModelInstance;
	unsigned char				m_Alpha;
	unsigned char				m_nSolidType;
	unsigned char				m_Skin;
	unsigned char				m_Flags;
	unsigned short				m_FirstLeaf;
	unsigned short				m_LeafCount;
	CBaseHandle					m_EntHandle;	// FIXME: Do I need client + server handles?
	ClientRenderHandle_t		m_RenderHandle;
	unsigned short				m_FadeIndex;	// Index into the m_StaticPropFade dictionary
	float						m_flForcedFadeScale;

	// bbox is the same for both GetBounds and GetRenderBounds since static props never move.
	// GetRenderBounds is interpolated data, and GetBounds is last networked.
	Vector					m_RenderBBoxMin;
	Vector					m_RenderBBoxMax;
	matrix3x4				m_ModelToWorld;
	float					m_flRadius;

	Vector					m_WorldRenderBBoxMin;
	Vector					m_WorldRenderBBoxMax;

	// FIXME: This sucks. Need to store the lighting origin off
	// because the time at which the static props are unserialized
	// doesn't necessarily match the time at which we can initialize the light cache
	Vector					m_LightingOrigin;
};

MAKE_HOOK(CStaticPropMgr_ComputePropOpacity, S::CStaticPropMgr_ComputePropOpacity(), void,
	void* rcx, CStaticProp* pProp)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::CStaticPropMgr_ComputePropOpacity.Map[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx, pProp);
#endif

	if (Vars::Visuals::World::NoPropFade.Value && pProp)
	{
		pProp->m_Alpha = 255;
		return;
	}

	CALL_ORIGINAL(rcx, pProp);
}