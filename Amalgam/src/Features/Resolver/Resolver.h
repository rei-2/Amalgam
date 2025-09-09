#pragma once
#include "../../SDK/SDK.h"
#include <optional>

struct ResolveData
{
	float m_flYaw = 0.f; // to be added
	float m_flPitch = 0.f; // to be set, if pitch oob

	bool m_bYaw = false;
	bool m_bPitch = false;
	bool m_bMinwalk = true;
	bool m_bView = false; // instead of offsetting from sent angle, offset from view to local

	bool m_bAutoSetYaw = true;
	bool m_bAutoSetPitch = true;
	bool m_bFirstOOBPitch = false;
	bool m_bInversePitch = false;
};

class CResolver
{
private:
	void StoreSniperDots(CTFPlayerResource* pResource);
	std::optional<float> GetPitchForSniperDot(CTFPlayer* pEntity, CTFPlayerResource* pResource);
	
	std::unordered_map<int, ResolveData> m_mResolverData = {};
	std::unordered_map<int, Vec3> m_mSniperDots = {};

	int m_iWaitingForTarget = -1;
	float m_flWaitingForDamage = 0.f;
	bool m_bWaitingForHeadshot = false;

	float m_flLastYawCycle = 0.f;
	float m_flLastPitchCycle = 0.f;
	float m_flLastMinwalkCycle = 0.f;
	float m_flLastViewCycle = 0.f;

public:
	void FrameStageNotify();
	void CreateMove(CTFPlayer* pLocal);
	void HitscanRan(CTFPlayer* pLocal, CTFPlayer* pTarget, CTFWeaponBase* pWeapon, int nHitbox = HITBOX_MAX);
	void PlayerHurt(IGameEvent* pEvent);
	void SetYaw(int iUserID, float flValue, bool bAuto = false);
	void SetPitch(int iUserID, float flValue, bool bInverse = false, bool bAuto = false);
	void SetMinwalk(int iUserID, bool bValue);
	void SetView(int iUserID, bool bValue);
	bool GetAngles(CTFPlayer* pPlayer, float* pYaw = nullptr, float* pPitch = nullptr, bool* pMinwalk = nullptr, bool bFake = false);
	void Reset();
};

ADD_FEATURE(CResolver, Resolver);