#include "MarkSpot.h"
#include "../../../SDK/SDK.h"
#include "../../Chat/Chat.h"
#include "../../Misc/SpectateAll/SpectateAll.h"
#include <algorithm>

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
    
    Vec3 vEyePos;
    Vec3 vViewAngles = I::EngineClient->GetViewAngles();
    
    // Use freecam position if in freecam mode, otherwise use player eye position
    if (F::SpectateAll.IsInFreecam())
    {
        vEyePos = F::SpectateAll.GetFreecamPosition();
    }
    else
    {
        vEyePos = pLocal->GetEyePosition();
    }
    
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
    if (!pLocal)
        return;
    
    // Allow marking when alive or when in any freecam mode
    bool canMark = pLocal->IsAlive() || F::SpectateAll.IsInFreecam();
    if (!canMark)
        return;
    
    // Check if E key is pressed (only when game is focused)
    bool ePressed = GetAsyncKeyState('E') & 0x8000 && SDK::IsGameWindowInFocus();
    
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
    for (auto& markPair : m_MarkSpots)
    {
        auto& mark = markPair.second;
        if (mark.MapName != currentMap)
            continue;
        
        // Update and draw pulse rings if enabled
        if (Vars::Competitive::MarkSpot::ShowPulseRings.Value)
        {
            UpdatePulseRings(mark);
            DrawPulseRings(mark);
        }
        
        DrawGroundCircle(mark.Position, mark.Color);
        DrawPylon(mark.Position, mark.Color);
    }
    
    // Draw off-screen indicators for mark spots
    if (Vars::Competitive::MarkSpot::ShowOffScreenIndicators.Value)
    {
        DrawOffScreenIndicators();
    }
}


bool CMarkSpot::IsOnScreen(const Vec3& position)
{
    Vec3 w2s;
    if (SDK::W2S(position, w2s))
    {
        int screenW, screenH;
        I::MatSystemSurface->GetScreenSize(screenW, screenH);
        
        // Check if the position is within screen bounds
        return (w2s.x >= 0 && w2s.x <= screenW && w2s.y >= 0 && w2s.y <= screenH);
    }
    return false;
}

void CMarkSpot::DrawOffScreenIndicators()
{
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
    
    // Get current map name for filtering
    std::string currentMap = GetCurrentMapName();
    if (currentMap.empty())
        return;
    
    // Process all mark spots for current map
    for (const auto& [markId, mark] : m_MarkSpots)
    {
        if (mark.MapName != currentMap)
            continue;
        
        // Calculate ground target position (where the circle is drawn)
        Vec3 localPos = pLocal->GetAbsOrigin();
        
        // Force arrow to point horizontally by using local player's Z level
        // This ensures arrows point toward the mark location at ground level, not upward/downward
        Vec3 groundTargetPos = Vec3(mark.Position.x, mark.Position.y, localPos.z);
        
        // Check if ground target position is off-screen
        Vec3 w2s;
        if (SDK::W2S(groundTargetPos, w2s))
        {
            int screenW, screenH;
            I::MatSystemSurface->GetScreenSize(screenW, screenH);
            
            // If the ground position is within screen bounds, skip drawing indicator
            if (w2s.x >= 0 && w2s.x <= screenW && w2s.y >= 0 && w2s.y <= screenH)
                continue;
        }
        
        // Calculate directional position relative to local player
        // Point arrow toward ground circle center, not the aim position
        Vec3 positionDiff = localPos - groundTargetPos;
        
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
        
        // Calculate distance for alpha
        float distance = len;
        int alpha = static_cast<int>(std::max(0.0f, 255.0f - (distance * 0.05f)));
        
        // Use mark spot color with distance-based alpha
        Color_t indicatorColor = mark.Color;
        indicatorColor.a = static_cast<byte>(alpha);
        
        // Calculate angle pointing toward the target (opposite of the direction to indicator)
        float directionAngle = std::atan2(y, x); // Direction from indicator toward target
        
        // Draw arrow pointing toward the mark spot
        DrawArrow(finalX, finalY, directionAngle, indicatorColor, 12);
        
        // Draw distance text below the arrow
        std::string distanceText = std::to_string(static_cast<int>(distance)) + "u";
        Vec2 textSize = H::Draw.GetTextSize(distanceText.c_str(), font);
        
        H::Draw.String(font, finalX - static_cast<int>(textSize.x) / 2, finalY + 15, indicatorColor, ALIGN_TOPLEFT, distanceText.c_str());
    }
}

