-- Camera state and configuration
local camera_x_position = 5
local camera_y_position = 300
local camera_width = 500 
local camera_height = 300
local CAMERA_OFFSET = 35
local TARGET_LOCK_DURATION = 5
local TARGET_SEARCH_RADIUS = 146 * 4  -- Increased to 4x sticky explosion radius
local OCCLUSION_CHECK_INTERVAL = 0.1
local ANGLE_INTERPOLATION_SPEED = 0.1

-- Sticky and target tracking state
local stickies = {}
local current_sticky = nil
local current_target = nil
local smoothed_angles = EulerAngles(0, 0, 0)
local target_lock_time = 0
local target_visible = false
local last_occlusion_check = 0

-- Sticky cycling state
local last_key_press = globals.RealTime()
local key_delay = 0.2
local visited_stickies = {}

-- Material state
local materials_initialized = false
local stickyCameraTexture = nil
local stickyCameraMaterial = nil
local invisibleMaterial = nil

local STICKY_DORMANT_DISTANCE = 1200  -- stickies go dormant around min 800-1200 max 2000-2200 units
local STICKY_WARNING_THRESHOLD = 0.70  -- 0.75 = Show warning at 75% of max distance (around 1500 units)
local warning_font = draw.CreateFont("Tahoma", 16, 800, FONTFLAG_OUTLINE)

local chamsMaterial = nil
local STICKY_CHAMS_DISTANCE = 166 -- max distance for chams in hammer units

-- Add this function at the beginning of your script
local function IsDemoman()
    local localPlayer = entities.GetLocalPlayer()
    if not localPlayer then return false end
    return localPlayer:GetPropInt("m_iClass") == 4  -- 4 is Demoman's class ID
end

-- Initialize chams material
local function InitializeChams()
    if not chamsMaterial then
        chamsMaterial = materials.Create("sticky_target_chams", [[
            "UnlitGeneric"
            {
                "$basetexture" "vgui/white_additive"
                "$color2" "[1 0 0]"
            }
        ]])
    end
end

-- Font for HUD
local font = draw.CreateFont("Tahoma", 12, 800, FONTFLAG_OUTLINE)

local function InitializeMaterials()
    if materials_initialized then return true end
    
    -- Clean up any existing materials first
    if stickyCameraTexture then
        stickyCameraTexture = nil
    end
    
    -- Create texture
    stickyCameraTexture = materials.CreateTextureRenderTarget("stickyCamMaterial", camera_width, camera_height)
    if not stickyCameraTexture then
        print("Failed to create camera texture")
        return false
    end
    
    -- Create material using the texture
    local materialName = "stickyCamMaterial"
    stickyCameraMaterial = materials.Create("stickyCamMaterial", string.format([[
        UnlitGeneric
        {
            $basetexture    "%s"
            $ignorez        1
            $nofog         1
        }
    ]], materialName))
    if not stickyCameraMaterial then
        print("Failed to create camera material")
        if stickyCameraTexture then
            stickyCameraTexture = nil
        end
        return false
    end

    invisibleMaterial = materials.Create("invisible_material", [[
        VertexLitGeneric
        {
            $basetexture    "vgui/white"
            $no_draw        1
        }
    ]])
    
    materials_initialized = true
    return true
end

local function CalculateAngles(source, dest)
    local M_RADPI = 180 / math.pi
    local delta = Vector3(dest.x - source.x, dest.y - source.y, dest.z - source.z)
    local hyp = math.sqrt(delta.x * delta.x + delta.y * delta.y)
    
    -- Calculate pitch
    local pitch = math.atan(delta.z / hyp) * M_RADPI
    
    -- Calculate yaw using atan
    local yaw = math.atan(delta.y / delta.x) * M_RADPI
    
    -- Adjust yaw based on quadrant
    if delta.x < 0 then
        yaw = yaw + 180
    elseif delta.y < 0 then
        yaw = yaw + 360
    end
    
    -- Handle NaN cases
    if pitch ~= pitch then pitch = 0 end
    if yaw ~= yaw then yaw = 0 end
    
    return EulerAngles(-pitch, yaw, 0)
end

local function CheckPlayerVisibility(startPos, player)
    if not player:IsValid() or not player:IsAlive() or player:IsDormant() then
        return false
    end

    local hitboxes = player:GetHitboxes()
    if not hitboxes then return false end
    
    local spine = hitboxes[4]
    if not spine then return false end
    
    local spineCenter = Vector3(
        (spine[1].x + spine[2].x) / 2,
        (spine[1].y + spine[2].y) / 2,
        (spine[1].z + spine[2].z) / 2
    )
    
    local trace = engine.TraceLine(startPos, spineCenter, MASK_SHOT)
    return trace.fraction > 0.99 or trace.entity == player
