#pragma once
#include "CBaseAnimating.h"

class CHalloweenPickup : public CBaseAnimating
{
public:
};

class CHalloweenGiftPickup : public CHalloweenPickup
{
public:
	NETVAR(m_hTargetPlayer, EHANDLE, "CHalloweenGiftPickup", "m_hTargetPlayer");
};