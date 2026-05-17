#pragma once
#include "../../../SDK/SDK.h"

class CAntiAim
{
private:
	void FakeShotAngles(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd);
	float GetYawOffset(CTFPlayer* pEntity, bool bFake);
	float GetBaseYaw(CTFPlayer* pLocal, CUserCmd* pCmd, bool bFake);
	float GetYaw(CTFPlayer* pLocal, CUserCmd* pCmd, bool bFake);
	float GetPitch(float flCurPitch);
	void MinWalk(CTFPlayer* pLocal, CUserCmd* pCmd);

public:
	bool AntiAimOn();
	bool YawOn();
	bool ShouldRun(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd);
	void Run(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd, bool bSendPacket);
	void Draw(CTFPlayer* pLocal);

	inline int AntiAimTicks() { return 2; }

	Vec2 vFakeAngles = {};
	Vec2 vRealAngles = {};
	std::vector<std::pair<Vec3, Vec3>> vEdgeTrace = {};
};

ADD_FEATURE(CAntiAim, AntiAim);