#pragma once
#include "CPlayerResource.h"

enum MM_PlayerConnectionState_t
{
	MM_DISCONNECTED = 0,
	MM_CONNECTED,
	MM_CONNECTING,
	MM_LOADING,
	MM_WAITING_FOR_PLAYER
};

enum ETFStreak
{
	kTFStreak_Kills = 0,
	kTFStreak_KillsAll = 1,
	kTFStreak_Ducks = 2,
	kTFStreak_Duck_levelup = 3,
	kTFStreak_COUNT = 4
};

class CTFPlayerResource : public CPlayerResource
{
public:
	NETVAR_ARRAY(m_iTotalScore, int, "CTFPlayerResource", "m_iTotalScore");
	NETVAR_ARRAY(m_iMaxHealth, int, "CTFPlayerResource", "m_iMaxHealth");
	NETVAR_ARRAY(m_iMaxBuffedHealth, int, "CTFPlayerResource", "m_iMaxBuffedHealth");
	NETVAR_ARRAY(m_iPlayerClass, int, "CTFPlayerResource", "m_iPlayerClass");
	NETVAR_ARRAY(m_bArenaSpectator, bool, "CTFPlayerResource", "m_bArenaSpectator");
	NETVAR_ARRAY(m_iActiveDominations, int, "CTFPlayerResource", "m_iActiveDominations");
	NETVAR_ARRAY(m_flNextRespawnTime, float, "CTFPlayerResource", "m_flNextRespawnTime");
	NETVAR_ARRAY(m_iChargeLevel, int, "CTFPlayerResource", "m_iChargeLevel");
	NETVAR_ARRAY(m_iDamage, int, "CTFPlayerResource", "m_iDamage");
	NETVAR_ARRAY(m_iDamageAssist, int, "CTFPlayerResource", "m_iDamageAssist");
	NETVAR_ARRAY(m_iDamageBoss, int, "CTFPlayerResource", "m_iDamageBoss");
	NETVAR_ARRAY(m_iHealing, int, "CTFPlayerResource", "m_iHealing");
	NETVAR_ARRAY(m_iHealingAssist, int, "CTFPlayerResource", "m_iHealingAssist");
	NETVAR_ARRAY(m_iDamageBlocked, int, "CTFPlayerResource", "m_iDamageBlocked");
	NETVAR_ARRAY(m_iCurrencyCollected, int, "CTFPlayerResource", "m_iCurrencyCollected");
	NETVAR_ARRAY(m_iBonusPoints, int, "CTFPlayerResource", "m_iBonusPoints");
	NETVAR_ARRAY(m_iPlayerLevel, int, "CTFPlayerResource", "m_iPlayerLevel");
	NETVAR_ARRAY(m_iStreaks, int, "CTFPlayerResource", "m_iStreaks");
	NETVAR_ARRAY(m_iUpgradeRefundCredits, int, "CTFPlayerResource", "m_iUpgradeRefundCredits");
	NETVAR_ARRAY(m_iBuybackCredits, int, "CTFPlayerResource", "m_iBuybackCredits");
	NETVAR(m_iPartyLeaderRedTeamIndex, int, "CTFPlayerResource", "m_iPartyLeaderRedTeamIndex");
	NETVAR(m_iPartyLeaderBlueTeamIndex, int, "CTFPlayerResource", "m_iPartyLeaderBlueTeamIndex");
	NETVAR(m_iEventTeamStatus, int, "CTFPlayerResource", "m_iEventTeamStatus");
	NETVAR_ARRAY(m_iPlayerClassWhenKilled, int, "CTFPlayerResource", "m_iPlayerClassWhenKilled");
	NETVAR_ARRAY(m_iConnectionState, int, "CTFPlayerResource", "m_iConnectionState");
	NETVAR_ARRAY(m_flConnectTime, float, "CTFPlayerResource", "m_flConnectTime");
};