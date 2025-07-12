#pragma once
#include "../../../SDK/SDK.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

// Voice command data structure
struct VoiceCommand
{
    int menu;
    int item;
    std::string text;
};

// Chat message entry
struct ChatMessage
{
    std::string message;
    std::string playerName;
    float timestamp;
    bool isVoice;
    Vec3 smoothPos;
    bool hasSmoothPos;
    
    ChatMessage() : timestamp(0.0f), isVoice(false), hasSmoothPos(false) {}
};

// Player chat data
struct PlayerChatData
{
    std::vector<ChatMessage> messages;
    float lastVoiceTime;
    
    PlayerChatData() : lastVoiceTime(0.0f) {}
};

class CChatBubbles
{
private:
    // Configuration constants
    static constexpr int MAX_MESSAGES_PER_PLAYER = 3;
    static constexpr int MAX_GLOBAL_MESSAGES = 10;
    static constexpr float MESSAGE_LIFETIME = 10.0f;
    static constexpr float FADE_START_TIME = 7.0f;
    static constexpr float BUBBLE_MAX_WIDTH = 250.0f;
    static constexpr float BUBBLE_PADDING = 5.0f;
    static constexpr float SMOOTHING_FACTOR = 0.1f;
    static constexpr float VOICE_COOLDOWN = 1.0f;
    
    // Voice command mappings (improved and complete)
    static const std::unordered_map<int, std::unordered_map<int, std::string>> VOICE_MENU_MAP;
    
    // Player data storage
    std::unordered_map<int, PlayerChatData> m_PlayerData;
    std::vector<ChatMessage> m_GlobalChatLog;
    
    // Helper functions
    std::string GetVoiceCommandText(int menu, int item);
    void AddChatMessage(const std::string& message, const std::string& playerName, int entityIndex, bool isVoice);
    void CleanOldMessages();
    
    // Text rendering helpers
    std::vector<std::string> WrapText(const std::string& text, float maxWidth);
    Vec2 MeasureTextSize(const std::string& text);
    Vec2 CalculateBubbleDimensions(const std::vector<std::string>& lines);
    
    // Drawing functions
    int CalculateOpacity(float messageAge);
    float DrawChatBubble(ChatMessage& message, const Vec3& worldPos, float yOffset);
    void DrawPlayerBubbles(CTFPlayer* pPlayer);
    
public:
    // Main functions
    void OnVoiceSubtitle(int entityIndex, int menu, int item);
    void OnChatMessage(bf_read& msgData);
    void Draw();
    void Reset();
};

ADD_FEATURE(CChatBubbles, ChatBubbles)