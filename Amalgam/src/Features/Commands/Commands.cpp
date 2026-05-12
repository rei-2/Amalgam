#include "Commands.h"

#include "../../Core/Core.h"
#include "../ImGui/Menu/Menu.h"
#include <utility>
#include <boost/algorithm/string/replace.hpp>

#define AddCommand(sCommand, ...) \
{ \
	FNV1A::Hash32Const(sCommand), \
	[](const std::deque<const char*>& vArgs) \
		__VA_ARGS__ \
},

//struct ConVarValues_t
//{
//	int m_iValue;
//	float m_flValue;
//	const char* m_sValue;
//};
//static std::unordered_map<ConVar*, ConVarValues_t> s_mConVarValues = {};

static std::unordered_map<uint32_t, CommandCallback> s_mCommands = {
	AddCommand("setcvar",
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
	})
	AddCommand("getcvar",
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
	})
	AddCommand("queue",
	{
		if (!I::TFPartyClient->AnySelected())
			I::TFPartyClient->LoadSavedCasualCriteria();
		I::TFPartyClient->RequestQueueForMatch(k_eTFMatchGroup_Casual_Default);
	})
	AddCommand("clear_chat",
	{
		I::ClientModeShared->m_pChatElement->SetText("");
	})
	AddCommand("menu",
	{
		I::MatSystemSurface->SetCursorAlwaysVisible(F::Menu.m_bIsOpen = !F::Menu.m_bIsOpen);
	})
	AddCommand("unload",
	{
		if (F::Menu.m_bIsOpen)
			I::MatSystemSurface->SetCursorAlwaysVisible(F::Menu.m_bIsOpen = false);
		U::Core.m_bUnload = true;
	})
	AddCommand("crash",
	{	// if you want to time out of a server and rejoin
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
	})
	//AddCommand("recordcvars",
	//{
	//	for (ConCommandBase* pBase = I::CVar->GetCommands(); pBase; pBase = pBase->m_pNext)
	//	{
	//		if (pBase->IsCommand())
	//			continue;

	//		auto pCVar = reinterpret_cast<ConVar*>(pBase);
	//		s_mConVarValues[pCVar] = { pCVar->GetInt(), pCVar->GetFloat(), pCVar->GetString() };
	//	}
	//})
	//AddCommand("diffcvars",
	//{
	//	for (auto& [pCVar, tValues] : s_mConVarValues)
	//	{
	//		if (auto iOld = tValues.m_iValue, iNew = pCVar->GetInt(); iOld != iNew)
	//			SDK::Output(pCVar->GetName(), std::format("int: {} -> {}", iOld, iNew).c_str());
	//		else if (auto flOld = tValues.m_flValue, flNew = pCVar->GetFloat(); flOld != flNew)
	//			SDK::Output(pCVar->GetName(), std::format("float: {} -> {}", flOld, flNew).c_str());
	//		else if (auto sOld = tValues.m_sValue, sNew = pCVar->GetString(); sOld != sNew || FNV1A::Hash32(sOld) != FNV1A::Hash32(sNew))
	//			SDK::Output(pCVar->GetName(), std::format("string: {} -> {}", sOld, sNew).c_str());
	//	}
	//})
};

bool CCommands::Run(const char* sCmd, std::deque<const char*>& vArgs)
{
	std::string sLower = sCmd;
	std::transform(sLower.begin(), sLower.end(), sLower.begin(), ::tolower);

	auto uHash = FNV1A::Hash32(sLower.c_str());
	if (!s_mCommands.contains(uHash))
		return false;

	s_mCommands[uHash](vArgs);
	return true;
}