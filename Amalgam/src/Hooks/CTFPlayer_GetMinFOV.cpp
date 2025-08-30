#include "../SDK/SDK.h"

MAKE_SIGNATURE(CTFPlayer_GetMinFOV, "client.dll", "F3 0F 10 05 ? ? ? ? C3 CC CC CC CC CC CC CC 48 8B 81 ? ? ? ? 8B D2", 0x0);

MAKE_HOOK(CTFPlayer_GetMinFOV, S::CTFPlayer_GetMinFOV(), float,
	/*void* rcx*/)
{
	return 0.f;
}