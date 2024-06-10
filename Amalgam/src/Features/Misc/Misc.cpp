#include "Misc.h"

#include "../Backtrack/Backtrack.h"
#include "../CheaterDetection/CheaterDetection.h"
#include "../PacketManip/AntiAim/AntiAim.h"
#include "../TickHandler/TickHandler.h"
#include "../Players/PlayerUtils.h"

void CMisc::RunPre(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	CheatsBypass();
	PingReducer();
	WeaponSway();

	if (!pLocal)
		return;

	AntiAFK(pLocal, pCmd);
	InstantRespawnMVM(pLocal);

	if (!pLocal->IsAlive() || pLocal->IsAGhost() || pLocal->m_MoveType() != MOVETYPE_WALK || pLocal->IsSwimming() || pLocal->IsCharging() || pLocal->IsInBumperKart())
		return;

	AutoJump(pLocal, pCmd);
	AutoJumpbug(pLocal, pCmd);
	AutoStrafe(pLocal, pCmd);
	AntiBackstab(pLocal, pCmd);
	AutoPeek(pLocal, pCmd);
}

void CMisc::RunPost(CTFPlayer* pLocal, CUserCmd* pCmd, bool pSendPacket)
{
	if (!pLocal || !pLocal->IsAlive() || pLocal->IsAGhost() || pLocal->m_MoveType() != MOVETYPE_WALK || pLocal->IsSwimming() || pLocal->IsCharging())
		return;

	TauntKartControl(pLocal, pCmd);
	FastMovement(pLocal, pCmd);
	AntiWarp(pLocal, pCmd);
	LegJitter(pLocal, pCmd, pSendPacket);
}



void CMisc::AutoJump(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	if (!Vars::Misc::Movement::Bunnyhop.Value)
		return;

	static bool bStaticJump = false, bStaticGrounded = false, bLastAttempted = false;
	const bool bLastJump = bStaticJump, bLastGrounded = bStaticGrounded;
	const bool bCurJump = bStaticJump = pCmd->buttons & IN_JUMP, bCurGrounded = bStaticGrounded = pLocal->OnSolid();

	if (bCurJump && bLastJump)
	{
		if (!(bCurGrounded && !bLastGrounded))
			pCmd->buttons &= ~IN_JUMP;

		if (!(pCmd->buttons & IN_JUMP) && bCurGrounded && !bLastAttempted)
			pCmd->buttons |= IN_JUMP;
	}

	bLastAttempted = pCmd->buttons & IN_JUMP;
}

void CMisc::AutoJumpbug(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	if (!Vars::Misc::Movement::AutoJumpbug.Value || !(pCmd->buttons & IN_DUCK) || pLocal->OnSolid() || pLocal->m_vecVelocity().z > -650.f)
		return;

	CGameTrace trace;
	CTraceFilterWorldAndPropsOnly filter = {};
	filter.pSkip = pLocal;

	Vec3 origin = pLocal->m_vecOrigin();
	SDK::TraceHull(origin, origin - Vec3(0, 0, 22), pLocal->m_vecMins(), pLocal->m_vecMaxs(), MASK_PLAYERSOLID, &filter, &trace);
	if (!trace.DidHit()) // don't try if we aren't in range to unduck
		return;

	const float flDist = origin.DistTo(trace.endpos);
	if (20.f < flDist /*&& flDist < 22.f*/) // this seems to be the range where this works
	{
		pCmd->buttons &= ~IN_DUCK;
		pCmd->buttons |= IN_JUMP;
	}
}

