#pragma once
#include "../../../SDK/SDK.h"
#include <functional>

struct MoveStorage
{
	CTFPlayer* m_pPlayer = nullptr;
	CMoveData m_MoveData = {};
	byte* m_pData = nullptr;

	float m_flAverageYaw = 0.f;
	bool m_bBunnyHop = false;

	float m_flSimTime = 0.f;
	float m_flPredictedDelta = 0.f;
	float m_flPredictedSimTime = 0.f;
	bool m_bDirectMove = true;

	bool m_bPredictNetworked = true;
	Vec3 m_vPredictedOrigin = {};

	std::vector<Vec3> m_vPath = {};

	bool m_bFailed = false;
	bool m_bInitFailed = false;
};

struct MoveData
{
	Vec3 m_vDirection = {};
	float m_flSimTime = 0.f;
	int m_iMode = 0;
	Vec3 m_vVelocity = {};
	Vec3 m_vOrigin = {};
};

class CMovementSimulation
{
private:
	void Store(MoveStorage& tStorage);
	void Reset(MoveStorage& tStorage);

	bool SetupMoveData(MoveStorage& tStorage);
	void GetAverageYaw(MoveStorage& tStorage, int iSamples);
	bool StrafePrediction(MoveStorage& tStorage, int iSamples);

	void SetBounds(CTFPlayer* pPlayer);
	void RestoreBounds(CTFPlayer* pPlayer);

	bool m_bOldInPrediction = false;
	bool m_bOldFirstTimePredicted = false;
	float m_flOldFrametime = 0.f;

	std::unordered_map<int, std::deque<MoveData>> m_mRecords = {};
	std::unordered_map<int, std::deque<float>> m_mSimTimes = {};

public:
	void Store();

	bool Initialize(CBaseEntity* pEntity, MoveStorage& tStorage, bool bHitchance = true, bool bStrafe = true);
	bool SetDuck(MoveStorage& tStorage, bool bDuck);
	void RunTick(MoveStorage& tStorage, bool bPath = true, std::function<void(CMoveData&)>* pCallback = nullptr);
	void RunTick(MoveStorage& tStorage, bool bPath, std::function<void(CMoveData&)> fCallback);
	void Restore(MoveStorage& tStorage);

	float GetPredictedDelta(CBaseEntity* pEntity);
};

ADD_FEATURE(CMovementSimulation, MoveSim);