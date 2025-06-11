#include "../SDK/SDK.h"

MAKE_HOOK(CClient_Precipitation_Render, g_Pattern.Find(L"client.dll", L"55 8B EC 81 EC ? ? ? ? 33 C0 89 45 FC 39 05 ? ? ? ? 0F 8E ? ? ? ? 53 56 57 8D 49 00 8B 3D ? ? ? ? 8B 3C 87 A1"), void, __fastcall,
	void* ecx, void* edx)
{
	return Hook.Original<FN>()(ecx, edx);
}