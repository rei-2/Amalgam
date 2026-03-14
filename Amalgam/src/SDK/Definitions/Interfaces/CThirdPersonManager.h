#pragma once
#include "Interface.h"
#include "../Types.h"

class CThirdPersonManager
{
public:
	Vector m_vecCameraOffset;
	Vector m_vecDesiredCameraOffset;
	Vector m_vecCameraOrigin;
	bool m_bUseCameraOffsets;
	QAngle m_ViewAngles;
	float m_flFraction;
	float m_flUpFraction;
	float m_flTargetFraction;
	float m_flTargetUpFraction;
	bool m_bOverrideThirdPerson;
	bool m_bForced;
	float m_flUpOffset;
	float m_flLerpTime;
	float m_flUpLerpTime;
};

MAKE_INTERFACE_SIGNATURE(CThirdPersonManager, ThirdPersonManager, "client.dll", "48 8D 0D ? ? ? ? 0F 29 BC 24", 0x0, 0);