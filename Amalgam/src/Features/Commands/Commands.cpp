#include "Commands.h"

#include "../../Core/Core.h"
#include "../ImGui/Menu/Menu.h"
#include <utility>
#include <boost/algorithm/string/replace.hpp>

bool CCommands::Run(const char* sCmd, std::deque<const char*>& vArgs)
{
	std::string sLower = sCmd;
	std::transform(sLower.begin(), sLower.end(), sLower.begin(), ::tolower);

	auto uHash = FNV1A::Hash32(sLower.c_str());
	if (!m_mCommands.contains(uHash))
		return false;

	m_mCommands[uHash](vArgs);
	return true;
}

void CCommands::Register(const char* sName, CommandCallback fCallback)
{
	m_mCommands[FNV1A::Hash32(sName)] = std::move(fCallback);
}

void CCommands::Initialize()
{
	Register("setcvar", [](const std::deque<const char*>& vArgs)
		{
			if (vArgs.size() < 2)
			{
				SDK::Output("Usage:\n\tsetcvar <cvar> <value>");
				return;
			}

			const char* sCVar = vArgs[0];
			auto pCVar = I::CVar->FindVar(sCVar);
			if (!pCVar)
			{
				SDK::Output(std::format("Could not find {}", sCVar).c_str());
				return;
			}

			std::string sValue = "";
			for (int i = 1; i < vArgs.size(); i++)
				sValue += std::format("{} ", vArgs[i]);
			sValue.pop_back();
			boost::replace_all(sValue, "\"", "");

			pCVar->SetValue(sValue.c_str());
			SDK::Output(std::format("Set {} to {}", sCVar, sValue).c_str());
		});

	Register("getcvar", [](const std::deque<const char*>& vArgs)
		{
			if (vArgs.size() != 1)
			{
				SDK::Output("Usage:\n\tgetcvar <cvar>");
				return;
			}

			const char* sCVar = vArgs[0];
			auto pCVar = I::CVar->FindVar(sCVar);
			if (!pCVar)
			{
				SDK::Output(std::format("Could not find {}", sCVar).c_str());
				return;
			}

			SDK::Output(std::format("Value of {} is {}", sCVar, pCVar->GetString()).c_str());
		});

	Register("queue", [](const std::deque<const char*>& vArgs)
		{
			static bool bHasLoaded = false;
			if (!bHasLoaded)
			{
				I::TFPartyClient->LoadSavedCasualCriteria();
				bHasLoaded = true;
			}
			I::TFPartyClient->RequestQueueForMatch(k_eTFMatchGroup_Casual_Default);
		});

	Register("clearchat", [](const std::deque<const char*>& vArgs)
		{
			I::ClientModeShared->m_pChatElement->SetText("");
		});

	Register("menu", [](const std::deque<const char*>& vArgs)
		{
			I::MatSystemSurface->SetCursorAlwaysVisible(F::Menu.m_bIsOpen = !F::Menu.m_bIsOpen);
		});

	Register("unload", [](const std::deque<const char*>& vArgs)
		{
			if (F::Menu.m_bIsOpen)
				I::MatSystemSurface->SetCursorAlwaysVisible(F::Menu.m_bIsOpen = false);
			U::Core.m_bUnload = true;
		});

	Register("crash", [](const std::deque<const char*>& vArgs) // if you want to time out of a server and rejoin
		{
			switch (vArgs.empty() ? 0 : FNV1A::Hash32(vArgs.front()))
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