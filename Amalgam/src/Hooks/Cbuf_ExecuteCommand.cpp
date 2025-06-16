#include "../SDK/SDK.h"

#include "../Features/Commands/Commands.h"
#include <functional>
#include <regex>

MAKE_SIGNATURE(Cbuf_ExecuteCommand, "engine.dll", "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B 05 ? ? ? ? 48 8D 3D", 0x0);

enum cmd_source_t
{
	src_client,
	src_command
};

static std::string sCmdString;

#define PRE_STR "\x7\x7\x7\x7\x7\x7\x7"
static std::vector<std::pair<std::string, std::string>> vStatic = {
    { "\\x1", "\x1" },
    { "\\x01", "\x1" },
    { "\\x2", PRE_STR"\x2" },
    { "\\x02", PRE_STR"\x2" },
    { "\\x3", PRE_STR"\x3" },
    { "\\x03", PRE_STR"\x3" },
    { "\\x4", PRE_STR"\x4" },
    { "\\x04", PRE_STR"\x4" },
    { "\\x5", PRE_STR"\x5" },
    { "\\x05", PRE_STR"\x5" },
    { "\\x6", PRE_STR"\x6" },
    { "\\x06", PRE_STR"\x6" },
    { "\\x7", PRE_STR"\x7" },
    { "\\x07", PRE_STR"\x7" },
    { "\\x8", PRE_STR"\x8" },
    { "\\x08", PRE_STR"\x8" },

    { "\\{default}", "\x1" },
    { "\\{clear}", PRE_STR"\x8""00000000" },
    { "\\{red}", PRE_STR"\x7""ff0000" },
    { "\\{green}", PRE_STR"\x7""00ff00" },
    { "\\{blue}", PRE_STR"\x7""0000ff" },
    { "\\{yellow}", PRE_STR"\x7""ffff00" },
    { "\\{pink}", PRE_STR"\x7""ff00ff" },
    { "\\{cyan}", PRE_STR"\x7""00ffff" },
    { "\\{orange}", PRE_STR"\x7""ff7000" },
    { "\\{purple}", PRE_STR"\x7""7f00ff" },
    { "\\{brown}", PRE_STR"\x7""583927" },
    { "\\{gold}", PRE_STR"\x7""c8a900" },
    { "\\{gray}", PRE_STR"\x7""cccccc" },
    { "\\{black}", PRE_STR"\x7""000000" },
    { "\\{bluteam}", PRE_STR"\x7""99ccff" },
    { "\\{blueteam}", PRE_STR"\x7""99ccff" },
    { "\\{redteam}", PRE_STR"\x7""ff4040" },
    { "\\{normal}", PRE_STR"\x7""b2b2b2" },
    { "\\{unique}", PRE_STR"\x7""ffd700" },
    { "\\{strange}", PRE_STR"\x7""cf6a32" },
    { "\\{vintage}", PRE_STR"\x7""476291" },
    { "\\{haunted}", PRE_STR"\x7""38f3ab" },
    { "\\{genuine}", PRE_STR"\x7""4d7455" },
    { "\\{unusual}", PRE_STR"\x7""8650ac" },
    { "\\{collectors}", PRE_STR"\x7""aa0000" },
    { "\\{community}", PRE_STR"\x7""70b04a" },
    { "\\{selfmade}", PRE_STR"\x7""70b04a" },
    { "\\{valve}", PRE_STR"\x7""a50f79" },
    { "\\{elite}", PRE_STR"\x7""eb4b4b" },
    { "\\{assassin}", PRE_STR"\x7""d32ce6" },
    { "\\{commando}", PRE_STR"\x7""8847ff" },
    { "\\{mercenary}", PRE_STR"\x7""4b69ff" },

    { "\\t", "\t" },
};
static std::vector<std::function<void()>> vDynamic = {
    [&]()
    {
        auto pResource = H::Entities.GetPR();
        if (!pResource)
            return;

        std::string sFind = "\\{self}";
        std::string sReplace = pResource->m_pszPlayerName(I::EngineClient->GetLocalPlayer());

        size_t iPos = 0;
        while (true)
        {
            auto iFind = sCmdString.find(sFind, iPos);
            if (iFind == std::string::npos)
                break;
            
            iPos = iFind + sReplace.length();
            sCmdString = sCmdString.replace(iFind, sFind.length(), sReplace);
        }
    },
    [&]()
    {
        auto pResource = H::Entities.GetPR();
        if (!pResource)
            return;

        std::string sFind = "\\{team}";
        std::string sReplace = PRE_STR"\x7""cccccc";
        switch (pResource->m_iTeam(I::EngineClient->GetLocalPlayer()))
        {
        case TF_TEAM_BLUE: sReplace = PRE_STR"\x7""99ccff"; break;
        case TF_TEAM_RED: sReplace = PRE_STR"\x7""ff4040"; break;
        }

        size_t iPos = 0;
        while (true)
        {
            auto iFind = sCmdString.find(sFind, iPos);
            if (iFind == std::string::npos)
                break;
            
            iPos = iFind + sReplace.length();
            sCmdString = sCmdString.replace(iFind, sFind.length(), sReplace);
        }
    },
    [&]()
    {
        auto sRegex = R"(\\\{rgb:(\d+)?(?:,(\d+))?(?:,(\d+))?\})";

        while (true)
        {
            std::smatch match; std::regex_search(sCmdString, match, std::regex(sRegex));
            if (match.size() != 4)
                break;

            int r = !match[1].str().empty() ? std::stoi(match[1]) : 255;
            int g = !match[2].str().empty() ? std::stoi(match[2]) : 255;
            int b = !match[3].str().empty() ? std::stoi(match[3]) : 255;

            Color_t tColor; tColor.SetRGB(r, g, b);
            sCmdString = sCmdString.replace(match.position(), match.length(), std::format(PRE_STR"{}", tColor.ToHex()));
        }
    },
    [&]()
    {
        auto sRegex = R"(\\\{rgba:(\d+)?(?:,(\d+))?(?:,(\d+))?(?:,(\d+))?\})";

        while (true)
        {
            std::smatch match; std::regex_search(sCmdString, match, std::regex(sRegex));
            if (match.size() != 5)
                break;

            int r = !match[1].str().empty() ? std::stoi(match[1]) : 255;
            int g = !match[2].str().empty() ? std::stoi(match[2]) : 255;
            int b = !match[3].str().empty() ? std::stoi(match[3]) : 255;
            int a = !match[4].str().empty() ? std::stoi(match[4]) : 255;

            Color_t tColor; tColor.SetRGB(r, g, b, a);
            sCmdString = sCmdString.replace(match.position(), match.length(), std::format(PRE_STR"{}", tColor.ToHexA()));
        }
    },
    [&]()
    {
        auto sRegex = R"(\\\{hsv:(\d+)?(?:,(\d+))?(?:,(\d+))?\})";

        while (true)
        {
            std::smatch match; std::regex_search(sCmdString, match, std::regex(sRegex));
            if (match.size() != 4)
                break;

            int h = !match[1].str().empty() ? std::stoi(match[1]) : 0;
            int s = !match[2].str().empty() ? std::stoi(match[2]) : 100;
            int v = !match[3].str().empty() ? std::stoi(match[3]) : 100;

            Color_t tColor; tColor.SetHSV(h, s, v);
            sCmdString = sCmdString.replace(match.position(), match.length(), std::format(PRE_STR"{}", tColor.ToHex()));
        }
    },
    [&]()
    {
        auto sRegex = R"(\\\{hsva:(\d+)?(?:,(\d+))?(?:,(\d+))?(?:,(\d+))?\})";

        while (true)
        {
            std::smatch match; std::regex_search(sCmdString, match, std::regex(sRegex));
            if (match.size() != 5)
                break;

            int h = !match[1].str().empty() ? std::stoi(match[1]) : 0;
            int s = !match[2].str().empty() ? std::stoi(match[2]) : 100;
            int v = !match[3].str().empty() ? std::stoi(match[3]) : 100;
            int a = !match[4].str().empty() ? std::stoi(match[4]) : 255;

            Color_t tColor; tColor.SetHSV(h, s, v, a);
            sCmdString = sCmdString.replace(match.position(), match.length(), std::format(PRE_STR"{}", tColor.ToHexA()));
        }
    },
    [&]()
    {
        auto sRegex = R"(\\\{(\d+):(.+)\})";

        while (true)
        {
            std::smatch match; std::regex_search(sCmdString, match, std::regex(sRegex));
            if (match.size() != 3)
                break;

            int n = std::stoi(match[1]);
            auto str = match[2].str();

            std::ostringstream sStream;
            for (int i = 0; i < n; i++)
                sStream << str;

            sCmdString = sCmdString.replace(match.position(), match.length(), sStream.str());
        }
    },
};

