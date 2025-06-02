#include "CBaseObject.h"

#include "../../SDK.h"

bool CBaseObject::IsDisabled()
{
	return m_bDisabled() || m_bHasSapper();
}



int CObjectSentrygun::MaxAmmoShells()
{
	if (m_iUpgradeLevel() == 1 || m_bMiniBuilding())
		return 150;
	else
		return 200;
}

void CObjectSentrygun::GetAmmoCount(int& iShells, int& iMaxShells, int& iRockets, int& iMaxRockets)
{
	const bool bIsMini = m_bMiniBuilding();

	iShells = m_iAmmoShells();
	iMaxShells = MaxAmmoShells();
	iRockets = bIsMini ? 0 : m_iAmmoRockets();
	iMaxRockets = (bIsMini || m_iUpgradeLevel() < 3) ? 0 : 20;
}