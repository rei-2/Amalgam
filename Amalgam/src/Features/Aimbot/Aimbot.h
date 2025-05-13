#pragma once
#include "../../SDK/SDK.h"

class CAimbot
{
private:
	bool ShouldRun(CTFPlayer* pLocal, CTFWeaponBase* pWeapon);
	void RunMain(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd);
	void RunAimbot(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd, bool bSecondaryType = false);

public:
	void Run(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd);
	void Draw(CTFPlayer* pLocal);

	bool m_bRan = false;
	bool m_bRunningSecondary = false;
};

ADD_FEATURE(CAimbot, Aimbot)