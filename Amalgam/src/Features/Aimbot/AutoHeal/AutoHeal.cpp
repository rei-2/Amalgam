#include "AutoHeal.h"

#include "../../Players/PlayerUtils.h"

bool CAutoHeal::ActivateOnVoice(CTFPlayer* pLocal, CWeaponMedigun* pWeapon, CUserCmd* pCmd)
{
	if (!Vars::Aimbot::Healing::ActivateOnVoice.Value)
		return false;

	auto pTarget = pWeapon->m_hHealingTarget().Get();
	if (!pTarget || Vars::Aimbot::Healing::FriendsOnly.Value && !H::Entities.IsFriend(pTarget->entindex()) && !H::Entities.InParty(pTarget->entindex()))
		return false;

	bool bReturn = m_mMedicCallers.contains(pTarget->entindex());
	if (bReturn)
		pCmd->buttons |= IN_ATTACK2;

	return bReturn;
}

void CAutoHeal::Run(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd)
{
	bool bActivated = ActivateOnVoice(pLocal, pWeapon->As<CWeaponMedigun>(), pCmd);
	m_mMedicCallers.clear();
	if (bActivated)
		return;
}