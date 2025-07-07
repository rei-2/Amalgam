#include "MarkSpot.h"
#include "../../../SDK/SDK.h"
#include "../../Chat/Chat.h"
#include <algorithm>
#include <format>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

std::string CMarkSpot::GetCurrentMapName()
{
    auto pEngine = I::EngineClient;
    if (!pEngine)
        return "";
        
    std::string fullPath = pEngine->GetLevelName();
    if (fullPath.empty())
        return "";
    
    // Extract just the map name from full path (e.g., "maps/cp_sunshine.bsp" -> "cp_sunshine")
    size_t lastSlash = fullPath.find_last_of('/');
    if (lastSlash != std::string::npos)
        fullPath = fullPath.substr(lastSlash + 1);
    
    // Remove .bsp extension
    size_t bspPos = fullPath.find(".bsp");
    if (bspPos != std::string::npos)
        fullPath = fullPath.substr(0, bspPos);
    
    return fullPath;
}

std::string CMarkSpot::GetLocalSteamID()
{
    auto pLocal = H::Entities.GetLocal();
    if (!pLocal)
        return "";
    
    PlayerInfo_t playerInfo;
    if (I::EngineClient->GetPlayerInfo(pLocal->entindex(), &playerInfo))
    {
        if (std::string(playerInfo.guid) == "BOT")
            return "";
        return std::string(playerInfo.guid);
    }
    return "";
}

Color_t CMarkSpot::GenerateColor(const std::string& steamID)
{
    if (m_ColorCache.find(steamID) != m_ColorCache.end())
        return m_ColorCache[steamID];
    
    // Simple hash function for color generation (same as PlayerTrails)
    uint32_t hash = 0;
    for (char c : steamID)
    {
        hash += static_cast<uint32_t>(c);
    }
    
    int r = static_cast<int>((hash * 11) % 200 + 55);
    int g = static_cast<int>((hash * 23) % 200 + 55);
    int b = static_cast<int>((hash * 37) % 200 + 55);
    
    Color_t color = {static_cast<byte>(r), static_cast<byte>(g), static_cast<byte>(b), 200};
    m_ColorCache[steamID] = color;
    return color;
}

Vec3 CMarkSpot::GetAimPosition()
{
    auto pLocal = H::Entities.GetLocal();
    if (!pLocal)
        return Vec3(0, 0, 0);
    
    // Get view angles and eye position
    Vec3 vEyePos = pLocal->GetEyePosition();
    Vec3 vViewAngles = I::EngineClient->GetViewAngles();
    
    // Convert angles to direction vector
    Vec3 vForward, vRight, vUp;
    Math::AngleVectors(vViewAngles, &vForward, &vRight, &vUp);
    
    // Trace forward to find aim position
    Vec3 vEndPos = vEyePos + (vForward * 8192.0f); // Max trace distance
    
    CGameTrace trace;
    CTraceFilterHitscan filter;
    filter.pSkip = pLocal;
    
    SDK::Trace(vEyePos, vEndPos, MASK_SOLID, &filter, &trace);
    
    return trace.endpos;
}

std::string CMarkSpot::GenerateMarkId(const MarkSpotInfo& mark)
{
    // Use color as key to allow only one mark per player
    return std::format("{}_{}_{}_{}", 
        mark.MapName, mark.Color.r, mark.Color.g, mark.Color.b);
}

