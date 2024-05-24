#include "../SDK/SDK.h"

#include "../Features/Misc/Misc.h"

MAKE_HOOK(BaseClientDLL_Shutdown, U::Memory.GetVFunc(I::BaseClientDLL, 7), void, __fastcall,
	void* ecx)
{
	H::Entities.Clear();
	G::DormancyMap.clear();
	G::ChokeMap.clear();

	CALL_ORIGINAL(ecx);
}