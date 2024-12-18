#pragma once
#include "IClientNetworkable.h"
#include "IClientRenderable.h"
#include "IClientThinkable.h"
#include "../Interfaces.h"

class CMouthInfo;
struct SpatializationInfo_t;

class IClientEntity : public IClientUnknown, public IClientRenderable, public IClientNetworkable, public IClientThinkable
{
public:
	virtual void Release(void) = 0;
	virtual const Vector& GetAbsOrigin(void) const = 0;
	virtual const QAngle& GetAbsAngles(void) const = 0;
	virtual CMouthInfo* GetMouth(void) = 0;
	virtual bool GetSoundSpatialization(SpatializationInfo_t& info) = 0;

	inline bool IsPlayer()
	{
		return GetClassID() == ETFClassID::CTFPlayer;
	}

	inline bool IsSentrygun()
	{
		return GetClassID() == ETFClassID::CObjectSentrygun;
	}

	inline bool IsDispenser()
	{
		return GetClassID() == ETFClassID::CObjectDispenser;
	}

	inline bool IsTeleporter()
	{
		return GetClassID() == ETFClassID::CObjectTeleporter;
	}

	inline bool IsBaseCombatWeapon()
	{
		return GetClassID() == ETFClassID::CBaseCombatWeapon;
	}

	inline bool IsWearable()
	{
		return GetClassID() == ETFClassID::CTFWearable;
	}

	inline bool IsBuilding()
	{
		switch (GetClassID())
		{
		case ETFClassID::CObjectDispenser:
		case ETFClassID::CObjectSentrygun:
		case ETFClassID::CObjectTeleporter: return true;
		default: return false;
		}
	}

	inline bool IsPickup()
	{
		switch (GetClassID())
		{
		case ETFClassID::CBaseAnimating: return I::ModelInfoClient->GetModelName(GetModel())[24] != 'h';
		case ETFClassID::CTFAmmoPack: return true;
		default: return false;
		}
	}

	inline bool IsNPC()
	{
		switch (GetClassID())
		{
		case ETFClassID::CEyeballBoss:
		case ETFClassID::CHeadlessHatman:
		case ETFClassID::CMerasmus:
		case ETFClassID::CTFBaseBoss:
		case ETFClassID::CTFTankBoss:
		case ETFClassID::CZombie:
			return true;
		default: return false;
		}
	}

	inline bool IsBomb()
	{
		switch (GetClassID())
		{
		case ETFClassID::CTFPumpkinBomb:
		case ETFClassID::CTFGenericBomb:
			return true;
		default: return false;
		}
	}

	template <typename T> inline T* As() { return reinterpret_cast<T*>(this); }
};