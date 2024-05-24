#pragma once
#include "../../SDK/SDK.h"
#include <functional>

using CommandCallback = std::function<void(std::deque<std::string>)>;

class CCommands
{
private:
    std::unordered_map<std::string, CommandCallback> CommandMap;

public:
    void Initialize();
    bool Run(const std::string& cmd, std::deque<std::string>& args);
    void Register(const std::string& name, CommandCallback callback);
};

ADD_FEATURE(CCommands, Commands)