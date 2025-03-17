#pragma once
#include "../../../SDK/SDK.h"
#include <functional>

struct PlayerData
{
	Vec3 m_vecOrigin = {};
	Vec3 m_vecVelocity = {};
	Vec3 m_vecBaseVelocity = {};
	Vec3 m_vecViewOffset = {};
	EHANDLE m_hGroundEntity = nullptr;
	int m_fFlags = 0;
	float m_flDucktime = 0.f;
	float m_flDuckJumpTime = 0.f;
	bool m_bDucked = false;
	bool m_bDucking = false;
	bool m_bInDuckJump = false;
	float m_flModelScale = 0.f;
	int m_nButtons = 0;
	float m_flLastMovementStunChange = 0.f;
	float m_flStunLerpTarget = 0.f;
	bool m_bStunNeedsFadeOut = false;
	float m_flPrevTauntYaw = 0.f;
	float m_flTauntYaw = 0.f;
	float m_flCurrentTauntMoveSpeed = 0.f;
	int m_iKartState = 0;
	float m_flVehicleReverseTime = 0.f;
	float m_flHypeMeter = 0.f;
	float m_flMaxspeed = 0.f;
	int m_nAirDucked = 0;
	bool m_bJumping = false;
	int m_iAirDash = 0;
	float m_flWaterJumpTime = 0.f;
	float m_flSwimSoundTime = 0.f;
	int m_surfaceProps = 0;
	void* m_pSurfaceData = nullptr;
	float m_surfaceFriction = 0.f;
	char m_chTextureType = 0;
	Vec3 m_vecPunchAngle = {};
	Vec3 m_vecPunchAngleVel = {};
	float m_flJumpTime = 0.f;
	byte m_MoveType = 0;
	byte m_MoveCollide = 0;
	Vec3 m_vecLadderNormal = {};
	float m_flGravity = 0.f;
	byte m_nWaterLevel = 0;
	byte m_nWaterType = 0;
	float m_flFallVelocity = 0.f;
	int m_nPlayerCond = 0;
	int m_nPlayerCondEx = 0;
	int m_nPlayerCondEx2 = 0;
	int m_nPlayerCondEx3 = 0;
	int m_nPlayerCondEx4 = 0;
	int _condition_bits = 0;
};
struct PlayerStorage
{
	CTFPlayer* m_pPlayer = nullptr;
	CMoveData m_MoveData = {};
	PlayerData m_PlayerData = {};

	float m_flAverageYaw = 0.f;
	bool m_bBunnyHop = false;

	float m_flSimTime = 0.f;
	float m_flPredictedDelta = 0.f;
	float m_flPredictedSimTime = 0.f;
	bool m_bDirectMove = true;

	bool m_bPredictNetworked = true;
	Vec3 m_vPredictedOrigin = {};

	std::deque<Vec3> m_vPath;

	bool m_bFailed = false;
	bool m_bInitFailed = false;
};

struct MoveData
{
	Vec3 m_vDirection = {};
	Vec3 m_vAngles = {};
	float m_flSimTime = 0.f;
	int m_iMode = 0;
	Vec3 m_vVelocity = {};
	Vec3 m_vOrigin = {};
};

class CMovementSimulation
{
private:
	void Store(PlayerStorage& playerStorage);
	void Reset(PlayerStorage& playerStorage);

	bool SetupMoveData(PlayerStorage& playerStorage);
	void GetAverageYaw(PlayerStorage& playerStorage, int iSamples);
	bool StrafePrediction(PlayerStorage& playerStorage, int iSamples);

	bool m_bOldInPrediction = false;
	bool m_bOldFirstTimePredicted = false;
	float m_flOldFrametime = 0.f;

	std::unordered_map<int, std::deque<MoveData>> mRecords;
	std::unordered_map<int, std::deque<float>> mSimTimes;

public:
	void Store();

	bool Initialize(CBaseEntity* pEntity, PlayerStorage& playerStorageOut, bool useHitchance = true, bool cancelStrafe = false);
	float GetPlayerGravity(CTFPlayer* pEntity);
	bool SetDuck(PlayerStorage& playerStorage, bool bDuck);
	void RunTick(PlayerStorage& playerStorage, bool bPath = true, std::function<void(CMoveData&)>* pCallback = nullptr);
	void RunTick(PlayerStorage& playerStorage, bool bPath, std::function<void(CMoveData&)> fCallback);
	void Restore(PlayerStorage& playerStorage);

	float GetPredictedDelta(CBaseEntity* pEntity);
};

ADD_FEATURE(CMovementSimulation, MoveSim)