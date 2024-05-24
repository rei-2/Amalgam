#include "Commands.h"

#include "../../Core/Core.h"
#include "../ImGui/Menu/Menu.h"
#include <utility>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/join.hpp>

bool CCommands::Run(const std::string& cmd, std::deque<std::string>& args)
{
	if (!CommandMap.contains(cmd))
		return false;

	CommandMap[cmd](args);
	return true;
}

void CCommands::Register(const std::string& name, CommandCallback callback)
{
	CommandMap[name] = std::move(callback);
}

void CCommands::Initialize()
{
	Register("queue", [](const std::deque<std::string>& args)
		{
			I::TFPartyClient->LoadSavedCasualCriteria();
			I::TFPartyClient->RequestQueueForMatch(k_eTFMatchGroup_Casual_Default);
		});

	Register("setcvar", [](std::deque<std::string> args)
		{
			// Check if the user provided at least 2 args
			if (args.size() < 2)
			{
				I::CVar->ConsoleColorPrintf({ 255, 255, 255, 255 }, "Usage: setcvar <cvar> <value>\n");
				return;
			}

			// Find the given CVar
			const auto foundCVar = I::CVar->FindVar(args[0].c_str());
			const std::string cvarName = args[0];
			if (!foundCVar)
			{
				I::CVar->ConsoleColorPrintf({ 255, 255, 255, 255 }, "Could not find %s\n", cvarName.c_str());
				return;
			}

			// Set the CVar to the given value
			args.pop_front();
			std::string newValue = boost::algorithm::join(args, " ");
			boost::replace_all(newValue, "\"", "");
			foundCVar->SetValue(newValue.c_str());
			I::CVar->ConsoleColorPrintf({ 255, 255, 255, 255 }, "Set %s to: %s\n", cvarName.c_str(), newValue.c_str());
		});

	Register("getcvar", [](std::deque<std::string> args)
		{
			// Check if the user provided 1 arg
			if (args.size() != 1)
			{
				I::CVar->ConsoleColorPrintf({ 255, 255, 255, 255 }, "Usage: getcvar <cvar>\n");
				return;
			}

			const auto foundCVar = I::CVar->FindVar(args[0].c_str());
			const std::string cvarName = args[0];
			if (!foundCVar)
			{
				I::CVar->ConsoleColorPrintf({ 255, 255, 255, 255 }, "Could not find %s\n", cvarName.c_str());
				return;
			}

			I::CVar->ConsoleColorPrintf({ 255, 255, 255, 255 }, "Value of %s is: %s\n", cvarName.c_str(), foundCVar->GetString());
		});

	Register("unload", [](std::deque<std::string> args)
		{
			if (F::Menu.IsOpen)
				I::MatSystemSurface->SetCursorAlwaysVisible(F::Menu.IsOpen = false);
			U::Core.bUnload = true;
		});
}