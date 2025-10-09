#pragma once
#include "../../SDK/SDK.h"

struct Sequence_t
{
	int m_nInReliableState;
	int m_nSequenceNr;
	float m_flTime;
};

struct TickRecord
{
	float m_flSimTime = 0.f;
	Vec3 m_vOrigin = {};
	Vec3 m_vMins = {};
	Vec3 m_vMaxs = {};
	bool m_bOnShot = false;
	bool m_bInvalid = false;
	matrix3x4 m_aBones[MAXSTUDIOBONES];
};

class CBacktrack
{
private:
	void UpdateDatagram();
	void MakeRecords();
	void CleanRecords();

	std::unordered_map<CBaseEntity*, std::deque<TickRecord>> m_mRecords = {};
	std::unordered_map<int, bool> m_mDidShoot = {};

	std::deque<Sequence_t> m_dSequences;
	int m_iLastInSequence = 0;
	int m_nOldInSequenceNr = 0;
	int m_nOldInReliableState = 0;
	int m_nLastInSequenceNr = 0;
	int m_nOldTickBase = 0;
	float m_flMaxUnlag = 1.f;

	float m_flFakeLatency = 0.f;
	float m_flFakeInterp = 0.015f;

	bool m_bSettingUpBones = false;

public:
	void Store();
	void CreateMove(CUserCmd* pCmd);
	void SendLerp();
	void Draw(CTFPlayer* pLocal);
	void Reset();

	bool GetRecords(CBaseEntity* pEntity, std::vector<TickRecord*>& vReturn);
	std::vector<TickRecord*> GetValidRecords(std::vector<TickRecord*>& vRecords, CTFPlayer* pLocal = nullptr, bool bDistance = false, float flTimeMod = 0.f);
	matrix3x4* GetBones(CBaseEntity* pEntity);

	float GetReal(int iFlow = MAX_FLOWS, bool bNoFake = true);
	float GetWishFake();
	float GetWishLerp();
	float GetFakeLatency();
	float GetFakeInterp();
	float GetWindow();
	void SetLerp(IGameEvent* pEvent);
	int GetAnticipatedChoke(int iMethod = Vars::Aimbot::General::AimType.Value);

	void ResolverUpdate(CBaseEntity* pEntity);
	void ReportShot(int iIndex);
	void AdjustPing(CNetChannel* netChannel);
	void RestorePing(CNetChannel* netChannel);

	bool IsSettingUpBones() { return m_bSettingUpBones; }

	int m_iTickCount = 0;
	float m_flSentInterp = -1.f;

	TickRecord m_tRecord = {}; // for temporary use
};

ADD_FEATURE(CBacktrack, Backtrack);