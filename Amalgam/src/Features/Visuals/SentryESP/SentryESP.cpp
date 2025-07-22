#include "SentryESP.h"

void CSentryESP::Draw()
{
    if (!Vars::Competitive::Features::SentryESP.Value)
        return;
    
    if (I::EngineVGui->IsGameUIVisible())
        return;
    
    auto pLocal = H::Entities.GetLocal();
    if (!pLocal)
        return;
    
    // Clear targeting data each frame
    m_SentriesTargetingLocal.clear();
    
    // Find all sentry guns
    for (auto pEntity : H::Entities.GetGroup(EGroupType::BUILDINGS_ALL))
    {
        if (!pEntity || pEntity->IsDormant())
            continue;
            
        // Check if it's a sentry gun
        if (pEntity->GetClassID() != ETFClassID::CObjectSentrygun)
            continue;
            
        auto pSentry = pEntity->As<CObjectSentrygun>();
        if (!pSentry)
            continue;
            
        // Skip if building
        if (pSentry->m_bBuilding())
            continue;
            
        // Skip friendly sentries if not showing them
        if (!Vars::Competitive::SentryESP::ShowFriendly.Value && pEntity->m_iTeamNum() == pLocal->m_iTeamNum())
            continue;
            
        SentryAimData aimData = GetSentryAimData(pEntity);
        
        if (!aimData.IsValid)
            continue;
            
        // Check if targeting local player
        bool isTargetingLocal = false;
        if (aimData.Target && aimData.Target->IsPlayer())
        {
            auto pTargetPlayer = aimData.Target->As<CTFPlayer>();
            if (pTargetPlayer && pTargetPlayer->entindex() == pLocal->entindex())
            {
                isTargetingLocal = true;
                m_SentriesTargetingLocal.insert(pEntity->entindex());
            }
        }
        
        // Draw ESP box
        if (Vars::Competitive::SentryESP::ShowBoxes.Value)
        {
            Vec3 vPos = pEntity->GetAbsOrigin();
            Vec3 vMins = pEntity->m_vecMins();
            Vec3 vMaxs = pEntity->m_vecMaxs();
            
            Vec3 vBottomPos = {vPos.x, vPos.y, vPos.z + vMins.z};
            Vec3 vTopPos = {vPos.x, vPos.y, vPos.z + vMaxs.z};
            
            Vec3 vScreenBottom, vScreenTop;
            if (SDK::W2S(vBottomPos, vScreenBottom) && SDK::W2S(vTopPos, vScreenTop))
            {
                int height = static_cast<int>(vScreenBottom.y - vScreenTop.y);
                int width = static_cast<int>(height * 0.75f);
                
                int x1 = static_cast<int>(vScreenBottom.x - width / 2);
                int y1 = static_cast<int>(vScreenTop.y);
                int x2 = static_cast<int>(vScreenBottom.x + width / 2);
                int y2 = static_cast<int>(vScreenBottom.y);
                
                bool isActuallyVisible = IsVisible(pEntity, pLocal);
                
                // Skip if not showing through walls and not visible
                if (!Vars::Competitive::SentryESP::ShowThroughWalls.Value && !isActuallyVisible)
                    continue;
                
                // Determine color
                Color_t color;
                if (!isActuallyVisible)
                    color = Vars::Competitive::SentryESP::HiddenColor.Value;
                else
                    color = isTargetingLocal ? Vars::Competitive::SentryESP::DangerColor.Value : Vars::Competitive::SentryESP::SafeColor.Value;
                
                DrawESPBox(x1, y1, x2, y2, color);
                
                // Draw level indicator
                bool isMini = pSentry->m_bMiniBuilding();
                int level = pSentry->m_iUpgradeLevel();
                std::string levelText = isMini ? "M" : std::to_string(level);
                
                int textX = x1 + (x2 - x1) / 2;
                int textY = y1 - 20;
                
                H::Draw.String(H::Fonts.GetFont(FONT_ESP), textX, textY, 
                    Vars::Competitive::SentryESP::TextColor.Value, ALIGN_CENTER, levelText.c_str());
            }
        }
        
        // Draw aim line with interpolation
        if (Vars::Competitive::SentryESP::ShowLine.Value && aimData.IsValid)
        {
            // Get interpolated aim data for smoother line movement
            SentryAimData interpolatedData = GetInterpolatedAimData(pEntity->entindex(), aimData);
            Vec3 vEndPos = GetAimLineEndPos(interpolatedData.MuzzlePos, interpolatedData.AimAngles);
            
            Vec3 vScreenStart, vScreenEnd;
            if (SDK::W2S(interpolatedData.MuzzlePos, vScreenStart) && SDK::W2S(vEndPos, vScreenEnd))
            {
                H::Draw.Line(static_cast<int>(vScreenStart.x), static_cast<int>(vScreenStart.y),
                    static_cast<int>(vScreenEnd.x), static_cast<int>(vScreenEnd.y), 
                    Vars::Competitive::SentryESP::LineColor.Value);
            }
        }
    }
}

