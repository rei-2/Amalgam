-- User Options
local SHOW_TOP_LEFT_BOX = false         -- Show the top-left information box
local SHOW_ADV_TEXT = true              -- Show the uber advantage text
local SHOW_KRITZ = true                 -- Show KRITZ warning when enemy has kritz equipped
local IGNORE_KRITZ = false               -- Replace kritz percentage with fake uber. When this is enabled SHOW_KRITZ does nothing

local CONFIG = {
    showTopLeftBox = SHOW_TOP_LEFT_BOX, -- Use the user option instead of forcing true
    topLeftBox = {
        x = 22 * 6,
        y = 115,
        backgroundColor = {0, 0, 0, 100},
        font = {
            name = "Tahoma",
            size = -11,
            weight = 500,
            flags = FONTFLAG_OUTLINE | FONTFLAG_CUSTOM
        }
    },
    
    advantageText = {
        enabled = SHOW_ADV_TEXT,
        positionMode = "center",
        centerOffset = {
            x = -250,
            y = 250
        },
        position = {
            x = 500,
            y = 500
        },
        font = {
            name = "Tahoma",
            size = -18,
            weight = 700,
            flags = FONTFLAG_OUTLINE | FONTFLAG_CUSTOM
        }
    },
    
    kritzWarning = {
        enabled = SHOW_KRITZ,
        color = {128, 0, 128, 255} -- Purple color
    },
    
    ignoreKritz = IGNORE_KRITZ,
    medDeathDuration = 3,
    evenThresholdRange = 5,
    
    colors = {
        dead = {170, 170, 170, 255},
        lowUber = {255, 0, 0, 255},
        midUber = {255, 255, 0, 255},
        highUber = {0, 255, 0, 255},
        fullAd = {0, 255, 0, 255},
        fullDisad = {255, 0, 0, 255},
        even = {128, 128, 128, 255},
        theyUsed = {255, 165, 0, 255},
        weUsed = {0, 191, 255, 255}
    },
    
    uberThresholds = {
        mid = 40,
        high = 70
    },

    -- Official TF2 uber build rates
    buildRates = {
        uberRate = 2.5,         -- Normal Uber: 2.5% per second
        kritzRate = 3.125,      -- Kritzkrieg: 3.125% per second
        overhealPenalty = 0.5,  -- Rate is halved when overhealed
        kritzDetection = {
            dropThreshold = 10,  -- If Uber % drops by this amount, consider it used
            resetValue = 0      -- Value to reset to after usage detection
        }
    }
}

-- Initialize fonts
local mainFont = draw.CreateFont(CONFIG.topLeftBox.font.name, CONFIG.topLeftBox.font.size, CONFIG.topLeftBox.font.weight, CONFIG.topLeftBox.font.flags)
local advantageFont = draw.CreateFont(CONFIG.advantageText.font.name, CONFIG.advantageText.font.size, CONFIG.advantageText.font.weight, CONFIG.advantageText.font.flags)

-- State tracking variables
local prevUber = {[2] = 0, [3] = 0}
local status = {[2] = "", [3] = ""}
local uberDecreasing = {[2] = false, [3] = false}
local medDeathTime = {[2] = 0, [3] = 0}

-- Storage for tracking kritz and simulated uber
local medicsKritzTracker = {}

-- Helper to get a unique ID for the medic
local function getMedicID(entity)
    if not entity then return "unknown" end
    return entity:GetIndex() .. "_" .. (entity:GetName() or "unnamed")
end

-- Check if game is in pregame
local function isInPregame()
    return gamerules.GetRoundState() == 1 -- ROUND_PREGAME
end

