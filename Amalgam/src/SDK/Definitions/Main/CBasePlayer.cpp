#include "CBasePlayer.h"

#include "../../SDK.h"

bool CBasePlayer::IsAlive()
{
	return m_lifeState() == LIFE_ALIVE;
}

Vec3 CBasePlayer::GetShootPos()
{
	return m_vecOrigin() + m_vecViewOffset();
}

Vec3 CBasePlayer::GetEyePosition()
{
	return GetAbsOrigin() + m_vecViewOffset();
}

bool CBasePlayer::OnSolid()
{
	return m_hGroundEntity() || IsOnGround();
}

bool CBasePlayer::IsSwimming()
{
	return m_nWaterLevel() > 1;
}

bool CBasePlayer::IsUnderwater()
{
	return m_nWaterLevel() > 2;
}