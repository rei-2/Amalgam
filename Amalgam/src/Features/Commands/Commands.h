#pragma once
#include "../../SDK/SDK.h"
#include <functional>

using CommandCallback = std::function<void(std::deque<const char*>&)>;

class CCommands
{
public:
    bool Run(const char* sCmd, std::deque<const char*>& vArgs);
};

ADD_FEATURE(CCommands, Commands);