local function getWeaponType(itemDefinitionIndex)
    if not itemDefinitionIndex then return "UBER", false end
    
    -- Important: This is where we decide to replace Kritzkrieg with fake Uber
    if CONFIG.ignoreKritz and itemDefinitionIndex == 35 then
        return "UBER", true  -- Return UBER as display type, with flag indicating it's actually Kritz
    end
    
    local types = {
        [29] = "UBER",
        [211] = "UBER",
        [663] = "UBER",
        [35] = "KRITZ",
        [411] = "QUICKFIX",
        [998] = "VACCINATOR"
    }
    return types[itemDefinitionIndex] or "UBER", false
end

local function getColorForUber(percentage, isAlive)
    if not isAlive then return CONFIG.colors.dead end
    if percentage == 0 then return CONFIG.colors.dead end
    if percentage > CONFIG.uberThresholds.high then return CONFIG.colors.highUber end
    if percentage > CONFIG.uberThresholds.mid then return CONFIG.colors.midUber end
    return CONFIG.colors.lowUber
end

local function drawWithColor(color, func)
    if color and func then
        draw.Color(table.unpack(color))
        func()
    end
end

local function getAdvantageTextPosition(textWidth, textHeight)
    if CONFIG.advantageText.positionMode == "center" then
        local screenWidth, screenHeight = draw.GetScreenSize()
        return math.floor(screenWidth / 2 - textWidth / 2 + CONFIG.advantageText.centerOffset.x),
               math.floor(screenHeight / 2 + CONFIG.advantageText.centerOffset.y)
    else
        return CONFIG.advantageText.position.x,
               CONFIG.advantageText.position.y
    end
end

-- Check if a medic is healing an overhealed target
local function isHealingOverhealedTarget(entity)
    if not entity or not entity:IsAlive() then
        return false
    end
    
    local medigun = entity:GetEntityForLoadoutSlot(1)
    if not medigun then
        return false
    end
    
    local healTarget = medigun:GetPropEntity("m_hHealingTarget")
    if not healTarget then
        return false
    end
    
    local maxHealth = healTarget:GetMaxHealth()
    local currentHealth = healTarget:GetHealth()
    
    -- In TF2, overheal penalty applies at 142.5% health
    return currentHealth > (maxHealth * 1.425)
end

-- Check if a medic is healing any target
local function isHealing(entity)
    if not entity or not entity:IsAlive() then
        return false
    end
    
    local medigun = entity:GetEntityForLoadoutSlot(1)
    if not medigun then
        return false
    end
    
    return medigun:GetPropEntity("m_hHealingTarget") ~= nil
end

-- Detect Kritz usage by analyzing patterns in the charge value over time
local function detectKritzUsage(tracker, actualKritzPercentage, currentTime)
    -- If kritz percentage dropped significantly, it was likely used
    if tracker.lastKritzValue > 0 and actualKritzPercentage < tracker.lastKritzValue - CONFIG.buildRates.kritzDetection.dropThreshold then
        return true
    end
    
    -- Track if we're currently building charge
    tracker.wasBuilding = actualKritzPercentage > tracker.lastKritzValue
    
    return false
end