end

local function IsStickyOnCeiling(sticky)
    local pos = sticky:GetAbsOrigin()
    
    -- Check if there's something solid above us and nothing below
    local ceilingTrace = engine.TraceLine(pos, pos + Vector3(0, 0, 10), MASK_SOLID)
    local floorTrace = engine.TraceLine(pos, pos - Vector3(0, 0, 10), MASK_SOLID)
    
    return ceilingTrace.fraction < 0.3 and floorTrace.fraction > 0.7
end

-- Add proper nil checks for Vector3 operations
local function AddVectors(vec1, vec2)
    if not vec1 or not vec2 then return Vector3(0,0,0) end
    return Vector3(
        vec1.x + vec2.x,
        vec1.y + vec2.y, 
        vec1.z + vec2.z
    )
end

local function SubtractVectors(vec1, vec2)
    if not vec1 or not vec2 then return Vector3(0,0,0) end
    return Vector3(
        vec1.x - vec2.x,
        vec1.y - vec2.y,
        vec1.z - vec2.z
    )
end

local function MultiplyVector(vec, scalar)
    if not vec then return Vector3(0,0,0) end
    return Vector3(
        vec.x * scalar,
        vec.y * scalar,
        vec.z * scalar
    )
end

local function GetStickyNormal(sticky)
    if not sticky or not sticky:IsValid() then
        return Vector3(0, 0, 1) -- Default upward normal
    end
    
    local pos = sticky:GetAbsOrigin()
    if not pos then return Vector3(0, 0, 1) end
    
    local upTrace = engine.TraceLine(pos, AddVectors(pos, Vector3(0, 0, 5)), MASK_SOLID)
    
    -- If there's something very close above us, we're probably on a ceiling
    if upTrace.fraction < 0.2 then
        return Vector3(0, 0, 1)  -- Return upward normal
    end
    
    local downTrace = engine.TraceLine(pos, AddVectors(pos, Vector3(0, 0, -5)), MASK_SOLID)
    if downTrace.fraction < 1 then
        return downTrace.plane or Vector3(0, 0, 1)
    end
    
    return Vector3(0, 0, 1)  -- Default to up if no surface found
end

local function CalculateCameraOffset(stickyPos, normal)
    if not stickyPos or not normal then
        return Vector3(0, 0, 0)
    end

    -- Check which direction is blocked
    local upTrace = engine.TraceLine(
        stickyPos, 
        AddVectors(stickyPos, Vector3(0, 0, 15)), 
        MASK_SOLID
    )
    
    local downTrace = engine.TraceLine(
        stickyPos, 
        AddVectors(stickyPos, Vector3(0, 0, -15)), 
        MASK_SOLID
    )
    
    -- If up is blocked but down is open, force camera downward
    if upTrace.fraction < 0.5 and downTrace.fraction > 0.5 then
        return AddVectors(stickyPos, Vector3(0, 0, -CAMERA_OFFSET))
    end
    
    -- If down is blocked but up is open, force camera upward
    if downTrace.fraction < 0.5 and upTrace.fraction > 0.5 then
        return AddVectors(stickyPos, Vector3(0, 0, CAMERA_OFFSET))
    end
    
    -- If neither is clearly blocked, use the normal
    return AddVectors(stickyPos, MultiplyVector(normal, CAMERA_OFFSET))
end

local function FindNearestVisiblePlayer(position)
    if not position then return nil end
    
    local localPlayer = entities.GetLocalPlayer()
    if not localPlayer then return nil end
    
    if current_target and globals.RealTime() - target_lock_time < TARGET_LOCK_DURATION then
        if current_target:IsValid() and current_target:IsAlive() and not current_target:IsDormant() then
            if globals.RealTime() - last_occlusion_check > OCCLUSION_CHECK_INTERVAL then
                target_visible = CheckPlayerVisibility(position, current_target)
                last_occlusion_check = globals.RealTime()
            end
            
            if target_visible then
                local targetPos = current_target:GetAbsOrigin()
                if targetPos then
                    local delta = SubtractVectors(targetPos, position)
                    if delta and delta:Length() <= TARGET_SEARCH_RADIUS then
                        return current_target
                    end
                end
            end
        end
    end
    
    local nearest = nil
    local nearestDist = TARGET_SEARCH_RADIUS
    
    local players = entities.FindByClass("CTFPlayer")
    for _, player in pairs(players) do
        if player:IsValid() and player:IsAlive() and not player:IsDormant() and
           player:GetTeamNumber() ~= localPlayer:GetTeamNumber() then
            
            local playerPos = player:GetAbsOrigin()
            local dist = (playerPos - position):Length()
            
            if dist < nearestDist and CheckPlayerVisibility(position, player) then
                nearest = player
                nearestDist = dist
            end
        end
    end
    
    if nearest and nearest ~= current_target then
        target_lock_time = globals.RealTime()
        target_visible = true
        last_occlusion_check = globals.RealTime()
    end
    
    return nearest
