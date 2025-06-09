#pragma once
#include "../../../SDK/SDK.h"

#include <boost/functional/hash.hpp>

class CGlow
{
	bool GetGlow(CTFPlayer* pLocal, CBaseEntity* pEntity, Glow_t* pGlow, Color_t* pColor);

	void StencilBegin(IMatRenderContext* pRenderContext);
	void StencilPreDraw(IMatRenderContext* pRenderContext);
	void StencilEnd(IMatRenderContext* pRenderContext);

	void SetupBegin(Glow_t glow, IMatRenderContext* pRenderContext, IMaterial* m_pMatGlowColor, IMaterial* m_pMatBlurY);
	void SetupMid(IMatRenderContext* pRenderContext, int w, int h);
	void SetupEnd(Glow_t glow, IMatRenderContext* pRenderContext, IMaterial* m_pMatBlurX, IMaterial* m_pMatBlurY, IMaterial* m_pMatHaloAddToScreen, int w, int h);

	void DrawModel(CBaseEntity* pEntity);

	void RenderBacktrack(const DrawModelState_t& pState, const ModelRenderInfo_t& pInfo);
	void RenderFakeAngle(const DrawModelState_t& pState, const ModelRenderInfo_t& pInfo);

	IMaterial* m_pMatGlowColor;
	ITexture* m_pRenderBuffer1;
	ITexture* m_pRenderBuffer2;
	IMaterial* m_pMatHaloAddToScreen;
	IMaterial* m_pMatBlurX;
	IMaterial* m_pMatBlurY;



	struct GlowHasher_t
	{
		std::size_t operator()(const Glow_t& k) const
		{
			std::size_t seed = 0;

			boost::hash_combine(seed, boost::hash_value(k.Stencil));
			boost::hash_combine(seed, boost::hash_value(k.Blur));

			return seed;
		}
	};
	struct GlowInfo_t
	{
		CBaseEntity* m_pEntity;
		Color_t m_cColor;
		bool m_bExtra = false;
	};
	std::unordered_map<Glow_t, std::vector<GlowInfo_t>, GlowHasher_t> m_mEntities = {};

	float m_flSavedBlend = 1.f;
	bool m_bExtra = false;

public:
	void Initialize();
	void Unload();

	void Store(CTFPlayer* pLocal);

	void RenderMain();
	void RenderHandler(const DrawModelState_t& pState, const ModelRenderInfo_t& pInfo, matrix3x4* pBoneToWorld);

	void RenderViewmodel(void* ecx, int flags);
	void RenderViewmodel(const DrawModelState_t& pState, const ModelRenderInfo_t& pInfo, matrix3x4* pBoneToWorld);

	bool m_bRendering = false;
};

ADD_FEATURE(CGlow, Glow);