#include "../SDK/SDK.h"
#include "../SDK/Definitions/Misc/UtlObjectReference.h"

MAKE_SIGNATURE(COPRenderSprites_Render, "client.dll", "48 89 54 24 ? 55 53 57 41 55 41 56", 0x0);
MAKE_SIGNATURE(COPRenderSprites_RenderSpriteCard, "client.dll", "48 8B C4 48 89 58 ? 57 41 54", 0x0);
MAKE_SIGNATURE(COPRenderSprites_RenderTwoSequenceSpriteCard, "client.dll", "48 8B C4 48 89 58 ? 48 89 68 ? 56 57 41 54 41 56 41 57 48 81 EC ? ? ? ? F3 0F 10 1D", 0x0);

class CSheet;
typedef __m128 fltx4;

class CParticleSystemDefinition
{
public:
	byte pad0[12]; // ?
	int m_nInitialParticles;
	int m_nPerParticleUpdatedAttributeMask;
	int m_nPerParticleInitializedAttributeMask;
	int m_nInitialAttributeReadMask;
	int m_nAttributeReadMask;
	uint64 m_nControlPointReadMask;
	Vector m_BoundingBoxMin;
	Vector m_BoundingBoxMax;
	char m_pszMaterialName[MAX_PATH];
};

class CParticleCollection
{
public:
    CUtlReference<CSheet> m_Sheet;
	fltx4 m_fl4CurTime;
	int m_nPaddedActiveParticles;
    float m_flCurTime;
    int m_nActiveParticles;
    float m_flDt;
    float m_flPreviousDt;
    float m_flNextSleepTime;
	CUtlReference<CParticleSystemDefinition> m_pDef;
};

struct SpriteRenderInfo_t
{
    size_t m_nXYZStride{};
    fltx4* m_pXYZ{};
    size_t m_nRotStride{};
    fltx4* m_pRot{};
    size_t m_nYawStride{};
    fltx4* m_pYaw{};
    size_t m_nRGBStride{};
    fltx4* m_pRGB{};
    size_t m_nCreationTimeStride{};
    fltx4* m_pCreationTimeStamp{};
    size_t m_nSequenceStride{};
    fltx4* m_pSequenceNumber{};
    size_t m_nSequence1Stride{};
    fltx4* m_pSequence1Number{};
    float m_flAgeScale{};
    float m_flAgeScale2{};
    void* m_pSheet{};
    int m_nVertexOffset{};
    CParticleCollection* m_pParticles{};
};

