#pragma once
#include "../../SDK/SDK.h"

class CSpectate
{
public:
	void NetUpdateEnd(CTFPlayer* pLocal);
	void NetUpdateStart(CTFPlayer* pLocal);
	void CreateMove(CTFPlayer* pLocal, CUserCmd* pCmd);
	void SetTarget(int iTarget);

	int m_iTarget = -1;
	int m_iIntendedTarget = -1;
	Vec3 m_vOldView = {}; // don't let spectating change viewangles

	CBaseEntity* m_pOriginalTarget;
	int m_iOriginalMode = OBS_MODE_NONE;
	CBaseEntity* m_pTargetTarget;
	int m_iTargetMode = OBS_MODE_NONE;
};

ADD_FEATURE(CSpectate, Spectate)