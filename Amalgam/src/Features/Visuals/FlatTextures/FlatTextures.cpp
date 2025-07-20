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
        // Don't try to restore materials during level shutdown as the game
        // is already cleaning everything up - just clear our cache
        m_OriginalTextures.clear();
        m_CachedMaterials.clear();
        m_bEnabled = false;
    }
}

void CFlatTextures::ProcessMaterials()
{
    if (!I::MaterialSystem)
        return;
        
    try
    {
        MaterialHandle_t matHandle = I::MaterialSystem->FirstMaterial();
        
        while (matHandle != I::MaterialSystem->InvalidMaterial())
        {
            IMaterial* pMaterial = nullptr;
            try 
            {
                pMaterial = I::MaterialSystem->GetMaterial(matHandle);
            }
            catch (...)
            {
                matHandle = I::MaterialSystem->NextMaterial(matHandle);
                continue;
            }
            
            if (pMaterial)
            {
                try
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
                        IMaterialVar* pBaseTexVar = nullptr;
                        try
                        {
                            pBaseTexVar = pMaterial->FindVar("$basetexture", nullptr);
                        }
                        catch (...) {}
                        
                        if (pBaseTexVar && m_OriginalTextures.find(matName) == m_OriginalTextures.end())
                        {
                            try
                            {
                                ITexture* originalTex = pBaseTexVar->GetTextureValue();
                                if (originalTex)
                                    m_OriginalTextures[matName] = originalTex;
                            }
                            catch (...) {}
                        }
                        
                        // Create a flat 1x1 texture from the sampled color
                        ITexture* pFlatTexture = I::MaterialSystem->CreateTextureFromBits(
                            1, 1, 1, IMAGE_FORMAT_RGBA8888, 4, flatPixel);
                        
                        if (pFlatTexture && pBaseTexVar)
                        {
                            try
                            {
                                pBaseTexVar->SetTextureValue(pFlatTexture);
                            }
                            catch (...) {}
                        }
                    }
                }
                catch (...)
                {
                    // Skip this material if any operation fails
                }
            }
            
            try
            {
                matHandle = I::MaterialSystem->NextMaterial(matHandle);
            }
            catch (...)
            {
                break;
            }
        }
    }
    catch (...)
    {
        // Handle any top-level exceptions during material processing
    }
}

void CFlatTextures::RestoreMaterials()
{
    if (!I::MaterialSystem)
    {
        // Clear cache even if MaterialSystem is invalid
        m_OriginalTextures.clear();
        return;
    }
    
    // Additional safety check: don't restore during engine shutdown or when not in game
    if (!I::EngineClient || !I::EngineClient->IsInGame())
    {
        // Clear cache if not in game to prevent stale texture pointers
        m_OriginalTextures.clear();
        return;
    }
    
    // Safely iterate through cached materials to avoid crashes during shutdown
    // Create a copy of the map to avoid iterator invalidation issues
    auto originalTexturesCopy = m_OriginalTextures;
    
    // Clear the original map immediately to prevent recursive calls
    m_OriginalTextures.clear();
    
    for (const auto& pair : originalTexturesCopy)
    {
        const std::string& matName = pair.first;
        ITexture* pOriginalTexture = pair.second;
        
        // Skip if texture pointer is null or invalid
        if (!pOriginalTexture)
            continue;
            
        try
        {
            // Find the material by name safely
            IMaterial* pMaterial = nullptr;
            try
            {
                pMaterial = I::MaterialSystem->FindMaterial(matName.c_str(), TEXTURE_GROUP_OTHER, false);
            }
            catch (...)
            {
                continue;
            }
            
            if (pMaterial)
            {
                try
                {
                    if (!pMaterial->IsErrorMaterial())
                    {
                        IMaterialVar* pBaseTexVar = pMaterial->FindVar("$basetexture", nullptr);
                        if (pBaseTexVar)
                        {
                            pBaseTexVar->SetTextureValue(pOriginalTexture);
                        }
                    }
                }
                catch (...)
                {
                    // Material operations failed, likely due to corrupted object
                    continue;
                }
            }
        }
        catch (...)
        {
            // Silently handle any exceptions during material restoration
            continue;
        }
    }
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