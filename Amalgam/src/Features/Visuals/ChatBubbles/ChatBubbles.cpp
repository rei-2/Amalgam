#include "ChatBubbles.h"

// Complete voice command mappings based on TF2 source code analysis
// These map menu + item combinations to readable text
const std::unordered_map<int, std::unordered_map<int, std::string>> CChatBubbles::VOICE_MENU_MAP = {
    // Voice Menu 1 (Z key) - Basic commands
    {0, {
        {0, "MEDIC!"},
        {1, "Thanks!"},
        {2, "Go! Go! Go!"},
        {3, "Move Up!"},
        {4, "Go Left"},
        {5, "Go Right"},
        {6, "Yes"},
        {7, "No"}
    }},
    // Voice Menu 2 (X key) - Tactical commands  
    {1, {
        {0, "Incoming"},
        {1, "Spy!"},
        {2, "Sentry Ahead!"},
        {3, "Teleporter Here"},
        {4, "Dispenser Here"},
        {5, "Sentry Here"},
        {6, "Activate Charge!"},
        {7, "MEDIC: ÜberCharge Ready"}
    }},
    // Voice Menu 3 (C key) - Social commands
    {2, {
        {0, "Help!"},
        {1, "Cheers"},
        {2, "Jeers"},
        {3, "Positive"},
        {4, "Negative"},
        {5, "Nice Shot"},
        {6, "Good Job"},
        {7, "Battle Cry"}
    }}
};

std::string CChatBubbles::GetVoiceCommandText(int menu, int item)
{
    auto menuIt = VOICE_MENU_MAP.find(menu);
    if (menuIt != VOICE_MENU_MAP.end())
    {
        auto itemIt = menuIt->second.find(item);
        if (itemIt != menuIt->second.end())
        {
            return itemIt->second;
        }
    }
    return "Unknown Command";
}

Color_t CChatBubbles::GenerateColor(const std::string& steamID)
{
    if (m_ColorCache.find(steamID) != m_ColorCache.end())
        return m_ColorCache[steamID];
    
    // Simple hash function for color generation (same as OffScreenIndicators)
    uint32_t hash = 0;
    for (char c : steamID)
    {
        hash += static_cast<uint32_t>(c);
    }
    
    int r = static_cast<int>((hash * 11) % 200 + 55);
    int g = static_cast<int>((hash * 23) % 200 + 55);
    int b = static_cast<int>((hash * 37) % 200 + 55);
    
    Color_t color = {static_cast<byte>(r), static_cast<byte>(g), static_cast<byte>(b), 255};
    m_ColorCache[steamID] = color;
    return color;
}

