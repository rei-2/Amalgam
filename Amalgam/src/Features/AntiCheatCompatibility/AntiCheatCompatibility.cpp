#include "AntiCheatCompatibility.h"

#include "../Backtrack/Backtrack.h"
#include "../Misc/Misc.h"

void CAntiCheatCompatibility::CreateMove(CUserCmd* pCmd, bool* pSendPacket)
{
	if (!Active())
		return;

	Math::ClampAngles(pCmd->viewangles); // shouldn't happen, but failsafe

	m_vHistory.emplace_front(pCmd->viewangles, pCmd->buttons & IN_ATTACK, pCmd->buttons & IN_ATTACK2, *pSendPacket);
	if (m_vHistory.size() > 5)
		m_vHistory.pop_back();
	if (m_vHistory.size() < 3)
		return;

	// prevent trigger checks, though this shouldn't happen ordinarily
	if (!m_vHistory[0].m_bAttack1 && m_vHistory[1].m_bAttack1 && !m_vHistory[2].m_bAttack1)
		pCmd->buttons |= IN_ATTACK;
	if (!m_vHistory[0].m_bAttack2 && m_vHistory[1].m_bAttack2 && !m_vHistory[2].m_bAttack2)
		pCmd->buttons |= IN_ATTACK2;

	// don't care if we are actually attacking or not, a miss is less important than a detection
	if (m_vHistory[0].m_bAttack1 || m_vHistory[1].m_bAttack1 || m_vHistory[2].m_bAttack1)
	{
		// prevent silent aim checks
		if (Math::CalcFov(m_vHistory[0].m_vAngle, m_vHistory[1].m_vAngle) > PSILENT_EPSILON
			&& Math::CalcFov(m_vHistory[0].m_vAngle, m_vHistory[2].m_vAngle) < REAL_EPSILON)
		{
			pCmd->viewangles = m_vHistory[1].m_vAngle.LerpAngle(m_vHistory[0].m_vAngle, 0.5f);
			if (Math::CalcFov(pCmd->viewangles, m_vHistory[2].m_vAngle) < REAL_EPSILON)
				pCmd->viewangles = m_vHistory[0].m_vAngle + Vec3(0.f, REAL_EPSILON * 2);
			m_vHistory[0].m_vAngle = pCmd->viewangles;
			m_vHistory[0].m_bSendingPacket = *pSendPacket = m_vHistory[1].m_bSendingPacket;
		}

		// prevent aim snap checks
		if (m_vHistory.size() == 5)
		{
			float flDelta01 = Math::CalcFov(m_vHistory[0].m_vAngle, m_vHistory[1].m_vAngle);
			float flDelta12 = Math::CalcFov(m_vHistory[1].m_vAngle, m_vHistory[2].m_vAngle);
			float flDelta23 = Math::CalcFov(m_vHistory[2].m_vAngle, m_vHistory[3].m_vAngle);
			float flDelta34 = Math::CalcFov(m_vHistory[3].m_vAngle, m_vHistory[4].m_vAngle);

			if ((flDelta12 > SNAP_SIZE_EPSILON && flDelta23 < SNAP_NOISE_EPSILON && m_vHistory[2].m_vAngle != m_vHistory[3].m_vAngle
				|| flDelta23 > SNAP_SIZE_EPSILON && flDelta12 < SNAP_NOISE_EPSILON && m_vHistory[1].m_vAngle != m_vHistory[2].m_vAngle)
				&& flDelta01 < SNAP_NOISE_EPSILON && m_vHistory[0].m_vAngle != m_vHistory[1].m_vAngle
				&& flDelta34 < SNAP_NOISE_EPSILON && m_vHistory[3].m_vAngle != m_vHistory[4].m_vAngle)
			{
				pCmd->viewangles.y += SNAP_NOISE_EPSILON * 2;
				m_vHistory[0].m_vAngle = pCmd->viewangles;
				m_vHistory[0].m_bSendingPacket = *pSendPacket = m_vHistory[1].m_bSendingPacket;
			}
		}
	}
}

void CAntiCheatCompatibility::RespondCvarValue(INetMessage& msg)
{
	if (!Active())
		return;

	auto pMsg = reinterpret_cast<CLC_RespondCvarValue*>(&msg);
	if (!pMsg->m_szCvarName)
		return;

	auto pConVar = H::ConVars.FindVar(pMsg->m_szCvarName);
	if (!pConVar)
		return;

	switch (FNV1A::Hash32(pMsg->m_szCvarName))
	{
	case FNV1A::Hash32Const("cl_interp"):
		if (F::Backtrack.m_flSentInterp != -1.f)
			m_sValue = std::to_string(std::min(F::Backtrack.m_flSentInterp, 0.1f));
		else
			m_sValue = pConVar->GetString();
		break;
	case FNV1A::Hash32Const("cl_interp_ratio"):
		m_sValue = "1";
		break;
	case FNV1A::Hash32Const("cl_cmdrate"):
		if (F::Misc.m_iWishCmdrate != -1)
			m_sValue = std::to_string(F::Misc.m_iWishCmdrate);
		else
			m_sValue = pConVar->GetString();
		break;
	case FNV1A::Hash32Const("cl_updaterate"):
		if (F::Misc.m_iWishUpdaterate != -1)
			m_sValue = std::to_string(F::Misc.m_iWishUpdaterate);
		else
			m_sValue = pConVar->GetString();
		break;
	case FNV1A::Hash32Const("mat_dxlevel"):
		m_sValue = pConVar->GetString();
		break;
	default:
		m_sValue = pConVar->m_pParent->m_pszDefaultValue;
	}
	pMsg->m_szCvarValue = m_sValue.c_str();

	SDK::Output("Convar spoof", std::format("{}: {} ({})", pMsg->m_szCvarName, pMsg->m_szCvarValue, msg.ToString()).c_str(), Vars::Menu::Theme::Accent.Value, Vars::Debug::Logging.Value);
}

void CAntiCheatCompatibility::BunnyHop(CUserCmd* pCmd, bool bCurGrounded, bool bLastGrounded)
{
	if (!Active())
		return;

	// prevent more than 9 bhops occurring. if a server has this under that threshold they're retarded anyways
	if (bCurGrounded)
	{
		if (!bLastGrounded && pCmd->buttons & IN_JUMP)
			m_iJumps++;
		else
			m_iJumps = 0;

		if (m_iJumps > 9)
			pCmd->buttons &= ~IN_JUMP;
	}
}