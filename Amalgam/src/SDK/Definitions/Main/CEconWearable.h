#pragma once
#include "CEconEntity.h"

class CEconWearable : public CEconEntity
{
public:
	inline void Equip(CBasePlayer* pOwner)
	{
		reinterpret_cast<void(*)(void*, CBasePlayer*)>(U::Memory.GetVFunc(this, 230))(this, pOwner);
	};

	inline void UnEquip(CBasePlayer* pOwner)
	{
		reinterpret_cast<void(*)(void*, CBasePlayer*)>(U::Memory.GetVFunc(this, 231))(this, pOwner);
	};
};