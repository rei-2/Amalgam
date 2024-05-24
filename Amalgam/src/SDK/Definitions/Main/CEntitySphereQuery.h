#pragma once
#include "../../../Utils/Signatures/Signatures.h"
#include "../Definitions.h"
#include "CBaseEntity.h"

#define MAX_SPHERE_QUERY	512

#pragma warning (disable : 26495)

MAKE_SIGNATURE(CEntitySphereQuery, "client.dll", "40 53 48 83 EC ? 48 8B D9 C7 44 24 ? ? ? ? ? 33 C9", 0x0);

//credits to KGB
class CEntitySphereQuery
{
public:
	CEntitySphereQuery(const Vec3& center, const float radius, const int flagMask = 0, const int partitionMask = PARTITION_CLIENT_NON_STATIC_EDICTS)
	{
		static auto dwAddress = S::CEntitySphereQuery();
		reinterpret_cast<void(__thiscall*)(void*, const Vec3&, float, int, int)>(dwAddress)(this, center, radius, flagMask, partitionMask);
	}

	CBaseEntity* GetCurrentEntity()
	{
		return (m_nListIndex < m_nListCount) ? m_pList[m_nListIndex] : nullptr;
	}

	void NextEntity()
	{
		m_nListIndex++;
	}

private:
	int m_nListIndex, m_nListCount;
	CBaseEntity* m_pList[MAX_SPHERE_QUERY];
};