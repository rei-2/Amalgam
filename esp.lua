local ALPHA = 70
local BAR_HEIGHT = 10
local BAR_WIDTH = 90  -- Fixed width for the non-wobble mode
local OVERHEAL_COLOR = {71, 166, 255}
local MAX_DISTANCE_SQR = 3500 * 3500
local MEDIC_MODE = true -- Toggle for showing teammate health bars when playing medic
local CLOAK_COND = 4
local DISGUISE_COND = 3
local MEDIC_CLASS = 5
local VISIBILITY_CHECK_INTERVAL = 0.1 -- 100ms between visibility checks

-- Configuration
local config = {
    fixedHealthBars = true -- When true, health bars won't wobble with player models
}

-- Cache frequently used functions
local floor = math.floor
local min = math.min
local max = math.max
local huge = math.huge
local WorldToScreen = client.WorldToScreen
local FrameCount = globals.FrameCount
local RealTime = globals.RealTime
local GetLocalPlayer = entities.GetLocalPlayer
local FindByClass = entities.FindByClass
local TraceLine = engine.TraceLine
local Con_IsVisible = engine.Con_IsVisible
local IsGameUIVisible = engine.IsGameUIVisible
local Color = draw.Color
local FilledRect = draw.FilledRect

-- Reuse vector for calculations
local reuseVector = Vector3(0, 0, 0)

-- Visibility caching
local nextVisibilityCheck = {}
local visibilityCache = {}
local lastCheckTime = 0

-- Health bar color cache (to avoid recalculating colors for the same health percentages)
local healthColorCache = {}

local function Get2DBoundingBox(entity, corners)
    local hitbox = entity:HitboxSurroundingBox()
    if not hitbox or not hitbox[1] or not hitbox[2] then return nil end
    
    -- Use the passed corners array to avoid allocations
    corners[1].x, corners[1].y, corners[1].z = hitbox[1].x, hitbox[1].y, hitbox[1].z
    corners[2].x, corners[2].y, corners[2].z = hitbox[1].x, hitbox[2].y, hitbox[1].z
    corners[3].x, corners[3].y, corners[3].z = hitbox[2].x, hitbox[2].y, hitbox[1].z
    corners[4].x, corners[4].y, corners[4].z = hitbox[2].x, hitbox[1].y, hitbox[1].z
    corners[5].x, corners[5].y, corners[5].z = hitbox[2].x, hitbox[2].y, hitbox[2].z
    corners[6].x, corners[6].y, corners[6].z = hitbox[1].x, hitbox[2].y, hitbox[2].z
    corners[7].x, corners[7].y, corners[7].z = hitbox[1].x, hitbox[1].y, hitbox[2].z
    corners[8].x, corners[8].y, corners[8].z = hitbox[2].x, hitbox[1].y, hitbox[2].z
    
    local minX, minY, maxX, maxY = huge, huge, -huge, -huge
    local validCorners = 0
    
    for i = 1, 8 do
        local onScreen = WorldToScreen(corners[i])
        if onScreen then
            validCorners = validCorners + 1
            minX = min(minX, onScreen[1])
            minY = min(minY, onScreen[2])
            maxX = max(maxX, onScreen[1])
            maxY = max(maxY, onScreen[2])
        end
    end
    
    if validCorners >= 6 then -- Allow some corners to be offscreen
        return floor(minX), floor(minY), floor(maxX), floor(maxY)
    end
    
    return nil
end

local function GetHealthBarColor(health, maxHealth)
    -- Check cache first
    local ratio = health / maxHealth
    local cacheKey = floor(ratio * 100) -- Round to nearest percent for caching
    
    -- Use different cache keys for overheal vs normal health
    if health > maxHealth then
        cacheKey = "overheal_" .. cacheKey
    else
        cacheKey = "normal_" .. cacheKey
    end
    
    if healthColorCache[cacheKey] then
        local cache = healthColorCache[cacheKey]
        return cache[1], cache[2], cache[3], cache[4]
    end
    
    -- Calculate if not cached
    local currentAlpha = floor(255 - (ratio * (255 - ALPHA)))
    currentAlpha = max(ALPHA, min(255, currentAlpha))
    
    local r, g, b, a
    if health > maxHealth then
        r, g, b, a = OVERHEAL_COLOR[1], OVERHEAL_COLOR[2], OVERHEAL_COLOR[3], ALPHA
    else
        r = floor(255 * (1 - ratio))
        g = floor(255 * ratio)
        b = 0
        a = currentAlpha
    end
    
    -- Cache result
    healthColorCache[cacheKey] = {r, g, b, a}
    return r, g, b, a
end

local function IsMedic(player)
    return player:GetPropInt("m_iClass") == MEDIC_CLASS
end