end

local function LerpAngles(start, target, factor)
    local dx = target.x - start.x
    local dy = target.y - start.y
    
    while dy > 180 do dy = dy - 360 end
    while dy < -180 do dy = dy + 360 end
    
    return EulerAngles(
        start.x + dx * factor,
        start.y + dy * factor,
        0
    )
end

local function UpdateStickies()
    local localPlayer = entities.GetLocalPlayer()
    if not localPlayer then return end
    
    -- Clean up invalid stickies from our list
    for i = #stickies, 1, -1 do
        if not stickies[i]:IsValid() or stickies[i]:IsDormant() then
            table.remove(stickies, i)
            -- Also remove from visited if it was there
            for j = #visited_stickies, 1, -1 do
                if visited_stickies[j] == stickies[i] then
                    table.remove(visited_stickies, j)
                end
            end
        end
    end
    
    -- Find and add new stickies
    local projectiles = entities.FindByClass("CTFGrenadePipebombProjectile")
    for _, proj in pairs(projectiles) do
        if proj:IsValid() and not proj:IsDormant() then
            local thrower = proj:GetPropEntity("m_hThrower")
            local isSticky = proj:GetPropInt("m_iType") == 1
            
            if thrower and thrower == localPlayer and isSticky then
                local found = false
                for _, existing in pairs(stickies) do
                    if existing == proj then
                        found = true
                        break
                    end
                end
                
                if not found then
                    table.insert(stickies, proj)
                end
            end
        end
    end

    -- Only update current_sticky if we don't have a valid one from cycling
    if not current_sticky or not current_sticky:IsValid() or current_sticky:IsDormant() then
        current_sticky = nil
        current_target = nil
        target_visible = false
        
        -- Find any valid sticky to use
        for i = #stickies, 1, -1 do
            local sticky = stickies[i]
            local vel = sticky:EstimateAbsVelocity()
            
            if vel:Length() < 1 then
                local stickyPos = sticky:GetAbsOrigin()
                if FindNearestVisiblePlayer(stickyPos) then
                    current_sticky = sticky
                    break
                end
            end
        end
    end
end

local function IsCameraViewClear(origin, angles)
    local forward = angles:Forward()
    local trace = engine.TraceLine(
        origin, 
        origin + forward * 50,
        MASK_SOLID
    )
    return trace.fraction > 0.9
end

-- Move this function definition before UpdateUserInput
local function CycleNextSticky()
    -- Find available stationary stickies
    local available_stickies = {}
    for _, sticky in ipairs(stickies) do
        if sticky and sticky:IsValid() and not sticky:IsDormant() then
            local vel = sticky:EstimateAbsVelocity()
            if vel:Length() < 1 then
                table.insert(available_stickies, sticky)
            end
        end
    end
    
    if #available_stickies == 0 then
        current_sticky = nil
        current_target = nil
        target_visible = false
        visited_stickies = {}
        return
    end
    
    -- Find unvisited stickies
    local unvisited_stickies = {}
    for _, sticky in ipairs(available_stickies) do
        local already_visited = false
        for _, visited in ipairs(visited_stickies) do
            if visited == sticky then
                already_visited = true
                break
            end
        end
        
        if not already_visited then
            table.insert(unvisited_stickies, sticky)
        end
    end
    
    -- Reset if all stickies visited
    if #unvisited_stickies == 0 then
        visited_stickies = {}
        unvisited_stickies = available_stickies
    end
    
    if #unvisited_stickies > 0 then
        current_sticky = unvisited_stickies[1]
        table.insert(visited_stickies, current_sticky)
        current_target = nil
        target_visible = false
        target_lock_time = 0
    end
end

local function UpdateUserInput()
    local current_time = globals.RealTime()
    
    if input.IsButtonPressed(KEY_TAB) and current_time - last_key_press > key_delay then
        CycleNextSticky()
        last_key_press = current_time
        return true
    end
    
    return false
end