void CChatBubbles::AddChatMessage(const std::string& message, const std::string& playerName, int entityIndex, bool isVoice)
{
    float currentTime = I::GlobalVars->curtime;
    
    // Add to player-specific data
    if (entityIndex > 0)
    {
        auto& playerData = m_PlayerData[entityIndex];
        
        // Voice command cooldown check
        if (isVoice && currentTime - playerData.lastVoiceTime < VOICE_COOLDOWN)
            return;
        
        if (isVoice)
            playerData.lastVoiceTime = currentTime;
        
        // Check for duplicate message in recent messages
        bool foundDuplicate = false;
        for (auto& existingMsg : playerData.messages)
        {
            if (existingMsg.message == message && (currentTime - existingMsg.timestamp) < 3.0f)
            {
                // Update existing message count and timestamp
                existingMsg.count++;
                existingMsg.timestamp = currentTime;
                foundDuplicate = true;
                
#ifdef _DEBUG
                // Debug: Confirm message count updated
                if (isVoice || message.find("[Voice]") != std::string::npos)
                {
                    I::CVar->ConsolePrintf("AddChatMessage: Updated '%s' count to %d for player %d\n", 
                                          message.c_str(), existingMsg.count, entityIndex);
                }
#endif
                break;
            }
        }
        
        if (!foundDuplicate)
        {
            // Create new message
            ChatBubbleMessage newMessage;
            newMessage.message = message;
            newMessage.playerName = playerName;
            newMessage.timestamp = currentTime;
            newMessage.isVoice = isVoice;
            newMessage.hasSmoothPos = false;
            newMessage.count = 1;
            
            // Add to front of list
            playerData.messages.insert(playerData.messages.begin(), newMessage);
            
#ifdef _DEBUG
            // Debug: Confirm message was added (only for voicelines and non-action sounds)
            if (isVoice || message.find("[Voice]") != std::string::npos)
            {
                I::CVar->ConsolePrintf("AddChatMessage: Added '%s' for player %d\n", 
                                      message.c_str(), entityIndex);
            }
#endif
            
            // Limit messages per player
            while (playerData.messages.size() > MAX_MESSAGES_PER_PLAYER)
            {
                playerData.messages.pop_back();
            }
        }
    }
    
    // Add to global chat log
    ChatBubbleMessage globalMessage;
    globalMessage.message = message;
    globalMessage.playerName = playerName;
    globalMessage.timestamp = currentTime;
    globalMessage.isVoice = isVoice;
    globalMessage.hasSmoothPos = false;
    globalMessage.count = 1;
    
    m_GlobalChatLog.insert(m_GlobalChatLog.begin(), globalMessage);
    
    // Limit global messages
    while (m_GlobalChatLog.size() > MAX_GLOBAL_MESSAGES)
    {
        m_GlobalChatLog.pop_back();
    }
}

void CChatBubbles::CleanOldMessages()
{
    float currentTime = I::GlobalVars->curtime;
    
    // Clean player messages
    for (auto it = m_PlayerData.begin(); it != m_PlayerData.end();)
    {
        auto& messages = it->second.messages;
        
        // Remove expired messages
        messages.erase(
            std::remove_if(messages.begin(), messages.end(),
                [currentTime](const ChatBubbleMessage& msg) {
                    return currentTime - msg.timestamp > MESSAGE_LIFETIME;
                }),
            messages.end()
        );
        
        // Remove player data if no messages remain
        if (messages.empty())
        {
            it = m_PlayerData.erase(it);
        }
        else
        {
            ++it;
        }
    }
    
    // Clean global messages
    m_GlobalChatLog.erase(
        std::remove_if(m_GlobalChatLog.begin(), m_GlobalChatLog.end(),
            [currentTime](const ChatBubbleMessage& msg) {
                return currentTime - msg.timestamp > MESSAGE_LIFETIME;
            }),
        m_GlobalChatLog.end()
    );
}

std::vector<std::string> CChatBubbles::WrapText(const std::string& text, float maxWidth)
{
    std::vector<std::string> words;
    std::vector<std::string> lines;
    
    // Split text into words
    std::string word;
    for (char c : text)
    {
        if (c == ' ' || c == '\t' || c == '\n')
        {
            if (!word.empty())
            {
                words.push_back(word);
                word.clear();
            }
        }
        else
        {
            word += c;
        }
    }
    if (!word.empty())
        words.push_back(word);
    
    // Wrap words into lines
    std::string currentLine;
    for (const auto& w : words)
    {
        std::string testLine = currentLine.empty() ? w : (currentLine + " " + w);
        Vec2 size = MeasureTextSize(testLine);
        
        if (size.x > maxWidth && !currentLine.empty())
        {
            lines.push_back(currentLine);
            currentLine = w;
        }
        else
        {
            currentLine = testLine;
        }
    }
    
    if (!currentLine.empty())
        lines.push_back(currentLine);
    
    return lines;
}

Vec2 CChatBubbles::MeasureTextSize(const std::string& text)
{
    auto font = H::Fonts.GetFont(FONT_ESP);
    return H::Draw.GetTextSize(text.c_str(), font);
}

