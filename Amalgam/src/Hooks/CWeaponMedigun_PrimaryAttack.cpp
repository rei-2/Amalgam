#include "../SDK/SDK.h"

#include "../Features/Backtrack/Backtrack.h"

MAKE_SIGNATURE(CWeaponMedigun_PrimaryAttack, "client.dll", "40 53 48 83 EC ? 8B 91 ? ? ? ? 48 8B D9 85 D2 0F 84 ? ? ? ? 48 89 7C 24 ? B8 ? ? ? ? 83 FA ? 74 ? 0F B7 C2 48 8B 3D ? ? ? ? 8B C8 48 83 C7 ? 48 C1 E1 ? 48 03 F9 0F 84 ? ? ? ? C1 EA ? 39 57 ? 0F 85 ? ? ? ? 48 8B 3F 48 85 FF 74", 0x0);

struct Restore_t
{
	Vec3 m_vOrigin;
	Vec3 m_vMins;
	Vec3 m_vMaxs;
};

MAKE_HOOK(CWeaponMedigun_PrimaryAttack, S::CWeaponMedigun_PrimaryAttack(), void,
	void* rcx)
{
	auto pWeapon = reinterpret_cast<CWeaponMedigun*>(rcx);
	auto pOwner = pWeapon->m_hOwner()->As<CTFPlayer>();
	if (!pOwner || !pOwner->m_pCurrentCommand())
		return CALL_ORIGINAL(rcx);

	// do pseudo lagcomp for the client
	std::unordered_map<CBaseEntity*, Restore_t> mRestore = {};

	auto pCmd = pOwner->m_pCurrentCommand();
	float flTargetTime = TICKS_TO_TIME(pCmd->tick_count - TIME_TO_TICKS(F::Backtrack.GetFakeInterp()));
	for (auto pEntity : H::Entities.GetGroup(EntityEnum::PlayerTeam))
	{
		auto pPlayer = pEntity->As<CTFPlayer>();
		if (pPlayer == pOwner || !pPlayer->IsAlive() || pPlayer->IsAGhost())
			continue;

		std::vector<TickRecord*> vRecords = {};
		if (!F::Backtrack.GetRecords(pEntity, vRecords))
			continue;
		vRecords = F::Backtrack.GetValidRecords(vRecords);
		if (!vRecords.size())
			continue;

		for (auto pRecord : vRecords)
		{
			if (pRecord->m_flSimTime <= flTargetTime)
			{
				mRestore[pEntity] = { pEntity->GetAbsOrigin(), pEntity->m_vecMins(), pEntity->m_vecMaxs() };
				pEntity->SetAbsOrigin(pRecord->m_vOrigin);
				pEntity->m_vecMins() = pRecord->m_vMins;
				pEntity->m_vecMaxs() = pRecord->m_vMaxs;
				break;
			}
		}
	}

	CALL_ORIGINAL(rcx);

	for (auto& [pEntity, tRestore] : mRestore)
	{
		pEntity->SetAbsOrigin(tRestore.m_vOrigin);
		pEntity->m_vecMins() = tRestore.m_vMins;
		pEntity->m_vecMaxs() = tRestore.m_vMaxs;
	}
	mRestore.clear();
}