void CMarkSpot::CleanupExpiredMarks()
{
    float currentTime = I::GlobalVars->curtime;
    
    for (auto it = m_MarkSpots.begin(); it != m_MarkSpots.end();)
    {
        if (currentTime - it->second.CreatedTime > Vars::Competitive::MarkSpot::MarkDuration.Value)
        {
            it = m_MarkSpots.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

bool CMarkSpot::IsVisible(const Vec3& fromPos, const Vec3& targetPos)
{
    CGameTrace trace;
    CTraceFilterHitscan filter;
    filter.pSkip = H::Entities.GetLocal();
    SDK::Trace(fromPos, targetPos, MASK_VISIBLE, &filter, &trace);
    
    return trace.fraction >= 0.99f;
}

void CMarkSpot::DrawGroundCircle(const Vec3& position, const Color_t& color)
{
    auto pLocal = H::Entities.GetLocal();
    if (!pLocal)
        return;
    
    Vec3 eyePos = pLocal->GetEyePosition();
    
    // Check if center is visible (simple check)
    if (!Vars::Competitive::MarkSpot::ShowThroughWalls.Value && !IsVisible(eyePos, position))
        return;
    
    // Create vertices for the filled circle on the ground
    std::vector<Vertex_t> vertices;
    
    // Add center point
    Vec3 centerScreen;
    if (!SDK::W2S(position, centerScreen))
        return;
    
    vertices.emplace_back(Vertex_t({ { centerScreen.x, centerScreen.y } }));
    
    // Add edge vertices
    int segments = Vars::Competitive::MarkSpot::CircleSegments.Value;
    int radius = Vars::Competitive::MarkSpot::CircleRadius.Value;
    
    for (int i = 0; i <= segments; i++)
    {
        float angle = i * (2.0f * M_PI / segments);
        
        Vec3 worldPos = {
            position.x + radius * cos(angle),
            position.y + radius * sin(angle),
            position.z + 2.0f // Slightly above ground
        };
        
        Vec3 screenPos;
        if (SDK::W2S(worldPos, screenPos))
        {
            vertices.emplace_back(Vertex_t({ { screenPos.x, screenPos.y } }));
        }
        else
        {
            return; // Can't draw if any point is off-screen
        }
    }
    
    // Draw filled polygon with configurable alpha
    if (vertices.size() >= 3)
    {
        Color_t circleColor = color;
        circleColor.a = Vars::Competitive::MarkSpot::CircleAlpha.Value;
        H::Draw.FillPolygon(vertices, circleColor);
    }
}

void CMarkSpot::DrawPylon(const Vec3& basePosition, const Color_t& color)
{
    auto pLocal = H::Entities.GetLocal();
    if (!pLocal)
        return;
    
    Vec3 eyePos = pLocal->GetEyePosition();
    
    // Get configurable values
    int pylonSegments = Vars::Competitive::MarkSpot::PylonSegments.Value;
    float pylonHeight = Vars::Competitive::MarkSpot::PylonHeight.Value;
    
    // Check if any segment is visible
    bool anySegmentVisible = false;
    for (int i = 0; i <= pylonSegments; i++)
    {
        Vec3 segmentPos = Vec3(basePosition.x, basePosition.y, 
            basePosition.z + (i * (pylonHeight / pylonSegments)));
        
        if (Vars::Competitive::MarkSpot::ShowThroughWalls.Value || IsVisible(eyePos, segmentPos))
        {
            anySegmentVisible = true;
            break;
        }
    }
    
    if (!anySegmentVisible)
        return;
    
    // Draw pylon segments
    Vec3 lastScreenPos;
    bool hasLastScreenPos = false;
    
    int startAlpha = Vars::Competitive::MarkSpot::PylonStartAlpha.Value;
    int endAlpha = Vars::Competitive::MarkSpot::PylonEndAlpha.Value;
    
    for (int i = 0; i <= pylonSegments; i++)
    {
        Vec3 segmentPos = Vec3(basePosition.x, basePosition.y, 
            basePosition.z + (i * (pylonHeight / pylonSegments)));
        
        bool visible = Vars::Competitive::MarkSpot::ShowThroughWalls.Value || IsVisible(eyePos, segmentPos);
        
        Vec3 screenPos;
        if (SDK::W2S(segmentPos, screenPos))
        {
            if (visible && hasLastScreenPos)
            {
                // Calculate alpha based on height with configurable values
                float progress = static_cast<float>(i) / pylonSegments;
                int alpha = static_cast<int>(startAlpha - (progress * (startAlpha - endAlpha)));
                
                Color_t segmentColor = color;
                segmentColor.a = alpha;
                
                // Draw line with configurable width by drawing multiple offset lines
                int pylonWidth = Vars::Competitive::MarkSpot::PylonWidth.Value;
                int halfWidth = pylonWidth / 2;
                
                for (int w = -halfWidth; w <= halfWidth; w++)
                {
                    for (int h = -halfWidth; h <= halfWidth; h++)
                    {
                        if (w == 0 && h == 0)
                        {
                            // Main line
                            H::Draw.Line(static_cast<int>(lastScreenPos.x), static_cast<int>(lastScreenPos.y),
                                       static_cast<int>(screenPos.x), static_cast<int>(screenPos.y), segmentColor);
                        }
                        else if (pylonWidth > 1)
                        {
                            // Offset lines for thickness
                            H::Draw.Line(static_cast<int>(lastScreenPos.x + w), static_cast<int>(lastScreenPos.y + h),
                                       static_cast<int>(screenPos.x + w), static_cast<int>(screenPos.y + h), segmentColor);
                        }
                    }
                }
            }
            
            if (visible)
            {
                lastScreenPos = screenPos;
                hasLastScreenPos = true;
            }
        }
    }
}

void CMarkSpot::SendMarkSpotMessage(const MarkSpotInfo& mark)
{
    // Format: !MARK:mapname:x:y:z:r:g:b
    std::string message = std::format("!MARK:{}:{:.1f}:{:.1f}:{:.1f}:{}:{}:{}", 
        mark.MapName, mark.Position.x, mark.Position.y, mark.Position.z, 
        mark.Color.r, mark.Color.g, mark.Color.b);
    
    // Send through Matrix chat
    if (F::Chat.IsConnected())
    {
        F::Chat.SendMessage(message);
    }
}

void CMarkSpot::HandleInput()
{
    if (!Vars::Competitive::Features::MarkSpot.Value)
        return;
    
    // Only allow input when properly in-game
    if (I::EngineVGui->IsGameUIVisible())
        return;
        
    auto pLocal = H::Entities.GetLocal();
    if (!pLocal || !pLocal->IsAlive())
        return;
    
    // Check if E key is pressed
    bool ePressed = GetAsyncKeyState('E') & 0x8000;
    
    // Only trigger on key press (not hold)
    if (ePressed && !m_bLastEPressed)
    {
        // Check if Matrix is connected
        if (!F::Chat.IsConnected())
        {
            m_bLastEPressed = ePressed;
            return;
        }
        
        // Rate limiting to respect Matrix spec and prevent spam
        float currentTime = I::GlobalVars->curtime;
        if (currentTime - m_fLastMessageTime < Vars::Competitive::MarkSpot::RateLimit.Value)
        {
            m_bLastEPressed = ePressed;
            return; // Too soon, skip this request
        }
        
        // Get current aim position and Steam ID
        Vec3 aimPos = GetAimPosition();
        std::string mapName = GetCurrentMapName();
        std::string steamID = GetLocalSteamID();
        
        if (mapName.empty() || steamID.empty())
        {
            m_bLastEPressed = ePressed;
            return;
        }
        
        // Create mark spot info
        MarkSpotInfo mark;
        mark.Position = aimPos;
        mark.MapName = mapName;
        mark.CreatedTime = I::GlobalVars->curtime;
        mark.SenderSteamID = steamID;
        mark.Color = GenerateColor(steamID);
        
        // Send Matrix message
        SendMarkSpotMessage(mark);
        
        // Update rate limiting timestamp
        m_fLastMessageTime = currentTime;
        
        // Add to local storage (will be replaced when we receive our own message)
        AddMarkSpot(mark);
    }
    
    m_bLastEPressed = ePressed;
}

void CMarkSpot::AddMarkSpot(const MarkSpotInfo& mark)
{
    std::string markId = GenerateMarkId(mark);
    
    // This will automatically replace any existing mark from the same player (same color)
    // since we're using color as the key
    m_MarkSpots[markId] = mark;
}

bool CMarkSpot::IsMarkSpotMessage(const std::string& message)
{
    return message.starts_with("!MARK:");
}

void CMarkSpot::ProcessMatrixMessage(const std::string& sender, const std::string& message)
{
    if (!IsMarkSpotMessage(message))
        return;
    
    // Parse message format: !MARK:mapname:x:y:z:r:g:b
    std::vector<std::string> parts;
    size_t start = 0;
    size_t end = 0;
    
    while ((end = message.find(':', start)) != std::string::npos)
    {
        parts.push_back(message.substr(start, end - start));
        start = end + 1;
    }
    parts.push_back(message.substr(start)); // Last part
    
    if (parts.size() != 8) // !MARK, mapname, x, y, z, r, g, b
        return;
    
    try
    {
        MarkSpotInfo mark;
        mark.MapName = parts[1];
        mark.Position.x = std::stof(parts[2]);
        mark.Position.y = std::stof(parts[3]);
        mark.Position.z = std::stof(parts[4]);
        mark.Color.r = static_cast<byte>(std::stoi(parts[5]));
        mark.Color.g = static_cast<byte>(std::stoi(parts[6]));
        mark.Color.b = static_cast<byte>(std::stoi(parts[7]));
        mark.Color.a = 200; // Fixed alpha
        mark.CreatedTime = I::GlobalVars->curtime;
        mark.SenderSteamID = ""; // Not needed anymore
        
        AddMarkSpot(mark);
    }
    catch (const std::exception&)
    {
        // Invalid coordinates or colors, ignore message
    }
}

void CMarkSpot::Draw()
{
    if (!Vars::Competitive::Features::MarkSpot.Value)
        return;
        
    if (I::EngineVGui->IsGameUIVisible())
        return;
    
    auto pLocal = H::Entities.GetLocal();
    if (!pLocal || !pLocal->IsAlive())
        return;
    
    // Cleanup expired marks
    CleanupExpiredMarks();
    
    // Get current map name for filtering
    std::string currentMap = GetCurrentMapName();
    if (currentMap.empty())
        return;
    
    // Draw all marks for current map
    for (const auto& [markId, mark] : m_MarkSpots)
    {
        if (mark.MapName != currentMap)
            continue;
        
        DrawGroundCircle(mark.Position, mark.Color);
        DrawPylon(mark.Position, mark.Color);
    }
}