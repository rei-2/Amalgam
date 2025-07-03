local enemy_only = true
local max_dist = 2800 -- hammer units

local box_3d = false  -- Toggle for 3D box ESP
local box_2d = true  -- Toggle for 2D box ESP
local box_color_visible = { 0, 255, 0, 255 }  -- Green for visible stickies
local box_color_invisible = { 255, 0, 0, 255 }  -- Red for not visible stickies

local box_2d_size = 20 -- Size of the 2D box in pixels
local boxOnlyWhenVisible = true

-- Chams settings
local chamsOn = false -- Disabled by default because of chams being bugged in lbox
local chamsOnlyWhenVisible = false

-- Cache system configuration
local visibilityCheckInterval = 0.1  -- Check visibility every 0.1 seconds
local cacheCleanupInterval = 1.0     -- Clean caches every 1.0 seconds
local nextCleanupTime = 0
local hitboxCacheLifetime = 0.2      -- Hitbox cache lifetime in seconds

-- Centralized cache tables
local visibilityCache = {}
local hitboxCache = {}
local cachedStickies = {}
local lastStickyUpdate = 0
local stickyUpdateInterval = 0.1

-- Cham material definitions
local visibleMaterial = materials.Create("VisibleSticky", [[
"UnlitGeneric"
{
    $basetexture "vgui/white_additive"
    $model "1"
    $color2 "[0 1 0]"
    $ignorez "0"
}
]])

local invisibleMaterial = materials.Create("InvisibleSticky", [[
"UnlitGeneric"
{
    $basetexture "vgui/white_additive"
    $model "1"
    $color2 "[1 0 0]"
    $ignorez "1"
}
]])

local function cleanCaches()
    local currentTime = globals.RealTime()
    if currentTime < nextCleanupTime then return end
    
    for entityIndex, data in pairs(visibilityCache) do
        if (currentTime - data.time) > visibilityCheckInterval * 2 then
            visibilityCache[entityIndex] = nil
        end
    end
    
    for entityIndex, data in pairs(hitboxCache) do
        if (currentTime - data.time) > hitboxCacheLifetime * 2 then
            hitboxCache[entityIndex] = nil
        end
    end
    
    nextCleanupTime = currentTime + cacheCleanupInterval
end

local function IsVisible(entity, localPlayer)
    if not entity or not localPlayer then return false end
    
    local currentTime = globals.RealTime()
    local entityIndex = entity:GetIndex()
    
    if visibilityCache[entityIndex] and 
       (currentTime - visibilityCache[entityIndex].time) < visibilityCheckInterval then
        return visibilityCache[entityIndex].visible
    end
    
    local source = localPlayer:GetAbsOrigin() + localPlayer:GetPropVector("localdata", "m_vecViewOffset[0]")
    local targetPos = entity:GetAbsOrigin()
    
    local trace = engine.TraceLine(source, targetPos, MASK_SHOT)
    local isVisible = trace.fraction > 0.99 or trace.entity == entity
    
    visibilityCache[entityIndex] = {
        time = currentTime,
        visible = isVisible
    }
    
    return isVisible
end

local function getHitboxWithCache(entity)
    if not entity then return nil end
    
    local currentTime = globals.RealTime()
    local entityIndex = entity:GetIndex()
    
    if hitboxCache[entityIndex] and 
       (currentTime - hitboxCache[entityIndex].time) < hitboxCacheLifetime then
        return hitboxCache[entityIndex].hitbox
    end
    
    local hitbox = entity:HitboxSurroundingBox()
    if hitbox then
        hitboxCache[entityIndex] = {
            time = currentTime,
            hitbox = hitbox
        }
    end
    
    return hitbox
end

local function draw_3d_box(vertices, color)
    if #vertices < 8 then return end
    
    draw.Color(table.unpack(color))
    local edges = {
        {1,2}, {2,3}, {3,4}, {4,1},
        {5,6}, {6,7}, {7,8}, {8,5},
        {1,5}, {2,6}, {3,7}, {4,8}
    }
    
    for _, edge in ipairs(edges) do
        local v1, v2 = vertices[edge[1]], vertices[edge[2]]
        if v1 and v2 then
            draw.Line(v1.x, v1.y, v2.x, v2.y)
        end
    end
