#pragma once
#include "CBaseEntity.h"

class CCaptureFlag : public CBaseEntity
{
public:
	NETVAR(m_bDisabled, bool, "CCaptureFlag", "m_bDisabled");
	NETVAR(m_bVisibleWhenDisabled, bool, "CCaptureFlag", "m_bVisibleWhenDisabled");
	NETVAR(m_nType, int, "CCaptureFlag", "m_nType");
	NETVAR(m_nFlagStatus, int, "CCaptureFlag", "m_nFlagStatus");
	NETVAR(m_flResetTime, float, "CCaptureFlag", "m_flResetTime");
	NETVAR(m_flNeutralTime, float, "CCaptureFlag", "m_flNeutralTime");
	NETVAR(m_flMaxResetTime, float, "CCaptureFlag", "m_flMaxResetTime");
	NETVAR(m_hPrevOwner, EHANDLE, "CCaptureFlag", "m_hPrevOwner");
	NETVAR(m_szModel, const char*, "CCaptureFlag", "m_szModel");
	NETVAR(m_szHudIcon, const char*, "CCaptureFlag", "m_szHudIcon");
	NETVAR(m_szPaperEffect, const char*, "CCaptureFlag", "m_szPaperEffect");
	NETVAR(m_szTrailEffect, const char*, "CCaptureFlag", "m_szTrailEffect");
	NETVAR(m_nUseTrailEffect, int, "CCaptureFlag", "m_nUseTrailEffect");
	NETVAR(m_nPointValue, int, "CCaptureFlag", "m_nPointValue");
	NETVAR(m_flAutoCapTime, float, "CCaptureFlag", "m_flAutoCapTime");
	NETVAR(m_bGlowEnabled, bool, "CCaptureFlag", "m_bGlowEnabled");
	NETVAR(m_flTimeToSetPoisonous, float, "CCaptureFlag", "m_flTimeToSetPoisonous");
};