#include "CTFPlayer.h"

#include "../../SDK.h"

Vec3 CTFPlayer::GetEyeAngles()
{
	return { m_angEyeAnglesX(), m_angEyeAnglesY(), 0.f };
}

Vec3 CTFPlayer::GetViewOffset(bool bScale)
{
	Vec3 vOffset = GetOffset() / 2;

	if (!IsPlayer())
		return vOffset;

	if (IsDucking())
		vOffset = { 0.f, 0.f, 45.f };
	else switch (m_iClass())
		{
		case TF_CLASS_SCOUT: vOffset = { 0.f, 0.f, 65.f }; break;
		case TF_CLASS_SOLDIER: vOffset = { 0.f, 0.f, 68.f }; break;
		case TF_CLASS_PYRO: vOffset = { 0.f, 0.f, 68.f }; break;
		case TF_CLASS_DEMOMAN: vOffset = { 0.f, 0.f, 68.f }; break;
		case TF_CLASS_HEAVY: vOffset = { 0.f, 0.f, 75.f }; break;
		case TF_CLASS_ENGINEER: vOffset = { 0.f, 0.f, 68.f }; break;
		case TF_CLASS_MEDIC: vOffset = { 0.f, 0.f, 75.f }; break;
		case TF_CLASS_SNIPER: vOffset = { 0.f, 0.f, 75.f }; break;
		case TF_CLASS_SPY: vOffset = { 0.f, 0.f, 75.f }; break;
		default: vOffset = m_vecViewOffset().z ? m_vecViewOffset() : Vec3(0.f, 0.f, 72.f);
		}

	return bScale ? vOffset * m_flModelScale() : vOffset;
}

bool CTFPlayer::InCond(ETFCond eCond)
{
	switch (eCond / 32)
	{
	case 0: return m_nPlayerCond() & (1 << eCond) || _condition_bits() & (1 << eCond);
	case 1: return m_nPlayerCondEx() & (1 << (eCond - 32));
	case 2: return m_nPlayerCondEx2() & (1 << (eCond - 64));
	case 3: return m_nPlayerCondEx3() & (1 << (eCond - 96));
	case 4: return m_nPlayerCondEx4() & (1 << (eCond - 128));
	}
	return false;
}

void CTFPlayer::AddCond(ETFCond eCond)
{
	switch (eCond / 32)
	{
	case 0: m_nPlayerCond() |= (1 << eCond), _condition_bits() |= (1 << eCond); break;
	case 1: m_nPlayerCondEx() |= (1 << (eCond - 32)); break;
	case 2: m_nPlayerCondEx2() |= (1 << (eCond - 64)); break;
	case 3: m_nPlayerCondEx3() |= (1 << (eCond - 96)); break;
	case 4: m_nPlayerCondEx4() |= (1 << (eCond - 128)); break;
	}
}

void CTFPlayer::RemoveCond(ETFCond eCond)
{
	switch (eCond / 32)
	{
	case 0: m_nPlayerCond() &= ~(1 << eCond), _condition_bits() &= ~(1 << eCond); break;
	case 1: m_nPlayerCondEx() &= ~(1 << (eCond - 32)); break;
	case 2: m_nPlayerCondEx2() &= ~(1 << (eCond - 64)); break;
	case 3: m_nPlayerCondEx3() &= ~(1 << (eCond - 96)); break;
	case 4: m_nPlayerCondEx4() &= ~(1 << (eCond - 128)); break;
	}
}

bool CTFPlayer::IsAGhost()
{
	return InCond(TF_COND_HALLOWEEN_GHOST_MODE);
};
bool CTFPlayer::IsTaunting()
{
	return InCond(TF_COND_TAUNTING);
};

bool CTFPlayer::IsInvisible(float flValue)
{
	if (InCond(TF_COND_BURNING)
		|| InCond(TF_COND_BURNING_PYRO)
		|| InCond(TF_COND_MAD_MILK)
		|| InCond(TF_COND_URINE))
		return false;

	float flInvis = GetEffectiveInvisibilityLevel();
	return flInvis && flInvis >= 1.f;
}

bool CTFPlayer::IsInvulnerable()
{
	return InCond(TF_COND_INVULNERABLE)
		|| InCond(TF_COND_INVULNERABLE_CARD_EFFECT)
		|| InCond(TF_COND_INVULNERABLE_HIDE_UNLESS_DAMAGED)
		|| InCond(TF_COND_INVULNERABLE_USER_BUFF)
		|| InCond(TF_COND_PHASE);
}