local function IsPlayerVisible(player, eyePos, isFriendly)
    local id = player:GetIndex()
    local currentTime = RealTime()
    
    -- Only update visibility check if enough time has passed
    if visibilityCache[id] and currentTime - nextVisibilityCheck[id] < VISIBILITY_CHECK_INTERVAL then
        return visibilityCache[id]
    end
    
    -- Reuse vector to calculate player center position
    local playerPos = player:GetAbsOrigin()
    if not playerPos then return false end
    
    -- Aim for center mass for better visibility checks
    reuseVector.x = playerPos.x
    reuseVector.y = playerPos.y
    reuseVector.z = playerPos.z + 40 -- Roughly chest height
    
    local mask = isFriendly and MASK_SHOT_HULL or MASK_VISIBLE
    local trace = TraceLine(eyePos, reuseVector, mask)
    
    local isVisible
    if isFriendly then
        isVisible = trace.fraction > 0.97
    else
        isVisible = trace.entity == player
    end
    
    -- Update cache
    visibilityCache[id] = isVisible
    nextVisibilityCheck[id] = currentTime + VISIBILITY_CHECK_INTERVAL
    
    return isVisible
end

local function DrawHealthBar(x, y, width, health, maxHealth)
    -- Ensure integer coordinates for better performance
    x, y, width = floor(x), floor(y), floor(width)
    
    local healthBarSize = floor(width * (min(health, maxHealth) / maxHealth))
    local overhealSize = health > maxHealth and floor(width * ((health - maxHealth) / maxHealth)) or 0

    -- Background
    Color(0, 0, 0, ALPHA)
    FilledRect(x, y, x + width, y + BAR_HEIGHT)

    -- Main health bar
    local r, g, b, a = GetHealthBarColor(min(health, maxHealth), maxHealth)
    Color(r, g, b, a)
    FilledRect(x + 1, y + 1, x + healthBarSize - 1, y + BAR_HEIGHT - 1)

    -- Overheal bar
    if overhealSize > 0 then
        Color(OVERHEAL_COLOR[1], OVERHEAL_COLOR[2], OVERHEAL_COLOR[3], ALPHA)
        FilledRect(x + healthBarSize, y + 1, x + healthBarSize + overhealSize - 1, y + BAR_HEIGHT - 1)
    end
end

-- Reusable array for corner vectors
local cornerVectors = {}
for i = 1, 8 do
    cornerVectors[i] = Vector3(0, 0, 0)
end

-- Clean up stale visibility data periodically
local function CleanupVisibilityCache()
    local currentTime = RealTime()
    if currentTime - lastCheckTime < 1.0 then return end
    
    lastCheckTime = currentTime
    
    -- Remove entries for entities that might no longer exist
    for id in pairs(visibilityCache) do
        local entity = entities.GetByIndex(id)
        if not entity or not entity:IsValid() or entity:IsDormant() or not entity:IsAlive() then
            visibilityCache[id] = nil
            nextVisibilityCheck[id] = nil
        end
    end
end

callbacks.Register("Draw", "HealthBarESP", function()
    if Con_IsVisible() or IsGameUIVisible() then return end

    local localPlayer = GetLocalPlayer()
    if not localPlayer then return end
    
    local localPos = localPlayer:GetAbsOrigin()
    if not localPos then return end

    local isLocalPlayerMedic = IsMedic(localPlayer)
    local showTeammates = MEDIC_MODE and isLocalPlayerMedic
    
    -- Get eye position for visibility checks
    local eyePos = localPlayer:GetPropVector("localdata", "m_vecViewOffset[0]")
    if eyePos then
        eyePos.x = localPos.x + eyePos.x
        eyePos.y = localPos.y + eyePos.y
        eyePos.z = localPos.z + eyePos.z
    else
        eyePos = localPos
    end
    
    -- Clean up visibility cache periodically
    CleanupVisibilityCache()

    local players = FindByClass("CTFPlayer")
    for _, player in pairs(players) do
        if not player:IsAlive() or player:IsDormant() or player == localPlayer then goto continue end
        
        -- Skip if cloaked or disguised
        if player:InCond(CLOAK_COND) or player:InCond(DISGUISE_COND) then goto continue end
        
        local isFriendly = player:GetTeamNumber() == localPlayer:GetTeamNumber()
        if not (showTeammates and isFriendly or not showTeammates and not isFriendly) then goto continue end

        local playerPos = player:GetAbsOrigin()
        if not playerPos then goto continue end

        -- Fast distance check using squared distance
        local dx = playerPos.x - localPos.x
        local dy = playerPos.y - localPos.y
        local dz = playerPos.z - localPos.z
        local distSqr = dx * dx + dy * dy + dz * dz
        if distSqr > MAX_DISTANCE_SQR then goto continue end

        -- Visibility check with caching
        if not IsPlayerVisible(player, eyePos, isFriendly) then goto continue end

        local health = player:GetHealth()
        local maxHealth = player:GetMaxHealth()

        if config.fixedHealthBars then
            local basePos = WorldToScreen(playerPos)
            if basePos then
                DrawHealthBar(basePos[1] - BAR_WIDTH/2, basePos[2] + 30, BAR_WIDTH, health, maxHealth)
            end
        else
            local x, y, x2, y2 = Get2DBoundingBox(player, cornerVectors)
            if x then
                DrawHealthBar(x, y2 + 2, x2 - x, health, maxHealth)
            end
        end

        ::continue::
    end
end)

-- Clean up resources when script unloads
callbacks.Register("Unload", function()
    healthColorCache = nil
    visibilityCache = nil
    nextVisibilityCheck = nil
    cornerVectors = nil
end)