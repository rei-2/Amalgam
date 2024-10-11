#pragma once
#include "../../SDK/SDK.h"

#pragma warning ( disable : 4091 )

class CIncomingSequence
{
public:
	int InReliableState;
	int SequenceNr;
	float CurTime;

	CIncomingSequence(int inState, int seqNr, float time)
	{
		InReliableState = inState;
		SequenceNr = seqNr;
		CurTime = time;
	}
};

struct BoneMatrix
{
	matrix3x4 aBones[128];
};

struct TickRecord
{
	float flSimTime = 0.f;
	BoneMatrix BoneMatrix = {};
	Vec3 vOrigin = {};
	bool bOnShot = false;
	bool bInvalid = false;
};

class CBacktrack
{
	// logic
	bool WithinRewind(const TickRecord& record);

	// utils
	void SendLerp();
	void UpdateDatagram();
	void MakeRecords();
	void CleanRecords();

	// data
	std::unordered_map<int, bool> mDidShoot;

	// data - fake latency
	std::deque<CIncomingSequence> dSequences;
	int iLastInSequence = 0;

	bool bLastTickHeld = false;

public:
	float GetLerp();
	float GetFake();
	float GetReal(int iFlow = -1);

	std::deque<TickRecord>* GetRecords(CBaseEntity* pEntity);
	std::deque<TickRecord> GetValidRecords(std::deque<TickRecord>* pRecords, CTFPlayer* pLocal = nullptr, bool bDistance = false);

	void FrameStageNotify();
	void Run(CUserCmd* pCmd);
	void Restart();
	void SetLerp(IGameEvent* pEvent);
	void ResolverUpdate(CBaseEntity* pEntity);
	void ReportShot(int iIndex);
	void AdjustPing(CNetChannel* netChannel);

	int iTickCount = 0;
	float flMaxUnlag = 1.f;

	float flFakeLatency = 0.f;
	float flFakeInterp = 0.015f;
	float flWishInterp = 0.015f;

	std::unordered_map<CBaseEntity*, std::deque<TickRecord>> mRecords;
};

ADD_FEATURE(CBacktrack, Backtrack)