#pragma once
#include "CBaseEntity.h"

class CSniperDot : public CBaseEntity
{
public:
	NETVAR(m_flChargeStartTime, float, "CSniperDot", "m_flChargeStartTime");
};