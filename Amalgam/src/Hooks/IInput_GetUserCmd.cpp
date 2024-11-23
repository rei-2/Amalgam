#include "../SDK/SDK.h"

MAKE_HOOK(IInput_GetUserCmd, U::Memory.GetVFunc(I::Input, 8), CUserCmd*,
	void* rcx, int sequence_number)
{
	return &I::Input->GetCommands()[sequence_number % MULTIPLAYER_BACKUP];
}