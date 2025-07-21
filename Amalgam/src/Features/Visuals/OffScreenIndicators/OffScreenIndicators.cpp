#include "OffScreenIndicators.h"
#include "../../../SDK/SDK.h"
#include "../../Players/PlayerUtils.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Static member initialization
int COffScreenIndicators::m_iFallbackAvatarTexture = 0;


bool COffScreenIndicators::IsOnScreen(CBaseEntity* entity)
{
    Vec3 w2s;
    // Use extended bounds checking like FOV arrows for better accuracy
    if (SDK::W2S(entity->GetAbsOrigin(), w2s, true))
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

std::string COffScreenIndicators::GetPlayerClassName(int classId)
{
    switch (classId)
    {
        case 1: return "Scout";
        case 2: return "Sniper";
        case 3: return "Soldier";
        case 4: return "Demoman";
        case 5: return "Medic";
        case 6: return "Heavy";
        case 7: return "Pyro";
        case 8: return "Spy";
        case 9: return "Engineer";
        default: return "Unknown";
    }
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
    // Use FOV arrows drawing logic for better accuracy
    Vec2 vCenter = { static_cast<float>(centerX), static_cast<float>(centerY) };
    
    // Convert angle to match FOV arrows calculation
    const float flDeg = DEG2RAD(angle * 180.0f / M_PI);
    const float flCos = cos(flDeg);
    const float flSin = sin(flDeg);

    float flScale = size;
    Vec2 v1 = { -flScale, flScale / 2 },
        v2 = { -flScale, -flScale / 2 },
        v3 = { -flScale * sqrt(3.f) / 2, 0 };
    
    // Use filled polygon like FOV arrows for better visibility
    H::Draw.FillPolygon(
        {
            { { vCenter.x + v1.x * flCos - v1.y * flSin, vCenter.y + v1.y * flCos + v1.x * flSin } },
            { { vCenter.x + v2.x * flCos - v2.y * flSin, vCenter.y + v2.y * flCos + v2.x * flSin } },
            { { vCenter.x + v3.x * flCos - v3.y * flSin, vCenter.y + v3.y * flCos + v3.x * flSin } }
        }, color
    );
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
        
        // Use more accurate FOV arrows screen detection
        Vec3 vScreenPos;
        bool isOnScreen = SDK::W2S(pPlayer->GetCenter(), vScreenPos, true);
        if (isOnScreen) {
            // Additional screen bounds check like FOV arrows
            if (vScreenPos.x >= 0 && vScreenPos.x <= screenW && vScreenPos.y >= 0 && vScreenPos.y <= screenH)
                continue;
        }
        
        // Use FOV arrows positioning logic for better accuracy
        Vec2 vCenter = { screenW / 2.f, screenH / 2.f };
        Vec3 vLocalPos = pLocal->GetEyePosition();
        Vec3 vPlayerPos = pPlayer->GetCenter();
        
        // Calculate angle between screen center and target
        Vec3 vAngle = Math::VectorAngles({ vCenter.x - vScreenPos.x, vCenter.y - vScreenPos.y, 0 });
        float angle = vAngle.y * M_PI / 180.0f;
        
        // Position indicator at configurable distance from screen center (like FOV arrows offset)
        float flOffset = Vars::Competitive::Features::OffScreenIndicatorsRange.Value;
        int finalX = static_cast<int>(vCenter.x + cos(angle) * flOffset);
        int finalY = static_cast<int>(vCenter.y + sin(angle) * flOffset);
        
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
        
        // Calculate distance for alpha using FOV arrows logic
        float distance = vLocalPos.DistTo(vPlayerPos);
        float flMaxDistance = 1000.0f; // Similar to FOV arrows max distance
        float flMap = Math::RemapVal(distance, flMaxDistance, flMaxDistance * 0.9f, 0.f, 1.f);
        int baseAlpha = Vars::Competitive::Features::OffScreenIndicatorsAlpha.Value;
        int alpha = static_cast<int>(flMap * baseAlpha);
        
        // Additional screen-based transparency like FOV arrows
        if (isOnScreen) {
            float flMin = std::min(vCenter.x, vCenter.y);
            float flMax = std::max(vCenter.x, vCenter.y);
            float flDist = sqrt(powf(vScreenPos.x - vCenter.x, 2) + powf(vScreenPos.y - vCenter.y, 2));
            float flTransparency = 1.f - std::clamp((flDist - flMin) / (flMin != flMax ? flMax - flMin : 1), 0.f, 1.f);
            alpha = static_cast<int>(std::max(float(alpha) - flTransparency * baseAlpha, 0.f));
        }
        
        // Early exit if alpha is too low (like FOV arrows)
        if (!alpha)
            continue;
        
        // Get color based on player Steam ID
        std::string steamID = GetPlayerSteamID(pPlayer);
        Color_t playerColor = {255, 255, 255, static_cast<byte>(alpha)};
        if (!steamID.empty())
        {
            playerColor = GenerateColor(steamID);
            playerColor.a = static_cast<byte>(alpha);
        }
        
        // Use the angle calculated from FOV arrows logic
        float directionAngle = angle;
        
        // Draw indicator - either Steam avatar or arrow
        int arrowSize = Vars::Competitive::Features::OffScreenIndicatorsSize.Value;
        
        if (Vars::Competitive::Features::OffScreenIndicatorsAvatars.Value)
        {
            int avatarSize = arrowSize * 2; // Make avatar bigger than arrow for visibility
            bool avatarDrawn = false;
            
            // Calculate avatar position
            int avatarX = finalX - avatarSize/2;
            int avatarY = finalY - avatarSize/2;
            
            if (pi.friendsID != 0)
            {
                // Draw 1-pixel Steam ID color border around Steam avatar
                H::Draw.LineRect(avatarX - 1, avatarY - 1, avatarSize + 2, avatarSize + 2, playerColor);
                
                // Draw Steam avatar for real players
                H::Draw.Avatar(avatarX, avatarY, avatarSize, avatarSize, pi.friendsID, ALIGN_TOPLEFT);
                avatarDrawn = true;
            }
            
            if (!avatarDrawn)
            {
                // Fallback to embedded avatar for bots/players without Steam ID or failed Steam avatars
                int fallbackTexture = GetFallbackAvatarTexture();
                if (fallbackTexture > 0)
                {
                    // Draw 1-pixel Steam ID color border around embedded avatar
                    H::Draw.LineRect(avatarX - 1, avatarY - 1, avatarSize + 2, avatarSize + 2, playerColor);
                    
                    // Draw the embedded avatar texture
                    I::MatSystemSurface->DrawSetTexture(fallbackTexture);
                    I::MatSystemSurface->DrawSetColor(255, 255, 255, 255);
                    I::MatSystemSurface->DrawTexturedRect(avatarX, avatarY, avatarX + avatarSize, avatarY + avatarSize);
                    avatarDrawn = true;
                }
            }
            
            if (!avatarDrawn)
            {
                // Final fallback to arrow if entire avatar system failed
                DrawArrow(finalX, finalY, directionAngle, playerColor, arrowSize);
            }
        }
        else
        {
            // Use arrows when avatar mode is disabled
            DrawArrow(finalX, finalY, directionAngle, playerColor, arrowSize);
        }
        
        // Draw information with toggleable options
        int currentY = finalY + (Vars::Competitive::Features::OffScreenIndicatorsAvatars.Value ? arrowSize * 2 : arrowSize) + 5;
        
        // Draw player name if enabled
        if (Vars::Competitive::Features::OffScreenIndicatorsName.Value)
        {
            Vec2 nameSize = H::Draw.GetTextSize(playerName.c_str(), font);
            H::Draw.String(font, finalX - static_cast<int>(nameSize.x) / 2, currentY, {255, 255, 255, static_cast<byte>(alpha)}, ALIGN_TOPLEFT, playerName.c_str());
            currentY += 15;
        }
        
        // Draw player class if enabled
        if (Vars::Competitive::Features::OffScreenIndicatorsClass.Value)
        {
            std::string className = GetPlayerClassName(pPlayer->m_iClass());
            Vec2 classSize = H::Draw.GetTextSize(className.c_str(), font);
            H::Draw.String(font, finalX - static_cast<int>(classSize.x) / 2, currentY, {255, 255, 0, static_cast<byte>(alpha)}, ALIGN_TOPLEFT, className.c_str());
            currentY += 15;
        }
        
        // Draw health if enabled
        if (Vars::Competitive::Features::OffScreenIndicatorsHealth.Value)
        {
            Vec2 healthSize = H::Draw.GetTextSize(healthText.c_str(), font);
            H::Draw.String(font, finalX - static_cast<int>(healthSize.x) / 2, currentY, {0, 255, 0, static_cast<byte>(alpha)}, ALIGN_TOPLEFT, healthText.c_str());
            currentY += 15;
        }
        
        // Draw distance if enabled
        if (Vars::Competitive::Features::OffScreenIndicatorsDistance.Value)
        {
            std::string distanceText = std::to_string(static_cast<int>(distance)) + "u";
            Vec2 distSize = H::Draw.GetTextSize(distanceText.c_str(), font);
            H::Draw.String(font, finalX - static_cast<int>(distSize.x) / 2, currentY, {200, 200, 200, static_cast<byte>(alpha)}, ALIGN_TOPLEFT, distanceText.c_str());
        }
    }
}

