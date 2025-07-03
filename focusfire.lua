-- Configuration
local CONFIG = {
    minAttackers = 2,  -- Minimum number of different attackers required to highlight target
    trackerTimeWindow = 4.5,  -- How long to consider someone as being "targeted" after being shot
    cleanupInterval = 0.5,  -- How often to clean up invalid targets (seconds)
    
    -- Visualization modes
    enableChams = true,  -- Enable chams visualization
    enableBox = true,  -- Enable border box visualization
    visibleOnly = false, -- Only show effects on visible players
    
    -- Box settings
    boxColor = {r = 255, g = 0, b = 0, a = 190},  -- RGBA color for the box
    boxThickness = 2,  -- Thickness of the box border
    boxPadding = 3,    -- Padding around the player model
    useCorners = true,  -- Use corners instead of full box
    cornerLength = 10   -- Length of corner lines when using corners
}

-- Pre-cache functions and variables
local RealTime = globals.RealTime
local TraceLine = engine.TraceLine
local GetByUserID = entities.GetByUserID
local GetLocalPlayer = entities.GetLocalPlayer
local targetData = {}
local lastLifeState = 2  -- Start with LIFE_DEAD (2)
local chamsMaterial = nil
local myfont = draw.CreateFont("Verdana", 16, 800)
local nextCleanupTime = 0
local nextVisCheck = {}
local visibilityCache = {}

-- Create materials
local function InitializeMaterials()
    if not chamsMaterial then
        chamsMaterial = materials.Create("targeted_chams", [[
            "VertexLitGeneric"
            {
                "$basetexture" "vgui/white_additive"
                "$bumpmap" "vgui/white_additive"
                "$color2" "[100 0.5 0.5]"
                "$selfillum" "1"
                "$ignorez" "1"
                "$selfIllumFresnel" "1"
                "$selfIllumFresnelMinMaxExp" "[0.1 0.2 0.3]"
                "$selfillumtint" "[0 0.3 0.6]"
            }
        ]])
    end
end

-- Initialize materials on script load
InitializeMaterials()

-- Improved visibility check with caching
local function IsPlayerVisible(player)
    if not CONFIG.visibleOnly then return true end
    
    local localPlayer = GetLocalPlayer()
    if not player or not localPlayer then return false end
    
    local id = player:GetIndex()
    local curTick = globals.TickCount()
    
    -- Use cached result if available
    if nextVisCheck[id] and curTick < nextVisCheck[id] then
        return visibilityCache[id] or false
    end
    
    -- Get eye position for more accurate tracing
    local localEyePos = localPlayer:GetAbsOrigin() + localPlayer:GetPropVector("localdata", "m_vecViewOffset[0]")
    local targetPos = player:GetAbsOrigin() + Vector3(0, 0, 40) -- Aim for chest area
    
    -- Use appropriate mask based on team
    local isTeammate = player:GetTeamNumber() == localPlayer:GetTeamNumber()
    local mask = isTeammate and MASK_SHOT_HULL or MASK_SHOT
    
    local trace = TraceLine(localEyePos, targetPos, mask)
    
    -- Cache result
    visibilityCache[id] = trace.entity == player or trace.fraction > 0.99
    nextVisCheck[id] = curTick + 2 -- Cache for 2 ticks
    
    return visibilityCache[id]
end

-- Draw corner lines
local function DrawCorners(x1, y1, x2, y2)
    local length = CONFIG.cornerLength
    
    -- Top left corner
    draw.Line(x1, y1, x1 + length, y1)  -- Horizontal
    draw.Line(x1, y1, x1, y1 + length)  -- Vertical
    
    -- Top right corner
    draw.Line(x2, y1, x2 - length, y1)  -- Horizontal
    draw.Line(x2, y1, x2, y1 + length)  -- Vertical
    
    -- Bottom left corner
    draw.Line(x1, y2, x1 + length, y2)  -- Horizontal
    draw.Line(x1, y2, x1, y2 - length)  -- Vertical
    
    -- Bottom right corner
    draw.Line(x2, y2, x2 - length, y2)  -- Horizontal
    draw.Line(x2, y2, x2, y2 - length)  -- Vertical