Vec2 CChatBubbles::CalculateBubbleDimensions(const std::vector<std::string>& lines)
{
    float maxWidth = 0.0f;
    float totalHeight = 0.0f;
    
    for (const auto& line : lines)
    {
        Vec2 size = MeasureTextSize(line);
        maxWidth = std::max(maxWidth, size.x);
        totalHeight += size.y;
    }
    
    // Add padding and line spacing
    maxWidth = std::min(maxWidth + (BUBBLE_PADDING * 2), BUBBLE_MAX_WIDTH);
    totalHeight += (BUBBLE_PADDING * 2) + ((lines.size() - 1) * 2);
    
    return {maxWidth, totalHeight};
}

int CChatBubbles::CalculateOpacity(float messageAge)
{
    if (messageAge <= FADE_START_TIME)
        return 255;
    
    float fadeProgress = (messageAge - FADE_START_TIME) / (MESSAGE_LIFETIME - FADE_START_TIME);
    int opacity = static_cast<int>(255 * std::max(0.01f, 1.0f - fadeProgress));
    
    return opacity < 2 ? 0 : opacity;
}

float CChatBubbles::DrawChatBubble(ChatBubbleMessage& message, const Vec3& worldPos, float yOffset, int entityIndex)
{
    // Safety checks
    if (entityIndex <= 0 || entityIndex > I::GlobalVars->maxClients)
        return 0.0f;
        
    if (message.message.empty() || message.message.length() > 500)
        return 0.0f;
        
    // Get screen position
    Vec3 screenPos;
    if (!SDK::W2S(worldPos, screenPos))
        return 0.0f;
    
    // Calculate opacity based on message age
    float messageAge = I::GlobalVars->curtime - message.timestamp;
    int opacity = CalculateOpacity(messageAge);
    
    if (opacity <= 2)
        return 0.0f;
    
    // Prepare display text with count
    std::string displayText = message.message;
    if (message.count > 1) {
        displayText += " x" + std::to_string(message.count);
    }
    
    // Wrap text and calculate dimensions
    auto wrappedLines = WrapText(displayText, BUBBLE_MAX_WIDTH - (BUBBLE_PADDING * 2));
    Vec2 bubbleDims = CalculateBubbleDimensions(wrappedLines);
    
    // Calculate bubble position
    float bubbleX, bubbleY;
    
    if (Vars::Competitive::Features::ChatBubblesNonFloat.Value)
    {
        // Non-float behavior: Direct positioning like health bars
        bubbleX = screenPos.x;
        bubbleY = screenPos.y - bubbleDims.y - 20 - yOffset;
    }
    else
    {
        // Original floating behavior with screen bounds and smoothing
        int screenW, screenH;
        I::MatSystemSurface->GetScreenSize(screenW, screenH);
        
        bubbleX = std::max(bubbleDims.x / 2, 
                          std::min(static_cast<float>(screenW) - bubbleDims.x / 2, screenPos.x));
        bubbleY = std::max(bubbleDims.y, 
                          std::min(static_cast<float>(screenH) - 20, screenPos.y - bubbleDims.y - 20 - yOffset));
        
        // Smooth position interpolation
        if (!message.hasSmoothPos)
        {
            message.smoothPos = {bubbleX, bubbleY, 0.0f};
            message.hasSmoothPos = true;
        }
        else
        {
            message.smoothPos.x += (bubbleX - message.smoothPos.x) * SMOOTHING_FACTOR;
            message.smoothPos.y += (bubbleY - message.smoothPos.y) * SMOOTHING_FACTOR;
        }
        
        bubbleX = message.smoothPos.x;
        bubbleY = message.smoothPos.y;
    }
    
    // Get background color
    Color_t bgColor;
    if (Vars::Competitive::Features::ChatBubblesSteamIDColor.Value)
    {
        // Get player's SteamID for color generation (with safety checks)
        PlayerInfo_t pInfo = {};  // Initialize to zero
        std::string steamID = "default";
        
        try 
        {
            if (I::EngineClient && I::EngineClient->GetPlayerInfo(entityIndex, &pInfo) && pInfo.friendsID != 0)
            {
                steamID = std::to_string(pInfo.friendsID);
            }
        }
        catch (...)
        {
            steamID = "default";
        }
        
        Color_t playerColor = GenerateColor(steamID);
        bgColor = {playerColor.r, playerColor.g, playerColor.b, static_cast<byte>(opacity * 0.8f)};
    }
    else
    {
        // Default black background
        bgColor = {0, 0, 0, static_cast<byte>(opacity * 0.8f)};
    }
    
    // Draw background
    int rectX = static_cast<int>(bubbleX - bubbleDims.x / 2);
    int rectY = static_cast<int>(bubbleY - bubbleDims.y);
    int rectW = static_cast<int>(bubbleDims.x);
    int rectH = static_cast<int>(bubbleDims.y);
    
    H::Draw.FillRect(rectX, rectY, rectW, rectH, bgColor);
    
    // No border (white outline removed)
    
    // Calculate text color based on background brightness
    Color_t textColor;
    if (Vars::Competitive::Features::ChatBubblesSteamIDColor.Value)
    {
        // Calculate luminance of background color to determine text color
        float luminance = (0.299f * bgColor.r + 0.587f * bgColor.g + 0.114f * bgColor.b) / 255.0f;
        
        // Use black text on bright backgrounds, white text on dark backgrounds
        if (luminance > 0.5f)
        {
            textColor = {0, 0, 0, static_cast<byte>(opacity)}; // Black text
        }
        else
        {
            textColor = {255, 255, 255, static_cast<byte>(opacity)}; // White text
        }
    }
    else
    {
        // Default white text on black background
        textColor = {255, 255, 255, static_cast<byte>(opacity)};
    }
    
    float yTextOffset = 0.0f;
    
    for (const auto& line : wrappedLines)
    {
        Vec2 lineSize = MeasureTextSize(line);
        
        int textX = static_cast<int>(bubbleX - bubbleDims.x / 2 + BUBBLE_PADDING);
        int textY = static_cast<int>(bubbleY - bubbleDims.y + BUBBLE_PADDING + yTextOffset);
        
        H::Draw.String(H::Fonts.GetFont(FONT_ESP), textX, textY, textColor, ALIGN_TOPLEFT, line.c_str());
        
        yTextOffset += lineSize.y + 2;
    }
    
    return bubbleDims.y + 5;
}

