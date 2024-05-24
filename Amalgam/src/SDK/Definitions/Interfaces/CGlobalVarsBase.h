#pragma once
#include "Interface.h"

class CSaveRestoreData;

class CGlobalVarsBase
{
public:
	float realtime;
	int framecount;
	float absoluteframetime;
	float curtime;
	float frametime;
	int maxClients;
	int tickcount;
	float interval_per_tick;
	float interpolation_amount;
	int simTicksThisFrame;
	int network_protocol;
	CSaveRestoreData* pSaveData;
	bool m_bClient;
	int nTimestampNetworkingBase;
	int nTimestampRandomizeWindow;
};

MAKE_INTERFACE_SIGNATURE(CGlobalVarsBase, GlobalVars, "engine.dll", "48 8D 05 ? ? ? ? C3 CC CC CC CC CC CC CC CC 48 8B CA", 0x0, 0);