local function GetStickyPlayerDistance(sticky)
    if not sticky or not sticky:IsValid() then return 0 end
    
    local localPlayer = entities.GetLocalPlayer()
    if not localPlayer then return 0 end
    
    local stickyPos = sticky:GetAbsOrigin()
    local playerPos = localPlayer:GetAbsOrigin()
    if not stickyPos or not playerPos then return 0 end
    
    local delta = SubtractVectors(stickyPos, playerPos)
    if not delta then return 0 end
    
    return delta:Length()
end

local function DrawStickyRangeWarning()
    if not current_sticky then return end
    
    local distance = GetStickyPlayerDistance(current_sticky)
    local warning_distance = STICKY_DORMANT_DISTANCE * STICKY_WARNING_THRESHOLD
    
    -- Only show warning when approaching max range
    if distance > warning_distance then
        draw.SetFont(warning_font)
        
        local remaining_distance = STICKY_DORMANT_DISTANCE - distance
        local warning_text = string.format("WARNING: Camera will go dormant in %.1f units", remaining_distance)
        
        -- Position warning at top of camera view, ensure integers
        local text_w, text_h = draw.GetTextSize(warning_text)
        local warning_x = math.floor(camera_x_position + (camera_width - text_w) / 2)
        local warning_y = math.floor(camera_y_position + 10)
        
        -- Draw warning background
        draw.Color(0, 0, 0, 180)
        draw.FilledRect(
            math.floor(warning_x - 5),
            math.floor(warning_y - 5), 
            math.floor(warning_x + text_w + 5),
            math.floor(warning_y + text_h + 5)
        )
        
        -- Draw warning text
        draw.Color(255, 50, 50, 255)
        draw.Text(warning_x, warning_y, warning_text)
        
        -- Draw distance bar, ensure integers
        local bar_width = 200
        local bar_height = 6
        local bar_x = math.floor(camera_x_position + (camera_width - bar_width) / 2)
        local bar_y = math.floor(warning_y + text_h + 8)
        
        -- Background bar
        draw.Color(50, 50, 50, 180)
        draw.FilledRect(math.floor(bar_x), math.floor(bar_y), 
                          math.floor(bar_x + bar_width), math.floor(bar_y + bar_height))
        
        -- Progress bar
        local progress = math.max(0, math.min(1, 1 - (distance / STICKY_DORMANT_DISTANCE)))
        local progress_width = math.floor(bar_width * progress)
        
        -- Color changes from green to red based on distance
        local r = math.floor(math.min(255, (1 - progress) * 510))
        local g = math.floor(math.min(255, progress * 510))
        draw.Color(r, g, 0, 255)
        draw.FilledRect(math.floor(bar_x), math.floor(bar_y), 
                       math.floor(bar_x + progress_width), math.floor(bar_y + bar_height))
    end
end

-- Add distance logging to help determine actual dormant distance
local function LogStickyDormantDistance()
    if not current_sticky then return end
    
    local distance = GetStickyPlayerDistance(current_sticky)
    if current_sticky:IsDormant() then
        print(string.format("Sticky went dormant at distance: %.1f units", distance))
    end
end

-- Check if target is within chams range
local function IsTargetWithinRange(target, stickyPos)
    if not target or not target:IsValid() or target:IsDormant() then return false end
    
    local targetPos = target:GetAbsOrigin()
    if not targetPos or not stickyPos then return false end
    
    -- Use SubtractVectors to safely handle nil values
    local delta = SubtractVectors(targetPos, stickyPos)
    local distance = delta:Length()
    return distance <= STICKY_CHAMS_DISTANCE
end

-- Register draw model callback for chams
callbacks.Register("DrawModel", function(ctx)
    if not IsDemoman() then return end
    if not current_sticky or not current_target or not chamsMaterial then return end
    
    local entity = ctx:GetEntity()
    if not entity or not entity:IsValid() or entity:IsDormant() then return end
    
    -- Only apply chams to our current target
    if entity == current_target then
        local stickyPos = current_sticky:GetAbsOrigin()
        if stickyPos and IsTargetWithinRange(current_target, stickyPos) then
            ctx:ForcedMaterialOverride(chamsMaterial)
        end
    end
    
    -- Make the current sticky invisible
    if entity == current_sticky and invisibleMaterial then
        ctx:ForcedMaterialOverride(invisibleMaterial)
    end
end)