MAKE_HOOK(COPRenderSprites_Render, S::COPRenderSprites_Render(), void,
    void* rcx, IMatRenderContext* pRenderContext, CParticleCollection* pParticles, void* pContext)
{
#ifdef DEBUG_HOOKS
    if (!Vars::Hooks::COPRenderSprites_Render.Map[DEFAULT_BIND])
        return CALL_ORIGINAL(rcx, pRenderContext, pParticles, pContext);
#endif

    if (!Vars::Visuals::Particles::DrawIconsThroughWalls.Value || Vars::Visuals::UI::CleanScreenshots.Value && I::EngineClient->IsTakingScreenshot())
        return CALL_ORIGINAL(rcx, pRenderContext, pParticles, pContext);

    bool bValid = false;
    switch (FNV1A::Hash32(pParticles->m_pDef->m_pszMaterialName))
    {
    // blue icons
    case FNV1A::Hash32Const("effects\\defense_buff_bullet_blue.vmt"):
    case FNV1A::Hash32Const("effects\\defense_buff_explosion_blue.vmt"):
    case FNV1A::Hash32Const("effects\\defense_buff_fire_blue.vmt"):
    case FNV1A::Hash32Const("effects\\powerup_agility_icon_blue.vmt"):
    case FNV1A::Hash32Const("effects\\powerup_haste_icon_blue.vmt"):
    case FNV1A::Hash32Const("effects\\powerup_king_icon_blue.vmt"):
    case FNV1A::Hash32Const("effects\\powerup_knockout_icon_blue.vmt"):
    case FNV1A::Hash32Const("effects\\powerup_plague_icon_blue.vmt"):
    case FNV1A::Hash32Const("effects\\powerup_precision_icon_blue.vmt"):
    case FNV1A::Hash32Const("effects\\powerup_reflect_icon_blue.vmt"):
    case FNV1A::Hash32Const("effects\\powerup_resist_icon_blue.vmt"):
    case FNV1A::Hash32Const("effects\\powerup_strength_icon_blue.vmt"):
    case FNV1A::Hash32Const("effects\\powerup_supernova_icon_blue.vmt"):
    case FNV1A::Hash32Const("effects\\powerup_thorns_icon_blue.vmt"):
    case FNV1A::Hash32Const("effects\\powerup_vampire_icon_blue.vmt"):
    {
        auto pLocal = H::Entities.GetLocal();
        bValid = !pLocal || pLocal->m_iTeamNum() != TF_TEAM_BLUE;
        break;
    }
    // red icons
    case FNV1A::Hash32Const("effects\\defense_buff_bullet_red.vmt"):
    case FNV1A::Hash32Const("effects\\defense_buff_explosion_red.vmt"):
    case FNV1A::Hash32Const("effects\\defense_buff_fire_red.vmt"):
    case FNV1A::Hash32Const("effects\\powerup_agility_icon_red.vmt"):
    case FNV1A::Hash32Const("effects\\powerup_haste_icon_red.vmt"):
    case FNV1A::Hash32Const("effects\\powerup_king_icon_red.vmt"):
    case FNV1A::Hash32Const("effects\\powerup_knockout_icon_red.vmt"):
    case FNV1A::Hash32Const("effects\\powerup_plague_icon_red.vmt"):
    case FNV1A::Hash32Const("effects\\powerup_precision_icon_red.vmt"):
    case FNV1A::Hash32Const("effects\\powerup_reflect_icon_red.vmt"):
    case FNV1A::Hash32Const("effects\\powerup_regen_icon_blue.vmt"):
    case FNV1A::Hash32Const("effects\\powerup_regen_icon_red.vmt"):
    case FNV1A::Hash32Const("effects\\powerup_resist_icon_red.vmt"):
    case FNV1A::Hash32Const("effects\\powerup_strength_icon_red.vmt"):
    case FNV1A::Hash32Const("effects\\powerup_supernova_icon_red.vmt"):
    case FNV1A::Hash32Const("effects\\powerup_thorns_icon_red.vmt"):
    case FNV1A::Hash32Const("effects\\powerup_vampire_icon_red.vmt"):
    {
        auto pLocal = H::Entities.GetLocal();
        bValid = !pLocal || pLocal->m_iTeamNum() != TF_TEAM_RED;
        break;
    }
    // global icons
    /*
    case FNV1A::Hash32Const("effects\\powerup_agility_icon.vmt"):
    case FNV1A::Hash32Const("effects\\powerup_haste_icon.vmt"):
    case FNV1A::Hash32Const("effects\\powerup_king_icon.vmt"):
    case FNV1A::Hash32Const("effects\\powerup_knockout_icon.vmt"):
    case FNV1A::Hash32Const("effects\\powerup_pestilence_icon.vmt"): // is this even used?
    case FNV1A::Hash32Const("effects\\powerup_plague_icon.vmt"):
    case FNV1A::Hash32Const("effects\\powerup_precision_icon.vmt"):
    case FNV1A::Hash32Const("effects\\powerup_reflect_icon.vmt"):
    case FNV1A::Hash32Const("effects\\powerup_regen_icon.vmt"):
    case FNV1A::Hash32Const("effects\\powerup_resist_icon.vmt"):
    case FNV1A::Hash32Const("effects\\powerup_strength_icon.vmt"):
    case FNV1A::Hash32Const("effects\\powerup_supernova_icon.vmt"):
    case FNV1A::Hash32Const("effects\\powerup_thorns_icon.vmt"):
    case FNV1A::Hash32Const("effects\\powerup_vampire_icon.vmt"):
    */
    case FNV1A::Hash32Const("effects\\particle_nemesis_blue.vmt"):
    case FNV1A::Hash32Const("effects\\particle_nemesis_red.vmt"):
    case FNV1A::Hash32Const("effects\\particle_nemesis_burst.vmt"):
    case FNV1A::Hash32Const("effects\\duel_blue.vmt"):
    case FNV1A::Hash32Const("effects\\duel_red.vmt"):
    case FNV1A::Hash32Const("effects\\duel_burst.vmt"):
    case FNV1A::Hash32Const("effects\\crit.vmt"):
    case FNV1A::Hash32Const("effects\\yikes.vmt"):
        bValid = true;
    }
    if (!bValid)
        return CALL_ORIGINAL(rcx, pRenderContext, pParticles, pContext);

    pRenderContext->DepthRange(0.f, 0.2f);
    CALL_ORIGINAL(rcx, pRenderContext, pParticles, pContext);
    pRenderContext->DepthRange(0.f, 1.f);
}

