-- Adjustable parameters
local trail_length = 19 -- Number of positions to remember (adjust for longer/shorter trails)
local update_interval = 6  -- Update position every X ticks (lower for smoother trails, higher for performance)
local fade_time = 7 -- Time in seconds for the trail to fade completely (higher for longer-lasting trails)
local max_trail_distance = 1850 -- Maximum distance to show trails
local visibility_timeout = 4 -- Time in seconds to keep showing the trail after losing sight of the enemy
local max_visible_duration = 2 -- Time in seconds to show the trail while enemy is continuously visible
local trail_height = 1.5 -- Adjusts the start height of the trail (0-5, can use decimals)
local cleanup_interval = 0.5 -- How often to clean up invalid data (seconds)

-- Cache math functions
local floor = math.floor
local max = math.max
local sqrt = math.sqrt

-- Pre-allocated vectors for calculations
local viewOffset = Vector3(0, 0, 0)
local heightOffset = Vector3(0, 0, 0)

-- Visibility check optimization
local nextVisCheck = {}
local visibilityCache = {}

-- State tracking
local nextCleanupTime = 0
local lastUpdateTick = 0
local currentMap = ""
local scriptActive = false
local lastLifeState = 2  -- Track respawn state (2 = dead)

-- Enum for player visibility states
local VisibilityState = {
    UNSEEN = 1,
    VISIBLE = 2,
    RECENTLY_INVISIBLE = 3
}

local playerData = {}

-- Pre-calculate trail height offset
local trailHeightUnits = math.min(5, math.max(0, trail_height)) * 20

-- Function to generate a color based on SteamID (now cached)
local colorCache = {}
local function generateColor(steamID)
    if colorCache[steamID] then
        return colorCache[steamID]
    end
    
    local hash = 0
    for i = 1, #steamID do
        hash = hash + steamID:byte(i)
    end
    local r = floor((hash * 11) % 200 + 55)
    local g = floor((hash * 23) % 200 + 55)
    local b = floor((hash * 37) % 200 + 55)
    
    colorCache[steamID] = {r, g, b}
    return colorCache[steamID]
end

-- Clear data function
local function ClearData()
    playerData = {}
    nextVisCheck = {}
    visibilityCache = {}
    lastUpdateTick = 0
    nextCleanupTime = 0
end

-- Clean up invalid targets and stale data
local function CleanInvalidTargets()
    local currentTime = globals.RealTime()
    if currentTime < nextCleanupTime then return end
    
    nextCleanupTime = currentTime + cleanup_interval
    
    local localPlayer = entities.GetLocalPlayer()
    if not localPlayer then return end
    
    for steamID, data in pairs(playerData) do
        -- Find the player entity by iterating through entities
        local playerFound = false
        local playerDormant = false
        local playerValid = false
        
        for i = 1, globals.MaxClients() do
            local player = entities.GetByIndex(i)
            if player then
                local info = client.GetPlayerInfo(i)
                if info and info.SteamID == steamID then
                    playerFound = true
                    playerDormant = player:IsDormant()
                    playerValid = player:IsValid() and player:IsAlive()
                    break
                end
            end
        end
        
        -- Clean if:
        -- 1. Player not found
        -- 2. Player is dormant
        -- 3. Player is invalid/dead
        -- 4. Last trail position is too old
        -- 5. Player hasn't been seen recently in visibility tracking
        if not playerFound or 
           playerDormant or 
           not playerValid or
           (data.times[1] and currentTime - data.times[1] > fade_time) or
           (data.lastSeenTime and currentTime - data.lastSeenTime > visibility_timeout) then
            playerData[steamID] = nil
            nextVisCheck[steamID] = nil
            visibilityCache[steamID] = nil
        end
    end
end

-- Optimized visibility check with caching
local function IsVisible(entity, localPlayer)
    if not entity or not localPlayer then return false end
    
    local id = entity:GetIndex()
    local curTick = globals.TickCount()
    
    if nextVisCheck[id] and curTick < nextVisCheck[id] then
        return visibilityCache[id] or false
    end
    
    -- Get positions only once
    local source = localPlayer:GetAbsOrigin()
    viewOffset = localPlayer:GetPropVector("localdata", "m_vecViewOffset[0]")
    source.x = source.x + viewOffset.x
    source.y = source.y + viewOffset.y
    source.z = source.z + viewOffset.z
    
    local targetPos = entity:GetAbsOrigin()
    local trace = engine.TraceLine(source, targetPos, MASK_SHOT)
    
    -- Cache result
    visibilityCache[id] = trace.entity == entity or trace.fraction > 0.99
    nextVisCheck[id] = curTick + 2 -- Check every 2 ticks
    
    return visibilityCache[id]
end

-- Optimized distance calculation (squared distance to avoid sqrt)
local function GetDistanceSqr(v1, v2)
    local dx = v1.x - v2.x
    local dy = v1.y - v2.y
    local dz = v1.z - v2.z
    return dx * dx + dy * dy + dz * dz
end

local function IsEnemy(player, localPlayer)
    return player:GetTeamNumber() ~= localPlayer:GetTeamNumber()
end

local function IsInvisibleOrDisguisedSpy(player)
    return player:InCond(4) or player:InCond(2) or player:InCond(3)
