#include "OffScreenIndicators.h"
#include "../../../SDK/SDK.h"
#include "../../Players/PlayerUtils.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


bool COffScreenIndicators::IsOnScreen(CBaseEntity* entity)
{
    Vec3 w2s;
    if (SDK::W2S(entity->GetAbsOrigin(), w2s))
    {
        int screenW, screenH;
        I::MatSystemSurface->GetScreenSize(screenW, screenH);
        
        // Check if the entity is within screen bounds
        return (w2s.x >= 0 && w2s.x <= screenW && w2s.y >= 0 && w2s.y <= screenH);
    }
    return false;
}

std::string COffScreenIndicators::GetPlayerSteamID(CTFPlayer* pPlayer)
{
    if (!pPlayer)
        return "";
        
    PlayerInfo_t playerInfo;
    if (I::EngineClient->GetPlayerInfo(pPlayer->entindex(), &playerInfo))
    {
        if (std::string(playerInfo.guid) == "BOT")
            return "";
        return std::string(playerInfo.guid);
    }
    return "";
}

Color_t COffScreenIndicators::GenerateColor(const std::string& steamID)
{
    if (m_ColorCache.find(steamID) != m_ColorCache.end())
        return m_ColorCache[steamID];
    
    // Simple hash function for color generation (same as MarkSpot/PlayerTrails)
    uint32_t hash = 0;
    for (char c : steamID)
    {
        hash += static_cast<uint32_t>(c);
    }
    
    int r = static_cast<int>((hash * 11) % 200 + 55);
    int g = static_cast<int>((hash * 23) % 200 + 55);
    int b = static_cast<int>((hash * 37) % 200 + 55);
    
    Color_t color = {static_cast<byte>(r), static_cast<byte>(g), static_cast<byte>(b), 255};
    m_ColorCache[steamID] = color;
    return color;
}

void COffScreenIndicators::DrawArrow(int centerX, int centerY, float angle, const Color_t& color, int size)
{
    // Calculate arrow points based on angle
    float halfSize = size * 0.5f;
    
    // Arrow points (pointing right initially)
    Vec3 tip = {static_cast<float>(centerX) + halfSize, static_cast<float>(centerY), 0.0f};
    Vec3 base1 = {static_cast<float>(centerX) - halfSize, static_cast<float>(centerY) - halfSize * 0.6f, 0.0f};
    Vec3 base2 = {static_cast<float>(centerX) - halfSize, static_cast<float>(centerY) + halfSize * 0.6f, 0.0f};
    
    // Rotate points around center
    float cosA = std::cos(angle);
    float sinA = std::sin(angle);
    
    auto rotatePoint = [&](Vec3& point) {
        float x = point.x - static_cast<float>(centerX);
        float y = point.y - static_cast<float>(centerY);
        point.x = static_cast<float>(centerX) + (x * cosA - y * sinA);
        point.y = static_cast<float>(centerY) + (x * sinA + y * cosA);
    };
    
    rotatePoint(tip);
    rotatePoint(base1);
    rotatePoint(base2);
    
    // Draw arrow using lines
    H::Draw.Line(static_cast<int>(tip.x), static_cast<int>(tip.y),
                static_cast<int>(base1.x), static_cast<int>(base1.y), color);
    H::Draw.Line(static_cast<int>(tip.x), static_cast<int>(tip.y),
                static_cast<int>(base2.x), static_cast<int>(base2.y), color);
    H::Draw.Line(static_cast<int>(base1.x), static_cast<int>(base1.y),
                static_cast<int>(base2.x), static_cast<int>(base2.y), color);
}

void COffScreenIndicators::Draw()
{
    // Early exits
    if (!Vars::Competitive::Features::OffScreenIndicators.Value)
        return;
        
    if (I::EngineVGui->IsGameUIVisible())
        return;
    
    auto pLocal = H::Entities.GetLocal();
    if (!pLocal)
        return;
    
    // Get screen dimensions
    int screenW, screenH;
    I::MatSystemSurface->GetScreenSize(screenW, screenH);
    
    // Get local player's view angle for directional calculations
    Vec3 realAngle = I::EngineClient->GetViewAngles();
    float yaw = realAngle.y * 3.14159265358979323846f / 180.0f;
    
    // Set font for text rendering
    auto font = H::Fonts.GetFont(FONT_ESP);
    
    // Process all players
    for (auto pEntity : H::Entities.GetGroup(EGroupType::PLAYERS_ALL))
    {
        auto pPlayer = pEntity->As<CTFPlayer>();
        if (!pPlayer || pPlayer->IsDormant() || !pPlayer->IsAlive())
            continue;
            
        // Skip local player and teammates
        if (pPlayer == pLocal || pPlayer->m_iTeamNum() == pLocal->m_iTeamNum())
            continue;
        
        // Check if player is off-screen
        if (IsOnScreen(pPlayer))
            continue;
        
        // Calculate directional position relative to local player
        Vec3 localPos = pLocal->GetAbsOrigin();
        Vec3 playerPos = pPlayer->GetAbsOrigin();
        Vec3 positionDiff = localPos - playerPos;
        
        float x = std::cos(yaw) * positionDiff.y - std::sin(yaw) * positionDiff.x;
        float y = std::cos(yaw) * positionDiff.x + std::sin(yaw) * positionDiff.y;
        float len = std::sqrt(x * x + y * y);
        
        if (len == 0)
            continue;
        
        // Normalize direction
        x = x / len;
        y = y / len;
        
        // Position indicator 200 pixels from screen center
        float pos1 = screenW / 2.0f + x * 200.0f;
        float pos2 = screenH / 2.0f + y * 200.0f;
        
        if (pos1 != pos1 || pos2 != pos2)  // NaN check
            continue;
        
        int finalX = static_cast<int>(std::floor(pos1));
        int finalY = static_cast<int>(std::floor(pos2));
        
        // Get player name
        std::string playerName = "Player " + std::to_string(pPlayer->entindex());
        PlayerInfo_t pi{};
        if (I::EngineClient->GetPlayerInfo(pPlayer->entindex(), &pi))
        {
            playerName = F::PlayerUtils.GetPlayerName(pPlayer->entindex(), pi.name);
        }
        
        // Get health info
        int health = pPlayer->m_iHealth();
        std::string healthText = std::to_string(health);
        
        // Calculate distance for alpha
        float distance = len;
        int alpha = static_cast<int>(std::max(0.0f, 255.0f - (distance * 0.05f)));
        
        // Get color based on player Steam ID
        std::string steamID = GetPlayerSteamID(pPlayer);
        Color_t playerColor = {255, 255, 255, static_cast<byte>(alpha)};
        if (!steamID.empty())
        {
            playerColor = GenerateColor(steamID);
            playerColor.a = static_cast<byte>(alpha);
        }
        
        // Calculate angle pointing toward the target (opposite of the direction to indicator)
        float directionAngle = std::atan2(y, x); // Direction from indicator toward target
        
        // Draw arrow pointing toward the player
        DrawArrow(finalX, finalY, directionAngle, playerColor, 15);
        
        // Draw player name below arrow
        Vec2 nameSize = H::Draw.GetTextSize(playerName.c_str(), font);
        H::Draw.String(font, finalX - static_cast<int>(nameSize.x) / 2, finalY + 20, {255, 255, 255, static_cast<byte>(alpha)}, ALIGN_TOPLEFT, playerName.c_str());
        
        // Draw health below name
        H::Draw.String(font, finalX - static_cast<int>(nameSize.x) / 2, finalY + 35, {0, 255, 0, static_cast<byte>(alpha)}, ALIGN_TOPLEFT, healthText.c_str());
    }
}