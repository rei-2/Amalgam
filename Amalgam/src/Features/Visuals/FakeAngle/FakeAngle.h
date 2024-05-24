#pragma once
#include "../../../SDK/SDK.h"

class CFakeAngle
{
public:
	void Run(CTFPlayer* pLocal);

	matrix3x4 BoneMatrix[128];
	bool BonesSetup = false;

	bool DrawChams = false;
};

ADD_FEATURE(CFakeAngle, FakeAngle)