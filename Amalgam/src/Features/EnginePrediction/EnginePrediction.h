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
	Vec3 m_vOriginalOrigin = {};
	Vec3 m_vOriginalVelocity = {};

	std::unordered_map<CTFPlayer*, RestoreInfo_t> m_mRestore = {};

public:
	void Start(CTFPlayer* pLocal, CUserCmd* pCmd);
	void End(CTFPlayer* pLocal, CUserCmd* pCmd);

	void ScalePlayers(CBaseEntity* pLocal);
	void RestorePlayers();

	bool m_bInPrediction = false;

	// localplayer use in net_update_end
	Vec3 m_vOrigin = {};
	Vec3 m_vVelocity = {};
	Vec3 m_vDirection = {};
	Vec3 m_vAngles = {};
};

ADD_FEATURE(CEnginePrediction, EnginePrediction)