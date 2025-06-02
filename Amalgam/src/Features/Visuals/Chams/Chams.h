#pragma once
#include "../../../SDK/SDK.h"

class CChams
{
	bool GetChams(CTFPlayer* pLocal, CBaseEntity* pEntity, Chams_t* pChams);

	void StencilBegin(IMatRenderContext* pRenderContext, bool bTwoModels = false);
	void StencilVisible(IMatRenderContext* pRenderContext, bool bTwoModels = false);
	void StencilOccluded(IMatRenderContext* pRenderContext);
	void StencilEnd(IMatRenderContext* pRenderContext, bool bTwoModels = false);

	void DrawModel(CBaseEntity* pEntity, Chams_t& tChams, IMatRenderContext* pRenderContext, bool bExtra = false);

	void RenderBacktrack(const DrawModelState_t& pState, const ModelRenderInfo_t& pInfo);
	void RenderFakeAngle(const DrawModelState_t& pState, const ModelRenderInfo_t& pInfo);

	struct ChamsInfo_t
	{
		CBaseEntity* m_pEntity;
		Chams_t m_tChams;
		bool m_bExtra = false;
	};
	std::vector<ChamsInfo_t> m_vEntities = {};

	bool m_bExtra = false;

public:
	void Store(CTFPlayer* pLocal);
	void RenderMain();
	void RenderHandler(const DrawModelState_t& pState, const ModelRenderInfo_t& pInfo, matrix3x4* pBoneToWorld);

	bool RenderViewmodel(void* ecx, int flags, int* iReturn);
	bool RenderViewmodel(const DrawModelState_t& pState, const ModelRenderInfo_t& pInfo, matrix3x4* pBoneToWorld);

	bool m_bRendering = false;

	std::unordered_map<int, bool> m_mEntities = {};
};

ADD_FEATURE(CChams, Chams);