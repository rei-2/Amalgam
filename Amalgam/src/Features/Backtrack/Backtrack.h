#pragma once
#include "../../SDK/SDK.h"

#pragma warning ( disable : 4091 )

class CIncomingSequence
{
public:
	int m_nInReliableState;
	int m_nSequenceNr;
	float m_flTime;

	CIncomingSequence(int iState, int iSequence, float flTime)
	{
		m_nInReliableState = iState;
		m_nSequenceNr = iSequence;
		m_flTime = flTime;
	}
};

struct BoneMatrix
{
	matrix3x4 m_aBones[128];
};

struct TickRecord
{
	float m_flSimTime = 0.f;
	BoneMatrix m_BoneMatrix = {};
	Vec3 m_vOrigin = {};
	Vec3 m_vMins = {};
	Vec3 m_vMaxs = {};
	bool m_bOnShot = false;
	bool m_bInvalid = false;
};

class CBacktrack
{
	void SendLerp();
	void UpdateDatagram();
	void MakeRecords();
	void CleanRecords();

	std::unordered_map<int, bool> m_mDidShoot;

	std::deque<CIncomingSequence> m_dSequences;
	int m_iLastInSequence = 0;
	int m_nOldInSequenceNr = 0;
	int m_nOldInReliableState = 0;
	int m_nLastInSequenceNr = 0;
	int m_nOldTickBase = 0;

public:
	float GetLerp();
	float GetFake();
	float GetReal(int iFlow = -1, bool bNoFake = true);
	int GetAnticipatedChoke(int iMethod = Vars::Aimbot::General::AimType.Value);

	std::deque<TickRecord>* GetRecords(CBaseEntity* pEntity);
	std::deque<TickRecord> GetValidRecords(std::deque<TickRecord>* pRecords, CTFPlayer* pLocal = nullptr, bool bDistance = false);

	void FrameStageNotify();
	void Run(CUserCmd* pCmd);
	void Reset();
	void SetLerp(IGameEvent* pEvent);
	void ResolverUpdate(CBaseEntity* pEntity);
	void ReportShot(int iIndex);
	void AdjustPing(CNetChannel* netChannel);
	void RestorePing(CNetChannel* netChannel);

	int m_iTickCount = 0;
	float m_flMaxUnlag = 1.f;

	float m_flFakeLatency = 0.f;
	float m_flFakeInterp = 0.015f;
	float m_flWishInterp = 0.015f;

	std::unordered_map<CBaseEntity*, std::deque<TickRecord>> m_mRecords;
};

ADD_FEATURE(CBacktrack, Backtrack)