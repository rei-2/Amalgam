#include "../SDK/SDK.h"

MAKE_SIGNATURE(CL_ProcessPacketEntities, "engine.dll", "48 89 7C 24 ? 4C 89 64 24 ? 55 41 56 41 57 48 8D AC 24 ? ? ? ? B8 ? ? ? ? E8 ? ? ? ? 48 2B E0 4C 8B F9", 0x0);

class SVC_PacketEntities : public CNetMessage
{
	int	GetGroup() const { return ENTITIES; }

public:

	byte pad1[8];
	int			m_nMaxEntries;
	int			m_nUpdatedEntries;
	bool		m_bIsDelta;
	bool		m_bUpdateBaseline;
	int			m_nBaseline;
	int			m_nDeltaFrom;
	int			m_nLength;
	bf_read		m_DataIn;
	bf_write	m_DataOut;
};

struct CriticalStorage_t
{
	float m_flCritTokenBucket;
	int m_nCritChecks;
	int m_nCritSeedRequests;
};

MAKE_HOOK(CL_ProcessPacketEntities, S::CL_ProcessPacketEntities(), bool, __fastcall,
	SVC_PacketEntities* entmsg)
{
	if (entmsg->m_bIsDelta) // we won't need to restore
		return CALL_ORIGINAL(entmsg);

	CTFPlayer* pLocal = I::ClientEntityList->GetClientEntity(I::EngineClient->GetLocalPlayer())->As<CTFPlayer>();
	if (!pLocal)
	{
		SDK::Output("ProcessPacketEntities", "Failed to restore weapon crit data! (1)", { 255, 100, 100, 255 }, Vars::Debug::Logging.Value);
		return CALL_ORIGINAL(entmsg);
	}

	auto hWeapons = pLocal->GetMyWeapons();
	if (!hWeapons)
	{
		SDK::Output("ProcessPacketEntities", "Failed to restore weapon crit data! (2)", { 255, 100, 100, 255 }, Vars::Debug::Logging.Value);
		return CALL_ORIGINAL(entmsg);
	}

	std::unordered_map<int, CriticalStorage_t> mCriticalStorage = {};
	for (size_t i = 0; hWeapons[i]; i++)
	{
		auto pWeapon = pLocal->GetWeaponFromSlot(int(i));
		if (!pWeapon)
			break;

		const int iSlot = pWeapon->GetSlot();
		mCriticalStorage[iSlot].m_flCritTokenBucket = pWeapon->m_flCritTokenBucket();
		mCriticalStorage[iSlot].m_nCritChecks = pWeapon->m_nCritChecks();
		mCriticalStorage[iSlot].m_nCritSeedRequests = pWeapon->m_nCritSeedRequests();

		if (Vars::Debug::Logging.Value) I::CVar->ConsolePrintf("\n");
		SDK::Output("ProcessPacketEntities", std::format("{} ({:#x}): mCriticalStorage[iSlot].m_flCritTokenBucket = {}", iSlot, uintptr_t(pWeapon), pWeapon->m_flCritTokenBucket()).c_str(), { 100, 150, 255, 255 }, Vars::Debug::Logging.Value);
		SDK::Output("ProcessPacketEntities", std::format("{} ({:#x}): mCriticalStorage[iSlot].m_nCritChecks = {}", iSlot, uintptr_t(pWeapon), pWeapon->m_nCritChecks()).c_str(), { 100, 150, 255, 255 }, Vars::Debug::Logging.Value);
		SDK::Output("ProcessPacketEntities", std::format("{} ({:#x}): mCriticalStorage[iSlot].m_nCritSeedRequests = {}", iSlot, uintptr_t(pWeapon), pWeapon->m_nCritSeedRequests()).c_str(), { 100, 150, 255, 255 }, Vars::Debug::Logging.Value);
	}

	bool bReturn = CALL_ORIGINAL(entmsg);

	pLocal = I::ClientEntityList->GetClientEntity(I::EngineClient->GetLocalPlayer())->As<CTFPlayer>();
	if (!pLocal)
	{
		SDK::Output("ProcessPacketEntities", "Failed to restore weapon crit data! (3)", { 255, 100, 100, 255 }, Vars::Debug::Logging.Value);
		return bReturn;
	}

	hWeapons = pLocal->GetMyWeapons();
	if (!hWeapons)
	{
		SDK::Output("ProcessPacketEntities", "Failed to restore weapon crit data! (4)", { 255, 100, 100, 255 }, Vars::Debug::Logging.Value);
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
		SDK::Output("ProcessPacketEntities", std::format("{} ({:#x}): pWeapon->m_flCritTokenBucket() = {}", iSlot, uintptr_t(pWeapon), pWeapon->m_flCritTokenBucket()).c_str(), { 100, 255, 150, 255 }, Vars::Debug::Logging.Value);
		SDK::Output("ProcessPacketEntities", std::format("{} ({:#x}): pWeapon->m_nCritChecks() = {}", iSlot, uintptr_t(pWeapon), pWeapon->m_nCritChecks()).c_str(), { 100, 255, 150, 255 }, Vars::Debug::Logging.Value);
		SDK::Output("ProcessPacketEntities", std::format("{} ({:#x}): pWeapon->m_nCritSeedRequests() = {}", iSlot, uintptr_t(pWeapon), pWeapon->m_nCritSeedRequests()).c_str(), { 100, 255, 150, 255 }, Vars::Debug::Logging.Value);
	}

	return bReturn;
}