end

-- Clean expired attackers 
local function CleanExpiredAttackers(currentTime, targetInfo)
    local newAttackers = {}
    local count = 0
    local hasExpired = false
    
    for attackerIndex, timestamp in pairs(targetInfo.attackers) do
        if currentTime - timestamp <= CONFIG.trackerTimeWindow then
            newAttackers[attackerIndex] = timestamp
            count = count + 1
        else
            hasExpired = true
        end
    end
    
    targetInfo.attackerCount = count
    targetInfo.isMultiTargeted = (count >= CONFIG.minAttackers)
    
    return hasExpired and newAttackers or targetInfo.attackers
end

-- Clean up invalid targets
local function CleanInvalidTargets()
    local currentTime = RealTime()
    if currentTime < nextCleanupTime then return end
    
    nextCleanupTime = currentTime + CONFIG.cleanupInterval
    
    local localPlayer = GetLocalPlayer()
    if not localPlayer then return end
    
    for entIndex, targetInfo in pairs(targetData) do
        local entity = entities.GetByIndex(entIndex)
        
        -- Clean if entity is invalid, dead, dormant, or on our team
        if not entity or 
           not entity:IsValid() or 
           not entity:IsAlive() or
           entity:IsDormant() or
           entity:GetTeamNumber() == localPlayer:GetTeamNumber() then
            targetData[entIndex] = nil
            nextVisCheck[entIndex] = nil
            visibilityCache[entIndex] = nil
            goto continue
        end
        
        -- Update target status and clean if no longer multi-targeted
        targetInfo.attackers = CleanExpiredAttackers(currentTime, targetInfo)
        if targetInfo.attackerCount < CONFIG.minAttackers then
            targetData[entIndex] = nil
            nextVisCheck[entIndex] = nil
            visibilityCache[entIndex] = nil
        end
        
        ::continue::
    end
end

-- Reset all data (used on respawn)
local function ResetTargetData()
    targetData = {}
    nextVisCheck = {}
    visibilityCache = {}
    nextCleanupTime = 0
end

-- Check if player should be visualized
local function ShouldVisualizePlayer(player)
    if not player or 
       not player:IsValid() or 
       not player:IsAlive() or
       player:IsDormant() then 
        return false 
    end
    
    local localPlayer = GetLocalPlayer()
    if not localPlayer or player:GetTeamNumber() == localPlayer:GetTeamNumber() then 
        return false 
    end
    
    local targetInfo = targetData[player:GetIndex()]
    return targetInfo and targetInfo.isMultiTargeted and IsPlayerVisible(player)
end

-- Damage event handler
local function OnPlayerHurt(event)
    if event:GetName() ~= 'player_hurt' then return end
    
    local victim = GetByUserID(event:GetInt("userid"))
    local attacker = GetByUserID(event:GetInt("attacker"))
    
    if not victim or not attacker or victim == attacker then return end
    
    -- Skip dormant entities
    if victim:IsDormant() or attacker:IsDormant() then return end
    
    local victimIndex = victim:GetIndex()
    if not targetData[victimIndex] then
        targetData[victimIndex] = {
            attackers = {},
            isMultiTargeted = false,
            attackerCount = 0
        }
    end
    
    local currentTime = RealTime()
    local targetInfo = targetData[victimIndex]
    
    -- Update attackers list
    targetInfo.attackers[attacker:GetIndex()] = currentTime
    CleanExpiredAttackers(currentTime, targetInfo)
end

-- Main draw function
local function OnDrawModel(ctx)
    local entity = ctx:GetEntity()
    if not entity or not entity:IsPlayer() then return end
    
    -- Skip dormant entities
    if entity:IsDormant() then return end
    
    if ShouldVisualizePlayer(entity) then
        -- Apply chams if enabled
        if CONFIG.enableChams then
            ctx:ForcedMaterialOverride(chamsMaterial)
        end
    end
end

