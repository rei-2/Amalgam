#pragma once
#include "../../SDK/SDK.h"

struct RestoreInfo_t
{
	Vec3 m_vecMins = {};
	Vec3 m_vecMaxs = {};
};

class CEnginePrediction
{
private:
	CMoveData m_MoveData = {};

private:
	void Simulate(CTFPlayer* pLocal, CUserCmd* pCmd);

	int m_nOldTickCount = 0;
	float m_flOldCurrentTime = 0.f;
	float m_flOldFrameTime = 0.f;

	bool m_bDoubletap = false;
	Vec3 m_vOrigin = {};

	std::unordered_map<CBaseEntity*, RestoreInfo_t> m_mRestore = {};

public:
	void Start(CTFPlayer* pLocal, CUserCmd* pCmd);
	void End(CTFPlayer* pLocal, CUserCmd* pCmd);

	void ScalePlayers(CBaseEntity* pLocal);
	void RestorePlayers();

	// localplayer use in net_update_end
	Vec3 vOrigin = {};
	Vec3 vVelocity = {};
	Vec3 vDirection = {};
	Vec3 vAngles = {};
};

ADD_FEATURE(CEnginePrediction, EnginePrediction)