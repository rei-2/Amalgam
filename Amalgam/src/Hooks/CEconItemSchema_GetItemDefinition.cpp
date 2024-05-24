#include "../SDK/SDK.h"

struct CEconItemDefinition_t
{
	byte pad0[524];
	uint32 m_unEquipRegionMask;
	uint32 m_unEquipRegionConflictMask;
};

MAKE_SIGNATURE(CEconItemSchema_GetItemDefinition, "client.dll", "89 54 24 ? 53 48 83 EC ? 48 8B D9 48 8D 54 24 ? 48 81 C1 ? ? ? ? E8 ? ? ? ? 85 C0", 0x0);

MAKE_HOOK(CEconItemSchema_GetItemDefinition, S::CEconItemSchema_GetItemDefinition(), CEconItemDefinition_t*, __fastcall,
	void* ecx, int iItemIndex)
{
	const auto pItemDefinition = CALL_ORIGINAL(ecx, iItemIndex);
	if (Vars::Misc::Exploits::EquipRegionUnlock.Value && pItemDefinition)
	{
		pItemDefinition->m_unEquipRegionMask = 0;
		pItemDefinition->m_unEquipRegionConflictMask = 0;
	}

	return pItemDefinition;
}