MAKE_HOOK(Cbuf_ExecuteCommand, S::Cbuf_ExecuteCommand(), void,
	CCommand& args, cmd_source_t source)
{
#ifdef DEBUG_HOOKS
    if (!Vars::Hooks::Cbuf_ExecuteCommand[DEFAULT_BIND])
        return CALL_ORIGINAL(args, source);
#endif

	if (args.ArgC())
	{
		std::string sCommand = args[0];
		std::deque<std::string> vArgs;
		for (int i = 1; i < args.ArgC(); i++)
			vArgs.push_back(args[i]);

        if (F::Commands.Run(sCommand, vArgs))
            return;

		switch (FNV1A::Hash32(sCommand.c_str()))
		{
		case FNV1A::Hash32Const("say"):
		case FNV1A::Hash32Const("say_team"):
		{
			sCmdString = args.m_pArgSBuffer;
			sCmdString = sCmdString.replace(0, args.m_nArgv0Size, "");

			for (auto& [sFind, sReplace] : vStatic)
			{
				size_t iPos = 0;
				while (true)
				{
					auto iFind = sCmdString.find(sFind, iPos);
					if (iFind == std::string::npos)
						break;

					iPos = iFind + sReplace.length();
					sCmdString = sCmdString.replace(iFind, sFind.length(), sReplace);
				}
			}
			for (auto& fFunction : vDynamic)
                fFunction();

			sCmdString = std::format("{} {}", sCommand, sCmdString).substr(0, COMMAND_MAX_LENGTH - 1);
			strncpy_s(args.m_pArgSBuffer, sCmdString.c_str(), COMMAND_MAX_LENGTH);
			args.m_nArgv0Size = int(sCommand.length() + 1);
		}
		}
	}

	CALL_ORIGINAL(args, source);
}