std::vector<unsigned char> COffScreenIndicators::DecodeBase64(const std::string& base64)
{
    // Simple Base64 decoder
    static const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::vector<unsigned char> decoded;
    
    int padding = 0;
    if (base64.length() >= 2)
    {
        if (base64[base64.length()-1] == '=') padding++;
        if (base64[base64.length()-2] == '=') padding++;
    }
    
    for (size_t i = 0; i < base64.length(); i += 4)
    {
        unsigned int temp = 0;
        for (int j = 0; j < 4; j++)
        {
            if (i + j < base64.length() && base64[i + j] != '=')
            {
                size_t pos = chars.find(base64[i + j]);
                if (pos != std::string::npos)
                    temp = (temp << 6) | static_cast<unsigned int>(pos);
                else
                    temp <<= 6;
            }
            else
            {
                temp <<= 6;
            }
        }
        
        decoded.push_back((temp >> 16) & 0xFF);
        if (padding < 2) decoded.push_back((temp >> 8) & 0xFF);
        if (padding < 1) decoded.push_back(temp & 0xFF);
    }
    
    return decoded;
}

int COffScreenIndicators::GetFallbackAvatarTexture()
{
    if (m_iFallbackAvatarTexture <= 0)
    {
        // Embedded Base64 JPEG avatar data
        const std::string avatarB64 = "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";
        
        try
        {
            // Decode the Base64 JPEG data
            auto jpegData = DecodeBase64(avatarB64);
            
            if (!jpegData.empty())
            {
                // Create texture from JPEG data using the existing Steam avatar system
                // Note: This is a simplified approach. In practice, you'd need to decode JPEG to RGBA
                // For now, we'll create a simple colored texture as fallback
                
                // Create a simple 32x32 colored texture as fallback
                const int size = 32;
                std::vector<unsigned char> rgbaData(size * size * 4);
                
                // Fill with a default avatar color (gray)
                for (int i = 0; i < size * size; i++)
                {
                    rgbaData[i * 4 + 0] = 128; // R
                    rgbaData[i * 4 + 1] = 128; // G
                    rgbaData[i * 4 + 2] = 128; // B
                    rgbaData[i * 4 + 3] = 255; // A
                }
                
                // Create texture using the same method as Steam avatars
                if (I::MatSystemSurface)
                {
                    m_iFallbackAvatarTexture = I::MatSystemSurface->CreateNewTextureID(true);
                    I::MatSystemSurface->DrawSetTextureRGBA(m_iFallbackAvatarTexture, rgbaData.data(), size, size, 0, false);
                }
            }
        }
        catch (...)
        {
            // Fallback to invalid texture ID if something goes wrong
            m_iFallbackAvatarTexture = -1;
        }
    }
    
    return m_iFallbackAvatarTexture > 0 ? m_iFallbackAvatarTexture : -1;
}