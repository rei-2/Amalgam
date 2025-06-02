#pragma once
#include "CEconEntity.h"

class CEconWearable : public CEconEntity
{
public:
	VIRTUAL_ARGS(Equip, void, 230, (CBasePlayer* pOwner), this, pOwner);
	VIRTUAL_ARGS(UnEquip, void, 231, (CBasePlayer* pOwner), this, pOwner);
};