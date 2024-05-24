#pragma once
#include "IGameResources.h"
#include "String.h"
#include "../Main/CBaseEntity.h"

#define PLAYER_UNCONNECTED_NAME	"unconnected"
#define PLAYER_ERROR_NAME "ERRORNAME"

class CPlayerResource : public CBaseEntity, public IGameResources
{
public:
	NETVAR(m_iPing, void*, "CPlayerResource", "m_iPing");
	NETVAR(m_iScore, void*, "CPlayerResource", "m_iScore");
	NETVAR(m_iDeaths, void*, "CPlayerResource", "m_iDeaths");
	NETVAR(m_bConnected, void*, "CPlayerResource", "m_bConnected");
	NETVAR(m_iTeam, void*, "CPlayerResource", "m_iTeam");
	NETVAR(m_bAlive, void*, "CPlayerResource", "m_bAlive");
	NETVAR(m_iHealth, void*, "CPlayerResource", "m_iHealth");
	NETVAR(m_iAccountID, void*, "CPlayerResource", "m_iAccountID");
	NETVAR(m_bValid, void*, "CPlayerResource", "m_bValid");
	NETVAR(m_iUserID, void*, "CPlayerResource", "m_iUserID");
};