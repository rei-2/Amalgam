#pragma once
#include "../../SDK/SDK.h"
#include <functional>

using CommandCallback = std::function<void(std::deque<std::string>)>;

class CCommands
{
private:
    std::unordered_map<uint32_t, CommandCallback> m_mCommands;

public:
    void Initialize();
    bool Run(const std::string& sCmd, std::deque<std::string>& vArgs);
    void Register(const std::string& sName, CommandCallback fCallback);
};

ADD_FEATURE(CCommands, Commands);