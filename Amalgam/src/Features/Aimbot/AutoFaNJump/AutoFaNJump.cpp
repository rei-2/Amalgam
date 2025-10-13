#include "AutoFaNJump.h"

bool CAutoFaNJump::SetAngles(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd)
{
	m_vAngles = pCmd->viewangles;
	if (!Vars::Misc::Movement::AutoFaNJump.Value)
		return true;

	Vec3 vOrigin = pLocal->m_vecOrigin();
	Vec3 vLocalPos = pLocal->GetShootPos();
	
	m_vAngles.x = 25.0f; // i think 25 is good, but it can be 30-ish
	m_vAngles.y = pCmd->viewangles.y;
	m_vAngles.z = 0.0f;

	return true;
}

void CAutoFaNJump::Run(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd)
{
	if (!pWeapon || !pLocal->IsAlive() || pLocal->IsAGhost() || I::EngineVGui->IsGameUIVisible())
	{
		m_iFrame = -1;
		return;
	}

	static bool bStaticGrounded = false;
	bool bLastGrounded = bStaticGrounded;
	bool bCurrGrounded = bStaticGrounded = pLocal->m_hGroundEntity();
	
	if (m_iFrame == -1)
	{
		bool bDuck = pLocal->IsDucking();
		if ((bCurrGrounded ? bDuck || I::GlobalVars->curtime < pLocal->m_flDuckTimer() : !bDuck))
			return;
	}

	bool bValidWeapon = false;
	{
		if (pWeapon->m_iItemDefinitionIndex() == Scout_m_ForceANature || 
			pWeapon->m_iItemDefinitionIndex() == Scout_m_FestiveForceANature)
		{
			bValidWeapon = true;
		}
	}
	
	if (!bValidWeapon)
	{
		m_iFrame = -1;
		return;
	}
	else if (Vars::Misc::Movement::AutoFaNJump.Value)
		pCmd->buttons &= ~IN_ATTACK2;

	static bool bPrevActive = false;
	bool bActive = Vars::Misc::Movement::AutoFaNJump.Value;
	if (m_iFrame == -1 && bActive && !bPrevActive && bCurrGrounded && G::CanPrimaryAttack)
	{
		if (bCurrGrounded && bCurrGrounded == bLastGrounded)
		{
			m_iFrame = 0;
			m_bFull = true;
		}
	}
	bPrevActive = bActive;

	if (m_iFrame != -1)
	{
		m_iFrame++;
		
		switch (m_iFrame)
		{
		case 1:
			// todo: actual move forward (lazy)
			pCmd->forwardmove = 4.0f;
			pCmd->buttons |= IN_JUMP;
			break;

		case 2:
			pCmd->forwardmove = 4.0f;
			if (SetAngles(pLocal, pWeapon, pCmd) && G::CanPrimaryAttack)
			{
				G::SilentAngles = true;
				pCmd->viewangles = m_vAngles;
				pCmd->buttons |= IN_ATTACK;
			}
			break;

		default:
			m_iFrame = -1;
			m_bFull = false;
		}

		if (m_iFrame >= 3)
		{
			m_iFrame = -1;
			m_bFull = false;
		}
	}

	m_bRunning = m_iFrame != -1;
}