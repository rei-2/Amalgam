#pragma once
#include "../../../SDK/SDK.h"
#include <unordered_map>

class CStickyESP
{
private:
    // Configuration values are now read from Vars::Competitive::StickyESP::* variables
    
    // Helper functions (no caching, direct like HealthBarESP)
    bool IsVisible(CBaseEntity* pEntity, CBaseEntity* pLocal);
    void Draw3DBox(const std::vector<Vec3>& vertices, const Color_t& color);
    void Draw2DBox(int x, int y, int size, const Color_t& color);
    std::pair<Vec3, Vec3> GetHitbox(CBaseEntity* pEntity);
    
public:
    // Chams tracking for stickies
    std::unordered_map<int, bool> m_mEntities;
    
    void Draw();
    void UpdateChamsEntities(); // Separate function to populate chams entities
};

ADD_FEATURE(CStickyESP, StickyESP)