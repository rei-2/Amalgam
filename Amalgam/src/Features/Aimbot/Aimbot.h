#pragma once
#include "../../SDK/SDK.h"

class CAimbot
{
private:
	bool ShouldRun(CTFPlayer* pLocal, CTFWeaponBase* pWeapon);
	void RunMain(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd);
	void RunAimbot(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd, bool bSecondaryType = false);

	size_t m_iSize = 0;
	int m_iPlayer = 0;

public:
	void Run(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd);
	void Draw(CTFPlayer* pLocal);
	void Store(CBaseEntity* pEntity = nullptr, size_t iSize = 0);

	bool m_bRan = false;
	bool m_bRunningSecondary = false;

	DrawPath_t m_tPath = {};
};

ADD_FEATURE(CAimbot, Aimbot);