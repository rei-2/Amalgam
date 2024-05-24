#pragma once
#include "../../SDK/SDK.h"

class CEnginePrediction
{
private:
	CMoveData m_MoveData = {};

private:
	int GetTickbase(CUserCmd* pCmd, CTFPlayer* pLocal);
	void Simulate(CTFPlayer* pLocal, CUserCmd* pCmd);

	int m_nOldTickCount = 0;
	float m_fOldCurrentTime = 0.f;
	float m_fOldFrameTime = 0.f;

	bool bSimulated = false;

public:
	void Start(CTFPlayer* pLocal, CUserCmd* pCmd);
	void End(CTFPlayer* pLocal, CUserCmd* pCmd);
};

ADD_FEATURE(CEnginePrediction, EnginePrediction)