#pragma once
#include "../../../SDK/SDK.h"
#include <vector>
#include <unordered_map>
#include <string>

struct PulseRing
{
    float StartTime;
    float CurrentRadius;
    Color_t Color;
};

struct MarkSpotInfo
{
    Vec3 Position;
    std::string MapName;
    float CreatedTime;
    std::string SenderSteamID;
    Color_t Color;
    std::vector<PulseRing> PulseRings;
};

class CMarkSpot
{
private:
    // All configuration now comes from Vars (no hardcoded constants)
    
    // Storage for mark spots (keyed by color hash to allow one mark per player)
    std::unordered_map<std::string, MarkSpotInfo> m_MarkSpots;
    
    // Color cache for Steam IDs
    std::unordered_map<std::string, Color_t> m_ColorCache;
    
    // Input handling
    bool m_bLastEPressed = false;
    
    // Rate limiting to respect Matrix spec
    float m_fLastMessageTime = 0.0f;
    
    // Helper functions
    std::string GetCurrentMapName();
    Vec3 GetAimPosition();
    std::string GenerateMarkId(const MarkSpotInfo& mark);
    void CleanupExpiredMarks();
    void DrawGroundCircle(const Vec3& position, const Color_t& color);
    void DrawPylon(const Vec3& basePosition, const Color_t& color);
    void DrawPulseRings(MarkSpotInfo& mark);
    void UpdatePulseRings(MarkSpotInfo& mark);
    bool IsVisible(const Vec3& fromPos, const Vec3& targetPos);
    void SendMarkSpotMessage(const MarkSpotInfo& mark);
    Color_t GenerateColor(const std::string& steamID);
    std::string GetLocalSteamID();
    
    // Off-screen indicators for mark spots
    void DrawOffScreenIndicators();
    bool IsOnScreen(const Vec3& position);
    void DrawArrow(int centerX, int centerY, float angle, const Color_t& color, int size = 10);
    
public:
    CMarkSpot() = default;
    void Draw();
    void HandleInput();
    
    // Matrix integration functions
    void AddMarkSpot(const MarkSpotInfo& mark);
    void ProcessMatrixMessage(const std::string& sender, const std::string& message);
    bool IsMarkSpotMessage(const std::string& message);
};

ADD_FEATURE(CMarkSpot, MarkSpot)