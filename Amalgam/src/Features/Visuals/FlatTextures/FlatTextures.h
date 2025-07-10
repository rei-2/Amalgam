#pragma once
#include "../../../SDK/SDK.h"
#include <unordered_map>

class CFlatTextures
{
private:
    std::unordered_map<std::string, Color_t> m_CachedMaterials;
    std::unordered_map<std::string, ITexture*> m_OriginalTextures;
    bool m_bEnabled = false;

public:
    void OnLevelInitPostEntity();
    void OnLevelShutdown();
    void ProcessMaterials();
    void RestoreMaterials();
    void Update();
    
    void SetEnabled(bool enabled);
    bool IsEnabled() const { return m_bEnabled; }
};

ADD_FEATURE(CFlatTextures, FlatTextures)