void CChatBubbles::DrawPlayerBubbles(CTFPlayer* pPlayer)
{
    if (!pPlayer || !pPlayer->IsAlive())
        return;
    
    int playerIndex = pPlayer->entindex();
    if (playerIndex <= 0 || playerIndex > I::GlobalVars->maxClients)
        return;
        
    auto it = m_PlayerData.find(playerIndex);
    if (it == m_PlayerData.end() || it->second.messages.empty())
        return;
    
    // Debug removed to prevent spam
    
    // Get player head position
    Vec3 headPos = pPlayer->GetAbsOrigin() + Vec3(0, 0, 75);
    
    // Draw all messages for this player
    float yOffset = 0.0f;
    for (auto& message : it->second.messages)
    {
        yOffset += DrawChatBubble(message, headPos, yOffset, playerIndex);
    }
}

void CChatBubbles::OnVoiceSubtitle(int entityIndex, int menu, int item)
{
    // Voice commands are now disabled - they only work for teammates which is not useful
    // This function is kept for compatibility but does nothing
    return;
}

void CChatBubbles::OnChatMessage(bf_read& msgData)
{
    if (!Vars::Competitive::Features::ChatBubbles.Value)
        return;
    
    // This would handle SayText/SayText2 messages for chat
    // For now, we'll focus on voice commands which are more reliable
    // Chat message parsing can be added later if needed
}

