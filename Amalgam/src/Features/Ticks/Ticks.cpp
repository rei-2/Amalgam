#include "Ticks.h"

#include "../NetworkFix/NetworkFix.h"
#include "../PacketManip/AntiAim/AntiAim.h"
#include "../Aimbot/AutoRocketJump/AutoRocketJump.h"
#include "../Backtrack/Backtrack.h"

// this is different from crithack for some reason, im not changing this since it works anyways
static Color_t BlendColors(const Color_t& a, const Color_t& b, float ratio) 
{
    Color_t result;
    result.r = static_cast<byte>(a.r * (1.0f - ratio) + b.r * ratio);
    result.g = static_cast<byte>(a.g * (1.0f - ratio) + b.g * ratio);
    result.b = static_cast<byte>(a.b * (1.0f - ratio) + b.b * ratio);
    result.a = static_cast<byte>(a.a * (1.0f - ratio) + b.a * ratio);
    return result;
}

void CTickshiftHandler::Reset()
{
	m_bSpeedhack = m_bDoubletap = m_bRecharge = m_bWarp = false;
	m_iShiftedTicks = m_iShiftedGoal = 0;
}

void CTickshiftHandler::Recharge(CTFPlayer* pLocal)
{
	if (!m_bGoalReached)
		return;

	bool bPassive = m_bRecharge = false;

	static float flPassiveTime = 0.f;
	flPassiveTime = std::max(flPassiveTime - TICK_INTERVAL, -TICK_INTERVAL);
	if (Vars::Doubletap::PassiveRecharge.Value && 0.f >= flPassiveTime)
	{
		bPassive = true;
		flPassiveTime += 1.f / Vars::Doubletap::PassiveRecharge.Value;
	}

	if (m_iDeficit)
	{
		bPassive = true;
		m_iDeficit--, m_iShiftedTicks--;
	}

	if (!Vars::Doubletap::RechargeTicks.Value && !bPassive
		|| m_bDoubletap || m_bWarp || m_iShiftedTicks == m_iMaxShift || m_bSpeedhack)
		return;

	m_bRecharge = true;
	m_iShiftedGoal = m_iShiftedTicks + 1;
}

void CTickshiftHandler::Warp()
{
	if (!m_bGoalReached)
		return;

	m_bWarp = false;
	if (!Vars::Doubletap::Warp.Value
		|| !m_iShiftedTicks || m_bDoubletap || m_bRecharge || m_bSpeedhack)
		return;

	m_bWarp = true;
	m_iShiftedGoal = std::max(m_iShiftedTicks - Vars::Doubletap::WarpRate.Value + 1, 0);
}

void CTickshiftHandler::Doubletap(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	if (!m_bGoalReached)
		return;

	if (!Vars::Doubletap::Doubletap.Value
		|| m_iWait || m_bWarp || m_bRecharge || m_bSpeedhack)
		return;

	int iTicks = std::min(m_iShiftedTicks + 1, 22);
	auto pWeapon = H::Entities.GetWeapon();
	if (!(iTicks >= Vars::Doubletap::TickLimit.Value || pWeapon && GetShotsWithinPacket(pWeapon, iTicks) > 1))
		return;

	bool bAttacking = G::PrimaryWeaponType == EWeaponType::MELEE ? pCmd->buttons & IN_ATTACK : G::Attacking;
	if (!G::CanPrimaryAttack && !G::Reloading || !bAttacking && !m_bDoubletap || F::AutoRocketJump.IsRunning())
		return;

	m_bDoubletap = true;
	m_iShiftedGoal = std::max(m_iShiftedTicks - Vars::Doubletap::TickLimit.Value + 1, 0);
	if (Vars::Doubletap::AntiWarp.Value)
		m_bAntiWarp = pLocal->m_hGroundEntity();
}

void CTickshiftHandler::Speedhack()
{
	m_bSpeedhack = Vars::Speedhack::Enabled.Value;
	if (!m_bSpeedhack)
		return;

	m_bDoubletap = m_bWarp = m_bRecharge = false;
}

