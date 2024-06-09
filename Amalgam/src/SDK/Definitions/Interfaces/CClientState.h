#pragma once
#include "Interface.h"
#include "../Main/INetChannel.h"

MAKE_SIGNATURE(CBaseClientState_SendStringCmd, "engine.dll", "48 81 EC ? ? ? ? 48 8B 49", 0x0);

struct CUtlString
{
	char* m_pString;
};

struct CClockDriftMgr
{
	float m_ClockOffsets[16];
	int m_iCurClockOffset;
	int m_nServerTick;
	int m_nClientTick;
};

class CClientState
{
public:
	byte gap0[24];
	int m_Socket;
	INetChannel* m_NetChannel;
	unsigned int m_nChallengeNr;
	double m_flConnectTime;
	int m_nRetryNumber;
	char m_szRetryAddress[260];
	CUtlString m_sRetrySourceTag;
	int m_retryChallenge;
	int m_nSignonState;
	double m_flNextCmdTime;
	int m_nServerCount;
	unsigned __int64 m_ulGameServerSteamID;
	int m_nCurrentSequence;
	CClockDriftMgr m_ClockDriftMgr;
	int m_nDeltaTick;
	bool m_bPaused;
	float m_flPausedExpireTime;
	int m_nViewEntity;
	int m_nPlayerSlot;
	char m_szLevelFileName[128];
	byte gap24C[132];
	char m_szLevelBaseName[128];
	byte gap350[116];
	int m_nMaxClients;
	byte gap3C8[34856];
	void* m_StringTableContainer;
	bool m_bRestrictServerCommands;
	bool m_bRestrictClientCommands;
	byte gap8BFC[106];
	bool insimulation;
	int oldtickcount;
	float m_tickRemainder;
	float m_frameTime;
	int lastoutgoingcommand;
	int chokedcommands;
	int last_command_ack;
	int command_ack;
	int m_nSoundSequence;
	bool ishltv;
	bool isreplay;
	byte gap8C8A[278];
	int demonum;
	CUtlString demos[32];
	byte gap8EE8[344184];
	bool m_bMarkedCRCsUnverified;

public:
	void SendStringCmd(const char* command)
	{
		reinterpret_cast<void(__fastcall*)(void*, const char*)>(S::CBaseClientState_SendStringCmd())(this, command);
	}
};

MAKE_INTERFACE_SIGNATURE(CClientState, ClientState, "engine.dll", "48 8D 0D ? ? ? ? E8 ? ? ? ? F3 0F 5E 05", 0x0, 0);