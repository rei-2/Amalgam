#pragma once
#include "../../../SDK/SDK.h"

class CAutoHeal
{
	bool ActivateOnVoice(CTFPlayer* pLocal, CWeaponMedigun* pWeapon, CUserCmd* pCmd);

public:
	void Run(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd);

	std::unordered_map<int, bool> m_mMedicCallers = {};
};

ADD_FEATURE(CAutoHeal, AutoHeal)