void CChatBubbles::OnSoundPlayed(int entityIndex, const char* soundName)
{
    if (!Vars::Competitive::Features::ChatBubbles.Value || !Vars::Competitive::Features::ChatBubblesVoiceSounds.Value)
        return;
    
    if (!soundName || entityIndex <= 0)
        return;
    
    // Get the entity to validate it's a player
    auto pEntity = I::ClientEntityList->GetClientEntity(entityIndex);
    if (!pEntity)
        return;
    
    auto pPlayer = pEntity->As<CTFPlayer>();
    if (!pPlayer)
        return;
    
    // Optional team filtering
    if (Vars::Competitive::Features::ChatBubblesEnemyOnly.Value)
    {
        auto pLocal = H::Entities.GetLocal();
        if (!pLocal || pPlayer->m_iTeamNum() == pLocal->m_iTeamNum())
            return;
    }
    
    // Get player name (with safety checks)
    std::string playerName = "Player";
    try 
    {
        PlayerInfo_t pInfo = {};  // Initialize to zero
        if (I::EngineClient && I::EngineClient->GetPlayerInfo(entityIndex, &pInfo) && strlen(pInfo.name) > 0)
        {
            playerName = std::string(pInfo.name, strnlen(pInfo.name, sizeof(pInfo.name) - 1));
        }
    }
    catch (...)
    {
        playerName = "Player";
    }
    
    // Filter out irrelevant sounds - only show voice commands and important player sounds
    std::string soundPath = soundName;
    
    // Remove the `)` character that appears at the start of some sound paths
    if (!soundPath.empty() && soundPath[0] == ')')
        soundPath = soundPath.substr(1);
    
    std::transform(soundPath.begin(), soundPath.end(), soundPath.begin(), ::tolower);
    
    // Check for voice commands and voicelines - comprehensive detection  
    bool isVoiceSound = soundPath.find("vo/") != std::string::npos ||
                       soundPath.find("voice/") != std::string::npos ||
                       soundPath.find("misc/") != std::string::npos ||
                       soundPath.find("domination") != std::string::npos ||
                       soundPath.find("revenge") != std::string::npos ||
                       soundPath.find("battlecry") != std::string::npos ||
                       soundPath.find("paincrit") != std::string::npos ||
                       soundPath.find("painsharp") != std::string::npos ||
                       soundPath.find("painsevere") != std::string::npos ||
                       soundPath.find("autoonfire") != std::string::npos ||
                       soundPath.find("taunt") != std::string::npos ||
                       soundPath.find("_pain") != std::string::npos ||
                       soundPath.find("_death") != std::string::npos ||
                       soundPath.find("_laugh") != std::string::npos ||
                       soundPath.find("_yell") != std::string::npos ||
                       soundPath.find("medic") != std::string::npos ||
                       soundPath.find("thanks") != std::string::npos ||
                       soundPath.find("moveup") != std::string::npos ||
                       soundPath.find("incoming") != std::string::npos;
    
    // Check if it's a weapon sound
    //bool isWeaponSound = soundPath.find("weapons/") != std::string::npos;
    
    // Check if it's a player action sound (re-enabled for testing drawing)
    bool isPlayerAction = soundPath.find("player/") != std::string::npos ||
                         soundPath.find("items/") != std::string::npos;
    
    // Filter out footsteps and other noisy player actions
    if (isPlayerAction && (soundPath.find("footsteps") != std::string::npos ||
                          soundPath.find("flesh_impact") != std::string::npos ||
                          soundPath.find("body_medium_impact") != std::string::npos ||
                          soundPath.find("dirt") != std::string::npos ||
                          soundPath.find("concrete") != std::string::npos ||
                          soundPath.find("metal") != std::string::npos ||
                          soundPath.find("wood") != std::string::npos ||
                          soundPath.find("gravel") != std::string::npos ||
                          soundPath.find("grass") != std::string::npos ||
                          soundPath.find("sand") != std::string::npos ||
                          soundPath.find("tile") != std::string::npos ||
                          soundPath.find("water") != std::string::npos))
        return;
    
#ifdef _DEBUG
    // Debug: Log detected voice sounds
    if (isVoiceSound && pPlayer)
    {
        std::string playerName = "Unknown";
        PlayerInfo_t pInfo;
        if (I::EngineClient->GetPlayerInfo(entityIndex, &pInfo))
            playerName = pInfo.name;
            
        I::CVar->ConsolePrintf("ChatBubbles Voice: %s from [%s] entity %d\n", 
                              soundName, playerName.c_str(), entityIndex);
    }
#endif
    
    // Only show relevant sounds
    if (!isVoiceSound && !isPlayerAction) // && !isWeaponSound)
        return;
    
    // Extract just the filename from the sound path
    std::string soundFile = soundName;
    size_t lastSlash = soundFile.find_last_of("/\\");
    if (lastSlash != std::string::npos)
        soundFile = soundFile.substr(lastSlash + 1);
    
    // Remove file extension
    size_t lastDot = soundFile.find_last_of(".");
    if (lastDot != std::string::npos)
        soundFile = soundFile.substr(0, lastDot);
    
    // Clean up the filename
    std::string cleanedFile = soundFile;
    
    // Special handling for grenade_jump_lp to prevent spam
    if (cleanedFile.find("grenade_jump_lp") != std::string::npos)
    {
        cleanedFile = "grenade_jump_lp";
        
        // Check if this player already has this sound within the last 2 seconds
        auto& playerData = m_PlayerData[entityIndex];
        float currentTime = I::GlobalVars->curtime;
        
        for (const auto& msg : playerData.messages)
        {
            if (msg.message.find("grenade jump lp") != std::string::npos && 
                (currentTime - msg.timestamp) < 2.0f)
            {
                return; // Skip this sound to prevent spam
            }
        }
    }
    
    // Replace underscores with spaces
    std::replace(cleanedFile.begin(), cleanedFile.end(), '_', ' ');
    
    // Remove numbers from the end (e.g., "domination01" -> "domination")
    size_t pos = cleanedFile.length();
    while (pos > 0 && std::isdigit(cleanedFile[pos - 1])) {
        pos--;
    }
    if (pos < cleanedFile.length()) {
        cleanedFile = cleanedFile.substr(0, pos);
    }
    
    // Add the sound as a chat message with appropriate prefix
    std::string prefix = "[Sound] ";
    if (isVoiceSound) prefix = "[Voice] ";
    //else if (isWeaponSound) prefix = "[Weapon] ";
    else if (isPlayerAction) prefix = "[Action] ";
    
    AddChatMessage(prefix + cleanedFile, playerName, entityIndex, false);
}

