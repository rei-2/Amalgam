#pragma once
#include "../../../SDK/SDK.h"

class CNoSpreadHitscan
{
private:
	int GetSeed(CUserCmd* pCmd);
	float CalcMantissaStep(float val);

public:
	void Reset(bool bResetPrint = false);
	bool ShouldRun(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, bool bCreateMove = false);

	void AskForPlayerPerf();
	bool ParsePlayerPerf(bf_read& msgData);

	void Run(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd);

	std::string GetFormat(int m_ServerTime);

	bool bWaitingForPlayerPerf = false;
	int bSynced = 0, iBestSync = 0;
	double dRequestTime = 0.0;
	float flServerTime = 0.f;
	double dTimeDelta = 0.0;
	std::deque<double> vTimeDeltas = {};

	int iSeed = 0;
	float flMantissaStep = 0;
};

ADD_FEATURE(CNoSpreadHitscan, NoSpreadHitscan)