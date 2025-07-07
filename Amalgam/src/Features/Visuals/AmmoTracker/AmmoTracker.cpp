#include "AmmoTracker.h"
#include "../../../SDK/SDK.h"
#include <algorithm>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


std::string CAmmoTracker::GetPickupType(CBaseEntity* pEntity)
{
    if (!pEntity)
        return "";
    
    const char* pModelName = I::ModelInfoClient->GetModelName(pEntity->GetModel());
    if (!pModelName)
        return "";
    
    std::string sModelName = pModelName;
    if (sModelName.empty())
        return "";
    
    // Convert to lowercase for comparison
    std::transform(sModelName.begin(), sModelName.end(), sModelName.begin(), ::tolower);
    
    // Check cache first
    if (m_ModelCache.find(sModelName) != m_ModelCache.end())
        return m_ModelCache[sModelName];
    
    std::string sType = "";
    if (sModelName.find("ammopack") != std::string::npos)
        sType = "ammo";
    else if (sModelName.find("medkit") != std::string::npos || sModelName.find("healthkit") != std::string::npos)
        sType = "health";
    
    // Cache result
    m_ModelCache[sModelName] = sType;
    return sType;
}

std::string CAmmoTracker::GetPositionKey(const Vec3& vPos)
{
    return std::format("{:.0f}_{:.0f}_{:.0f}", vPos.x, vPos.y, vPos.z);
}

bool CAmmoTracker::IsVisible(const Vec3& vPos)
{
    auto pLocal = H::Entities.GetLocal();
    if (!pLocal)
        return false;
    
    std::string sKey = GetPositionKey(vPos);
    int iCurrentTick = I::GlobalVars->tickcount;
    
    // Check cache first
    if (m_NextVisCheck.find(sKey) != m_NextVisCheck.end() && iCurrentTick < m_NextVisCheck[sKey])
        return m_VisibilityCache[sKey];
    
    // Get eye position
    Vec3 vEyePos = pLocal->GetAbsOrigin() + pLocal->As<CTFPlayer>()->GetViewOffset();
    
    // Perform trace
    CGameTrace trace = {};
    CTraceFilterHitscan filter = {};
    filter.pSkip = pLocal;
    
    SDK::Trace(vEyePos, vPos, MASK_VISIBLE, &filter, &trace);
    
    bool bVisible = trace.fraction > 0.99f;
    
    // Cache result
    m_VisibilityCache[sKey] = bVisible;
    m_NextVisCheck[sKey] = iCurrentTick + 3;
    
    return bVisible;
}

void CAmmoTracker::UpdateSupplyPositions()
{
    int iCurrentTick = I::GlobalVars->tickcount;
    if (iCurrentTick - m_LastUpdateTick < UPDATE_INTERVAL)
        return;
    
    m_LastUpdateTick = iCurrentTick;
    
    std::unordered_map<std::string, bool> vCurrentSupplies;
    float fCurrentTime = I::GlobalVars->curtime;
    
    // Find all current pickups
    std::vector<EGroupType> vPickupGroups = {EGroupType::PICKUPS_HEALTH, EGroupType::PICKUPS_AMMO};
    
    for (auto eGroupType : vPickupGroups)
    {
        for (auto pEntity : H::Entities.GetGroup(eGroupType))
        {
            if (!pEntity || pEntity->IsDormant())
                continue;
                
            std::string sType = GetPickupType(pEntity);
            if (sType.empty())
                continue;
                
            Vec3 vPos = pEntity->GetAbsOrigin();
            std::string sKey = GetPositionKey(vPos);
            vCurrentSupplies[sKey] = true;
            
            // Add new supply if not tracked
            if (m_SupplyPositions.find(sKey) == m_SupplyPositions.end())
            {
                SupplyInfo info;
                info.Position = vPos;
                info.Type = sType;
                info.Respawning = false;
                info.DisappearTime = 0.0f;
                m_SupplyPositions[sKey] = info;
            }
        }
    }
    
    // Update respawn status
    for (auto& [sKey, info] : m_SupplyPositions)
    {
        if (vCurrentSupplies.find(sKey) == vCurrentSupplies.end() && !info.Respawning)
        {
            // Supply disappeared, start respawn timer
            info.Respawning = true;
            info.DisappearTime = fCurrentTime;
        }
        else if (vCurrentSupplies.find(sKey) != vCurrentSupplies.end() && info.Respawning)
        {
            // Supply respawned
            info.Respawning = false;
            info.DisappearTime = 0.0f;
        }
    }
}