void CTickshiftHandler::SaveShootPos(CTFPlayer* pLocal)
{
	if (!m_bDoubletap && !m_bWarp)
		m_vShootPos = pLocal->GetShootPos();
}
Vec3 CTickshiftHandler::GetShootPos()
{
	return m_vShootPos;
}

void CTickshiftHandler::SaveShootAngle(CUserCmd* pCmd, bool bSendPacket)
{
	static auto sv_maxusrcmdprocessticks_holdaim = U::ConVars.FindVar("sv_maxusrcmdprocessticks_holdaim");

	if (bSendPacket)
		m_bShootAngle = false;
	else if (!m_bShootAngle && G::Attacking == 1 && sv_maxusrcmdprocessticks_holdaim->GetBool())
		m_vShootAngle = pCmd->viewangles, m_bShootAngle = true;
}
Vec3* CTickshiftHandler::GetShootAngle()
{
	if (m_bShootAngle && I::ClientState->chokedcommands)
		return &m_vShootAngle;
	return nullptr;
}

bool CTickshiftHandler::CanChoke()
{
	static auto sv_maxusrcmdprocessticks = U::ConVars.FindVar("sv_maxusrcmdprocessticks");
	int iMaxTicks = sv_maxusrcmdprocessticks->GetInt();
	if (Vars::Misc::Game::AntiCheatCompatibility.Value)
		iMaxTicks = std::min(iMaxTicks, 8);

	return I::ClientState->chokedcommands < 21 && m_iShiftedTicks + I::ClientState->chokedcommands < iMaxTicks;
}

void CTickshiftHandler::AntiWarp(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	static Vec3 vVelocity = {};
	static int iMaxTicks = 0;
	if (m_bAntiWarp)
	{
		int iTicks = GetTicks();
		iMaxTicks = std::max(iTicks + 1, iMaxTicks);

		Vec3 vAngles; Math::VectorAngles(vVelocity, vAngles);
		vAngles.y = pCmd->viewangles.y - vAngles.y;
		Vec3 vForward; Math::AngleVectors(vAngles, &vForward);
		vForward *= vVelocity.Length2D();

		if (iTicks > std::max(iMaxTicks - 8, 3))
			pCmd->forwardmove = -vForward.x, pCmd->sidemove = -vForward.y;
		else if (iTicks > 3)
		{
			pCmd->forwardmove = pCmd->sidemove = 0.f;
			pCmd->buttons &= ~(IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT);
		}
		else
			pCmd->forwardmove = vForward.x, pCmd->sidemove = vForward.y;
	}
	else
	{
		vVelocity = pLocal->m_vecVelocity();
		iMaxTicks = 0;
	}

	/*
	static bool bSet = false;

	if (!m_bAntiWarp)
	{
		bSet = false;
		return;
	}

	if (G::Attacking != 1 && !bSet)
	{
		bSet = true;
		SDK::StopMovement(pLocal, pCmd);
	}
	else
		pCmd->forwardmove = pCmd->sidemove = 0.f;
	*/
}

bool CTickshiftHandler::ValidWeapon(CTFWeaponBase* pWeapon)
{
	switch (pWeapon->GetWeaponID())
	{
	case TF_WEAPON_PDA:
	case TF_WEAPON_PDA_ENGINEER_BUILD:
	case TF_WEAPON_PDA_ENGINEER_DESTROY:
	case TF_WEAPON_PDA_SPY:
	case TF_WEAPON_PDA_SPY_BUILD:
	case TF_WEAPON_BUILDER:
	case TF_WEAPON_INVIS:
	case TF_WEAPON_GRAPPLINGHOOK:
	case TF_WEAPON_JAR_MILK:
	case TF_WEAPON_LUNCHBOX:
	case TF_WEAPON_BUFF_ITEM:
	case TF_WEAPON_ROCKETPACK:
	case TF_WEAPON_JAR_GAS:
	case TF_WEAPON_LASER_POINTER:
	case TF_WEAPON_MEDIGUN:
	case TF_WEAPON_SNIPERRIFLE:
	case TF_WEAPON_SNIPERRIFLE_DECAP:
	case TF_WEAPON_SNIPERRIFLE_CLASSIC:
	case TF_WEAPON_COMPOUND_BOW:
	case TF_WEAPON_JAR:
		return false;
	}

	return true;
}