callbacks.Register("Draw", function()
    if not IsDemoman() then return end
    if not current_sticky or not current_target then return end
    
    DrawStickyRangeWarning()
    
    local playerName = current_target:GetName() or "Unknown"
    local timeRemaining = math.max(0, TARGET_LOCK_DURATION - (globals.RealTime() - target_lock_time))
    local targetStatus = target_visible and "Tracking" or "Target Lost"
    
    -- Count only valid, stationary stickies
    local available_count = 0
    for _, sticky in ipairs(stickies) do
        if sticky and sticky:IsValid() and not sticky:IsDormant() then
            local vel = sticky:EstimateAbsVelocity()
            if vel and vel:Length() < 1 then
                available_count = available_count + 1
            end
        end
    end
    
    local title = string.format("Sticky Security Camera - %s: %s (%.1fs) [%d/%d]", 
        targetStatus, playerName, timeRemaining, 
        math.min(#visited_stickies, available_count), available_count)
        
    local w, h = draw.GetTextSize(title)
    draw.Color(255, 255, 255, 255)
    draw.SetFont(font)
    draw.Text(
        math.floor(camera_x_position + camera_width * 0.5 - w * 0.5),
        math.floor(camera_y_position - 16),
        title
    )
    
    -- Draw controls text
    draw.Color(255, 255, 255, 200)
    draw.Text(
        math.floor(camera_x_position + 5),
        math.floor(camera_y_position + camera_height + 5),
        "TAB - Cycle stickies"
    )
    
    -- Draw camera outline
    draw.Color(235, 64, 52, 255)
    draw.OutlinedRect(
        camera_x_position,
        camera_y_position,
        camera_x_position + camera_width,
        camera_y_position + camera_height
    )
    
    draw.OutlinedRect(
        camera_x_position,
        camera_y_position - 20,
        camera_x_position + camera_width,
        camera_y_position
    )
    
    draw.Color(130, 26, 17, 255)
    draw.FilledRect(
        camera_x_position + 1,
        camera_y_position - 19,
        camera_x_position + camera_width - 1,
        camera_y_position - 1
    )
end)

callbacks.Register("CreateMove", function(cmd)
    if not IsDemoman() then return end
    
    UpdateUserInput()
    UpdateStickies()
    
    if current_sticky and current_sticky:IsValid() and not current_sticky:IsDormant() then
        local stickyPos = current_sticky:GetAbsOrigin()
        if stickyPos then
            local normal = GetStickyNormal(current_sticky)
            local cameraPos = CalculateCameraOffset(stickyPos, normal)
            
            if cameraPos then
                current_target = FindNearestVisiblePlayer(cameraPos)
                
                if current_target and target_visible then
                    local targetPos = current_target:GetAbsOrigin()
                    if targetPos then
                        local targetAngles = CalculateAngles(cameraPos, AddVectors(targetPos, Vector3(0, 0, 50)))
                        smoothed_angles = LerpAngles(smoothed_angles, targetAngles, ANGLE_INTERPOLATION_SPEED)
                    end
                end
            end
        end
    else
        current_target = nil
        target_visible = false
    end
end)

-- Update PostRenderView to use correct ViewSetup
callbacks.Register("PostRenderView", function(view)
    if not IsDemoman() then return end
    if not materials_initialized and not InitializeMaterials() then
        return
    end
    
    if not current_sticky or not current_target or not stickyCameraMaterial or not stickyCameraTexture or not target_visible then 
        return 
    end
    
    local normal = GetStickyNormal(current_sticky)
    local stickyPos = current_sticky:GetAbsOrigin()
    if not stickyPos then return end
    
    local cameraOrigin = CalculateCameraOffset(stickyPos, normal)
    if not cameraOrigin then return end
    
    -- Create a proper ViewSetup object
    local customView = view
    customView.origin = cameraOrigin
    customView.angles = smoothed_angles

    if not IsCameraViewClear(cameraOrigin, smoothed_angles) then
        return
    end
    
    render.Push3DView(customView, E_ClearFlags.VIEW_CLEAR_COLOR | E_ClearFlags.VIEW_CLEAR_DEPTH, stickyCameraTexture)
    render.ViewDrawScene(true, true, customView)
    render.PopView()
    
    render.DrawScreenSpaceRectangle(
        stickyCameraMaterial, 
        camera_x_position, camera_y_position,
        camera_width, camera_height,
        0, 0, camera_width, camera_height,
        camera_width, camera_height
    )
end)

callbacks.Register("Unload", function()
    materials_initialized = false
    stickies = {}
    current_sticky = nil
    current_target = nil
    target_visible = false
    
    if stickyCameraTexture then
        stickyCameraTexture = nil
    end
    
    invisibleMaterial = nil
    stickyCameraMaterial = nil
    chamsMaterial = nil
end)

-- Initialize materials when script loads
InitializeMaterials()
InitializeChams()