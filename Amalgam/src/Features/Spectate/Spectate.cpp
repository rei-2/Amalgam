#include "Spectate.h"

void CSpectate::NetUpdateEnd(CTFPlayer* pLocal)
{
	if (!pLocal)
		return;

	m_iTarget = m_iIntendedTarget;
	CTFPlayer* pEntity = nullptr;
	if (m_iTarget != -1)
	{
		pEntity = I::ClientEntityList->GetClientEntity(I::EngineClient->GetPlayerForUserID(m_iTarget))->As<CTFPlayer>();
		if (pEntity == pLocal)
			m_iTarget = m_iIntendedTarget = -1;
	}
	if (m_iTarget == -1)
	{
		if (pLocal->IsAlive() && pLocal->m_hObserverTarget())
		{
			pLocal->m_vecViewOffset() = pLocal->GetViewOffset();
			pLocal->m_iObserverMode() = OBS_MODE_NONE;
			pLocal->m_hObserverTarget().Set(nullptr);
		}
		return;
	}

	m_pOriginalTarget = pLocal->m_hObserverTarget().Get();
	m_iOriginalMode = pLocal->m_iObserverMode();
	if (!pEntity)
		return;

	switch (pEntity->m_hObserverTarget() ? pEntity->m_iObserverMode() : OBS_MODE_NONE)
	{
	case OBS_MODE_FIRSTPERSON:
	case OBS_MODE_THIRDPERSON:
		pLocal->m_hObserverTarget().Set(pEntity->m_hObserverTarget());
		break;
	default:
		pLocal->m_hObserverTarget().Set(pEntity);
	}
	pLocal->m_iObserverMode() = Vars::Visuals::Thirdperson::Enabled.Value ? OBS_MODE_THIRDPERSON : OBS_MODE_FIRSTPERSON;
	pLocal->m_vecViewOffset() = pEntity->GetViewOffset();
	Vars::Visuals::Thirdperson::Enabled.Value ? I::Input->CAM_ToThirdPerson() : I::Input->CAM_ToFirstPerson();

	m_pTargetTarget = pLocal->m_hObserverTarget().Get();
	m_iTargetMode = pLocal->m_iObserverMode();
}

void CSpectate::NetUpdateStart(CTFPlayer* pLocal)
{
	if (!pLocal || m_iTarget == -1)
		return;

	pLocal->m_hObserverTarget().Set(m_pOriginalTarget);
	pLocal->m_iObserverMode() = m_iOriginalMode;
}

void CSpectate::CreateMove(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	int iButtons = pCmd->buttons & ~IN_SCORE;
	if (iButtons)
		m_iIntendedTarget = -1;

	static bool bStaticView = false;
	const bool bLastView = bStaticView;
	const bool bCurrView = bStaticView = m_iTarget != -1;
	if (!bCurrView)
	{
		if (bLastView)
			I::EngineClient->SetViewAngles(m_vOldView);
		m_vOldView = pCmd->viewangles;
	}
	else
		pCmd->viewangles = m_vOldView;
}

void CSpectate::SetTarget(int iTarget)
{
	m_iIntendedTarget = m_iIntendedTarget == iTarget ? -1 : iTarget;
}