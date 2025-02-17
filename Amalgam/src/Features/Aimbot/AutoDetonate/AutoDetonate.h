#pragma once
#include "../../../SDK/SDK.h"

#include "../AimbotGlobal/AimbotGlobal.h"

class CAutoDetonate
{
	bool CheckDetonation(CTFPlayer* pLocal, EGroupType entityGroup, float flRadiusScale, CUserCmd* pCmd);
	bool CheckSelf(CTFPlayer* pLocal, EGroupType entityGroup);

	void PredictPlayers(CTFPlayer* pLocal, float flLatency, bool bLocal = false);
	void RestorePlayers();

	std::unordered_map<CBaseEntity*, Vec3> m_mRestore = {};

public:
	void Run(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd);
};

ADD_FEATURE(CAutoDetonate, AutoDetonate)