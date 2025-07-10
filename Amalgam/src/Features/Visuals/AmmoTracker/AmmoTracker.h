#pragma once
#include "../../../SDK/SDK.h"
#include <map>
#include <unordered_map>
#include <string>

struct SupplyInfo
{
    Vec3 Position;
    std::string Type; // "ammo" or "health"
    bool Respawning;
    float DisappearTime;
};

class CAmmoTracker
{
private:
    static constexpr float RESPAWN_TIME = 10.0f;
    static constexpr int UPDATE_INTERVAL = 2; // Update positions every 2 ticks
    
    std::unordered_map<std::string, SupplyInfo> m_SupplyPositions;
    std::unordered_map<std::string, int> m_NextVisCheck;
    std::unordered_map<std::string, bool> m_VisibilityCache;
    std::unordered_map<std::string, std::string> m_ModelCache;
    
    int m_LastUpdateTick = 0;
    
    std::string GetPickupType(CBaseEntity* pEntity);
    bool IsVisible(const Vec3& vPos);
    std::string GetPositionKey(const Vec3& vPos);
    bool IsStationaryPickup(CBaseEntity* pEntity);
    void UpdateSupplyPositions();
    void DrawClockFill(int iCenterX, int iCenterY, int iRadius, float flPercentage, int iVertices, const Color_t& color);
    void DrawFilledProgress(const Vec3& vPos, int iRadius, float flPercentage, int iSegments, const Color_t& color);
    int GetScreenChartSize(float flDistance);
    int GetTextSize(float flDistance);
    void DrawHudOverlay(const Vec3& vPlayerPos);
    bool IsPlayerNearSupply(const Vec3& vPlayerPos, const Vec3& vSupplyPos, float flMaxDistance = 100.0f);
    bool IsAnyPlayerNearSupply(const Vec3& vSupplyPos, float flMaxDistance = 100.0f);
    
public:
    void Draw();
    void Event(IGameEvent* pEvent, uint32_t uHash);
};

ADD_FEATURE(CAmmoTracker, AmmoTracker)