-- Called when the medic has a Kritzkrieg but we want to display fake uber
local function translateKritzToUber(entity, medicID, actualKritzPercentage, currentTime)
    -- Create a new tracker entry if needed
    if not medicsKritzTracker[medicID] then
        medicsKritzTracker[medicID] = {
            lastKritzValue = actualKritzPercentage,
            lastUberValue = actualKritzPercentage * (CONFIG.buildRates.uberRate / CONFIG.buildRates.kritzRate), -- Convert initial value
            lastUpdateTime = currentTime,
            wasUsed = false,
            wasBuilding = false
        }
        return math.floor(medicsKritzTracker[medicID].lastUberValue)
    end
    
    local tracker = medicsKritzTracker[medicID]
    
    -- Check if kritz was used
    if detectKritzUsage(tracker, actualKritzPercentage, currentTime) then
        -- Mark as used and reset
        tracker.wasUsed = true
        tracker.lastKritzValue = actualKritzPercentage
        tracker.lastUberValue = CONFIG.buildRates.kritzDetection.resetValue
        tracker.lastUpdateTime = currentTime
        
        -- Force update status for immediate feedback
        local teamNumber = entity:GetTeamNumber()
        local localTeam = entities.GetLocalPlayer():GetTeamNumber()
        status[teamNumber] = teamNumber == localTeam and "WE USED" or "THEY USED"
        uberDecreasing[teamNumber] = true
        
        return tracker.lastUberValue
    end
    
    -- If Kritz value has increased, we need to translate to Uber rate
    if actualKritzPercentage > tracker.lastKritzValue then
        -- Calculate increase in Kritz percentage
        local kritzIncrease = actualKritzPercentage - tracker.lastKritzValue
        
        -- Translate to Uber increase using proper ratio of build rates
        local uberIncrease = kritzIncrease * (CONFIG.buildRates.uberRate / CONFIG.buildRates.kritzRate)
        
        -- Apply increase to our fake Uber value
        tracker.lastUberValue = math.min(100, tracker.lastUberValue + uberIncrease)
    -- If Kritz value has decreased (but not enough to trigger "used" detection), 
    -- we should still update our tracked value accordingly
    elseif actualKritzPercentage < tracker.lastKritzValue then
        local kritzDecrease = tracker.lastKritzValue - actualKritzPercentage
        -- Apply proportional decrease to our fake Uber value
        tracker.lastUberValue = math.max(0, tracker.lastUberValue - kritzDecrease)
    end
    
    -- Update tracker
    tracker.lastKritzValue = actualKritzPercentage
    tracker.lastUpdateTime = currentTime
    
    return math.floor(tracker.lastUberValue)
end