void CChatBubbles::Draw()
{
    if (!Vars::Competitive::Features::ChatBubbles.Value)
        return;
    
    if (I::EngineVGui->IsGameUIVisible())
        return;
    
    auto pLocal = H::Entities.GetLocal();
    if (!pLocal)
        return;
    
    // Clean old messages
    CleanOldMessages();
    
#ifdef _DEBUG
    // Debug: Check if we have any messages (limit spam)
    static int drawCallCount = 0;
    drawCallCount++;
    if (!m_PlayerData.empty() && drawCallCount % 60 == 0)  // Log every 60 frames
    {
        I::CVar->ConsolePrintf("Draw call %d: %d players with messages\n", 
                              drawCallCount, static_cast<int>(m_PlayerData.size()));
    }
#endif
    
    // Draw bubbles for all players (with safety checks)
    try 
    {
        auto entities = H::Entities.GetGroup(EGroupType::PLAYERS_ALL);
        for (auto pEntity : entities)
        {
            if (!pEntity)
                continue;
                
            auto pPlayer = pEntity->As<CTFPlayer>();
            if (pPlayer && pPlayer != pLocal && pPlayer->entindex() > 0 && pPlayer->entindex() <= I::GlobalVars->maxClients)
            {
                DrawPlayerBubbles(pPlayer);
            }
        }
    }
    catch (...)
    {
#ifdef _DEBUG
        I::CVar->ConsolePrintf("ChatBubbles Draw exception\n");
#endif
    }
}

void CChatBubbles::Reset()
{
    m_PlayerData.clear();
    m_GlobalChatLog.clear();
}