#pragma once
#include "../Misc/CPlayerResource.h"

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
	NETVAR(m_iTotalScore, void*, "CTFPlayerResource", "m_iTotalScore");
	NETVAR(m_iMaxHealth, void*, "CTFPlayerResource", "m_iMaxHealth");
	NETVAR(m_iMaxBuffedHealth, void*, "CTFPlayerResource", "m_iMaxBuffedHealth");
	NETVAR(m_iPlayerClass, void*, "CTFPlayerResource", "m_iPlayerClass");
	NETVAR(m_bArenaSpectator, void*, "CTFPlayerResource", "m_bArenaSpectator");
	NETVAR(m_iActiveDominations, void*, "CTFPlayerResource", "m_iActiveDominations");
	NETVAR(m_flNextRespawnTime, void*, "CTFPlayerResource", "m_flNextRespawnTime");
	NETVAR(m_iChargeLevel, void*, "CTFPlayerResource", "m_iChargeLevel");
	NETVAR(m_iDamage, void*, "CTFPlayerResource", "m_iDamage");
	NETVAR(m_iDamageAssist, void*, "CTFPlayerResource", "m_iDamageAssist");
	NETVAR(m_iDamageBoss, void*, "CTFPlayerResource", "m_iDamageBoss");
	NETVAR(m_iHealing, void*, "CTFPlayerResource", "m_iHealing");
	NETVAR(m_iHealingAssist, void*, "CTFPlayerResource", "m_iHealingAssist");
	NETVAR(m_iDamageBlocked, void*, "CTFPlayerResource", "m_iDamageBlocked");
	NETVAR(m_iCurrencyCollected, void*, "CTFPlayerResource", "m_iCurrencyCollected");
	NETVAR(m_iBonusPoints, void*, "CTFPlayerResource", "m_iBonusPoints");
	NETVAR(m_iPlayerLevel, void*, "CTFPlayerResource", "m_iPlayerLevel");
	NETVAR(m_iStreaks, void*, "CTFPlayerResource", "m_iStreaks");
	NETVAR(m_iUpgradeRefundCredits, void*, "CTFPlayerResource", "m_iUpgradeRefundCredits");
	NETVAR(m_iBuybackCredits, void*, "CTFPlayerResource", "m_iBuybackCredits");
	NETVAR(m_iPartyLeaderRedTeamIndex, int, "CTFPlayerResource", "m_iPartyLeaderRedTeamIndex");
	NETVAR(m_iPartyLeaderBlueTeamIndex, int, "CTFPlayerResource", "m_iPartyLeaderBlueTeamIndex");
	NETVAR(m_iEventTeamStatus, int, "CTFPlayerResource", "m_iEventTeamStatus");
	NETVAR(m_iPlayerClassWhenKilled, void*, "CTFPlayerResource", "m_iPlayerClassWhenKilled");
	NETVAR(m_iConnectionState, void*, "CTFPlayerResource", "m_iConnectionState");
	NETVAR(m_flConnectTime, void*, "CTFPlayerResource", "m_flConnectTime");

	int GetPing(int idx)
	{
		static int nOffset = U::NetVars.GetNetVar("CTFPlayerResource", "m_iPing");
		return *reinterpret_cast<int*>(std::uintptr_t(this) + nOffset + idx * 4);
	}

	int GetKills(int idx)
	{
		static int nOffset = U::NetVars.GetNetVar("CTFPlayerResource", "m_iScore");
		return *reinterpret_cast<int*>(std::uintptr_t(this) + nOffset + idx * 4);
	}

	int GetDeaths(int idx)
	{
		static int nOffset = U::NetVars.GetNetVar("CTFPlayerResource", "m_iDeaths");
		return *reinterpret_cast<int*>(std::uintptr_t(this) + nOffset + idx * 4);
	}

	bool GetConnected(int idx)
	{
		static int nOffset = U::NetVars.GetNetVar("CTFPlayerResource", "m_bConnected");
		return *reinterpret_cast<bool*>(std::uintptr_t(this) + nOffset + idx);
	}

	int GetTeam(int idx)
	{
		static int nOffset = U::NetVars.GetNetVar("CTFPlayerResource", "m_iTeam");
		return *reinterpret_cast<int*>(std::uintptr_t(this) + nOffset + idx * 4);
	}

	bool IsAlive(int idx)
	{
		static int nOffset = U::NetVars.GetNetVar("CTFPlayerResource", "m_bAlive");
		return *reinterpret_cast<bool*>(std::uintptr_t(this) + nOffset + idx);
	}

	int GetHealth(int idx)
	{
		static int nOffset = U::NetVars.GetNetVar("CTFPlayerResource", "m_iHealth");
		return *reinterpret_cast<int*>(std::uintptr_t(this) + nOffset + idx * 4);
	}

	unsigned GetAccountID(int idx)
	{
		static int nOffset = U::NetVars.GetNetVar("CTFPlayerResource", "m_iAccountID");
		return *reinterpret_cast<int*>(std::uintptr_t(this) + nOffset + idx * 4);
	}

	bool GetValid(int idx)
	{
		static int nOffset = U::NetVars.GetNetVar("CTFPlayerResource", "m_bValid");
		return *reinterpret_cast<bool*>(std::uintptr_t(this) + nOffset + idx);
	}

	int GetUserID(int idx)
	{
		static int nOffset = U::NetVars.GetNetVar("CTFPlayerResource", "m_iUserID");
		return *reinterpret_cast<int*>(std::uintptr_t(this) + nOffset + idx * 4);
	}

	const char* GetPlayerName(int idx)
	{
		static int nOffset = U::NetVars.GetNetVar("CTFPlayerResource", "m_iPing") - 816;
		return *reinterpret_cast<const char**>(std::uintptr_t(this) + nOffset + idx * 8);
	}

	int GetScore(int idx)
	{
		static int nOffset = U::NetVars.GetNetVar("CTFPlayerResource", "m_iTotalScore");
		return *reinterpret_cast<int*>(std::uintptr_t(this) + nOffset + idx * 4);
	}

	int GetLevel(int idx)
	{
		static int nOffset = U::NetVars.GetNetVar("CTFPlayerResource", "m_iPlayerLevel");
		return *reinterpret_cast<int*>(std::uintptr_t(this) + nOffset + idx * 4);
	}

	int GetDamage(int idx)
	{
		static int nOffset = U::NetVars.GetNetVar("CTFPlayerResource", "m_iDamage");
		return *reinterpret_cast<int*>(std::uintptr_t(this) + nOffset + idx * 4);
	}

	int GetClass(int idx)
	{
		static int nOffset = U::NetVars.GetNetVar("CTFPlayerResource", "m_iPlayerClass");
		return *reinterpret_cast<int*>(std::uintptr_t(this) + nOffset + idx * 4);
	}

	int GetMaxHealth(int idx)
	{
		static int nOffset = U::NetVars.GetNetVar("CTFPlayerResource", "m_iMaxHealth");
		return *reinterpret_cast<int*>(std::uintptr_t(this) + nOffset + idx * 4);
	}

	int* GetStreaks(int idx)
	{
		static int nOffset = U::NetVars.GetNetVar("CTFPlayerResource", "m_iStreaks");
		return reinterpret_cast<int*>(std::uintptr_t(this) + nOffset + idx * 4);
	}

	float GetConnectionTime(int idx)
	{
		static int nOffset = U::NetVars.GetNetVar("CTFPlayerResource", "m_flConnectTime");
		return *reinterpret_cast<float*>(std::uintptr_t(this) + nOffset + idx * 4);
	}

	float GetNextRespawnTime(int idx)
	{
		static int nOffset = U::NetVars.GetNetVar("CTFPlayerResource", "m_flNextRespawnTime");
		return *reinterpret_cast<float*>(std::uintptr_t(this) + nOffset + idx * 4);
	}
};