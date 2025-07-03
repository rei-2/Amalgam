#pragma once
#include "../../../SDK/SDK.h"

class CStickyESP
{
private:
    // Configuration constants (exactly from Lua script)
    static constexpr bool ENEMY_ONLY = true;
    static constexpr float MAX_DISTANCE = 2800.0f;
    static constexpr bool BOX_3D = false;
    static constexpr bool BOX_2D = true;
    static constexpr int BOX_2D_SIZE = 20;
    static constexpr bool BOX_ONLY_WHEN_VISIBLE = true;
    
    // Colors (exactly from Lua script)
    static constexpr Color_t BOX_COLOR_VISIBLE = {0, 255, 0, 255};   // Green
    static constexpr Color_t BOX_COLOR_INVISIBLE = {255, 0, 0, 255}; // Red
    
    // Helper functions (no caching, direct like HealthBarESP)
    bool IsVisible(CBaseEntity* pEntity, CBaseEntity* pLocal);
    void Draw3DBox(const std::vector<Vec3>& vertices, const Color_t& color);
    void Draw2DBox(int x, int y, int size, const Color_t& color);
    std::pair<Vec3, Vec3> GetHitbox(CBaseEntity* pEntity);
    
public:
    void Draw();
};

ADD_FEATURE(CStickyESP, StickyESP)