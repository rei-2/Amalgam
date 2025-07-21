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

// Chat bubble message entry
struct ChatBubbleMessage
{
    std::string message;
    std::string playerName;
    float timestamp;
    bool isVoice;
    Vec3 smoothPos;
    bool hasSmoothPos;
    int count;
    
    ChatBubbleMessage() : timestamp(0.0f), isVoice(false), hasSmoothPos(false), count(1) {}
};

// Player chat data
struct PlayerChatData
{
    std::vector<ChatBubbleMessage> messages;
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
    std::vector<ChatBubbleMessage> m_GlobalChatLog;
    
    // SteamID color cache
    std::unordered_map<std::string, Color_t> m_ColorCache;
    
    // Helper functions
    std::string GetVoiceCommandText(int menu, int item);
    Color_t GenerateColor(const std::string& steamID);
    void CleanOldMessages();
    
    // Text rendering helpers
    std::vector<std::string> WrapText(const std::string& text, float maxWidth);
    Vec2 MeasureTextSize(const std::string& text);
    Vec2 CalculateBubbleDimensions(const std::vector<std::string>& lines);
    
    // Drawing functions
    int CalculateOpacity(float messageAge);
    float DrawChatBubble(ChatBubbleMessage& message, const Vec3& worldPos, float yOffset, int entityIndex);
    void DrawPlayerBubbles(CTFPlayer* pPlayer);
    
public:
    // Main functions
    void OnVoiceSubtitle(int entityIndex, int menu, int item);
    void OnChatMessage(bf_read& msgData);
    void OnSoundPlayed(int entityIndex, const char* soundName);
    void Draw();
    void Reset();
    
    // Public interface for external access
    void AddChatMessage(const std::string& message, const std::string& playerName, int entityIndex, bool isVoice);
};

ADD_FEATURE(CChatBubbles, ChatBubbles)