void CMisc::AutoStrafe(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	if (!Vars::Misc::Movement::AutoStrafe.Value || pLocal->OnSolid() || !(pLocal->m_afButtonLast() & IN_JUMP) && (pCmd->buttons & IN_JUMP))
		return;

	switch (Vars::Misc::Movement::AutoStrafe.Value)
	{
	case 1:
	{
		static auto cl_sidespeed = U::ConVars.FindVar("cl_sidespeed");
		const float flSideSpeed = cl_sidespeed ? cl_sidespeed->GetFloat() : 450.f;

		if (pCmd->mousedx)
		{
			pCmd->forwardmove = 0.f;
			pCmd->sidemove = pCmd->mousedx > 0 ? flSideSpeed : -flSideSpeed;
		}
		break;
	}
	case 2:
	{
		//credits: KGB
		if (!(pCmd->buttons & (IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT)))
			break;

		float flForwardMove = pCmd->forwardmove;
		float flSideMove = pCmd->sidemove;

		Vec3 vForward = {}, vRight = {};
		Math::AngleVectors(pCmd->viewangles, &vForward, &vRight, nullptr);

		vForward.z = vRight.z = 0.f;

		vForward.Normalize();
		vRight.Normalize();

		Vec3 vWishDir = {};
		Math::VectorAngles({ (vForward.x * flForwardMove) + (vRight.x * flSideMove), (vForward.y * flForwardMove) + (vRight.y * flSideMove), 0.f }, vWishDir);

		Vec3 vCurDir = {};
		Math::VectorAngles(pLocal->m_vecVelocity(), vCurDir);

		float flDirDelta = Math::NormalizeAngle(vWishDir.y - vCurDir.y);
		float flTurnScale = Math::RemapValClamped(Vars::Misc::Movement::AutoStrafeTurnScale.Value, 0.f, 1.f, 0.9f, 1.f);
		float flRotation = DEG2RAD((flDirDelta > 0.f ? -90.f : 90.f) + (flDirDelta * flTurnScale));

		float flCosRot = cosf(flRotation);
		float flSinRot = sinf(flRotation);

		pCmd->forwardmove = (flCosRot * flForwardMove) - (flSinRot * flSideMove);
		pCmd->sidemove = (flSinRot * flForwardMove) + (flCosRot * flSideMove);
	}
	}
}

void CMisc::AntiBackstab(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	G::Busy = false;
	if (!Vars::Misc::Automation::AntiBackstab.Value || G::IsAttacking || pLocal->IsInBumperKart() || !Vars::Misc::Automation::AntiBackstab.Value)
		return;

	std::vector<Vec3> vTargets = {};
	for (auto pEntity : H::Entities.GetGroup(EGroupType::PLAYERS_ENEMIES))
	{
		auto pPlayer = pEntity->As<CTFPlayer>();
		if (!pPlayer->IsAlive() || pPlayer->IsAGhost() || pPlayer->IsCloaked() || pPlayer->m_bFeignDeathReady())
			continue;

		if (auto pWeapon = pPlayer->m_hActiveWeapon().Get()->As<CTFWeaponBase>())
		{
			if (pWeapon->m_iWeaponID() != TF_WEAPON_KNIFE)
				continue;
		}

		PlayerInfo_t pi{};
		if (I::EngineClient->GetPlayerInfo(pPlayer->entindex(), &pi) && F::PlayerUtils.IsIgnored(pi.friendsID))
			continue;

		Vec3 vTargetPos = pPlayer->m_vecOrigin() + pPlayer->m_vecVelocity() * F::Backtrack.GetReal();
		if (pLocal->m_vecOrigin().DistTo(vTargetPos) > 200.f || !SDK::VisPos(pLocal, pPlayer, pLocal->m_vecOrigin(), vTargetPos))
			continue;

		vTargets.push_back(vTargetPos);
	}

	std::sort(vTargets.begin(), vTargets.end(), [&](const auto& a, const auto& b) -> bool
		{
			return pLocal->m_vecOrigin().DistTo(a) < pLocal->m_vecOrigin().DistTo(b);
		});

	auto vTargetPos = vTargets.begin();
	if (vTargetPos != vTargets.end())
	{
		const Vec3 vAngleTo = Math::CalcAngle(pLocal->m_vecOrigin(), *vTargetPos);
		G::Busy = true;
		SDK::FixMovement(pCmd, vAngleTo);
		pCmd->viewangles.y = vAngleTo.y;
	}
}

void CMisc::AutoPeek(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	static bool bPosPlaced = false;
	static bool bReturning = false;

	if (Vars::CL_Move::AutoPeek.Value)
	{
		const Vec3 localPos = pLocal->GetAbsOrigin();

		// We just started peeking. Save the return position!
		if (!bPosPlaced)
		{
			if (pLocal->OnSolid())
			{
				vPeekReturnPos = localPos;
				bPosPlaced = true;
			}
		}
		else
		{
			static Timer particleTimer{};
			if (particleTimer.Run(700))
				H::Particles.DispatchParticleEffect("ping_circle", vPeekReturnPos, {});
		}

		// We've just attacked. Let's return!
		if (G::LastUserCmd->buttons & IN_ATTACK || G::IsAttacking)
			bReturning = true;

		if (bReturning)
		{
			if (localPos.DistTo(vPeekReturnPos) < 7.f)
			{
				bReturning = false;
				return;
			}

			SDK::WalkTo(pCmd, pLocal, vPeekReturnPos);
		}
	}
	else
	{
		bPosPlaced = bReturning = false;
		vPeekReturnPos = Vec3();
	}
}

