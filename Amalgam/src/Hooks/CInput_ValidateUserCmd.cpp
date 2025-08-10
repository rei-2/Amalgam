#include "../SDK/SDK.h"

MAKE_SIGNATURE(CInput_ValidateUserCmd, "client.dll", "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B F9 41 8B D8", 0x0);

MAKE_HOOK(CInput_ValidateUserCmd, S::CInput_ValidateUserCmd(), void,
	void* rcx, CUserCmd* usercmd, int sequence_number)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::CInput_ValidateUserCmd[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx, usercmd, sequence_number);
#endif
}