void CTickshiftHandler::CLMoveFunc(float accumulated_extra_samples, bool bFinalTick)
{
	static auto CL_Move = U::Hooks.m_mHooks["CL_Move"];

	m_iShiftedTicks--;
	if (m_iShiftedTicks < 0)
		return;
	if (m_iWait > 0)
		m_iWait--;

	int iTicks = std::min(m_iShiftedTicks + 1, 22);
	auto pWeapon = H::Entities.GetWeapon();
	if (!(iTicks >= Vars::Doubletap::TickLimit.Value || pWeapon && GetShotsWithinPacket(pWeapon, iTicks) > 1))
		m_iWait = 1;

	m_bGoalReached = bFinalTick && m_iShiftedTicks == m_iShiftedGoal;

	if (CL_Move)
		CL_Move->Call<void>(accumulated_extra_samples, bFinalTick);
}

void CTickshiftHandler::CLMove(float accumulated_extra_samples, bool bFinalTick)
{
	if (auto pWeapon = H::Entities.GetWeapon())
	{
		switch (pWeapon->GetWeaponID())
		{
		case TF_WEAPON_PIPEBOMBLAUNCHER:
		case TF_WEAPON_CANNON:
			if (!G::CanSecondaryAttack)
				m_iWait = Vars::Doubletap::TickLimit.Value;
			break;
		default:
			if (!ValidWeapon(pWeapon))
				m_iWait = 2;
			else if (G::Attacking || !G::CanPrimaryAttack && !G::Reloading)
				m_iWait = Vars::Doubletap::TickLimit.Value;
		}
	}
	else
		m_iWait = 2;

	static auto sv_maxusrcmdprocessticks = U::ConVars.FindVar("sv_maxusrcmdprocessticks");
	m_iMaxShift = sv_maxusrcmdprocessticks->GetInt();
	if (Vars::Misc::Game::AntiCheatCompatibility.Value)
		m_iMaxShift = std::min(m_iMaxShift, 8);
	m_iMaxShift -= std::max(24 - std::clamp(Vars::Doubletap::RechargeLimit.Value, 1, 24), F::AntiAim.YawOn() ? F::AntiAim.AntiAimTicks() : 0);

	while (m_iShiftedTicks > m_iMaxShift)
		CLMoveFunc(accumulated_extra_samples, false); // skim any excess ticks

	m_iShiftedTicks++; // since we now have full control over CL_Move, increment.
	if (m_iShiftedTicks <= 0)
	{
		m_iShiftedTicks = 0;
		return;
	}

	if (m_bSpeedhack)
	{
		m_iShiftedTicks = Vars::Speedhack::Amount.Value;
		m_iShiftedGoal = 0;
	}

	m_iShiftedGoal = std::clamp(m_iShiftedGoal, 0, m_iMaxShift);
	if (m_iShiftedTicks > m_iShiftedGoal) // normal use/doubletap/teleport
	{
		m_bShifting = m_bShifted = m_iShiftedTicks - 1 != m_iShiftedGoal;
		m_iShiftStart = m_iShiftedTicks;

#ifndef TICKBASE_DEBUG
		while (m_iShiftedTicks > m_iShiftedGoal)
			CLMoveFunc(accumulated_extra_samples, m_iShiftedTicks - 1 == m_iShiftedGoal);
			//CLMoveFunc(accumulated_extra_samples, bFinalTick);
#else
		if (Vars::Debug::Info.Value)
			SDK::Output("Pre loop", "", { 0, 255, 255, 255 });
		while (m_iShiftedTicks > m_iShiftedGoal)
		{
			if (Vars::Debug::Info.Value)
				SDK::Output("Pre move", "", { 0, 127, 255, 255 });
			CLMoveFunc(accumulated_extra_samples, m_iShiftedTicks - 1 == m_iShiftedGoal);
			if (Vars::Debug::Info.Value)
				SDK::Output("Post move", "\n", { 0, 127, 255, 255 });
		}
		if (Vars::Debug::Info.Value)
			SDK::Output("Post loop", "\n", { 0, 0, 255, 255 });
#endif

		m_bShifting = m_bAntiWarp = false;
		if (m_bWarp)
			m_iDeficit = 0;

		m_bDoubletap = m_bWarp = false;
	}
	else // else recharge, run once if we have any choked ticks
	{
		if (I::ClientState->chokedcommands)
			CLMoveFunc(accumulated_extra_samples, bFinalTick);
	}
}

