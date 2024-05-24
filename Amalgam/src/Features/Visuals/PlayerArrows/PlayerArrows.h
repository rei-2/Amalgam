#pragma once
#include "../../../SDK/SDK.h"

class CPlayerArrows
{
private:
	bool ShouldRun(CTFPlayer* pLocal);
	void DrawArrowTo(const Vec3& vecFromPos, const Vec3& vecToPos, Color_t color);

public:
	void Run(CTFPlayer* pLocal);
};

ADD_FEATURE(CPlayerArrows, PlayerArrows)