void CMarkSpot::DrawArrow(int centerX, int centerY, float angle, const Color_t& color, int size)
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

void CMarkSpot::UpdatePulseRings(MarkSpotInfo& mark)
{
    float currentTime = I::GlobalVars->curtime;
    float pulseInterval = 1.0f / Vars::Competitive::MarkSpot::PulseRingSpeed.Value; // Convert speed to interval
    float maxRadius = static_cast<float>(Vars::Competitive::MarkSpot::PulseRingMaxRadius.Value);
    
    // Create new pulse ring every second (adjusted by speed)
    if (mark.PulseRings.empty() || (currentTime - mark.PulseRings.back().StartTime) >= pulseInterval)
    {
        PulseRing newRing;
        newRing.StartTime = currentTime;
        newRing.CurrentRadius = 0.0f;
        newRing.Color = mark.Color;
        mark.PulseRings.push_back(newRing);
    }
    
    // Update existing pulse rings
    for (auto& ring : mark.PulseRings)
    {
        float elapsed = currentTime - ring.StartTime;
        float progress = elapsed * Vars::Competitive::MarkSpot::PulseRingSpeed.Value;
        
        // Expand ring outward
        ring.CurrentRadius = progress * maxRadius;
        
        // Fade out as ring expands
        float fadeProgress = ring.CurrentRadius / maxRadius;
        int alpha = static_cast<int>(Vars::Competitive::MarkSpot::PulseRingAlpha.Value * (1.0f - fadeProgress));
        ring.Color.a = static_cast<byte>(std::max(0, alpha));
    }
    
    // Remove expired rings
    for (auto it = mark.PulseRings.begin(); it != mark.PulseRings.end();)
    {
        if (it->CurrentRadius >= maxRadius)
        {
            it = mark.PulseRings.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void CMarkSpot::DrawPulseRings(MarkSpotInfo& mark)
{
    // Draw each pulse ring as a circle outline
    for (const auto& ring : mark.PulseRings)
    {
        if (ring.CurrentRadius <= 0.0f || ring.Color.a <= 0)
            continue;
        
        // Create circle points
        const int segments = 32; // Fixed segments for smooth circles
        std::vector<Vertex_t> vertices;
        
        for (int i = 0; i < segments; i++)
        {
            float angle = (2.0f * M_PI * i) / segments;
            Vec3 worldPoint = mark.Position;
            worldPoint.x += cos(angle) * ring.CurrentRadius;
            worldPoint.y += sin(angle) * ring.CurrentRadius;
            worldPoint.z += 5.0f; // Slightly above ground to avoid z-fighting
            
            Vec3 screenPoint;
            if (SDK::W2S(worldPoint, screenPoint))
            {
                vertices.emplace_back(Vertex_t(Vector2D(screenPoint.x, screenPoint.y)));
            }
            else
            {
                return; // If any point is off-screen, don't draw this ring
            }
        }
        
        // Only draw if we have enough vertices
        if (vertices.size() >= 3)
        {
            // Check visibility if through walls is disabled
            if (Vars::Competitive::MarkSpot::ShowThroughWalls.Value || 
                IsVisible(H::Entities.GetLocal()->GetEyePosition(), mark.Position))
            {
                // Draw pulse ring outline
                H::Draw.LinePolygon(vertices, ring.Color);
            }
        }
    }
}