void CMisc::AntiAFK(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	static Timer afkTimer{};

	static auto mp_idledealmethod = U::ConVars.FindVar("mp_idledealmethod");
	static auto mp_idlemaxtime = U::ConVars.FindVar("mp_idlemaxtime");
	const int iIdleMethod = mp_idledealmethod ? mp_idledealmethod->GetInt() : 1;
	const float flMaxIdleTime = mp_idlemaxtime ? mp_idlemaxtime->GetFloat() : 3.f;

	if (pCmd->buttons & (IN_MOVELEFT | IN_MOVERIGHT | IN_FORWARD | IN_BACK) || !pLocal->IsAlive())
		afkTimer.Update();
	// Trigger 10 seconds before kick
	else if (Vars::Misc::Automation::AntiAFK.Value && iIdleMethod && afkTimer.Check(flMaxIdleTime * 60 * 1000 - 10000))
		pCmd->buttons |= pCmd->command_number % 2 ? IN_FORWARD : IN_BACK;
}

void CMisc::InstantRespawnMVM(CTFPlayer* pLocal)
{
	if (Vars::Misc::MannVsMachine::InstantRespawn.Value && I::EngineClient->IsInGame() && !pLocal->IsAlive())
	{
		auto kv = new KeyValues("MVM_Revive_Response");
		kv->SetInt("accepted", 1);
		I::EngineClient->ServerCmdKeyValues(kv);
	}
}

void CMisc::CheatsBypass()
{
	static bool bCheatSet = false;
	static auto sv_cheats = U::ConVars.FindVar("sv_cheats");
	if (sv_cheats)
	{
		if (Vars::Misc::Exploits::CheatsBypass.Value)
		{
			sv_cheats->m_nValue = 1;
			bCheatSet = true;
		}
		else if (bCheatSet)
		{
			sv_cheats->m_nValue = 0;
			bCheatSet = false;
		}
	}
}

void CMisc::PingReducer()
{
	auto pNetChan = reinterpret_cast<CNetChannel*>(I::EngineClient->GetNetChannelInfo());
	if (!pNetChan)
		return;

	static auto cl_cmdrate = U::ConVars.FindVar("cl_cmdrate");
	const int iCmdRate = cl_cmdrate ? cl_cmdrate->GetInt() : 66;

	static Timer updateRateTimer{};
	if (updateRateTimer.Run(100))
	{
		const int iTarget = Vars::Misc::Exploits::PingReducer.Value ? Vars::Misc::Exploits::PingTarget.Value : iCmdRate;
		if (iTarget == iLastCmdrate)
			return;
		iLastCmdrate = iTarget;

		SDK::Output("SendNetMsg", std::format("cl_cmdrate: {}", iTarget).c_str(), { 224, 255, 131, 255 }, Vars::Debug::Logging.Value);

		NET_SetConVar cmd("cl_cmdrate", std::to_string(iTarget).c_str());
		pNetChan->SendNetMsg(cmd);
	}
}

void CMisc::WeaponSway()
{
	static auto cl_wpn_sway_interp = U::ConVars.FindVar("cl_wpn_sway_interp");
	static auto cl_wpn_sway_scale = U::ConVars.FindVar("cl_wpn_sway_scale");
	if (cl_wpn_sway_interp)
		cl_wpn_sway_interp->SetValue(Vars::Visuals::Viewmodel::Sway.Value ? Vars::Visuals::Viewmodel::SwayInterp.Value : 0.f);
	if (cl_wpn_sway_scale)
		cl_wpn_sway_scale->SetValue(Vars::Visuals::Viewmodel::Sway.Value ? Vars::Visuals::Viewmodel::SwayScale.Value : 0.f);
}