int CAmmoTracker::GetScreenChartSize(float flDistance)
{
    if (!Vars::Competitive::AmmoTracker::ScaleWithDistance.Value)
        return Vars::Competitive::AmmoTracker::PieChartSize.Value;
    
    float flScale = 1.0f - std::min(flDistance / Vars::Competitive::AmmoTracker::MaxScaleDistance.Value, 1.0f);
    int iSize = Vars::Competitive::AmmoTracker::MinScreenChartSize.Value + 
                static_cast<int>((Vars::Competitive::AmmoTracker::MaxScreenChartSize.Value - Vars::Competitive::AmmoTracker::MinScreenChartSize.Value) * flScale);
    return std::max(Vars::Competitive::AmmoTracker::MinScreenChartSize.Value, 
                    std::min(Vars::Competitive::AmmoTracker::MaxScreenChartSize.Value, iSize));
}

int CAmmoTracker::GetTextSize(float flDistance)
{
    if (!Vars::Competitive::AmmoTracker::ScaleTextWithDistance.Value)
        return Vars::Competitive::AmmoTracker::TextSizeDefault.Value;
    
    float flScale = 1.0f - std::min(flDistance / Vars::Competitive::AmmoTracker::MaxScaleDistance.Value, 1.0f);
    int iResult = Vars::Competitive::AmmoTracker::TextSizeMin.Value + 
                  static_cast<int>((Vars::Competitive::AmmoTracker::TextSizeMax.Value - Vars::Competitive::AmmoTracker::TextSizeMin.Value) * flScale);
    return std::max(Vars::Competitive::AmmoTracker::TextSizeMin.Value,
                    std::min(Vars::Competitive::AmmoTracker::TextSizeMax.Value, iResult));
}

bool CAmmoTracker::IsPlayerNearSupply(const Vec3& vPlayerPos, const Vec3& vSupplyPos, float flMaxDistance)
{
    float fDistance = vPlayerPos.DistTo(vSupplyPos);
    return fDistance <= flMaxDistance;
}

void CAmmoTracker::DrawHudOverlay(const Vec3& vPlayerPos)
{
    if (!Vars::Competitive::AmmoTracker::ShowHudOverlay.Value)
        return;
    
    // Get screen dimensions
    int iScreenW, iScreenH;
    I::MatSystemSurface->GetScreenSize(iScreenW, iScreenH);
    
    // HUD overlay settings
    const int iHudCircleSize = 30;
    const int iHudSpacing = 10;
    const int iMarginFromEdge = 20;
    
    std::vector<std::pair<SupplyInfo, float>> vNearbySupplies;
    float fCurrentTime = I::GlobalVars->curtime;
    
    // Find supplies the player is standing on
    for (const auto& [sKey, info] : m_SupplyPositions)
    {
        if (!info.Respawning)
            continue;
            
        if (IsPlayerNearSupply(vPlayerPos, info.Position))
        {
            float fTimeLeft = std::max(0.0f, RESPAWN_TIME - (fCurrentTime - info.DisappearTime));
            vNearbySupplies.push_back({info, fTimeLeft});
        }
    }
    
    if (vNearbySupplies.empty())
        return;
    
    // Calculate starting position (bottom right, with spacing between multiple items)
    int iStartX = iScreenW - iMarginFromEdge - iHudCircleSize;
    int iStartY = iScreenH - iMarginFromEdge - iHudCircleSize;
    
    // Draw each nearby supply
    for (size_t i = 0; i < vNearbySupplies.size(); i++)
    {
        const auto& [info, fTimeLeft] = vNearbySupplies[i];
        
        // Calculate position for this circle (offset left for multiple items)
        int iCircleX = iStartX - (i * (iHudCircleSize * 2 + iHudSpacing)) + iHudCircleSize;
        int iCircleY = iStartY + iHudCircleSize;
        
        // Get colors
        Color_t color = (info.Type == "health") ? 
            Vars::Competitive::AmmoTracker::HealthColor.Value : 
            Vars::Competitive::AmmoTracker::AmmoColor.Value;
            
        // Calculate percentage
        float fTimePercent = fTimeLeft / RESPAWN_TIME;
        
        // Draw filled circle background
        Color_t bgColor = {0, 0, 0, 150};
        H::Draw.FillCircle(iCircleX, iCircleY, static_cast<float>(iHudCircleSize), 32, bgColor);
        
        // Draw progress pie chart
        DrawClockFill(iCircleX, iCircleY, iHudCircleSize - 2, fTimePercent, 32, color);
        
        // Draw timer text in center
        std::string sTimerText;
        if (Vars::Competitive::AmmoTracker::ShowMilliseconds.Value)
            sTimerText = std::format("{:.1f}", fTimeLeft);
        else
            sTimerText = std::format("{}", static_cast<int>(std::ceil(fTimeLeft)));
        
        auto pFont = H::Fonts.GetFont(FONT_ESP);
        Vec2 vTextSize = H::Draw.GetTextSize(sTimerText.c_str(), pFont);
        
        H::Draw.String(pFont, 
                      iCircleX - static_cast<int>(vTextSize.x / 2), iCircleY - static_cast<int>(vTextSize.y / 2), 
                      {255, 255, 255, 255}, ALIGN_CENTER, sTimerText.c_str());
    }
}

