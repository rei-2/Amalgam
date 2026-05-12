#pragma once
#include "../../SDK/SDK.h"

struct RealPath_t
{
	DrawPath_t m_tPath = {};
	size_t m_iSize = 0;
};

class CAimbot
{
private:
	bool ShouldRun(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd);
	void RunMain(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd);
	void RunAimbot(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd, bool bSecondaryType = false);

public:
	void Run(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd);
	void Draw(CTFPlayer* pLocal);
	void Store(CBaseEntity* pEntity, size_t iSize);
	void Store(bool bFrameStageNotify = true);

	bool m_bRan = false;
	bool m_bRunningSecondary = false;

	std::unordered_map<int, RealPath_t> m_mRealPaths = {};
};

ADD_FEATURE(CAimbot, Aimbot);