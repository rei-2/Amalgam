#pragma once
#include "../../../SDK/SDK.h"

class CCraterCheck
{
private:
    void Draw3DPolygon(const Vec3& center, float radius, const Vec3& normal, int segments, bool willDie);
    Vec3 CrossProduct(const Vec3& a, const Vec3& b);
    float CalculateFallDamage(CTFPlayer* pLocal, float fallVelocity);
    bool GetAimTraceResult(CTFPlayer* pLocal, Vec3& hitPos, Vec3& hitNormal);
    bool IsPlayerAirborne(CTFPlayer* pLocal);

public:
    void Draw();
};

ADD_FEATURE(CCraterCheck, CraterCheck);