local function onDraw()
    if engine.IsGameUIVisible() or isInPregame() then return end

    local lineoffset = 0
    local players = entities.FindByClass("CTFPlayer")
    local medics = {[2] = {}, [3] = {}}
    local localPlayer = entities.GetLocalPlayer()
    if not localPlayer then return end
    
    local localTeam = localPlayer:GetTeamNumber()
    if not localTeam then return end
    
    local currentTime = globals.CurTime()

    -- Collect all medics first to determine box size
    local totalMedics = 0
    
    -- Find and count all medics
    for _, entity in pairs(players) do
        if entity and entity:GetTeamNumber() and entity:GetPropInt("m_iClass") == 5 then
            totalMedics = totalMedics + 1
        end
    end
    
    -- Draw the background box with dynamic size based on medic count
    if CONFIG.showTopLeftBox then
        drawWithColor(CONFIG.topLeftBox.backgroundColor, function()
            -- Add padding to ensure we have enough space
            local boxHeight = math.max(30, 25 + (totalMedics * 15))
            draw.FilledRect(5, CONFIG.topLeftBox.y - 5, math.floor(24 * 15), 
                          math.floor(CONFIG.topLeftBox.y - 5 + boxHeight))
        end)
    end

    -- Find and process medics
    for _, entity in pairs(players) do
        if entity and entity:GetTeamNumber() and entity:GetPropInt("m_iClass") == 5 then
            local teamNumber = entity:GetTeamNumber()
            local isAlive = entity:IsAlive()
            local medigun = entity:GetEntityForLoadoutSlot(1)
            
            if medigun then
                local chargeLevel = medigun:GetPropFloat("LocalTFWeaponMedigunData", "m_flChargeLevel") or 0
                local itemDefinitionIndex = medigun:GetPropInt("m_iItemDefinitionIndex")
                local weaponName, isKritzOverride = getWeaponType(itemDefinitionIndex)
                local percentageValue = math.floor(chargeLevel * 100)
                local realWeaponName = weaponName
                
                -- If this is a Kritzkrieg but we're showing it as Uber
                if isKritzOverride then
                    local medicID = getMedicID(entity)
                    local actualKritzPercentage = percentageValue
                    
                    -- Translate Kritz percentage to Uber percentage
                    percentageValue = translateKritzToUber(entity, medicID, actualKritzPercentage, currentTime)
                    
                    -- Store real weapon name for kritz warning
                    realWeaponName = "KRITZ"
                end
                
                table.insert(medics[teamNumber], {
                    name = entity:GetName() or "Unknown",
                    weaponName = weaponName,        -- This is what's displayed (could be UBER when actually KRITZ)
                    realWeaponName = realWeaponName, -- This is the actual weapon type
                    uber = percentageValue,
                    isAlive = isAlive
                })

                -- Handle status tracking
                if not isAlive and status[teamNumber] ~= "MED DIED" then
                    status[teamNumber] = "MED DIED"
                    medDeathTime[teamNumber] = currentTime
                    uberDecreasing[teamNumber] = false
                elseif status[teamNumber] == "MED DIED" and isAlive then
                    status[teamNumber] = ""
                elseif isAlive and percentageValue < (prevUber[teamNumber] or 0) - 10 then
                    if status[teamNumber] ~= "THEY USED" and status[teamNumber] ~= "WE USED" then
                        status[teamNumber] = teamNumber == localTeam and "WE USED" or "THEY USED"
                    end
                    uberDecreasing[teamNumber] = true
                elseif isAlive and percentageValue > (prevUber[teamNumber] or 0) and uberDecreasing[teamNumber] then
                    status[teamNumber] = ""
                    uberDecreasing[teamNumber] = false
                end
                
                prevUber[teamNumber] = percentageValue

                if CONFIG.showTopLeftBox then
                    draw.SetFont(mainFont)
                    local color = getColorForUber(percentageValue, isAlive)
                    
                    -- Calculate y position with proper spacing
                    local yPos = math.floor(CONFIG.topLeftBox.y + 15 + (lineoffset * 15))
                    
                    drawWithColor(color, function()
                        local name = entity:GetName() or "Unknown"
                        -- Draw medic name and weapon type (shows UBER even if it's KRITZ when ignoreKritz=true)
                        draw.Text(20, yPos, name .. " -> " .. weaponName)
                        
                        -- Draw uber percentage or DEAD status
                        draw.Text(math.floor(22 * 15), yPos,
                                isAlive and (percentageValue .. "%") or "DEAD")
                    end)
                    lineoffset = lineoffset + 1
                end
            end
        end
    end

    -- Draw advantage text and kritz warning if we have medics on both teams
    if CONFIG.advantageText.enabled then
        -- First safely check if we have valid medics array
        local redMedics = medics[2]
        local bluMedics = medics[3]
        
        -- Verify both teams have medic tables
        if redMedics and bluMedics then
            -- Safely get first medic from each team
            local redMedic = redMedics[1]
            local bluMedic = bluMedics[1]
            
            -- Make sure we have both medics
            if redMedic and bluMedic then
                -- Get medic status safely with default values
                local redUber = (redMedic.isAlive and redMedic.uber) or 0
                local bluUber = (bluMedic.isAlive and bluMedic.uber) or 0
                
                -- Verify we have a valid local team
                if localTeam == 2 or localTeam == 3 then
                    local enemyTeam = localTeam == 2 and 3 or 2
                    local friendlyTeam = localTeam
                    
                    -- Calculate difference based on team
                    local difference = (localTeam == 2) and (redUber - bluUber) or (bluUber - redUber)
                    
                    -- Safely get friendly and enemy medics
                    local friendlyMedic = medics[friendlyTeam] and medics[friendlyTeam][1]
                    local enemyMedic = medics[enemyTeam] and medics[enemyTeam][1]
                    
                    -- Only proceed if we have both medics
                    if friendlyMedic and enemyMedic then
                        local displayText, textColor
                        
                        -- Safely check medic death times
                        local friendlyDeathTime = medDeathTime[friendlyTeam] or 0
                        local enemyDeathTime = medDeathTime[enemyTeam] or 0
                        
                        -- Status checks with safe defaults
                        local friendlyStatus = status[friendlyTeam] or ""
                        local enemyStatus = status[enemyTeam] or ""
                        
                        if friendlyStatus == "MED DIED" then
                            if currentTime - friendlyDeathTime <= CONFIG.medDeathDuration then
                                displayText = "OUR MED DIED"
                                textColor = CONFIG.colors.fullDisad
                            else
                                displayText = "FULL DISAD"
                                textColor = CONFIG.colors.fullDisad
                            end
                        elseif enemyStatus == "MED DIED" and 
                            currentTime - enemyDeathTime <= CONFIG.medDeathDuration then
                            displayText = "THEIR MED DIED"
                            textColor = CONFIG.colors.fullAd
                        elseif not enemyMedic.isAlive then
                            displayText = "FULL AD"
                            textColor = CONFIG.colors.fullAd
                        elseif not friendlyMedic.isAlive then
                            displayText = "FULL DISAD"
                            textColor = CONFIG.colors.fullDisad
                        elseif friendlyMedic.uber == 100 and enemyMedic.uber < 100 then
                            displayText = string.format("FULL AD: %d%%", difference)
                            textColor = CONFIG.colors.fullAd
                        elseif enemyMedic.uber == 100 and friendlyMedic.uber < 100 then
                            displayText = string.format("FULL DISAD: %d%%", math.abs(difference))
                            textColor = CONFIG.colors.fullDisad
                        elseif enemyStatus == "THEY USED" then
                            displayText = "THEY USED"
                            textColor = CONFIG.colors.theyUsed
                        elseif friendlyStatus == "WE USED" then
                            displayText = "WE USED"
                            textColor = CONFIG.colors.weUsed
                        elseif math.abs(difference) <= CONFIG.evenThresholdRange then
                            displayText = "EVEN"
                            textColor = CONFIG.colors.even
                        elseif difference > 0 then
                            displayText = string.format("AD: %d%%", difference)
                            textColor = CONFIG.colors.fullAd
                        else
                            displayText = string.format("DISAD: %d%%", math.abs(difference))
                            textColor = CONFIG.colors.fullDisad
                        end
                        
                        -- Only draw if we have both text and color
                        if displayText and textColor then
                            -- Safely draw with proper checks
                            if draw.SetFont and advantageFont then
                                draw.SetFont(advantageFont)
                                local textWidth, textHeight = draw.GetTextSize(displayText)
                                if textWidth and textHeight then
                                    local x, y = getAdvantageTextPosition(textWidth, textHeight)
                                    if x and y then
                                        -- Draw KRITZ warning if enabled and enemy has kritz
                                        -- Note we use realWeaponName to check for actual kritz
                                        if CONFIG.kritzWarning.enabled and 
                                           enemyMedic.isAlive and 
                                           enemyMedic.realWeaponName == "KRITZ" and
                                           not CONFIG.ignoreKritz then
                                            local kritzText = "KRITZ"
                                            -- Get size of the KRITZ text for centering
                                            local kritzWidth, kritzHeight = draw.GetTextSize(kritzText)
                                            -- Position directly above the advantage text, centered
                                            local kritzX = math.floor(x + (textWidth - kritzWidth) / 2)
                                            local kritzY = math.floor(y - kritzHeight - 5) -- 5 pixels gap
                                            
                                            drawWithColor(CONFIG.kritzWarning.color, function()
                                                draw.Text(kritzX, kritzY, kritzText)
                                            end)
                                        end
                                        
                                        -- Draw advantage text
                                        drawWithColor(textColor, function()
                                            draw.Text(x, y, displayText)
                                        end)
                                    end
                                end
                            end
                        end
                    end
                end
            end
        end
    end
end

-- Clean up any previous instances and register the new callback
callbacks.Unregister("Draw", "medigunDraw")
callbacks.Register("Draw", "medigunDraw", onDraw)