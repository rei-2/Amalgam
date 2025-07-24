#include "../SDK/SDK.h"

MAKE_SIGNATURE(CL_ProcessPacketEntities, "engine.dll", "48 89 7C 24 ? 4C 89 64 24 ? 55 41 56 41 57 48 8D AC 24 ? ? ? ? B8 ? ? ? ? E8 ? ? ? ? 48 2B E0 4C 8B F9", 0x0);

struct CriticalStorage_t
{
	float m_flCritTokenBucket;
	int m_nCritChecks;
	int m_nCritSeedRequests;
};

MAKE_HOOK(CL_ProcessPacketEntities, S::CL_ProcessPacketEntities(), bool,
	SVC_PacketEntities* entmsg)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::CL_ProcessPacketEntities[DEFAULT_BIND])
		return CALL_ORIGINAL(entmsg);
#endif

	if (entmsg->m_bIsDelta) // we won't need to restore
		return CALL_ORIGINAL(entmsg);

	CTFPlayer* pLocal = H::Entities.GetLocal();
	if (!pLocal || !pLocal->m_hMyWeapons())
	{
		SDK::Output("ProcessPacketEntities", "Failed to restore weapon crit data! (1)", { 255, 100, 100 });
		return CALL_ORIGINAL(entmsg);
	}

	std::unordered_map<int, CriticalStorage_t> mCriticalStorage = {};

	for (int i = 0; i < MAX_WEAPONS; i++)
	{
		auto pWeapon = pLocal->GetWeaponFromSlot(i);
		if (!pWeapon)
			continue;

		mCriticalStorage[i].m_flCritTokenBucket = pWeapon->m_flCritTokenBucket();
		mCriticalStorage[i].m_nCritChecks = pWeapon->m_nCritChecks();
		mCriticalStorage[i].m_nCritSeedRequests = pWeapon->m_nCritSeedRequests();

		if (Vars::Debug::Logging.Value) I::CVar->ConsolePrintf("\n");
		SDK::Output("ProcessPacketEntities", std::format("{} ({:#x}): mCriticalStorage[i].m_flCritTokenBucket = {}", i, uintptr_t(pWeapon), pWeapon->m_flCritTokenBucket()).c_str(), { 100, 150, 255 }, Vars::Debug::Logging.Value);
		SDK::Output("ProcessPacketEntities", std::format("{} ({:#x}): mCriticalStorage[i].m_nCritChecks = {}", i, uintptr_t(pWeapon), pWeapon->m_nCritChecks()).c_str(), { 100, 150, 255 }, Vars::Debug::Logging.Value);
		SDK::Output("ProcessPacketEntities", std::format("{} ({:#x}): mCriticalStorage[i].m_nCritSeedRequests = {}", i, uintptr_t(pWeapon), pWeapon->m_nCritSeedRequests()).c_str(), { 100, 150, 255 }, Vars::Debug::Logging.Value);
	}

	bool bReturn = CALL_ORIGINAL(entmsg);

	pLocal = H::Entities.GetLocal();
	if (!pLocal || !pLocal->m_hMyWeapons())
	{
		SDK::Output("ProcessPacketEntities", "Failed to restore weapon crit data! (2)", { 255, 100, 100 });
		return bReturn;
	}

	for (auto& [iSlot, tStorage] : mCriticalStorage)
	{
		auto pWeapon = pLocal->GetWeaponFromSlot(iSlot);
		if (!pWeapon)
			break;

		pWeapon->m_flCritTokenBucket() = tStorage.m_flCritTokenBucket;
		pWeapon->m_nCritChecks() = tStorage.m_nCritChecks;
		pWeapon->m_nCritSeedRequests() = tStorage.m_nCritSeedRequests;

		if (Vars::Debug::Logging.Value) I::CVar->ConsolePrintf("\n");
		SDK::Output("ProcessPacketEntities", std::format("{} ({:#x}): pWeapon->m_flCritTokenBucket() = {}", iSlot, uintptr_t(pWeapon), pWeapon->m_flCritTokenBucket()).c_str(), { 100, 255, 150 }, Vars::Debug::Logging.Value);
		SDK::Output("ProcessPacketEntities", std::format("{} ({:#x}): pWeapon->m_nCritChecks() = {}", iSlot, uintptr_t(pWeapon), pWeapon->m_nCritChecks()).c_str(), { 100, 255, 150 }, Vars::Debug::Logging.Value);
		SDK::Output("ProcessPacketEntities", std::format("{} ({:#x}): pWeapon->m_nCritSeedRequests() = {}", iSlot, uintptr_t(pWeapon), pWeapon->m_nCritSeedRequests()).c_str(), { 100, 255, 150 }, Vars::Debug::Logging.Value);
	}

	return bReturn;
}