#pragma once
#include "../../SDK/SDK.h"

struct ResolveData
{
	//config data
	bool bEnabled = false;			//	should we resolve this player

	//bruteforce data
	int iYawIndex = 0;

	//logical data
	std::pair<int, float> pLastSniperPitch = {0, 0.f};
	float flPitchNoise = 0.f;		//	noise around sniper dot pitch
	int iPitchNoiseSteps = 0;

	//historical data
	std::pair<std::pair<int, bool>, Vec3> pLastFireAngles = { { 0, false }, {} };
	Vec2 vOriginalAngles = {0.f, 0.f};
};

class PResolver
{
	void UpdateSniperDots();
	std::optional<float> GetPitchForSniperDot(CTFPlayer* pEntity);

	std::optional<float> PredictBaseYaw(CTFPlayer* pLocal, CTFPlayer* pEntity);

	bool ShouldRun(CTFPlayer* pLocal);
	bool ShouldRunEntity(CTFPlayer* pEntity);
	bool KeepOnShot(CTFPlayer* pEntity);
	bool IsOnShotPitchReliable(const float flPitch);
	float GetRealPitch(const float flPitch);
	void SetAngles(const Vec3 vAngles, CTFPlayer* pEntity);
	int GetPitchMode(CTFPlayer* pEntity);
	int GetYawMode(CTFPlayer* pEntity);
	void OnDormancy(CTFPlayer* pEntity);
	
	std::unordered_map<IClientEntity*, Vec3> mSniperDots;
	std::unordered_map<CTFPlayer*, ResolveData> mResolverData;
	std::pair<int, std::pair<CTFPlayer*, bool>> pWaiting = {0, {nullptr, false}};

public:
	void Aimbot(CTFPlayer* pEntity, const bool bHeadshot);
	void FrameStageNotify(CTFPlayer* pLocal);
	void CreateMove();
	void FXFireBullet(int iIndex, const Vec3 vAngles);
	void OnPlayerHurt(IGameEvent* pEvent);

	std::unordered_map<uint32_t, std::pair<int, int>> mResolverMode;
};

ADD_FEATURE(PResolver, Resolver)