void CSentryESP::UpdateChamsEntities()
{
    // Clear previous chams entries
    m_mEntities.clear();
    
    if (!Vars::Competitive::Features::SentryESP.Value || !Vars::Competitive::SentryESP::ShowChams.Value)
        return;
    
    auto pLocal = H::Entities.GetLocal();
    if (!pLocal)
        return;
    
    // Find all sentry guns targeting the local player
    for (auto pEntity : H::Entities.GetGroup(EGroupType::BUILDINGS_ALL))
    {
        if (!pEntity || pEntity->IsDormant())
            continue;
            
        // Check if it's a sentry gun
        if (pEntity->GetClassID() != ETFClassID::CObjectSentrygun)
            continue;
            
        auto pSentry = pEntity->As<CObjectSentrygun>();
        if (!pSentry)
            continue;
            
        // Skip if building
        if (pSentry->m_bBuilding())
            continue;
            
        // Skip friendly sentries if not showing them
        if (!Vars::Competitive::SentryESP::ShowFriendly.Value && pEntity->m_iTeamNum() == pLocal->m_iTeamNum())
            continue;
            
        SentryAimData aimData = GetSentryAimData(pEntity);
        
        if (!aimData.IsValid)
            continue;
            
        // Check if targeting local player
        bool isTargetingLocal = false;
        if (aimData.Target && aimData.Target->IsPlayer())
        {
            auto pTargetPlayer = aimData.Target->As<CTFPlayer>();
            if (pTargetPlayer && pTargetPlayer->entindex() == pLocal->entindex())
            {
                isTargetingLocal = true;
            }
        }
        
        // Add targeting sentries to chams tracking
        if (isTargetingLocal)
        {
            m_mEntities[pEntity->entindex()] = true;
        }
    }
}

bool CSentryESP::IsVisible(CBaseEntity* pEntity, CTFPlayer* pLocal)
{
    if (!pEntity || !pLocal)
        return false;
        
    Vec3 vEyePos = pLocal->GetAbsOrigin() + pLocal->m_vecViewOffset();
    Vec3 vTargetPos = pEntity->GetAbsOrigin();
    
    CGameTrace trace;
    CTraceFilterHitscan filter;
    filter.pSkip = pLocal;
    
    SDK::Trace(vEyePos, vTargetPos, MASK_VISIBLE, &filter, &trace);
    
    return trace.m_pEnt == pEntity;
}

SentryAimData CSentryESP::GetSentryAimData(CBaseEntity* pSentry)
{
    SentryAimData data;
    
    if (!pSentry)
        return data;
        
    auto pSentryObj = pSentry->As<CObjectSentrygun>();
    if (!pSentryObj)
        return data;
        
    // Get bone matrices
    matrix3x4 aBones[MAXSTUDIOBONES];
    if (!pSentry->SetupBones(aBones, MAXSTUDIOBONES, BONE_USED_BY_ANYTHING, I::GlobalVars->curtime))
        return data;
        
    // Determine turret bone index based on level
    int level = pSentryObj->m_iUpgradeLevel();
    int turretBoneIndex = 1; // Default
    
    if (level == 1)
        turretBoneIndex = 2;
    else if (level == 2)
        turretBoneIndex = 3;
        
    // Get muzzle position and angles from bone matrix
    data.MuzzlePos = GetPositionFromMatrix(aBones[turretBoneIndex]);
    data.AimAngles = GetAnglesFromMatrix(aBones[turretBoneIndex]);
    
    // Get target entity
    data.Target = pSentryObj->m_hEnemy().Get();
    
    // If we have a target, aim at it
    if (data.Target && data.Target->IsPlayer())
    {
        Vec3 vTargetPos = data.Target->GetAbsOrigin();
        vTargetPos.z += 40; // Aim at chest height
        
        Vec3 vDirection = vTargetPos - data.MuzzlePos;
        data.AimAngles = Math::VectorAngles(vDirection);
    }
    
    data.IsValid = true;
    return data;
}

Vec3 CSentryESP::GetAimLineEndPos(const Vec3& startPos, const Vec3& angles)
{
    Vec3 vForward;
    Math::AngleVectors(angles, &vForward);
    return startPos + (vForward * Vars::Competitive::SentryESP::LineLength.Value);
}