void CMisc::TauntKartControl(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	// Handle Taunt Slide
	if (Vars::Misc::Automation::TauntControl.Value && pLocal->IsTaunting())
	{
		if (pCmd->buttons & IN_FORWARD)
		{
			pCmd->forwardmove = 450.f;
			pCmd->viewangles.x = 0.f;
		}
		if (pCmd->buttons & IN_BACK)
		{
			pCmd->forwardmove = 450.f;
			pCmd->viewangles.x = 91.f;
		}
		if (pCmd->buttons & IN_MOVELEFT)
			pCmd->sidemove = -450.f;
		if (pCmd->buttons & IN_MOVERIGHT)
			pCmd->sidemove = 450.f;

		if (!(pCmd->buttons & (IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT)))
			pCmd->viewangles.x = 90.f;

		Vec3 vAngle = I::EngineClient->GetViewAngles();
		pCmd->viewangles.y = vAngle.y;

		G::SilentAngles = true;
	}
	else if (Vars::Misc::Automation::KartControl.Value && pLocal->IsInBumperKart())
	{
		const bool bForward = pCmd->buttons & IN_FORWARD;
		const bool bBack = pCmd->buttons & IN_BACK;
		const bool bLeft = pCmd->buttons & IN_MOVELEFT;
		const bool bRight = pCmd->buttons & IN_MOVERIGHT;

		const bool flipVar = pCmd->command_number % 2;
		if (bForward && (!bLeft && !bRight || !flipVar))
		{
			pCmd->forwardmove = 450.f;
			pCmd->viewangles.x = 0.f;
		}
		else if (bBack && (!bLeft && !bRight || !flipVar))
		{
			pCmd->forwardmove = 450.f;
			pCmd->viewangles.x = 91.f;
		}
		else if (pCmd->buttons & (IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT))
		{
			if (flipVar)
			{	// you could just do this if you didn't care about viewangles
				const Vec3 vecMove(pCmd->forwardmove, pCmd->sidemove, 0.f);
				const float flLength = vecMove.Length();
				Vec3 angMoveReverse;
				Math::VectorAngles(vecMove * -1.f, angMoveReverse);
				pCmd->forwardmove = -flLength;
				pCmd->sidemove = 0.f;
				pCmd->viewangles.y = fmodf(pCmd->viewangles.y - angMoveReverse.y, 360.f);
				pCmd->viewangles.z = 270.f;
				G::PSilentAngles = true;
			}
		}
		else
			pCmd->viewangles.x = 90.f;

		G::SilentAngles = true;
	}
}

void CMisc::FastMovement(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	if (!pLocal->OnSolid() || pLocal->IsInBumperKart())
		return;

	const float flSpeed = pLocal->m_vecVelocity().Length2D();
	const int flMaxSpeed = std::min(pLocal->m_flMaxspeed() * 0.9f, 520.f) - 10.f;
	const int iRun = !pCmd->forwardmove && !pCmd->sidemove ? 0 : flSpeed < flMaxSpeed ? 1 : 2;

	switch (iRun)
	{
	case 0:
	{
		if (!Vars::Misc::Movement::FastStop.Value || !flSpeed)
			return;

		if (G::ShiftedTicks != G::MaxShift && !G::IsAttacking && !G::AntiAim)
		{
			if (!SDK::StopMovement(pLocal, pCmd))
				return;

			if (!G::Recharge && !G::DoubleTap)
				G::PSilentAngles = true;
			else
				G::SilentAngles = true;
		}
		else
		{
			Vec3 direction = pLocal->m_vecVelocity().toAngle();
			direction.y = pCmd->viewangles.y - direction.y;
			const Vec3 negatedDirection = direction.fromAngle() * -flSpeed;
			pCmd->forwardmove = negatedDirection.x;
			pCmd->sidemove = negatedDirection.y;
		}

		break;
	}
	case 1:
	{
		if ((pLocal->IsDucking() ? !Vars::Misc::Movement::CrouchSpeed.Value : !Vars::Misc::Movement::FastAccel.Value) || G::IsAttacking || G::DoubleTap || G::Recharge || G::AntiAim || pCmd->command_number % 2)
			return;

		if (!(pCmd->buttons & (IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT)))
			return;

		const Vec3 vecMove(pCmd->forwardmove, pCmd->sidemove, 0.f);
		const float flLength = vecMove.Length();
		Vec3 angMoveReverse;
		Math::VectorAngles(vecMove * -1.f, angMoveReverse);
		pCmd->forwardmove = -flLength;
		pCmd->sidemove = 0.f;
		pCmd->viewangles.y = fmodf(pCmd->viewangles.y - angMoveReverse.y, 360.f);
		pCmd->viewangles.z = 270.f;
		G::PSilentAngles = true;

		break;
	}
	case 2:
	{
		if (!Vars::Misc::Movement::FastStrafe.Value || G::IsAttacking)
			return;

		static bool bFwd = pCmd->forwardmove > 0;
		static bool bSde = pCmd->sidemove > 0;
		const bool bCurFwd = pCmd->forwardmove > 0;
		const bool bCurSde = pCmd->sidemove > 0;

		bool bChanged = false;
		if (fabsf(pCmd->sidemove) > 400)
		{
			if (bSde != bCurSde)
			{
				pCmd->viewangles.x = 90.f;
				pCmd->viewangles.y += bSde ? -90.f : 90.f;
				pCmd->sidemove = bSde ? -pCmd->forwardmove : pCmd->forwardmove;

				G::PSilentAngles = bChanged = true;
			}

			bSde = bCurSde;
			if (bChanged)
				return;
		}
		if (fabsf(pCmd->forwardmove) > 400)
		{
			if (bFwd != bCurFwd)
			{
				pCmd->viewangles.x = 90.f;
				pCmd->viewangles.y += bFwd ? 0.f : 180.f;
				pCmd->sidemove *= bFwd ? 1 : -1;

				G::PSilentAngles = bChanged = true;
			}

			bFwd = bCurFwd;
			if (bChanged)
				return;
		}
	}
	}
}

