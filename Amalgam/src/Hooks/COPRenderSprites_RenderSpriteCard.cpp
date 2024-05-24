#include "../SDK/SDK.h"

MAKE_SIGNATURE(COPRenderSprites_RenderSpriteCard, "client.dll", "48 8B C4 48 89 58 ? 57 41 54", 0x0);

// https://www.unknowncheats.me/forum/team-fortress-2-a/520739-custom-particle-colors.html
typedef union
{
    float m128_f32[4];
    uint32_t m128_u32[4];
} fltx4;

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
    void* m_pParticles{};
};

MAKE_HOOK(COPRenderSprites_RenderSpriteCard, S::COPRenderSprites_RenderSpriteCard(), void, __fastcall,
    void* ecx, void* meshBuilder, void* pCtx, SpriteRenderInfo_t& info, int hParticle, void* pSortList, void* pCamera)
{
    if (Vars::Visuals::World::Modulations.Value & 1 << 3 && !(Vars::Visuals::UI::CleanScreenshots.Value && I::EngineClient->IsTakingScreenshot()))
    {
        info.m_pRGB[((hParticle / 4) * info.m_nRGBStride) + 0].m128_f32[hParticle & 0x3] = float(Vars::Colors::ParticleModulation.Value.r) / 255.f; // red
        info.m_pRGB[((hParticle / 4) * info.m_nRGBStride) + 1].m128_f32[hParticle & 0x3] = float(Vars::Colors::ParticleModulation.Value.g) / 255.f; // green
        info.m_pRGB[((hParticle / 4) * info.m_nRGBStride) + 2].m128_f32[hParticle & 0x3] = float(Vars::Colors::ParticleModulation.Value.b) / 255.f; // blue
    }

    CALL_ORIGINAL(ecx, meshBuilder, pCtx, info, hParticle, pSortList, pCamera);
}