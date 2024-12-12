#pragma once
#include "../../../SDK/SDK.h"

#include "../AimbotGlobal/AimbotGlobal.h"

class CAutoDetonate
{
	bool CheckDetonation(CTFPlayer* pLocal, EGroupType entityGroup, float flRadiusScale, CUserCmd* pCmd);

	void PredictPlayers(CBaseEntity* pLocal, float flLatency);
	void RestorePlayers();

	std::unordered_map<CBaseEntity*, Vec3> m_mRestore = {};

public:
	void Run(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd);
};

ADD_FEATURE(CAutoDetonate, AutoDetonate)