void CMisc::AntiWarp(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	static Vec3 vVelocity = {};
	if (G::AntiWarp)
	{
		const int iDoubletapTicks = F::Ticks.GetTicks(pLocal);

		Vec3 angles = {}; Math::VectorAngles(vVelocity, angles);
		angles.y = pCmd->viewangles.y - angles.y;
		Vec3 forward = {}; Math::AngleVectors(angles, &forward);
		forward *= vVelocity.Length();

		if (iDoubletapTicks > std::max(Vars::CL_Move::Doubletap::TickLimit.Value - 8, 3))
		{
			pCmd->forwardmove = -forward.x;
			pCmd->sidemove = -forward.y;
		}
		else if (iDoubletapTicks > 3)
		{
			pCmd->forwardmove = pCmd->sidemove = 0.f;
			pCmd->buttons &= ~(IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT);
		}
		else
		{
			pCmd->forwardmove = forward.x;
			pCmd->sidemove = forward.y;
		}
	}
	else
		vVelocity = pLocal->m_vecVelocity();

	/*
	static bool bSet = false;

	if (!G::AntiWarp)
	{
		bSet = false;
		return;
	}

	if (!G::IsAttacking && !bSet)
	{
		bSet = true;
		SDK::StopMovement(pLocal, pCmd);
	}
	else
		pCmd->forwardmove = pCmd->sidemove = 0.f;
	*/
}

void CMisc::LegJitter(CTFPlayer* pLocal, CUserCmd* pCmd, bool pSendPacket)
{
	if (!Vars::AntiHack::AntiAim::MinWalk.Value || !F::AntiAim.YawOn() || G::IsAttacking || G::DoubleTap || pSendPacket || !pLocal->OnSolid() || pLocal->IsInBumperKart())
		return;

	static bool pos = true;
	const float scale = pLocal->IsDucking() ? 14.f : 1.f;
	if (pCmd->forwardmove == 0.f && pCmd->sidemove == 0.f && pLocal->m_vecVelocity().Length2D() < 10.f)
	{
		pos ? pCmd->forwardmove = scale : pCmd->forwardmove = -scale;
		pos ? pCmd->sidemove = scale : pCmd->sidemove = -scale;
		pos = !pos;
	}
}



void CMisc::Event(IGameEvent* pEvent, FNV1A_t uHash)
{
	switch (uHash)
	{
	case FNV1A::HashConst("teamplay_round_start"):
	case FNV1A::HashConst("client_disconnect"):
	case FNV1A::HashConst("client_beginconnect"):
	case FNV1A::HashConst("game_newmap"):
		iLastCmdrate = -1;
		F::Backtrack.flWishInterp = 0.f;

		G::BulletsStorage.clear();
		G::BoxesStorage.clear();
		G::LinesStorage.clear();
	}
}

void CMisc::DoubletapPacket(CUserCmd* pCmd, bool* pSendPacket)
{
	if (G::DoubleTap || G::Warp)
	{
		*pSendPacket = G::ShiftedGoal == G::ShiftedTicks;
		if ((G::DoubleTap || pCmd->buttons & IN_ATTACK) && I::ClientState->chokedcommands >= 21)
			*pSendPacket = true;
	}
}