-- Draw border box around targeted players
local function DrawBox()
    if not CONFIG.enableBox then return end
    
    -- Set up drawing
    draw.SetFont(myfont)
    draw.Color(CONFIG.boxColor.r, CONFIG.boxColor.g, CONFIG.boxColor.b, CONFIG.boxColor.a)
    
    -- Find players being targeted by multiple teammates
    for index, targetInfo in pairs(targetData) do
        local player = entities.GetByIndex(index)
        if player and ShouldVisualizePlayer(player) then
            -- Get player bounds in screen space
            local origin = player:GetAbsOrigin()
            local mins = player:GetMins()
            local maxs = player:GetMaxs()
            
            -- Calculate corners in world space
            local corners = {
                Vector3(origin.x + mins.x - CONFIG.boxPadding, origin.y + mins.y - CONFIG.boxPadding, origin.z + mins.z),
                Vector3(origin.x + maxs.x + CONFIG.boxPadding, origin.y + mins.y - CONFIG.boxPadding, origin.z + mins.z),
                Vector3(origin.x + maxs.x + CONFIG.boxPadding, origin.y + maxs.y + CONFIG.boxPadding, origin.z + mins.z),
                Vector3(origin.x + mins.x - CONFIG.boxPadding, origin.y + maxs.y + CONFIG.boxPadding, origin.z + mins.z),
                Vector3(origin.x + mins.x - CONFIG.boxPadding, origin.y + mins.y - CONFIG.boxPadding, origin.z + maxs.z + CONFIG.boxPadding),
                Vector3(origin.x + maxs.x + CONFIG.boxPadding, origin.y + mins.y - CONFIG.boxPadding, origin.z + maxs.z + CONFIG.boxPadding),
                Vector3(origin.x + maxs.x + CONFIG.boxPadding, origin.y + maxs.y + CONFIG.boxPadding, origin.z + maxs.z + CONFIG.boxPadding),
                Vector3(origin.x + mins.x - CONFIG.boxPadding, origin.y + maxs.y + CONFIG.boxPadding, origin.z + maxs.z + CONFIG.boxPadding)
            }
            
            -- Convert to screen coordinates
            local screenCorners = {}
            for _, corner in ipairs(corners) do
                local screen = client.WorldToScreen(corner)
                if screen then
                    table.insert(screenCorners, screen)
                end
            end
            
            -- Draw the border if we have screen coordinates
            if #screenCorners > 0 then
                -- Find bounds
                local minX, minY = math.huge, math.huge
                local maxX, maxY = -math.huge, -math.huge
                
                for _, screen in ipairs(screenCorners) do
                    minX = math.min(minX, screen[1])
                    minY = math.min(minY, screen[2])
                    maxX = math.max(maxX, screen[1])
                    maxY = math.max(maxY, screen[2])
                end
                
                -- Draw either corners or full box based on config
                if CONFIG.useCorners then
                    -- Draw corners with thickness
                    for i = 0, CONFIG.boxThickness do
                        DrawCorners(minX - i, minY - i, maxX + i, maxY + i)
                    end
                else
                    -- Draw full box with thickness
                    for i = 0, CONFIG.boxThickness do
                        draw.OutlinedRect(minX - i, minY - i, maxX + i, maxY + i)
                    end
                end
            end
        end
    end
end

-- Monitor respawns to reset data
local function OnDraw()
    local localPlayer = GetLocalPlayer()
    if not localPlayer then return end
    
    local currentLifeState = localPlayer:GetPropInt("m_lifeState")
    if lastLifeState == 2 and currentLifeState == 0 then
        ResetTargetData()
    end
    lastLifeState = currentLifeState
    
    -- Clean up invalid targets periodically
    CleanInvalidTargets()
    
    -- Draw box effects
    DrawBox()
end

-- Register callbacks
callbacks.Register("FireGameEvent", "MultiTargetTracker", OnPlayerHurt)
callbacks.Register("DrawModel", "MultiTargetChams", OnDrawModel)
callbacks.Register("Draw", "MultiTargetEffects", OnDraw)
callbacks.Register("Unload", "MultiTargetCleanup", function()
    ResetTargetData()
    chamsMaterial = nil
end)