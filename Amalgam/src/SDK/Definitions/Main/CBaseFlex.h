#pragma once
#include "CBaseAnimating.h"

class IHasLocalToGlobalFlexSettings
{
public:
	virtual void EnsureTranslations(const /*flexsettinghdr_t*/ void* pSettinghdr) = 0;
};

class CBaseFlex : public CBaseAnimatingOverlay, public IHasLocalToGlobalFlexSettings
{
public:
	NETVAR(m_flexWeight, void*, "CBaseFlex", "m_flexWeight");
	NETVAR(m_blinktoggle, int, "CBaseFlex", "m_blinktoggle");
	NETVAR(m_viewtarget, Vec3, "CBaseFlex", "m_viewtarget");
};