void CAmmoTracker::DrawClockFill(int iCenterX, int iCenterY, int iRadius, float flPercentage, int iVertices, const Color_t& color)
{
    if (flPercentage <= 0.0f || flPercentage > 1.0f)
        return;
    
    // First draw a background circle (optional - creates border effect)
    Color_t bgColor = {0, 0, 0, 100};
    H::Draw.FillCircle(iCenterX, iCenterY, static_cast<float>(iRadius), iVertices, bgColor);
    
    // Create vertices for the pie slice
    std::vector<Vertex_t> vertices;
    
    // Add center point  
    vertices.emplace_back(Vertex_t({ { static_cast<float>(iCenterX), static_cast<float>(iCenterY) } }));
    
    // Calculate pie slice
    float flStartAngle = -90.0f * M_PI / 180.0f; // Start at top
    float flEndAngle = flStartAngle + (2.0f * M_PI * flPercentage);
    float flStep = (flEndAngle - flStartAngle) / iVertices;
    
    // Add edge vertices
    for (int i = 0; i <= iVertices; i++)
    {
        float flAngle = flStartAngle + (i * flStep);
        float fX = iCenterX + cos(flAngle) * iRadius;
        float fY = iCenterY + sin(flAngle) * iRadius;
        vertices.emplace_back(Vertex_t({ { fX, fY } }));
    }
    
    // Draw filled polygon
    if (vertices.size() >= 3)
        H::Draw.FillPolygon(vertices, color);
}

void CAmmoTracker::DrawFilledProgress(const Vec3& vPos, int iRadius, float flPercentage, int iSegments, const Color_t& color)
{
    Vec3 vCenterScreen;
    Vec3 vCenterWorld = {vPos.x, vPos.y, vPos.z + 1.0f};
    if (!SDK::W2S(vCenterWorld, vCenterScreen))
        return;

    int iVisibleVerts = static_cast<int>(std::floor((1.0f - flPercentage) * iSegments));
    if (iVisibleVerts <= 0)
        return;

    // Create vertices for the filled ground pie chart
    std::vector<Vertex_t> vertices;
    
    // Add center point
    vertices.emplace_back(Vertex_t({ { vCenterScreen.x, vCenterScreen.y } }));
    
    // Add edge vertices for the visible portion
    for (int i = 0; i <= iVisibleVerts; i++)
    {
        float fAngle = i * (2.0f * M_PI / iSegments);
        
        Vec3 vWorldPos = {
            vPos.x + iRadius * cos(fAngle),
            vPos.y + iRadius * sin(fAngle),
            vPos.z + 1.0f
        };
        
        Vec3 vScreenPos;
        if (SDK::W2S(vWorldPos, vScreenPos))
        {
            vertices.emplace_back(Vertex_t({ { vScreenPos.x, vScreenPos.y } }));
        }
        else
        {
            return; // Can't draw if any point is off-screen
        }
    }
    
    // Draw filled polygon
    if (vertices.size() >= 3)
        H::Draw.FillPolygon(vertices, color);
}