end

local function draw_2d_box(x, y, size, color)
    draw.Color(table.unpack(color))
    local half_size = size / 2
    local x1, y1 = x - half_size, y - half_size
    local x2, y2 = x + half_size, y + half_size
    
    draw.Line(x1, y1, x2, y1)
    draw.Line(x1, y2, x2, y2)
    draw.Line(x1, y1, x1, y2)
    draw.Line(x2, y1, x2, y2)
end

local function stickybomb_esp()
    local currentTime = globals.RealTime()
    local localPlayer = entities.GetLocalPlayer()
    if not localPlayer then return end
    
    if (currentTime - lastStickyUpdate) > stickyUpdateInterval then
        cachedStickies = entities.FindByClass("CTFGrenadePipebombProjectile")
        lastStickyUpdate = currentTime
    end
    
    cleanCaches()
    
    for _, projectile in pairs(cachedStickies) do
        if projectile:IsValid() and projectile:GetPropInt("m_iType") == 1 and not projectile:IsDormant() then
            local projectile_pos = projectile:GetAbsOrigin()
            
            if vector.Distance(localPlayer:GetAbsOrigin(), projectile_pos) > max_dist then
                goto continue
            end
            
            if enemy_only and projectile:GetTeamNumber() == localPlayer:GetTeamNumber() then
                goto continue
            end
            
            local projectile_screen = client.WorldToScreen(projectile_pos)
            if not projectile_screen then goto continue end
            
            local isVisible = IsVisible(projectile, localPlayer)
            if boxOnlyWhenVisible and not isVisible then
                goto continue
            end
            
            local color = isVisible and box_color_visible or box_color_invisible
            
            if box_2d then
                draw_2d_box(projectile_screen[1], projectile_screen[2], box_2d_size, color)
            end
            
            if box_3d then
                local hitboxes = getHitboxWithCache(projectile)
                if hitboxes then
                    local min, max = hitboxes[1], hitboxes[2]
                    local vertices = {
                        Vector3(min.x, min.y, min.z), Vector3(min.x, max.y, min.z),
                        Vector3(max.x, max.y, min.z), Vector3(max.x, min.y, min.z),
                        Vector3(min.x, min.y, max.z), Vector3(min.x, max.y, max.z),
                        Vector3(max.x, max.y, max.z), Vector3(max.x, min.y, max.z)
                    }
                    
                    local screenVertices = {}
                    local allValid = true
                    for j, vertex in ipairs(vertices) do
                        local screenPos = client.WorldToScreen(vertex)
                        if screenPos then
                            screenVertices[j] = {x = screenPos[1], y = screenPos[2]}
                        else
                            allValid = false
                            break
                        end
                    end
                    
                    if allValid then
                        draw_3d_box(screenVertices, color)
                    end
                end
            end
        end
        ::continue::
    end
end

local function onDrawModel(ctx)
    if not chamsOn then return end

    local entity = ctx:GetEntity()
    if entity and entity:IsValid() and entity:GetClass() == "CTFGrenadePipebombProjectile" and entity:GetPropInt("m_iType") == 1 then
        local localPlayer = entities.GetLocalPlayer()
        if not localPlayer then return end

        if enemy_only and entity:GetTeamNumber() == localPlayer:GetTeamNumber() then
            ctx:ForcedMaterialOverride(nil)
            return
        end

        local isVisible = IsVisible(entity, localPlayer)

        if isVisible then
            ctx:ForcedMaterialOverride(visibleMaterial)
        else
            if not chamsOnlyWhenVisible then
                ctx:ForcedMaterialOverride(invisibleMaterial)
            else
                ctx:ForcedMaterialOverride(nil)
            end
        end
    else
        ctx:ForcedMaterialOverride(nil)
    end
end

callbacks.Register("Draw", "stickybomb_esp", stickybomb_esp)
callbacks.Register("DrawModel", "StickyChams", onDrawModel)