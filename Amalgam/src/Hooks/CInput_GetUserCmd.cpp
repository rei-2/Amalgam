#include "../SDK/SDK.h"

MAKE_HOOK(CInput_GetUserCmd, U::Memory.GetVirtual(I::Input, 8), CUserCmd*,
	void* rcx, int sequence_number)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::CInput_GetUserCmd[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx, sequence_number);
#endif

	return &I::Input->m_pCommands[sequence_number % MULTIPLAYER_BACKUP];
}