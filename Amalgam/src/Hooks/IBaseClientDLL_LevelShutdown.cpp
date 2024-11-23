#include "../SDK/SDK.h"

#include "../Features/Misc/Misc.h"

MAKE_HOOK(IBaseClientDLL_LevelShutdown, U::Memory.GetVFunc(I::BaseClientDLL, 7), void,
	void* rcx)
{
	H::Entities.Clear(true);

	CALL_ORIGINAL(rcx);
}