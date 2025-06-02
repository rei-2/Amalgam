#include "CBaseEntity.h"

#include "../../SDK.h"

Vec3 CBaseEntity::GetSize()
{
	return m_vecMaxs() - m_vecMins();
}

Vec3 CBaseEntity::GetOffset()
{
	return m_vecMins() + m_vecMaxs();
}

Vec3 CBaseEntity::GetCenter()
{
	return m_vecOrigin() + GetOffset() / 2;
}

Vec3 CBaseEntity::GetRenderCenter()
{
	Vec3 vMins, vMaxs; GetRenderBounds(vMins, vMaxs);
	return GetRenderOrigin() + (vMaxs - vMins) / 2;
}

int CBaseEntity::IsInValidTeam()
{
	switch (int nTeamNum = m_iTeamNum())
	{
	case TF_TEAM_RED:
	case TF_TEAM_BLUE:
		return nTeamNum;
	}
	return 0;
}

int CBaseEntity::SolidMask()
{
	if (IsPlayer())
	{
		switch (m_iTeamNum())
		{
		case TF_TEAM_RED: return MASK_PLAYERSOLID | CONTENTS_BLUETEAM;
		case TF_TEAM_BLUE: return MASK_PLAYERSOLID | CONTENTS_REDTEAM;
		}
		return MASK_PLAYERSOLID;
	}
	return MASK_SOLID;
}