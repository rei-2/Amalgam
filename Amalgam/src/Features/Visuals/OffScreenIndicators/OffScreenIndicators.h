#pragma once
#include "../../../SDK/SDK.h"
#include <unordered_map>
#include <string>

class COffScreenIndicators
{
private:
    // Color cache for Steam IDs (same as MarkSpot)
    std::unordered_map<std::string, Color_t> m_ColorCache;
    
    // Embedded avatar for fallback
    static int m_iFallbackAvatarTexture;
    
    // Helper functions
    Color_t GenerateColor(const std::string& steamID);
    std::string GetPlayerSteamID(CTFPlayer* pPlayer);
    std::string GetPlayerClassName(int classId);
    bool IsOnScreen(CBaseEntity* entity);
    void DrawArrow(int centerX, int centerY, float angle, const Color_t& color, int size = 10);
    int GetFallbackAvatarTexture();
    std::vector<unsigned char> DecodeBase64(const std::string& base64);
    
public:
    void Draw();
};

ADD_FEATURE(COffScreenIndicators, OffScreenIndicators)