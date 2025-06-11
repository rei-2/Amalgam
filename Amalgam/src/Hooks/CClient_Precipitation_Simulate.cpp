#include "../SDK/SDK.h"

MAKE_HOOK(CClient_Precipitation_Simulate, g_Pattern.Find(L"client.dll", L"55 8B EC 83 EC 10 53 8B D9 8B 03 FF 50 7C E8 ? ? ? ? F3 0F 2A C0 F3 0F 59 05 ? ? ? ? F3 0F 11 83 ? ? ? ? F3 0F 10 45 ? 0F 2E 05 ? ? ? ? 9F F6 C4 44 7B 05 E8 ? ? ? ? 8B 83 ? ? ? ? 83 F8 02 75 0E 8B CB E8"), void, __fastcall,
	void* ecx, void* edx, float dt)
{
	return Hook.Original<FN>()(ecx, edx, dt);
}