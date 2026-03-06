#include "../SDK/SDK.h"

#include "../Features/Commands/Commands.h"

MAKE_SIGNATURE(Cbuf_ExecuteCommand, "engine.dll", "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B 05 ? ? ? ? 48 8D 3D", 0x0);

enum cmd_source_t
{
	src_client,
	src_command
};

MAKE_HOOK(Cbuf_ExecuteCommand, S::Cbuf_ExecuteCommand(), void,
	CCommand& args, cmd_source_t source)
{
    DEBUG_RETURN(Cbuf_ExecuteCommand, args, source);

	if (args.ArgC())
	{
        const char* sCommand = args[0];
		std::deque<const char*> vArgs;
		for (int i = 1; i < args.ArgC(); i++)
			vArgs.push_back(args[i]);

        if (F::Commands.Run(sCommand, vArgs))
            return;
	}

    CALL_ORIGINAL(args, source);
}