void CTickshiftHandler::CLMoveManage(CTFPlayer* pLocal)
{
	if (!pLocal)
		return;

	Recharge(pLocal);
	Warp();
	Speedhack();
}

void CTickshiftHandler::Run(float accumulated_extra_samples, bool bFinalTick, CTFPlayer* pLocal)
{
	F::NetworkFix.FixInputDelay(bFinalTick);

	CLMoveManage(pLocal);
	CLMove(accumulated_extra_samples, bFinalTick);
}

void CTickshiftHandler::CreateMove(CTFPlayer* pLocal, CUserCmd* pCmd, bool* pSendPacket)
{
	if (!pLocal)
		return;

	Doubletap(pLocal, pCmd);
	AntiWarp(pLocal, pCmd);
	ManagePacket(pCmd, pSendPacket);

	SaveShootPos(pLocal);
	SaveShootAngle(pCmd, *pSendPacket);
}

void CTickshiftHandler::ManagePacket(CUserCmd* pCmd, bool* pSendPacket)
{
	if (!m_bDoubletap && !m_bWarp && !m_bSpeedhack)
		return;

	if ((m_bSpeedhack || m_bWarp) && G::Attacking == 1)
	{
		*pSendPacket = true;
		return;
	}

	*pSendPacket = m_iShiftedGoal == m_iShiftedTicks;
	if (I::ClientState->chokedcommands >= 21) // prevent overchoking
		*pSendPacket = true;
}

int CTickshiftHandler::GetTicks(CTFWeaponBase* pWeapon)
{
	if (m_bDoubletap && m_iShiftedGoal < m_iShiftedTicks)
		return m_iShiftedTicks - m_iShiftedGoal;

	if (!Vars::Doubletap::Doubletap.Value
		|| m_iWait || m_bWarp || m_bRecharge || m_bSpeedhack || F::AutoRocketJump.IsRunning())
		return 0;

	int iTicks = std::min(m_iShiftedTicks + 1, 22);
	if (!(iTicks >= Vars::Doubletap::TickLimit.Value || pWeapon && GetShotsWithinPacket(pWeapon, iTicks) > 1))
		return 0;
	
	return std::min(Vars::Doubletap::TickLimit.Value - 1, m_iMaxShift);
}

int CTickshiftHandler::GetShotsWithinPacket(CTFWeaponBase* pWeapon, int iTicks)
{
	iTicks = std::min(m_iMaxShift + 1, iTicks);

	int iDelay = 1;
	switch (pWeapon->GetWeaponID())
	{
	case TF_WEAPON_MINIGUN:
	case TF_WEAPON_PIPEBOMBLAUNCHER:
	case TF_WEAPON_CANNON:
		iDelay = 2;
	}

	return 1 + (iTicks - iDelay) / std::ceilf(pWeapon->GetFireRate() / TICK_INTERVAL);
}

