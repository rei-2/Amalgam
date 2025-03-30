#include "../SDK/SDK.h"

MAKE_SIGNATURE(RecvProxy_SimulationTime, "client.dll", "48 89 5C 24 ? 57 48 83 EC ? 8B 59 ? 48 8B FA 48 8B 42 ? 48 8D 4A ? FF 50 ? 4C 8B 0D ? ? ? ? 99 41 F7 79 ? 41 8B 49 ? 44 8B C1 44 2B C2 41 8B C0 99 41 F7 79 ? 8D 41 ? 44 2B C2 44 03 C3 44 3B C0 7D ? 41 2B C0 41 81 C0 ? ? ? ? FF C8 25 ? ? ? ? 44 03 C0 8D 41 ? 44 3B C0 7E ? 41 8B C8 2B C8 B8 ? ? ? ? FF C9 81 E1 ? ? ? ? 2B C1 44 03 C0 48 8B 5C 24 ? 66 41 0F 6E C0 0F 5B C0 F3 41 0F 59 41 ? F3 0F 11 87 ? ? ? ? 48 83 C4 ? 5F C3 CC CC CC CC CC 48 89 5C 24", 0x0);

static int GetNetworkBase(int nTick, int nEntity)
{
	int nEntityMod = nEntity % I::GlobalVars->nTimestampRandomizeWindow;
	int nBaseTick = I::GlobalVars->nTimestampNetworkingBase * int((nTick - nEntityMod) / I::GlobalVars->nTimestampNetworkingBase);
	return nBaseTick;
}

MAKE_HOOK(RecvProxy_SimulationTime, S::RecvProxy_SimulationTime(), void,
	const CRecvProxyData* pData, void* pStruct, void* pOut)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::RecvProxy_SimulationTime[DEFAULT_BIND])
		return CALL_ORIGINAL(pData, pStruct, pOut);
#endif

	auto pEntity = reinterpret_cast<CBaseEntity*>(pStruct);
	if (!pEntity || !pEntity->IsPlayer() || pEntity->entindex() == I::EngineClient->GetLocalPlayer())
		return CALL_ORIGINAL(pData, pStruct, pOut);

	if (!pData->m_Value.m_Int) // fix setting invalid simtime every 100 ticks if choking
		return;

	int addt = pData->m_Value.m_Int;
	int t = GetNetworkBase(I::GlobalVars->tickcount, pEntity->entindex());
	t += addt;

	while (t < I::GlobalVars->tickcount - 127)
		t += 256;
	while (t > I::GlobalVars->tickcount + 127)
		t -= 256;

	pEntity->m_flSimulationTime() = TICKS_TO_TIME(t);
}