void CSentryESP::DrawESPBox(int x1, int y1, int x2, int y2, Color_t color)
{
    if (Vars::Competitive::SentryESP::UseCorners.Value)
        DrawCornerBox(x1, y1, x2, y2, color);
    else
    {
        for (int i = 0; i < Vars::Competitive::SentryESP::BoxThickness.Value; i++)
        {
            H::Draw.LineRect(x1 - i, y1 - i, x2 - x1 + 2 * i, y2 - y1 + 2 * i, color);
        }
    }
}

void CSentryESP::DrawCornerBox(int x1, int y1, int x2, int y2, Color_t color)
{
    int length = std::min({Vars::Competitive::SentryESP::CornerLength.Value, 
                          std::abs(x2 - x1) / 3, std::abs(y2 - y1) / 3});
    
    for (int i = 0; i < Vars::Competitive::SentryESP::BoxThickness.Value; i++)
    {
        // Top Left
        H::Draw.Line(x1, y1 + i, x1 + length, y1 + i, color);
        H::Draw.Line(x1 + i, y1, x1 + i, y1 + length, color);
        
        // Top Right
        H::Draw.Line(x2 - length, y1 + i, x2, y1 + i, color);
        H::Draw.Line(x2 - i, y1, x2 - i, y1 + length, color);
        
        // Bottom Left
        H::Draw.Line(x1, y2 - i, x1 + length, y2 - i, color);
        H::Draw.Line(x1 + i, y2 - length, x1 + i, y2, color);
        
        // Bottom Right
        H::Draw.Line(x2 - length, y2 - i, x2, y2 - i, color);
        H::Draw.Line(x2 - i, y2 - length, x2 - i, y2, color);
    }
}

Vec3 CSentryESP::GetPositionFromMatrix(const matrix3x4& matrix)
{
    return Vec3(matrix[0][3], matrix[1][3], matrix[2][3]);
}

Vec3 CSentryESP::GetAnglesFromMatrix(const matrix3x4& matrix)
{
    Vec3 forward(matrix[0][2], matrix[1][2], matrix[2][2]);
    return Math::VectorAngles(forward);
}

SentryAimData CSentryESP::GetInterpolatedAimData(int sentryIndex, const SentryAimData& currentData)
{
    float currentTime = I::GlobalVars->curtime;
    constexpr float INTERPOLATION_TIME = 0.1f; // 100ms interpolation window
    
    auto& interpData = m_InterpolationData[sentryIndex];
    
    if (!interpData.HasValidData)
    {
        // First time seeing this sentry, initialize with current data
        interpData.LastMuzzlePos = currentData.MuzzlePos;
        interpData.LastAimAngles = currentData.AimAngles;
        interpData.LastUpdateTime = currentTime;
        interpData.HasValidData = true;
        return currentData;
    }
    
    // Calculate interpolation factor
    float timeDelta = currentTime - interpData.LastUpdateTime;
    float lerpFactor = std::min(timeDelta / INTERPOLATION_TIME, 1.0f);
    
    // Create interpolated result
    SentryAimData result = currentData;
    result.MuzzlePos = LerpVec3(interpData.LastMuzzlePos, currentData.MuzzlePos, lerpFactor);
    result.AimAngles = LerpAngles(interpData.LastAimAngles, currentData.AimAngles, lerpFactor);
    
    // Update stored data for next frame
    interpData.LastMuzzlePos = result.MuzzlePos;
    interpData.LastAimAngles = result.AimAngles;
    interpData.LastUpdateTime = currentTime;
    
    return result;
}

Vec3 CSentryESP::LerpVec3(const Vec3& from, const Vec3& to, float t)
{
    return Vec3(
        from.x + (to.x - from.x) * t,
        from.y + (to.y - from.y) * t,
        from.z + (to.z - from.z) * t
    );
}

Vec3 CSentryESP::LerpAngles(const Vec3& from, const Vec3& to, float t)
{
    Vec3 result;
    
    // Lerp each angle component, handling wrap-around for yaw
    result.x = from.x + (to.x - from.x) * t;
    result.z = from.z + (to.z - from.z) * t;
    
    // Handle yaw wrap-around (y component)
    float yawDiff = to.y - from.y;
    
    // Normalize angle difference to [-180, 180]
    while (yawDiff > 180.0f)
        yawDiff -= 360.0f;
    while (yawDiff < -180.0f)
        yawDiff += 360.0f;
    
    result.y = from.y + yawDiff * t;
    
    // Normalize final yaw
    while (result.y > 180.0f)
        result.y -= 360.0f;
    while (result.y < -180.0f)
        result.y += 360.0f;
    
    return result;
}