MAKE_HOOK(COPRenderSprites_RenderSpriteCard, S::COPRenderSprites_RenderSpriteCard(), void,
    void* rcx, void* meshBuilder, void* pCtx, SpriteRenderInfo_t& info, int hParticle, void* pSortList, void* pCamera)
{
#ifdef DEBUG_HOOKS
    if (!Vars::Hooks::COPRenderSprites_Render.Map[DEFAULT_BIND])
        return CALL_ORIGINAL(rcx, meshBuilder, pCtx, info, hParticle, pSortList, pCamera);
#endif

    if (!(Vars::Visuals::World::Modulations.Value & Vars::Visuals::World::ModulationsEnum::Particle) || Vars::Visuals::UI::CleanScreenshots.Value && I::EngineClient->IsTakingScreenshot())
        return CALL_ORIGINAL(rcx, meshBuilder, pCtx, info, hParticle, pSortList, pCamera);

    info.m_pRGB[((hParticle / 4) * info.m_nRGBStride) + 0].m128_f32[hParticle & 0x3] = float(Vars::Colors::ParticleModulation.Value.r) / 255.f; // red
    info.m_pRGB[((hParticle / 4) * info.m_nRGBStride) + 1].m128_f32[hParticle & 0x3] = float(Vars::Colors::ParticleModulation.Value.g) / 255.f; // green
    info.m_pRGB[((hParticle / 4) * info.m_nRGBStride) + 2].m128_f32[hParticle & 0x3] = float(Vars::Colors::ParticleModulation.Value.b) / 255.f; // blue
    CALL_ORIGINAL(rcx, meshBuilder, pCtx, info, hParticle, pSortList, pCamera);
}

MAKE_HOOK(COPRenderSprites_RenderTwoSequenceSpriteCard, S::COPRenderSprites_RenderTwoSequenceSpriteCard(), void,
    void* rcx, void* meshBuilder, void* pCtx, SpriteRenderInfo_t& info, int hParticle, void* pSortList, void* pCamera)
{
#ifdef DEBUG_HOOKS
    if (!Vars::Hooks::COPRenderSprites_Render.Map[DEFAULT_BIND])
        return CALL_ORIGINAL(rcx, meshBuilder, pCtx, info, hParticle, pSortList, pCamera);
#endif

    if (!(Vars::Visuals::World::Modulations.Value &Vars::Visuals::World::ModulationsEnum::Particle) || Vars::Visuals::UI::CleanScreenshots.Value && I::EngineClient->IsTakingScreenshot())
        return CALL_ORIGINAL(rcx, meshBuilder, pCtx, info, hParticle, pSortList, pCamera);

    info.m_pRGB[((hParticle / 4) * info.m_nRGBStride) + 0].m128_f32[hParticle & 0x3] = float(Vars::Colors::ParticleModulation.Value.r) / 255.f; // red
    info.m_pRGB[((hParticle / 4) * info.m_nRGBStride) + 1].m128_f32[hParticle & 0x3] = float(Vars::Colors::ParticleModulation.Value.g) / 255.f; // green
    info.m_pRGB[((hParticle / 4) * info.m_nRGBStride) + 2].m128_f32[hParticle & 0x3] = float(Vars::Colors::ParticleModulation.Value.b) / 255.f; // blue
    CALL_ORIGINAL(rcx, meshBuilder, pCtx, info, hParticle, pSortList, pCamera);
}