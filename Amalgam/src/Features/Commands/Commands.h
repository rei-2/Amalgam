#pragma once
#include "../../SDK/SDK.h"
#include <functional>

using CommandCallback = std::function<void(std::deque<const char*>&)>;

class CCommands
{
private:
    void Register(const char* sName, CommandCallback fCallback);

    std::unordered_map<uint32_t, CommandCallback> m_mCommands = {};

public:
    void Initialize();
    bool Run(const char* sCmd, std::deque<const char*>& vArgs);
};

ADD_FEATURE(CCommands, Commands);