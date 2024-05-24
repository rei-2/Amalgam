#pragma once
#include "../../../SDK/SDK.h"

class CAntiAim
{
private:
	void FakeShotAngles(CUserCmd* pCmd);

	float EdgeDistance(CTFPlayer* pEntity, float flEdgeRayYaw, float flOffset);
	void RunOverlapping(CTFPlayer* pEntity, CUserCmd* pCmd, float& flRealYaw, bool bFake, float flEpsilon = 45.f);
	float GetYawOffset(CTFPlayer* pEntity, bool bFake);
	float GetBaseYaw(CTFPlayer* pLocal, CUserCmd* pCmd, bool bFake);
	float GetYaw(CTFPlayer* pLocal, CUserCmd* pCmd, bool bFake);

	float CalculateCustomRealPitch(float flWishPitch, bool bFakeDown);
	float GetPitch(float flCurPitch);

public:
	bool AntiAimOn();
	bool YawOn();
	bool ShouldRun(CTFPlayer* pLocal);

	bool GetEdge(CTFPlayer* pEntity, float flEdgeOrigYaw, bool bUpPitch);
	void Run(CTFPlayer* pLocal, CUserCmd* pCmd, bool* pSendPacket);

	Vec2 vFakeAngles = {};
	Vec2 vRealAngles = {};
	std::vector<std::pair<Vec3, Vec3>> vEdgeTrace = {};
};

ADD_FEATURE(CAntiAim, AntiAim)