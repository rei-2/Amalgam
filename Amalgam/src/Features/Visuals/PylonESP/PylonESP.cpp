#include "PylonESP.h"
#include "../../../SDK/SDK.h"

CPylonESP::CPylonESP()
{
    InitializeAlphaSteps();
}

void CPylonESP::InitializeAlphaSteps()
{
    int segments = Vars::Competitive::PylonESP::Segments.Value;
    m_AlphaSteps.reserve(segments + 1);
    for (int i = 0; i <= segments; i++)
    {
        float progress = static_cast<float>(i) / segments;
        int alpha = static_cast<int>(Vars::Competitive::PylonESP::StartAlpha.Value - (progress * (Vars::Competitive::PylonESP::StartAlpha.Value - Vars::Competitive::PylonESP::EndAlpha.Value)));
        m_AlphaSteps.push_back(alpha);
    }
}

void CPylonESP::Draw()
{
    if (!Vars::Competitive::Features::PylonESP.Value)
        return;
    
    // Early exits
    if (I::EngineVGui->IsGameUIVisible())
        return;
    
    auto pLocal = H::Entities.GetLocal();
    if (!pLocal)
        return;
    
    // Cache frequently used values
    Vec3 localPos = pLocal->GetAbsOrigin();
    Vec3 eyePos = pLocal->GetEyePosition();
    int localTeam = pLocal->m_iTeamNum();
    
    // Process all players in real-time
    for (auto pEntity : H::Entities.GetGroup(EGroupType::PLAYERS_ALL))
    {
        auto pPlayer = pEntity->As<CTFPlayer>();
        if (!pPlayer || !pPlayer->IsAlive() || pPlayer->IsDormant())
            continue;
        
        // Only track enemy medics
        if (pPlayer->m_iTeamNum() == localTeam || pPlayer->m_iClass() != TF_CLASS_MEDIC)
            continue;
        
        Vec3 playerPos = pPlayer->GetAbsOrigin();
        
        // Skip if we can see the medic directly
        if (IsPlayerDirectlyVisible(pPlayer, eyePos))
            continue;
        
        // Skip if we're too close to the medic
        float distanceSqr = GetDistanceSqr(localPos, playerPos);
        if (distanceSqr < (Vars::Competitive::PylonESP::MinDistance.Value * Vars::Competitive::PylonESP::MinDistance.Value))
            continue;
        
        // Calculate pylon base position (real-time)
        Vec3 playerMaxs = pPlayer->m_vecMaxs();
        Vec3 pylonBase = Vec3(playerPos.x, playerPos.y, playerPos.z + playerMaxs.z + Vars::Competitive::PylonESP::PylonOffset.Value);
        
        // Draw pylon immediately
        DrawPylon(pylonBase, eyePos);
    }
}

bool CPylonESP::IsVisible(const Vec3& fromPos, const Vec3& targetPos)
{
    CGameTrace trace;
    CTraceFilterHitscan filter;
    filter.pSkip = H::Entities.GetLocal();
    SDK::Trace(fromPos, targetPos, MASK_VISIBLE, &filter, &trace);
    
    return trace.fraction >= 0.99f;
}

float CPylonESP::GetDistanceSqr(const Vec3& pos1, const Vec3& pos2)
{
    Vec3 delta = pos2 - pos1;
    return delta.x * delta.x + delta.y * delta.y + delta.z * delta.z;
}

bool CPylonESP::IsPlayerDirectlyVisible(CTFPlayer* pPlayer, const Vec3& eyePos)
{
    Vec3 playerPos = pPlayer->GetAbsOrigin();
    return IsVisible(eyePos, playerPos);
}

void CPylonESP::DrawPylon(const Vec3& basePosition, const Vec3& eyePos)
{
    // First pass: check if any segment is visible
    bool anySegmentVisible = false;
    for (int i = 0; i <= Vars::Competitive::PylonESP::Segments.Value; i++)
    {
        Vec3 segmentPos = Vec3(basePosition.x, basePosition.y, basePosition.z + (i * (Vars::Competitive::PylonESP::PylonHeight.Value / Vars::Competitive::PylonESP::Segments.Value)));
        
        if (IsVisible(eyePos, segmentPos))
        {
            anySegmentVisible = true;
            break;
        }
    }
    
    // Skip drawing if no segments are visible
    if (!anySegmentVisible)
        return;
    
    // Second pass: draw visible segments
    Vec3 lastScreenPos;
    bool hasLastScreenPos = false;
    
    for (int i = 0; i <= Vars::Competitive::PylonESP::Segments.Value; i++)
    {
        Vec3 segmentPos = Vec3(basePosition.x, basePosition.y, basePosition.z + (i * (Vars::Competitive::PylonESP::PylonHeight.Value / Vars::Competitive::PylonESP::Segments.Value)));
        
        bool visible = IsVisible(eyePos, segmentPos);
        
        if (!visible)
        {
            hasLastScreenPos = false;
            continue;
        }
        
        Vec3 screenPos;
        if (!SDK::W2S(segmentPos, screenPos))
        {
            hasLastScreenPos = false;
            continue;
        }
        
        if (hasLastScreenPos)
        {
            // Draw lines with fading alpha
            Color_t segmentColor = Vars::Competitive::PylonESP::PylonColor.Value;
            segmentColor.a = static_cast<byte>(m_AlphaSteps[i]);
            
            for (int w = 0; w < Vars::Competitive::PylonESP::PylonWidth.Value; w++)
            {
                H::Draw.Line(static_cast<int>(lastScreenPos.x + w), static_cast<int>(lastScreenPos.y),
                           static_cast<int>(screenPos.x + w), static_cast<int>(screenPos.y), segmentColor);
            }
        }
        
        lastScreenPos = screenPos;
        hasLastScreenPos = true;
    }
}