bool CTFPlayer::IsUbered()
{
	return InCond(TF_COND_INVULNERABLE)
		|| InCond(TF_COND_INVULNERABLE_CARD_EFFECT)
		|| InCond(TF_COND_INVULNERABLE_HIDE_UNLESS_DAMAGED)
		|| InCond(TF_COND_INVULNERABLE_USER_BUFF);
}

bool CTFPlayer::IsCritBoosted()
{
	if (InCond(TF_COND_CRITBOOSTED)
		|| InCond(TF_COND_CRITBOOSTED_PUMPKIN)
		|| InCond(TF_COND_CRITBOOSTED_USER_BUFF)
		|| InCond(TF_COND_CRITBOOSTED_FIRST_BLOOD)
		|| InCond(TF_COND_CRITBOOSTED_BONUS_TIME)
		|| InCond(TF_COND_CRITBOOSTED_CTF_CAPTURE)
		|| InCond(TF_COND_CRITBOOSTED_ON_KILL)
		|| InCond(TF_COND_CRITBOOSTED_CARD_EFFECT)
		|| InCond(TF_COND_CRITBOOSTED_RUNE_TEMP))
		return true;

	if (auto pWeapon = m_hActiveWeapon()->As<CTFWeaponBase>())
	{
		if (auto pWeaponInfo = pWeapon->m_pWeaponInfo())
		{
			if (InCond(TF_COND_CRITBOOSTED_RAGE_BUFF) && pWeaponInfo->m_iWeaponType == TF_WPN_TYPE_PRIMARY)
				return true;
		}
	}

	return false;
}

bool CTFPlayer::IsMiniCritBoosted()
{
	return InCond(TF_COND_OFFENSEBUFF)
		|| InCond(TF_COND_ENERGY_BUFF)
		|| InCond(TF_COND_NOHEALINGDAMAGEBUFF)
		/*|| InCond(TF_COND_CRITBOOSTED_DEMO_CHARGE)*/;
}

bool CTFPlayer::IsMarked()
{
	return InCond(TF_COND_URINE)
		|| InCond(TF_COND_MARKEDFORDEATH)
		|| InCond(TF_COND_MARKEDFORDEATH_SILENT)
		|| InCond(TF_COND_PASSTIME_PENALTY_DEBUFF);
}

bool CTFPlayer::CanAttack(bool bCloak, bool bLocal)
{
	if (!IsAlive() || IsAGhost() || IsTaunting() || m_bViewingCYOAPDA()
		|| InCond(TF_COND_PHASE) || InCond(TF_COND_HALLOWEEN_KART) || InCond(TF_COND_STUNNED) && m_iStunFlags() & (TF_STUN_CONTROLS | TF_STUN_LOSER_STATE))
		return false;

	if (bCloak)
	{
		if (bLocal
			? (m_flStealthNoAttackExpire() > TICKS_TO_TIME(m_nTickBase()) && !InCond(TF_COND_STEALTHED_USER_BUFF)) || InCond(TF_COND_STEALTHED)
			: m_flInvisibility() && (InCond(TF_COND_STEALTHED) || !InCond(TF_COND_STEALTHED_USER_BUFF) && !InCond(TF_COND_STEALTHED_USER_BUFF_FADING)))
		{
			auto pWeapon = m_hActiveWeapon()->As<CTFWeaponBase>();
			if (!pWeapon || pWeapon->GetWeaponID() != TF_WEAPON_GRAPPLINGHOOK)
				return false;
		}

		if (m_bFeignDeathReady())
			return false;
	}

	auto pGameRules = I::TFGameRules();
	if (pGameRules)
	{
		switch (pGameRules->m_iRoundState())
		{
		case GR_STATE_TEAM_WIN:
			if (m_iTeamNum() != pGameRules->m_iWinningTeam())
				return false;
			break;
		case GR_STATE_BETWEEN_RNDS:
			if (m_fFlags() & FL_FROZEN)
				return false;
			break;
		case GR_STATE_GAME_OVER:
			if (m_fFlags() & FL_FROZEN || m_iTeamNum() != pGameRules->m_iWinningTeam())
				return false;
			break;
		}
	}

	if (SDK::AttribHookValue(0, "no_attack", this))
		return false;

	return true;
}

float CTFPlayer::GetCritMult()
{
	return Math::RemapVal(m_iCritMult(), 0.f, 255.f, 1.f, 4.f);
}