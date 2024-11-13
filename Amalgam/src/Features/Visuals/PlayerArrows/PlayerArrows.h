#pragma once
#include "../../../SDK/SDK.h"

class CPlayerArrows
{
private:
	void DrawArrowTo(const Vec3& vFromPos, const Vec3& vToPos, Color_t tColor);

public:
	void Run(CTFPlayer* pLocal);
};

ADD_FEATURE(CPlayerArrows, PlayerArrows)