void CAmmoTracker::Draw()
{
    if (!Vars::Competitive::Features::AmmoTracker.Value)
        return;
        
    if (I::EngineVGui->IsGameUIVisible())
        return;
    
    auto pLocal = H::Entities.GetLocal();
    if (!pLocal || !pLocal->IsAlive())
        return;
    
    UpdateSupplyPositions();
    
    float fCurrentTime = I::GlobalVars->curtime;
    Vec3 vPlayerPos = pLocal->GetAbsOrigin();
    
    // Draw HUD overlay for supplies the player is standing on
    DrawHudOverlay(vPlayerPos);
    
    for (const auto& [sKey, info] : m_SupplyPositions)
    {
        if (!info.Respawning)
            continue;
            
        // Check visibility if "Show through walls" is disabled
        if (!Vars::Competitive::AmmoTracker::ShowThroughWalls.Value && !IsVisible(info.Position))
            continue;
            
        Vec3 vScreenPos;
        if (!SDK::W2S(info.Position, vScreenPos))
            continue;
            
        float fTimeLeft = std::max(0.0f, RESPAWN_TIME - (fCurrentTime - info.DisappearTime));
        float fTimePercent = fTimeLeft / RESPAWN_TIME;
        
        Color_t colorConfig = (info.Type == "health") ? 
            Vars::Competitive::AmmoTracker::HealthColor.Value : 
            Vars::Competitive::AmmoTracker::AmmoColor.Value;
        
        float fDistance = vPlayerPos.DistTo(info.Position);
        
        // Draw ground pie chart
        if (Vars::Competitive::AmmoTracker::ShowGroundPieChart.Value && 
            fDistance <= Vars::Competitive::AmmoTracker::MaxDistanceGroundPieChart.Value)
        {
            DrawFilledProgress(info.Position, Vars::Competitive::AmmoTracker::PieChartSize.Value, 
                             fTimePercent, Vars::Competitive::AmmoTracker::PieChartSegments.Value, colorConfig);
        }
        
        // Draw screen pie chart
        if (Vars::Competitive::AmmoTracker::ShowScreenPieChart.Value && 
            fDistance <= Vars::Competitive::AmmoTracker::MaxDistanceScreenPieChart.Value)
        {
            int iScreenChartSize = GetScreenChartSize(fDistance);
            DrawClockFill(static_cast<int>(vScreenPos.x), static_cast<int>(vScreenPos.y), 
                         iScreenChartSize, fTimePercent, Vars::Competitive::AmmoTracker::PieChartSegments.Value, colorConfig);
        }
        
        // Draw text timer
        if (Vars::Competitive::AmmoTracker::ShowSeconds.Value && 
            fDistance <= Vars::Competitive::AmmoTracker::MaxDistanceText.Value)
        {
            std::string sTimerText;
            if (Vars::Competitive::AmmoTracker::ShowMilliseconds.Value)
                sTimerText = std::format("{:.1f}s", fTimeLeft);
            else
                sTimerText = std::format("{}s", static_cast<int>(std::ceil(fTimeLeft)));
            
            int iTextSize = GetTextSize(fDistance);
            auto pFont = (iTextSize <= 12) ? H::Fonts.GetFont(FONT_INDICATORS) : H::Fonts.GetFont(FONT_ESP);
            
            H::Draw.String(pFont, 
                          static_cast<int>(vScreenPos.x), static_cast<int>(vScreenPos.y - 15), 
                          Vars::Competitive::AmmoTracker::SecondsColor.Value, 
                          ALIGN_CENTER, sTimerText.c_str());
        }
    }
}