end

local function ResetScript()
    ClearData()
    scriptActive = true
end

local function UpdatePlayerState(data, isVisible, currentTime)
    if isVisible then
        data.state = VisibilityState.VISIBLE
        data.visibleStartTime = data.visibleStartTime or currentTime
        data.lastSeenTime = currentTime
    else
        if data.state == VisibilityState.VISIBLE then
            data.state = VisibilityState.RECENTLY_INVISIBLE
            data.lastInvisibleTime = currentTime
        elseif data.state == VisibilityState.RECENTLY_INVISIBLE and 
               currentTime - data.lastInvisibleTime > visibility_timeout then
            data.state = VisibilityState.UNSEEN
            data.visibleStartTime = nil
        end
    end
end

local function ShouldShowTrail(data, currentTime)
    if data.state == VisibilityState.VISIBLE then
        return currentTime - data.visibleStartTime <= max_visible_duration
    elseif data.state == VisibilityState.RECENTLY_INVISIBLE then
        return currentTime - data.lastInvisibleTime <= visibility_timeout
    end
    return false
end

local function aUpdate()
    local newMap = engine.GetMapName()
    if newMap ~= currentMap then
        currentMap = newMap
        ResetScript()
        return
    end

    if not scriptActive or engine.IsGameUIVisible() then return end
    
    local localPlayer = entities.GetLocalPlayer()
    if not localPlayer then return end

    -- Check for respawn
    local currentLifeState = localPlayer:GetPropInt("m_lifeState")
    if lastLifeState == 2 and currentLifeState == 0 then  -- Player respawned
        ClearData()
    end
    lastLifeState = currentLifeState

    -- Clean up invalid targets periodically
    CleanInvalidTargets()

    local localPos = localPlayer:GetAbsOrigin()
    local currentTick = globals.TickCount()
    local currentTime = globals.RealTime()
    local maxDistanceSqr = max_trail_distance * max_trail_distance

    if currentTick - lastUpdateTick >= update_interval then
        for i = 1, globals.MaxClients() do
            local player = entities.GetByIndex(i)
            if not player or not player:IsValid() or player:IsDormant() then goto continue end
            
            local info = client.GetPlayerInfo(i)
            local steamID = info and info.SteamID
            
            if not steamID or steamID == "BOT" then goto continue end
            
            if player:IsAlive() and player ~= localPlayer and 
               IsEnemy(player, localPlayer) and 
               not IsInvisibleOrDisguisedSpy(player) then
                
                local playerPos = player:GetAbsOrigin()
                local distanceSqr = GetDistanceSqr(localPos, playerPos)
                
                if distanceSqr > maxDistanceSqr then goto continue end
                
                local isVisible = IsVisible(player, localPlayer)
                
                -- Adjust height
                playerPos.z = playerPos.z + trailHeightUnits
                
                if not playerData[steamID] then
                    playerData[steamID] = {
                        positions = {},
                        times = {},
                        color = generateColor(steamID),
                        state = VisibilityState.UNSEEN
                    }
                end
                
                local data = playerData[steamID]
                UpdatePlayerState(data, isVisible, currentTime)
                
                if ShouldShowTrail(data, currentTime) then
                    if #data.positions == 0 or 
                       GetDistanceSqr(data.positions[1], playerPos) > 100 then -- Only add position if moved enough
                        table.insert(data.positions, 1, playerPos)
                        table.insert(data.times, 1, currentTime)
                        if #data.positions > trail_length then
                            table.remove(data.positions)
                            table.remove(data.times)
                        end
                    end
                elseif #data.positions > 0 then
                    data.positions = {}
                    data.times = {}
                end
            else
                playerData[steamID] = nil
            end
            
            ::continue::
        end
        
        lastUpdateTick = currentTick
    end
end

local function aDraw()
    if not scriptActive or engine.IsGameUIVisible() then return end
    
    local currentTime = globals.RealTime()
    
    for steamID, data in pairs(playerData) do
        if #data.positions > 1 and ShouldShowTrail(data, currentTime) then
            local step = data.state == VisibilityState.VISIBLE and 1 or 2
            local color = data.color
            
            for i = 1, #data.positions - 1, step do
                local timeDiff = currentTime - data.times[i]
                local alpha = floor(max(0, 255 * (1 - timeDiff / fade_time)))
                
                if alpha <= 0 then break end
                
                local startPos = client.WorldToScreen(data.positions[i])
                local endPos = client.WorldToScreen(data.positions[i + 1])
                
                if startPos and endPos then
                    -- Set color with proper alpha before drawing each line segment
                    draw.Color(color[1], color[2], color[3], alpha)
                    draw.Line(floor(startPos[1]), floor(startPos[2]), floor(endPos[1]), floor(endPos[2]))
                end
            end
        end
    end
end

local function OnLoad()
    ResetScript()
end

callbacks.Register("CreateMove", "aUpdate", aUpdate)
callbacks.Register("Draw", "aDraw", aDraw)
callbacks.Register("Unload", function()
    scriptActive = false
    colorCache = nil
    playerData = nil
    nextVisCheck = nil
    visibilityCache = nil
end)

OnLoad()
