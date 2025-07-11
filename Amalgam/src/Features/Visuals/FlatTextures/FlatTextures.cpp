#include "FlatTextures.h"

void CFlatTextures::OnLevelInitPostEntity()
{
    if (!Vars::Competitive::Features::FlatTextures.Value)
        return;
        
    m_bEnabled = true;
    ProcessMaterials();
}

void CFlatTextures::OnLevelShutdown()
{
    if (m_bEnabled)
    {
        RestoreMaterials();
        m_bEnabled = false;
    }
}

void CFlatTextures::ProcessMaterials()
{
    if (!I::MaterialSystem)
        return;
        
    MaterialHandle_t matHandle = I::MaterialSystem->FirstMaterial();
    
    while (matHandle != I::MaterialSystem->InvalidMaterial())
    {
        IMaterial* pMaterial = I::MaterialSystem->GetMaterial(matHandle);
        
        if (pMaterial)
        {
            const std::string matName = pMaterial->GetName();
            
            // Only process world textures, skip transparent, sprite, and special materials
            if (!pMaterial->IsTranslucent() && 
                !pMaterial->IsSpriteCard() &&
                matName.find("chicken") == std::string::npos &&
                matName.find("water") == std::string::npos &&
                matName.find("sky") == std::string::npos &&
                std::string(pMaterial->GetTextureGroupName()).find(TEXTURE_GROUP_WORLD) == 0)
            {
                // Check if we've already cached this material
                if (m_CachedMaterials.find(matName) == m_CachedMaterials.end())
                {
                    unsigned char pixel[4] = {0};
                    
                    // Get the preview color sample from the material
                    pMaterial->GetPreviewImage(pixel, 1, 1, IMAGE_FORMAT_RGBA8888);
                    
                    // Cache the color
                    m_CachedMaterials[matName] = {pixel[0], pixel[1], pixel[2], pixel[3]};
                }
                
                // Get the cached color
                Color_t cachedColor = m_CachedMaterials[matName];
                unsigned char flatPixel[4] = {cachedColor.r, cachedColor.g, cachedColor.b, cachedColor.a};
                
                // Store original texture if not already stored
                IMaterialVar* pBaseTexVar = pMaterial->FindVar("$basetexture", nullptr);
                if (pBaseTexVar && m_OriginalTextures.find(matName) == m_OriginalTextures.end())
                {
                    m_OriginalTextures[matName] = pBaseTexVar->GetTextureValue();
                }
                
                // Create a flat 1x1 texture from the sampled color
                ITexture* pFlatTexture = I::MaterialSystem->CreateTextureFromBits(
                    1, 1, 1, IMAGE_FORMAT_RGBA8888, 4, flatPixel);
                
                if (pFlatTexture && pBaseTexVar)
                {
                    pBaseTexVar->SetTextureValue(pFlatTexture);
                }
            }
        }
        
        matHandle = I::MaterialSystem->NextMaterial(matHandle);
    }
}

void CFlatTextures::RestoreMaterials()
{
    if (!I::MaterialSystem)
        return;
        
    // Instead of iterating through materials during shutdown,
    // iterate through our cached materials directly to avoid crashes
    for (auto& pair : m_OriginalTextures)
    {
        const std::string& matName = pair.first;
        ITexture* pOriginalTexture = pair.second;
        
        if (!pOriginalTexture)
            continue;
            
        // Find the material by name safely
        IMaterial* pMaterial = I::MaterialSystem->FindMaterial(matName.c_str(), TEXTURE_GROUP_OTHER, false);
        if (pMaterial && !pMaterial->IsErrorMaterial())
        {
            IMaterialVar* pBaseTexVar = pMaterial->FindVar("$basetexture", nullptr);
            if (pBaseTexVar)
            {
                pBaseTexVar->SetTextureValue(pOriginalTexture);
            }
        }
    }
    
    // Clear the cache
    m_OriginalTextures.clear();
}

void CFlatTextures::SetEnabled(bool enabled)
{
    if (enabled && !m_bEnabled)
    {
        m_bEnabled = true;
        ProcessMaterials();
    }
    else if (!enabled && m_bEnabled)
    {
        RestoreMaterials();
        m_bEnabled = false;
    }
}

// Called each frame to check for variable changes
void CFlatTextures::Update()
{
    static bool lastState = false;
    bool currentState = Vars::Competitive::Features::FlatTextures.Value;
    
    if (currentState != lastState)
    {
        SetEnabled(currentState);
        lastState = currentState;
    }
}