void CMisc::DetectChoke()
{
	for (auto pEntity : H::Entities.GetGroup(EGroupType::PLAYERS_ALL))
	{
		if (!pEntity->As<CTFPlayer>()->IsAlive() || pEntity->IsDormant())
		{
			G::ChokeMap[pEntity->entindex()] = 0;
			continue;
		}

		if (pEntity->m_flSimulationTime() == pEntity->m_flOldSimulationTime())
			G::ChokeMap[pEntity->entindex()]++;
		else
		{
			F::CheaterDetection.ReportChoke(pEntity->As<CTFPlayer>(), G::ChokeMap[pEntity->entindex()]);
			G::ChokeMap[pEntity->entindex()] = 0;
		}
	}
}

void CMisc::UnlockAchievements()
{
	const auto achievementmgr = reinterpret_cast<IAchievementMgr*(*)(void)>(U::Memory.GetVFunc(I::EngineClient, 114))();
	if (achievementmgr)
	{
		I::SteamUserStats->RequestCurrentStats();
		for (int i = 0; i < achievementmgr->GetAchievementCount(); i++)
			achievementmgr->AwardAchievement(achievementmgr->GetAchievementByIndex(i)->GetAchievementID());
		I::SteamUserStats->StoreStats();
		I::SteamUserStats->RequestCurrentStats();
	}
}

void CMisc::LockAchievements()
{
	const auto achievementmgr = reinterpret_cast<IAchievementMgr * (*)(void)>(U::Memory.GetVFunc(I::EngineClient, 114))();
	if (achievementmgr)
	{
		I::SteamUserStats->RequestCurrentStats();
		for (int i = 0; i < achievementmgr->GetAchievementCount(); i++)
			I::SteamUserStats->ClearAchievement(achievementmgr->GetAchievementByIndex(i)->GetName());
		I::SteamUserStats->StoreStats();
		I::SteamUserStats->RequestCurrentStats();
	}
}

bool CMisc::SteamRPC()
{
	/*
	if (!Vars::Misc::Steam::EnableRPC.Value)
	{
		if (!bSteamCleared) // stupid way to return back to normal rpc
		{
			I::SteamFriends->SetRichPresence("steam_display", ""); // this will only make it say "Team Fortress 2" until the player leaves/joins some server. its bad but its better than making 1000 checks to recreate the original
			bSteamCleared = true;
		}
		return false;
	}

	bSteamCleared = false;
	*/


	if (!Vars::Misc::Steam::EnableRPC.Value)
		return false;

	I::SteamFriends->SetRichPresence("steam_display", "#TF_RichPresence_Display");
	if (!I::EngineClient->IsInGame() && !Vars::Misc::Steam::OverrideMenu.Value)
		I::SteamFriends->SetRichPresence("state", "MainMenu");
	else
	{
		I::SteamFriends->SetRichPresence("state", "PlayingMatchGroup");

		switch (Vars::Misc::Steam::MatchGroup.Value)
		{
		case 0: I::SteamFriends->SetRichPresence("matchgrouploc", "SpecialEvent"); break;
		case 1: I::SteamFriends->SetRichPresence("matchgrouploc", "MannUp"); break;
		case 2: I::SteamFriends->SetRichPresence("matchgrouploc", "Competitive6v6"); break;
		case 3: I::SteamFriends->SetRichPresence("matchgrouploc", "Casual"); break;
		case 4: I::SteamFriends->SetRichPresence("matchgrouploc", "BootCamp"); break;
		default: I::SteamFriends->SetRichPresence("matchgrouploc", "SpecialEvent"); break;
		}
	}
	I::SteamFriends->SetRichPresence("currentmap", Vars::Misc::Steam::MapText.Value.c_str());
	I::SteamFriends->SetRichPresence("steam_player_group_size", std::to_string(Vars::Misc::Steam::GroupSize.Value).c_str());

	return true;
}

#ifdef DEBUG
void CMisc::DumpClassIDS() {
	std::ofstream fDump("CLASSIDDUMP.txt");
	fDump << "enum struct ETFClassID\n{\n";
	CClientClass* ClientClass = I::BaseClientDLL->GetAllClasses();
	while (ClientClass) {
		fDump << "	" << ClientClass->GetName() << " = " << ClientClass->m_ClassID << ",\n";
		ClientClass = ClientClass->m_pNext;
	}
	fDump << "}";
	fDump.close();
}
#endif