int CTickshiftHandler::GetMinimumTicksNeeded(CTFWeaponBase* pWeapon)
{
	int iDelay = 1;
	switch (pWeapon->GetWeaponID())
	{
	case TF_WEAPON_MINIGUN:
	case TF_WEAPON_PIPEBOMBLAUNCHER:
	case TF_WEAPON_CANNON:
		iDelay = 2;
	}

	return (GetShotsWithinPacket(pWeapon) - 1) * std::ceilf(pWeapon->GetFireRate() / TICK_INTERVAL) + iDelay;
}

void CTickshiftHandler::Draw(CTFPlayer* pLocal)
{
	if (!(Vars::Menu::Indicators.Value & Vars::Menu::IndicatorsEnum::Ticks) || !pLocal->IsAlive())
		return;

	const DragBox_t dtPos = Vars::Menu::TicksDisplay.Value;
	const auto& fFont = H::Fonts.GetFont(FONT_INDICATORS);
	const int iRounding = H::Draw.Scale(3);
	const int iBottomPadding = H::Draw.Scale(4, Scale_Round);
	const int iBarRounding = std::max(1, iRounding / 2);

	int w = H::Draw.Scale(150, Scale_Round);
	int h = H::Draw.Scale(24, Scale_Round) + iBottomPadding;
	int x = dtPos.x - w / 2;
	int y = dtPos.y;
	int barHeight = H::Draw.Scale(3, Scale_Round); 
	int barY = y + h - barHeight - iBottomPadding;
	int totalBarWidth = w - 2 * iRounding;

	H::Draw.FillRoundRect(x, y, w, h, iRounding, Vars::Menu::Theme::Background.Value);

	std::string statusText;
	Color_t textColor;
	Color_t barColor = Vars::Menu::Theme::Accent.Value;
	Color_t dimmedAccent = BlendColors(Vars::Menu::Theme::Accent.Value, Vars::Menu::Theme::Background.Value, 0.5f);
	Color_t barBackgroundColor = dimmedAccent;

	int iChoke = std::max(I::ClientState->chokedcommands - (F::AntiAim.YawOn() ? F::AntiAim.AntiAimTicks() : 0), 0);
	int iTicks = std::clamp(F::Ticks.m_iShiftedTicks + iChoke, 0, F::Ticks.m_iMaxShift);

	H::Draw.FillRoundRect(x + iRounding, barY, totalBarWidth, barHeight, iBarRounding, barBackgroundColor);

	static float flAnimatedRatio = 0.0f;
	float flTargetRatio = F::Ticks.m_iMaxShift > 0 ? float(iTicks) / F::Ticks.m_iMaxShift : 0.0f;
	flAnimatedRatio = flAnimatedRatio + (flTargetRatio - flAnimatedRatio) * std::min(I::GlobalVars->frametime * 11.3f, 1.0f);
	int barWidth = static_cast<int>(totalBarWidth * flAnimatedRatio);

	if (barWidth > 0)
		H::Draw.FillRoundRect(x + iRounding, barY, barWidth, barHeight, iBarRounding, barColor); 


	if (iTicks >= F::Ticks.m_iMaxShift) { // Use m_iMaxShift for comparison
		if (F::Ticks.m_iWait) {
			statusText = "Wait";
			textColor = Vars::Colors::IndicatorTextMid.Value;
		} else {
			statusText = "Ready";
			textColor = Vars::Colors::IndicatorTextGood.Value;
		}
	} else {
		statusText = "Not ready";
		textColor = Vars::Colors::IndicatorTextBad.Value;
	}

	statusText = std::format("{} ({}/{})", statusText, iTicks, F::Ticks.m_iMaxShift);


	Color_t borderColor = BlendColors(Vars::Menu::Theme::Background.Value, Color_t(255, 255, 255, 50), 0.1f);
	H::Draw.LineRoundRect(x, y, w, h, iRounding, borderColor);

	H::Draw.StringOutlined(
		fFont,
		x + H::Draw.Scale(4, Scale_Round),
		y + (h - barHeight - iBottomPadding) / 2,
		textColor,
		Vars::Menu::Theme::Background.Value.Alpha(150),
		ALIGN_LEFT,
		statusText.c_str()
	);
}