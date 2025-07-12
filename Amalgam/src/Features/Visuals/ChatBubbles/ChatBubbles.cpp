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
        
        // Create new message
        ChatMessage newMessage;
        newMessage.message = message;
        newMessage.playerName = playerName;
        newMessage.timestamp = currentTime;
        newMessage.isVoice = isVoice;
        newMessage.hasSmoothPos = false;
        
        // Add to front of list
        playerData.messages.insert(playerData.messages.begin(), newMessage);
        
        // Limit messages per player
        while (playerData.messages.size() > MAX_MESSAGES_PER_PLAYER)
        {
            playerData.messages.pop_back();
        }
    }
    
    // Add to global chat log
    ChatMessage globalMessage;
    globalMessage.message = message;
    globalMessage.playerName = playerName;
    globalMessage.timestamp = currentTime;
    globalMessage.isVoice = isVoice;
    globalMessage.hasSmoothPos = false;
    
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
                [currentTime](const ChatMessage& msg) {
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
            [currentTime](const ChatMessage& msg) {
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
    if (font == 0)
        return {0.0f, 0.0f};
    
    // Get text dimensions using proper wstring conversion
    std::wstring wideText = std::wstring(text.begin(), text.end());
    int width, height;
    I::MatSystemSurface->GetTextSize(font, wideText.c_str(), width, height);
    
    return {static_cast<float>(width), static_cast<float>(height)};
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

float CChatBubbles::DrawChatBubble(ChatMessage& message, const Vec3& worldPos, float yOffset)
{
    // Get screen position
    Vec3 screenPos;
    if (!SDK::W2S(worldPos, screenPos))
        return 0.0f;
    
    // Calculate opacity based on message age
    float messageAge = I::GlobalVars->curtime - message.timestamp;
    int opacity = CalculateOpacity(messageAge);
    
    if (opacity <= 2)
        return 0.0f;
    
    // Prepare display text
    std::string displayText = message.isVoice ? 
        ("(Voice) " + message.message) : message.message;
    
    // Wrap text and calculate dimensions
    auto wrappedLines = WrapText(displayText, BUBBLE_MAX_WIDTH - (BUBBLE_PADDING * 2));
    Vec2 bubbleDims = CalculateBubbleDimensions(wrappedLines);
    
    // Get screen dimensions
    int screenW, screenH;
    I::MatSystemSurface->GetScreenSize(screenW, screenH);
    
    // Calculate bubble position with screen bounds
    float bubbleX = std::max(bubbleDims.x / 2, 
                    std::min(static_cast<float>(screenW) - bubbleDims.x / 2, screenPos.x));
    float bubbleY = std::max(bubbleDims.y, 
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
    
    // Draw background
    Color_t bgColor = {0, 0, 0, static_cast<byte>(opacity * 0.8f)};
    H::Draw.FillRect(
        static_cast<int>(message.smoothPos.x - bubbleDims.x / 2),
        static_cast<int>(message.smoothPos.y - bubbleDims.y),
        static_cast<int>(bubbleDims.x),
        static_cast<int>(bubbleDims.y),
        bgColor
    );
    
    // Draw text lines
    Color_t textColor = {255, 255, 255, static_cast<byte>(opacity)};
    float yTextOffset = 0.0f;
    
    for (const auto& line : wrappedLines)
    {
        Vec2 lineSize = MeasureTextSize(line);
        
        H::Draw.String(
            H::Fonts.GetFont(FONT_ESP),
            static_cast<int>(message.smoothPos.x - bubbleDims.x / 2 + BUBBLE_PADDING),
            static_cast<int>(message.smoothPos.y - bubbleDims.y + BUBBLE_PADDING + yTextOffset),
            textColor,
            ALIGN_TOPLEFT,
            line.c_str()
        );
        
        yTextOffset += lineSize.y + 2;
    }
    
    return bubbleDims.y + 5;
}

void CChatBubbles::DrawPlayerBubbles(CTFPlayer* pPlayer)
{
    if (!pPlayer || !pPlayer->IsAlive())
        return;
    
    int playerIndex = pPlayer->entindex();
    auto it = m_PlayerData.find(playerIndex);
    if (it == m_PlayerData.end())
        return;
    
    // Get player head position
    Vec3 headPos = pPlayer->GetAbsOrigin() + Vec3(0, 0, 75);
    
    // Draw all messages for this player
    float yOffset = 0.0f;
    for (auto& message : it->second.messages)
    {
        yOffset += DrawChatBubble(message, headPos, yOffset);
    }
}

void CChatBubbles::OnVoiceSubtitle(int entityIndex, int menu, int item)
{
    if (!Vars::Misc::Features::ChatBubbles.Value)
        return;
    
    // Get player info
    auto pPlayer = I::ClientEntityList->GetClientEntity(entityIndex);
    if (!pPlayer) return;
    
    auto pTFPlayer = pPlayer->As<CTFPlayer>();
    if (!pTFPlayer) return;
    
    // Apply voice cooldown to prevent spam
    float currentTime = I::GlobalVars->curtime;
    auto& playerData = m_PlayerData[entityIndex];
    if (currentTime - playerData.lastVoiceTime < VOICE_COOLDOWN)
        return;
    
    playerData.lastVoiceTime = currentTime;
    
    // Get player name and voice command text
    std::string playerName = "Player"; // Default fallback
    if (auto pInfo = I::EngineClient->GetPlayerInfo(entityIndex))
        playerName = pInfo->name;
    std::string voiceCommand = GetVoiceCommandText(menu, item);
    
    // Add voice command message for ALL players (including enemies!)
    AddChatMessage(voiceCommand, playerName, entityIndex, true);
}

void CChatBubbles::OnChatMessage(bf_read& msgData)
{
    if (!Vars::Misc::Features::ChatBubbles.Value)
        return;
    
    // This would handle SayText/SayText2 messages for chat
    // For now, we'll focus on voice commands which are more reliable
    // Chat message parsing can be added later if needed
}


void CChatBubbles::Draw()
{
    if (!Vars::Misc::Features::ChatBubbles.Value)
        return;
    
    if (I::EngineVGui->IsGameUIVisible())
        return;
    
    auto pLocal = H::Entities.GetLocal();
    if (!pLocal)
        return;
    
    // Clean old messages
    CleanOldMessages();
    
    // Draw bubbles for all players
    for (auto pEntity : H::Entities.GetGroup(EGroupType::PLAYERS_ALL))
    {
        auto pPlayer = pEntity->As<CTFPlayer>();
        if (pPlayer && pPlayer != pLocal)
        {
            DrawPlayerBubbles(pPlayer);
        }
    }
}

void CChatBubbles::Reset()
{
    m_PlayerData.clear();
    m_GlobalChatLog.clear();
}