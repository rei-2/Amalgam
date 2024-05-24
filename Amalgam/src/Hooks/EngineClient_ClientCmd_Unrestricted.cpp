#include "../SDK/SDK.h"

#include "../Features/Commands/Commands.h"
#include <boost/algorithm/string/split.hpp>

class split_q
{
public:
	split_q() : in_q(false) {}

	bool operator()(char ch) const
	{
		if (ch == '\"')
		{
			in_q = !in_q;
		}
		return !in_q && ch == ' ';
	}

private:
	mutable bool in_q;
};

MAKE_HOOK(EngineClient_ClientCmd_Unrestricted, U::Memory.GetVFunc(I::EngineClient, 106), void, __fastcall,
	void* ecx, const char* szCmdString)
{
	std::string cmdString = szCmdString;
	std::deque<std::string> cmdArgs;
	boost::split(cmdArgs, cmdString, split_q());
	if (!cmdArgs.empty())
	{
		const std::string cmdName = cmdArgs.front();
		cmdArgs.pop_front();

		if (F::Commands.Run(cmdName, cmdArgs))
			return;
	}

	CALL_ORIGINAL(ecx, cmdString.c_str());
}