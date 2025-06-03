#include "Commands.h"

#include "../../Core/Core.h"
#include "../ImGui/Menu/Menu.h"
#include <utility>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/join.hpp>

bool CCommands::Run(const std::string& sCmd, std::deque<std::string>& vArgs)
{
	auto uHash = FNV1A::Hash32(sCmd.c_str());
	if (!m_mCommands.contains(uHash))
		return false;

	m_mCommands[uHash](vArgs);
	return true;
}

void CCommands::Register(const std::string& sName, CommandCallback fCallback)
{
	m_mCommands[FNV1A::Hash32(sName.c_str())] = std::move(fCallback);
}

void CCommands::Initialize()
{
	Register("queue", [](const std::deque<std::string>& vArgs)
		{
			static bool bHasLoaded = false;
			if (!bHasLoaded)
			{
				I::TFPartyClient->LoadSavedCasualCriteria();
				bHasLoaded = true;
			}
			I::TFPartyClient->RequestQueueForMatch(k_eTFMatchGroup_Casual_Default);
		});

	Register("setcvar", [](const std::deque<std::string>& vArgs)
		{
			if (vArgs.size() < 2)
			{
				SDK::Output("Usage:\n\tsetcvar <cvar> <value>");
				return;
			}

			std::string sCVar = vArgs[0];
			auto pCVar = I::CVar->FindVar(sCVar.c_str());
			if (!pCVar)
			{
				SDK::Output(std::format("Could not find {}", sCVar).c_str());
				return;
			}

			auto vArgs2 = vArgs; vArgs2.pop_front();
			std::string sValue = boost::algorithm::join(vArgs2, " ");
			boost::replace_all(sValue, "\"", "");
			pCVar->SetValue(sValue.c_str());
			SDK::Output(std::format("Set {} to {}", sCVar, sValue).c_str());
		});

	Register("getcvar", [](const std::deque<std::string>& vArgs)
		{
			if (vArgs.size() != 1)
			{
				SDK::Output("Usage:\n\tgetcvar <cvar>");
				return;
			}

			std::string sCVar = vArgs[0];
			auto pCVar = I::CVar->FindVar(sCVar.c_str());
			if (!pCVar)
			{
				SDK::Output(std::format("Could not find {}", sCVar).c_str());
				return;
			}

			SDK::Output(std::format("Value of {} is {}", sCVar, pCVar->GetString()).c_str());
		});

	Register("clearchat", [](const std::deque<std::string>& vArgs)
		{
			I::ClientModeShared->m_pChatElement->SetText("");
		});

	Register("menu", [](const std::deque<std::string>& vArgs)
		{
			I::MatSystemSurface->SetCursorAlwaysVisible(F::Menu.m_bIsOpen = !F::Menu.m_bIsOpen);
		});

	Register("unload", [](const std::deque<std::string>& vArgs)
		{
			if (F::Menu.m_bIsOpen)
				I::MatSystemSurface->SetCursorAlwaysVisible(F::Menu.m_bIsOpen = false);
			U::Core.m_bUnload = true;
		});

	Register("crash", [](const std::deque<std::string>& vArgs) // if you want to time out of a server and rejoin
		{
			switch (vArgs.empty() ? 0 : FNV1A::Hash32(vArgs.front().c_str()))
			{
			case FNV1A::Hash32Const("true"):
			case FNV1A::Hash32Const("t"):
			case FNV1A::Hash32Const("1"):
				break;
			default:
				Vars::Debug::CrashLogging.Value = false; // we are voluntarily crashing, don't give out log if we don't want one
			}
			reinterpret_cast<void(*)()>(0)();
		});
}