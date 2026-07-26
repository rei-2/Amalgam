#include "Misc.h"
#include "../Configs/Configs.h"

#include <array>

#include "../Backtrack/Backtrack.h"
#include "../Ticks/Ticks.h"
#include "../Players/PlayerUtils.h"
#include "../Players/SteamProfileCache.h"
#include "../Aimbot/AutoRocketJump/AutoRocketJump.h"
#include "../AntiCheatCompatibility/AntiCheatCompatibility.h"
#include "../EnginePrediction/EnginePrediction.h"
#include "../Aimbot/AimbotGlobal/AimbotGlobal.h"
#include "../NavBot/NavEngine/NavEngine.h"
#ifdef TEXTMODE
#include "NamedPipe/NamedPipe.h"
#endif

MAKE_SIGNATURE(Voice_IsRecording, "engine.dll", "80 3D ? ? ? ? ? 74 ? 80 3D ? ? ? ? ? 75", 0x0);
MAKE_SIGNATURE(ReportPlayerAccount, "client.dll", "48 89 5C 24 ? 57 48 83 EC ? 48 8B D9 8B FA 48 C1 E9", 0x0);

void CMisc::RunPre(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	EnsureChatUtilsDoc();
	NoiseSpam(pLocal);
	VoiceCommandSpam(pLocal);
	ChatSpam(pLocal);
	AutoDisguise(pLocal);
	AchievementSpam(pLocal);
	CallVoteSpam(pLocal);
	CheatsBypass();
	WeaponSway();
	AutoReport();
	AutoRetry(pLocal);

#ifdef TEXTMODE
	F::NamedPipe.Store(pLocal, true);
#endif
	AntiAFK(pLocal, pCmd);
	InstantRespawnMVM(pLocal);
	ExecBuyBot(pLocal, pCmd);

	if (!pLocal->IsAlive() || pLocal->IsAGhost() || pLocal->m_MoveType() != MOVETYPE_WALK || pLocal->IsSwimming()
		|| pLocal->IsTaunting())
		return;

	if (pLocal->InCond(TF_COND_SHIELD_CHARGE))
	{
		ChargeBot(pLocal, pCmd);
		return;
	}

	AutoJump(pLocal, pCmd);
	EdgeJump(pLocal, pCmd);
	if (pLocal->InCond(TF_COND_HALLOWEEN_KART))
		return;

	AutoJumpbug(pLocal, pCmd);
	AutoRevJump(pLocal, pCmd);
	AutoStrafe(pLocal, pCmd);
	AutoAirStickyPogo(pLocal, pCmd);
	AutoPeek(pLocal, pCmd);
	BreakJump(pLocal, pCmd);
}

void CMisc::RunPost(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	m_bDuckSpeedActive = false;
	AutoCrouchNavbot(pLocal, pCmd);

	if (!pLocal->IsAlive() || pLocal->IsAGhost() || pLocal->m_MoveType() != MOVETYPE_WALK || pLocal->IsSwimming()
		|| pLocal->InCond(TF_COND_SHIELD_CHARGE))
		return;

	if (pLocal->IsTaunting() || pLocal->InCond(TF_COND_HALLOWEEN_KART))
		TauntKartControl(pLocal, pCmd);
	else
	{
		F::EnginePrediction.End(pLocal, nullptr);
		AutoEdgebug(pLocal, pCmd);
		F::EnginePrediction.Start(pLocal, pCmd);
		if (!m_bEdgeBug)
		{
			EdgeJump(pLocal, pCmd, true);
			AutoPeek(pLocal, pCmd, true);
			FastMovement(pLocal, pCmd);
			BreakShootSound(pLocal, pCmd);
			MovementLock(pLocal, pCmd);
		}
	}
}

void CMisc::AutoCrouchNavbot(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	if (!Vars::Misc::Movement::DuckSpeed.Value
		|| !Vars::Misc::Movement::NavBot::Enabled.Value
		|| !Vars::Misc::Movement::NavEngine::Enabled.Value)
		return;

	if (pLocal->IsSwimming() || F::NavEngine.IsUnstucking() || pCmd->buttons & IN_JUMP)
	{
		pCmd->buttons &= ~IN_DUCK;
		return;
	}

	pCmd->buttons |= IN_DUCK;
}



void CMisc::AutoJump(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	if (!Vars::Misc::Movement::Bunnyhop.Value)
		return;

	if (auto pWeapon = H::Entities.GetWeapon(); pWeapon && pWeapon->GetWeaponID() == TF_WEAPON_GRAPPLINGHOOK && pWeapon->As<CTFGrapplingHook>()->m_hProjectile())
		return;

	static bool bStaticAttempted = false, bStaticValid = false;
	bool bLastAttempted = bStaticAttempted, bLastValid = bStaticValid;
	bool bCurrAttempted = bStaticAttempted = G::OriginalCmd.buttons & IN_JUMP, bCurrValid = bStaticValid = pLocal->m_hGroundEntity() && !pLocal->IsDucking();
	if (!bCurrValid || bCurrValid && G::LastUserCmd->buttons & IN_JUMP)
		pCmd->buttons &= ~IN_JUMP;

	static float flLastAttempt = 0.f;
	bool bPressed = bCurrAttempted && !bLastAttempted;
	bool bParachute = SDK::AttribHookValue(0, "parachute_attribute", pLocal) && !pLocal->InCond(TF_COND_PARACHUTE_ACTIVE);
	bool bAllow = !bParachute || G::OriginalCmd.buttons & IN_DUCK; // evil we don't want to manual
	bool bManual = bAllow && (bPressed || bParachute && I::GlobalVars->curtime < flLastAttempt + 0.1f);
	if (bPressed && !bCurrValid)
		flLastAttempt = I::GlobalVars->curtime;
	if (bManual)
		pCmd->buttons |= IN_JUMP;

	F::AntiCheatCompatibility.BunnyHop(pCmd, bCurrValid, bLastValid);
}

void CMisc::AutoJumpbug(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	if (!Vars::Misc::Movement::AutoJumpbug.Value || !(pCmd->buttons & IN_DUCK) || pLocal->m_hGroundEntity() || pLocal->m_vecVelocity().z > -650.f)
		return;

	float flUnduckHeight = 20 * pLocal->m_flModelScale();
	float flTraceDistance = flUnduckHeight + 2;

	CGameTrace trace = {};
	CTraceFilterWorldAndPropsOnly filter = {};

	Vec3 vOrigin = pLocal->m_vecOrigin();
	SDK::TraceHull(vOrigin, vOrigin - Vec3(0, 0, flTraceDistance), pLocal->m_vecMins(), pLocal->m_vecMaxs(), pLocal->SolidMask(), &filter, &trace);
	if (!trace.DidHit() || trace.fraction * flTraceDistance < flUnduckHeight) // don't try if we aren't in range to unduck or are too low
		return;

	pCmd->buttons &= ~IN_DUCK;
	pCmd->buttons |= IN_JUMP;
}

void CMisc::AutoEdgebug(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	if (!Vars::Misc::Movement::AutoEdgebug.Value || pLocal->OnSolid())
	{
		m_bEdgeBug = false;
		m_iEdgeBugMoveStage = EBStageEnum::Normal;
		m_iEdgeBugTicksLeft = m_iEdgeBugTicksTotal = 0;
		m_flEdgeBugStartYaw = m_flEdgeBugYawDelta = 0.f;
		m_vEdgeBugMove.Zero();
		m_vEdgebugPath.clear();
		return;
	}

	auto fRunStrafe = [&]()-> void
		{
			if (m_iEdgeBugTicksLeft < m_iEdgeBugTicksTotal
				&& m_vEdgebugPath.size() 
				&& m_vEdgebugPath.front().DistTo2D(pLocal->m_vecOrigin()) > 30.f)
			{
				m_bEdgeBug = false;
				m_iEdgeBugMoveStage = EBStageEnum::Normal;
				m_iEdgeBugTicksLeft = m_iEdgeBugTicksTotal = 0;
				m_flEdgeBugStartYaw = m_flEdgeBugYawDelta = 0.f;
				m_vEdgeBugMove.Zero();
				m_vEdgebugPath.clear();
				return;
			}
			if (m_flEdgeBugYawDelta)
			{
				if (G::Attacking == 1)
				{
					m_bEdgeBug = false;
					m_iEdgeBugMoveStage = EBStageEnum::Normal;
					m_iEdgeBugTicksLeft = m_iEdgeBugTicksTotal = 0;
					m_flEdgeBugStartYaw = m_flEdgeBugYawDelta = 0.f;
					m_vEdgeBugMove.Zero();
					m_vEdgebugPath.clear();
					return;
				}
				pCmd->viewangles.y = Math::NormalizeAngle(m_flEdgeBugStartYaw + m_flEdgeBugYawDelta * (1 + m_iEdgeBugTicksTotal - m_iEdgeBugTicksLeft));
				if (Vars::Misc::Movement::AutoEdgebug.Value != Vars::Misc::Movement::AutoEdgebugEnum::StrafeSilent)
					I::EngineClient->SetViewAngles(pCmd->viewangles);
			}
			pCmd->buttons = m_bEdgeBugCrouch ? (pCmd->buttons | IN_DUCK) : (pCmd->buttons & ~IN_DUCK);
			pCmd->forwardmove = m_vEdgeBugMove.x;
			pCmd->sidemove = m_vEdgeBugMove.y;
			m_iEdgeBugTicksLeft--;
		};

	if (m_bEdgeBug)
	{
		if (!m_iEdgeBugTicksLeft)
		{
			m_bEdgeBug = false;
			m_iEdgeBugMoveStage = EBStageEnum::Normal;
			m_iEdgeBugTicksTotal = 0;
			m_flEdgeBugStartYaw = m_flEdgeBugYawDelta = 0.f;
			m_vEdgeBugMove.Zero();
		}
		else
		{
			fRunStrafe();
			if (m_vEdgebugPath.size())
			{
				if (Vars::Colors::EdgebugPath.Value.a)
					G::PathStorage.emplace_back(m_vEdgebugPath, I::GlobalVars->curtime + TICK_INTERVAL, Vars::Colors::EdgebugPath.Value, Vars::Visuals::Prediction::PlayerPath.Value);
				m_vEdgebugPath.erase(m_vEdgebugPath.begin());
			}
			if (!m_bEdgeBugRepredict)
				return;
		}
	}

	size_t iSize = pLocal->GetIntermediateDataSize();
	auto pDataMap = pLocal->GetPredDescMap();
	if (!pDataMap)
		return;

	const int iLocalIdx = pLocal->entindex();
	byte* pOriginalData = reinterpret_cast<byte*>(I::MemAlloc->Alloc(iSize));
	{
		CPredictionCopy copy = { PC_EVERYTHING, pOriginalData, PC_DATA_PACKED, pLocal, PC_DATA_NORMAL };
		copy.TransferData("EdgebugStore", iLocalIdx, pDataMap);
	}

	const bool bOldIsFirstPrediction = I::Prediction->m_bFirstTimePredicted;
	const bool bOldInPrediction = I::Prediction->m_bInPrediction;
	const float flOldFrametime = I::GlobalVars->frametime;
	const float flOldCurtime = I::GlobalVars->curtime;

	if (m_iEdgeBugTicksUntilLand)
	{
		// disable random if we are about to land, its not going to change anything anyway
		if (m_iEdgeBugMoveStage > EBStageEnum::NormalInverted
			&& TICKS_TO_TIME(m_iEdgeBugTicksUntilLand) < 0.4f)
			m_iEdgeBugMoveStage -= 2;

		m_iEdgeBugTicksUntilLand--;
	}

	static auto sv_gravity = H::ConVars.FindVar("sv_gravity");
	const float flFallPerTick = round(TICKS_TO_TIME(-sv_gravity->GetFloat()));

	const bool bNegateDir = Vars::Misc::Movement::AutoEdgebugTryNegativeDir.Value && m_iEdgeBugMoveStage % 2 != 0;
	const float flDirMult = bNegateDir ? -1.f : 1.f;

	float flForwardmoveMult = 1.f, flSidemoveMult = flDirMult;
	if (Vars::Misc::Movement::AutoEdgebugTryRandomMove.Value 
		&& m_iEdgeBugMoveStage > EBStageEnum::NormalInverted)
	{
		flForwardmoveMult = SDK::RandomFloat(-0.2f, 4.f);
		flSidemoveMult *= SDK::RandomFloat(0.3f, 2.f);
	}

	float flCurrentDirDelta = 0.f;
	{
		float flForward = pCmd->forwardmove, flSide = pCmd->sidemove;
		Vec3 vForward, vRight; Math::AngleVectors(pCmd->viewangles, &vForward, &vRight, nullptr);
		vForward.Normalize2D(), vRight.Normalize2D();
		Vec3 vWishDir = Math::VectorAngles({ vForward.x * flForward + vRight.x * flSide, vForward.y * flForward + vRight.y * flSide, 0.f });
		Vec3 vCurDir = Math::VectorAngles(pLocal->m_vecVelocity());
		flCurrentDirDelta = Math::NormalizeAngle(vWishDir.y - vCurDir.y);
	}

	float flYawDelta = 30.f;
	bool bShouldStrafe = Vars::Misc::Movement::AutoEdgebug.Value > Vars::Misc::Movement::AutoEdgebugEnum::Legit && G::Attacking != 1;
	if (bShouldStrafe && abs(flCurrentDirDelta) > 1.f)
		flYawDelta = std::clamp(flCurrentDirDelta, -45.f, 45.f);
	flYawDelta *= flDirMult;

	const float flStartYaw = pCmd->viewangles.y;
	if (bShouldStrafe && abs(Math::NormalizeAngle(flStartYaw + flYawDelta) - flStartYaw) > Vars::Misc::Movement::AutoEdgebugStrafeMaxDelta.Value)
		bShouldStrafe = false;

	const int iMaxTicks = TIME_TO_TICKS(1.5f);
	int iMaxStages = bShouldStrafe ? 4 : 2;

	bool bSuccess = false;
	std::vector<Vector> vPath;
	const int iStrafeSamples = Vars::Misc::Movement::AutoEdgebugStrafeSamples.Value;
	for (int iStage = 0; iStage < iMaxStages; iStage++)
	{
		float flMaxYawDelta = abs(flYawDelta);
		float flYawDeltaAdd = flYawDelta /= iStrafeSamples;
		bool bEnd = false, bStrafe = iStage >= 2, bCrouch = iStage % 2 == 0;

		I::MoveHelper->SetHost(pLocal);
		while (!bEnd)
		{
			// restoring a prediction copy is ~5 times faster than calling RestoreEntityToPredictedFrame every time (plus its not even correct to use it here)
			CPredictionCopy copy = { PC_EVERYTHING, pLocal, PC_DATA_NORMAL, pOriginalData, PC_DATA_PACKED};
			copy.TransferData("EdgebugReset", iLocalIdx, pDataMap);

			vPath.clear();
			vPath.push_back(pLocal->m_vecOrigin());
			bEnd = !bStrafe || abs(flYawDelta) >= flMaxYawDelta;

			G::DummyCmd = *pCmd;
			G::DummyCmd.buttons = bCrouch ? (G::DummyCmd.buttons | IN_DUCK) : (G::DummyCmd.buttons & ~IN_DUCK);

			if (bStrafe)
			{
				if (!G::DummyCmd.forwardmove)
					G::DummyCmd.forwardmove = 30.f; // makes it detect wallbugs (will probably make it a separate feature later)
				if (!G::DummyCmd.sidemove)
					G::DummyCmd.sidemove = 450.f;
			}
			else
				G::DummyCmd.forwardmove = G::DummyCmd.sidemove = 0.f;

			G::DummyCmd.forwardmove = std::clamp(G::DummyCmd.forwardmove * flForwardmoveMult, -450.f, 450.f);
			G::DummyCmd.sidemove = std::clamp(G::DummyCmd.sidemove * flSidemoveMult, -450.f, 450.f);;
			
			pLocal->m_pCurrentCommand() = &G::DummyCmd;
			I::Prediction->m_bFirstTimePredicted = false;
			I::Prediction->m_bInPrediction = true;
			I::GlobalVars->frametime = I::Prediction->m_bEnginePaused ? 0.f : TICK_INTERVAL;
			I::GlobalVars->curtime = TICKS_TO_TIME(pLocal->m_nTickBase());

			CMoveData moveData;
			Vector vOriginalVelocity;
			for (int iTick = 1; iTick <= iMaxTicks; iTick++)
			{
				Vector vPreviousVelocity = pLocal->m_vecVelocity();
				bool bWasOnSolid = pLocal->OnSolid();
				if (bWasOnSolid && iStage == 0)
					m_iEdgeBugTicksUntilLand = iTick - 1;

				if (bStrafe)
				{
					G::DummyCmd.viewangles.y = Math::NormalizeAngle(pCmd->viewangles.y + flYawDelta * iTick);
					if (abs(G::DummyCmd.viewangles.y - flStartYaw) > Vars::Misc::Movement::AutoEdgebugStrafeMaxDelta.Value)
						break;
				}

				I::Prediction->SetLocalViewAngles(G::DummyCmd.viewangles);
				I::Prediction->SetupMove(pLocal, &G::DummyCmd, I::MoveHelper, &moveData);
				I::GameMovement->ProcessMovement(pLocal, &moveData); // dont mind the sudden water splashing sounds, its just your ghost copy drowning in another realm
				I::Prediction->FinishMove(pLocal, pCmd, &moveData);
				vPath.push_back(pLocal->m_vecOrigin());

				if (vPreviousVelocity.z >= 0.f || bWasOnSolid)
					break;

				if (iTick == 1)
				{
					// fix for non-local servers
					vOriginalVelocity = pLocal->m_vecVelocity();
					continue;
				}

				if (vPreviousVelocity.z > vOriginalVelocity.z)
				{
					float flExpectedFallSpeed = vPreviousVelocity.z + flFallPerTick;
					float flFallSpeed = round(pLocal->m_vecVelocity().z);

					if (flExpectedFallSpeed == flFallSpeed)
					{
						m_iEdgeBugTicksTotal = m_iEdgeBugTicksLeft = iTick;
						m_bEdgeBugCrouch = bCrouch;
						if (bStrafe)
						{
							m_flEdgeBugStartYaw = flStartYaw;
							m_flEdgeBugYawDelta = flYawDelta;
							m_vEdgeBugMove = { moveData.m_flForwardMove, moveData.m_flSideMove };
						}
						m_vEdgebugPath = vPath;
						m_bEdgeBug = bEnd = bSuccess = true;
						fRunStrafe();
						m_bEdgeBugRepredict = !m_bEdgeBug;
					}
					break;
				}
			}
			I::Prediction->m_bFirstTimePredicted = bOldIsFirstPrediction;
			I::Prediction->m_bInPrediction = bOldInPrediction;
			I::GlobalVars->frametime = flOldFrametime;
			I::GlobalVars->curtime = flOldCurtime;

			flYawDelta += flYawDeltaAdd;
		}
		I::MoveHelper->SetHost(nullptr);
		pLocal->m_pCurrentCommand() = nullptr;

		if (bSuccess)
			break;
	}
	const int iMaxMode = Vars::Misc::Movement::AutoEdgebugTryRandomMove.Value ? EBStageEnum::RandomInverted : EBStageEnum::NormalInverted;
	m_iEdgeBugMoveStage = m_iEdgeBugMoveStage < iMaxMode ? m_iEdgeBugMoveStage + 1 : EBStageEnum::Normal;

	CPredictionCopy copy = { PC_EVERYTHING, pLocal, PC_DATA_NORMAL, pOriginalData, PC_DATA_PACKED };
	copy.TransferData("EdgebugReset", iLocalIdx, pDataMap);
	I::MemAlloc->Free(pOriginalData);
	G::DummyCmd = {};
}

void CMisc::AutoStrafe(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	if (!Vars::Misc::Movement::AutoStrafe.Value || pLocal->m_hGroundEntity() || !(pLocal->m_afButtonLast() & IN_JUMP) && (pCmd->buttons & IN_JUMP))
		return;

	switch (Vars::Misc::Movement::AutoStrafe.Value)
	{
	case Vars::Misc::Movement::AutoStrafeEnum::Legit:
	{
		static auto cl_sidespeed = H::ConVars.FindVar("cl_sidespeed");
		const float flSideSpeed = cl_sidespeed->GetFloat();

		if (pCmd->mousedx)
		{
			pCmd->forwardmove = 0.f;
			pCmd->sidemove = pCmd->mousedx > 0 ? flSideSpeed : -flSideSpeed;
		}
		break;
	}
	case Vars::Misc::Movement::AutoStrafeEnum::Directional:
	{
		// credits: KGB
		if (!(pCmd->buttons & (IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT)))
			break;

		float flForward = pCmd->forwardmove, flSide = pCmd->sidemove;
		Vec3 vForward, vRight; Math::AngleVectors(pCmd->viewangles, &vForward, &vRight, nullptr);
		vForward.Normalize2D(), vRight.Normalize2D();

		Vec3 vWishDir = Math::VectorAngles({ vForward.x * flForward + vRight.x * flSide, vForward.y * flForward + vRight.y * flSide, 0.f });
		Vec3 vCurDir = Math::VectorAngles(pLocal->m_vecVelocity());
		float flDirDelta = Math::NormalizeAngle(vWishDir.y - vCurDir.y);
		if (fabsf(flDirDelta) > Vars::Misc::Movement::AutoStrafeMaxDelta.Value)
			break;

		float flTurnScale = Math::RemapVal(Vars::Misc::Movement::AutoStrafeTurnScale.Value, 0.f, 1.f, 0.9f, 1.f);
		float flRotation = Math::Deg2Rad((flDirDelta > 0.f ? -90.f : 90.f) + flDirDelta * flTurnScale);
		float flCosRot = cosf(flRotation), flSinRot = sinf(flRotation);

		pCmd->forwardmove = flCosRot * flForward - flSinRot * flSide;
		pCmd->sidemove = flSinRot * flForward + flCosRot * flSide;
	}
	}
}

void CMisc::AutoFaNJump(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd)
{
	static bool bDidJump = false, bCanJump = false, bShouldRun = false;
	static int iTicksOnSolid = Vars::Misc::Movement::AutoFaNJumpOnSolidTicks.Value;

	bool bOnSolid = pLocal->OnSolid();
	if (!bOnSolid)
	{
		iTicksOnSolid = 0;
		if (pLocal->InCond(TF_COND_STUNNED))
			bCanJump = false;
	}
	else iTicksOnSolid += bCanJump = true;
	
	if (bDidJump)
	{
		if (iTicksOnSolid >= Vars::Misc::Movement::AutoFaNJumpOnSolidTicks.Value)
		{
			bDidJump = false;
			bShouldRun = false;
		}
		return;
	}

	if (!Vars::Misc::Movement::AutoFaNJump.Value || !bCanJump
		|| G::Attacking == 1 || !G::CanPrimaryAttack
		|| !pLocal->IsAlive() || pLocal->IsAGhost()
		|| pLocal->m_bScattergunJump()
		|| pLocal->m_MoveType() != MOVETYPE_WALK || pLocal->IsSwimming() 
		|| pLocal->IsTaunting() || pLocal->InCond(TF_COND_HALLOWEEN_KART))
		return;

	if (!pWeapon || pWeapon->m_iClip1() <= 0)
		return;

	int iDefIndex = pWeapon->m_iItemDefinitionIndex();
	if (iDefIndex != Scout_m_ForceANature && iDefIndex != Scout_m_FestiveForceANature)
		return;

	const Vector vVelocity = pLocal->m_vecVelocity();

	// If we dont have enough speed we wont be able to climb anything.
	// From my experience 150.f is enough to climb the grate dropdowns on 2fort bases
	if (vVelocity.Length2D() <= 150.f)
		return;

	Vector vAngles;
	float flScale = 0.f;
	if (vVelocity.Length2D() > 300.f && !bOnSolid)
	{
		flScale = Math::RemapVal(vVelocity.Length2D(), 300.f, 450.f, 0.f, 1.f);

		CGameTrace wallTrace = {};
		CTraceFilterWorldAndPropsOnly filter(pLocal);
		Vector vTrace = pLocal->GetCenter(), vMins =  pLocal->m_vecMins() * 3, vMaxs = pLocal->m_vecMaxs() * 3;
		vMins.z = -1;
		SDK::TraceHull(vTrace, vTrace, vMins, vMaxs, MASK_PLAYERSOLID_BRUSHONLY, &filter, &wallTrace);
		if (!wallTrace.DidHit())
		{
			// Preserve speed if we are not attempting to climb
			vAngles = pCmd->viewangles;
			vAngles.x = 89.f; 
		}
	}

	if (vAngles.x == 0.f)
	{
		Math::VectorAngles(vVelocity, vAngles);
		
		// 2fort sewers to bridge jump pitch angle if we have enough speed and not on solid
		vAngles.x = 37.f * (1 + 0.5f * flScale); 
		vAngles.z = 0;
	}

	if (!bShouldRun)
	{
		if (!Vars::Misc::Movement::AutoFaNJumpCheckCeiling.Value || vVelocity.z <= -300.f)
			bShouldRun = true;
		else
		{
			CGameTrace ceilingTrace = {};
			CTraceFilterWorldAndPropsOnly filter(pLocal);
			Vector vStart = pLocal->m_vecOrigin(); vStart += Vector(0, 0, pLocal->m_vecMaxs().z);
			Vector vEnd = vStart + Vector(0, 0, pLocal->m_vecMaxs().z * 1.25);

			// Increase hull size because if we change the pitch the trajectory also changes
			float flSizeScale = 1 + 0.75f * flScale;
			SDK::TraceHull(vStart, vEnd, pLocal->m_vecMins() * flSizeScale, pLocal->m_vecMaxs() * flSizeScale, MASK_PLAYERSOLID_BRUSHONLY, &filter, &ceilingTrace);
			if (!ceilingTrace.DidHit())
				bShouldRun = true;
		}
	}

	if (bShouldRun)
	{
		if (bOnSolid)
			pCmd->buttons |= IN_JUMP;

		pCmd->buttons |= IN_ATTACK;
		G::Attacking = true;
		bDidJump = true;

		pCmd->viewangles = vAngles;
		G::SilentAngles = true;
	}
}

void CMisc::AutoRevJump(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	if (!Vars::Misc::Movement::AutoRevJump.Value || !pLocal->m_hGroundEntity())
		return;

	if (auto pWeapon = H::Entities.GetWeapon(); !pWeapon || pWeapon->GetWeaponID() != TF_WEAPON_MINIGUN || pWeapon->As<CTFMinigun>()->m_iWeaponState() != AC_STATE_IDLE)
		return;

	if (!(pCmd->buttons & IN_ATTACK2) || G::LastUserCmd->buttons & IN_ATTACK2)
		return;

	pCmd->buttons |= IN_JUMP;
}

void CMisc::MovementLock(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	static bool bLock = false;

	if (!Vars::Misc::Movement::MovementLock.Value || pLocal->InCond(TF_COND_HALLOWEEN_KART))
	{
		bLock = false;
		return;
	}

	static Vec3 vMove = {}, vView = {};
	if (!bLock)
	{
		bLock = true;
		vMove = { pCmd->forwardmove, pCmd->sidemove, pCmd->upmove };
		vView = pCmd->viewangles;
	}

	pCmd->forwardmove = vMove.x, pCmd->sidemove = vMove.y, pCmd->upmove = vMove.z;
	SDK::FixMovement(pCmd, vView, pCmd->viewangles);
}

void CMisc::BreakJump(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	if (!Vars::Misc::Movement::BreakJump.Value || F::AutoRocketJump.IsRunning())
		return;

	static bool bStaticJump = false;
	const bool bLastJump = bStaticJump;
	const bool bCurrJump = bStaticJump = pCmd->buttons & IN_JUMP;

	static int iTickSinceGrounded = -1;
	if (pLocal->m_hGroundEntity().Get())
		iTickSinceGrounded = -1;
	iTickSinceGrounded++;

	switch (iTickSinceGrounded)
	{
	case 0:
		if (bLastJump || !bCurrJump || pLocal->IsDucking())
			return;
		break;
	case 1:
		break;
	default:
		return;
	}

	pCmd->buttons |= IN_DUCK;
}

void CMisc::BreakShootSound(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	static int iOriginalWeaponSlot = -1;
	auto pWeapon = H::Entities.GetWeapon();
	if (!Vars::Misc::Exploits::BreakShootSound.Value || F::Ticks.m_bDoubletap || pLocal->m_iClass() != TF_CLASS_SOLDIER || !pWeapon)
		return;

	static bool bLastWasInAttack = false;
	static int iLastWeaponSelect = -1;
	const int iCurrSlot = pWeapon->GetSlot();
	if (pCmd->weaponselect && pCmd->weaponselect != iLastWeaponSelect)
		iOriginalWeaponSlot = -1;

	auto pSwap = iCurrSlot == SLOT_SECONDARY ? pLocal->GetWeaponFromSlot(SLOT_PRIMARY) : iCurrSlot == SLOT_PRIMARY ? pLocal->GetWeaponFromSlot(SLOT_SECONDARY) : pLocal->GetWeaponFromSlot(iOriginalWeaponSlot);
	if (pSwap && pSwap->CanBeSelected() && !pCmd->weaponselect)
	{
		if (bLastWasInAttack)
		{
			if (iOriginalWeaponSlot < 0)
			{
				iOriginalWeaponSlot = iCurrSlot;
				pCmd->weaponselect = pSwap->entindex();
				iLastWeaponSelect = pCmd->weaponselect;
			}
		}
		else
		{
			if (iOriginalWeaponSlot == iCurrSlot && G::CanPrimaryAttack)
				iOriginalWeaponSlot = -1;
			else if (iOriginalWeaponSlot >= 0 && iOriginalWeaponSlot != iCurrSlot)
			{
				pCmd->weaponselect = pSwap->entindex();
				iLastWeaponSelect = pCmd->weaponselect;
			}
		}
	}

	bLastWasInAttack = G::Attacking == 1 && G::CanPrimaryAttack;
}

void CMisc::AntiAFK(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	static Timer tTimer = {};
	m_bAntiAFK = false;
	static auto mp_idledealmethod = H::ConVars.FindVar("mp_idledealmethod");
	static auto mp_idlemaxtime = H::ConVars.FindVar("mp_idlemaxtime");
	const int iIdleMethod = mp_idledealmethod->GetInt();
	const float flMaxIdleTime = mp_idlemaxtime->GetFloat();
	static bool bForce = false;

	// Just in case there's a connection problem
	auto pNetChan = I::EngineClient->GetNetChannelInfo();
	bool bTimingOut = pNetChan && pNetChan->IsTimingOut();
	if (bTimingOut)
		bForce = true;

	if (pCmd->buttons & (IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT) || !pLocal->IsAlive())
	{
		tTimer.Update();
		bForce = false;
	}
	else if (Vars::Misc::Automation::AntiAFK.Value && iIdleMethod && (tTimer.Check(flMaxIdleTime * 60.f - 10.f) || (!bTimingOut && bForce))) // trigger 10 seconds before kick
	{
		pCmd->buttons |= I::GlobalVars->tickcount % 2 ? IN_FORWARD : IN_BACK;
		tTimer.Update();
		bForce = false;
		m_bAntiAFK = true;
	}
}

void CMisc::InstantRespawnMVM(CTFPlayer* pLocal)
{
	if (!Vars::Misc::MannVsMachine::InstantRespawn.Value || pLocal->IsAlive())
		return;

	KeyValues* kv = new KeyValues("MVM_Revive_Response");
	kv->SetBool("accepted", true);
	I::EngineClient->ServerCmdKeyValues(kv);
}

void CMisc::NoiseSpam(CTFPlayer* pLocal)
{
	if (!Vars::Misc::Automation::NoiseSpam.Value || pLocal->m_bUsingActionSlot())
		return;

	static float flLastSpamTime = 0.0f;
	float flCurrentTime = SDK::PlatFloatTime();
	if (flCurrentTime - flLastSpamTime < 0.2f)
		return;

	flLastSpamTime = flCurrentTime;
	I::EngineClient->ServerCmdKeyValues(new KeyValues("use_action_slot_item_server"));
}

void CMisc::AutoDisguise(CTFPlayer* pLocal)
{
	if (!Vars::Misc::Automation::AutoDisguise.Value || !pLocal->IsAlive() || !pLocal->IsInValidTeam() || pLocal->m_iClass() != TF_CLASS_SPY)
		return;

	if (pLocal->InCond(TF_COND_DISGUISING) || pLocal->InCond(TF_COND_DISGUISED) || pLocal->InCond(TF_COND_DISGUISE_WEARINGOFF))
		return;

	static Timer tDisguiseTimer{};
	if (!tDisguiseTimer.Run(0.75f))
		return;

	auto pResource = H::Entities.GetResource();
	if (!pResource)
		return;

	const int iEnemyTeam = pLocal->m_iTeamNum() == TF_TEAM_RED ? TF_TEAM_BLUE : TF_TEAM_RED;
	std::array<int, TF_CLASS_COUNT> aClassCounts = {};
	int iTotalClasses = 0;

	for (int i = 1; i <= I::EngineClient->GetMaxClients(); ++i)
	{
		if (!pResource->m_bValid(i) || !pResource->m_bConnected(i) || pResource->m_iTeam(i) != iEnemyTeam)
			continue;

		const int iEnemyClass = pResource->m_iPlayerClass(i);
		if (iEnemyClass <= TF_CLASS_UNDEFINED || iEnemyClass >= TF_CLASS_COUNT || iEnemyClass == TF_CLASS_HEAVY)
			continue;

		++aClassCounts[iEnemyClass];
		++iTotalClasses;
	}

	if (!iTotalClasses)
		return;

	int iClass = TF_CLASS_UNDEFINED;
	int iRandomClass = SDK::RandomInt(1, iTotalClasses);
	for (int i = TF_CLASS_SCOUT; i < TF_CLASS_COUNT; ++i)
	{
		if (iRandomClass > aClassCounts[i])
		{
			iRandomClass -= aClassCounts[i];
			continue;
		}

		iClass = i;
		break;
	}

	if (iClass == TF_CLASS_UNDEFINED)
		return;

	I::EngineClient->ClientCmd_Unrestricted(std::format("disguise {} -1", iClass).c_str());
}

void CMisc::CallVoteSpam(CTFPlayer* pLocal)
{
	if (!Vars::Misc::Automation::CallVoteSpam.Value || !m_tCallVoteSpamTimer.Run(1.0f))
		return;

	std::vector<std::string> vVoteOptions = {
		"callvote changelevel cp_badlands",
		"callvote changelevel cp_granary",
		"callvote changelevel cp_well",
		"callvote changelevel cp_5gorge",
		"callvote changelevel cp_freight_final1",
		"callvote changelevel cp_yukon_final",
		"callvote changelevel cp_gravelpit",
		"callvote changelevel cp_dustbowl",
		"callvote changelevel cp_egypt_final",
		"callvote changelevel cp_junction_final",
		"callvote changelevel cp_steel",
		"callvote changelevel ctf_2fort",
		"callvote changelevel ctf_well",
		"callvote changelevel ctf_sawmill",
		"callvote changelevel ctf_turbine",
		"callvote changelevel ctf_doublecross",
		"callvote changelevel pl_badwater",
		"callvote changelevel pl_goldrush",
		"callvote changelevel pl_dustbowl",
		"callvote changelevel pl_upward",
		"callvote changelevel pl_thundermountain",
		"callvote changelevel koth_harvest_final",
		"callvote changelevel koth_nucleus",
		"callvote changelevel koth_sawmill",
		"callvote changelevel koth_viaduct",
		"callvote changelevel cp_5gorge",
		"callvote changelevel cp_dustbowl",
		"callvote changelevel ctf_2fort",
		"callvote changelevel ctf_doublecross",
		"callvote changelevel ctf_turbine",
		"callvote changelevel koth_brazil",
		"callvote changelevel pl_badwater",
		"callvote changelevel pl_pheonix",
		"callvote changelevel plr_bananabay",
		"callvote changelevel plr_hightower",
		"callvote scrambleteams"
	};

	int iRandomIndex = SDK::RandomInt(0, static_cast<int>(vVoteOptions.size()) - 1);
	std::string strSelectedVote = vVoteOptions[iRandomIndex];

	I::ClientState->SendStringCmd(strSelectedVote.c_str());
}

void CMisc::CheatsBypass()
{
	static bool bCheatSet = false;
	static auto sv_cheats = H::ConVars.FindVar("sv_cheats");
	const bool bShouldBypass = Vars::Misc::Exploits::CheatsBypass.Value;
	if (bShouldBypass)
		bCheatSet = sv_cheats->m_nValue = 1;
	else if (bCheatSet)
		bCheatSet = false;
}

void CMisc::WeaponSway()
{
	static auto cl_wpn_sway_interp = H::ConVars.FindVar("cl_wpn_sway_interp");
	static auto cl_wpn_sway_scale = H::ConVars.FindVar("cl_wpn_sway_scale");

	bool bSway = Vars::Visuals::Viewmodel::SwayInterp.Value || Vars::Visuals::Viewmodel::SwayScale.Value;
	cl_wpn_sway_interp->SetValue(bSway ? Vars::Visuals::Viewmodel::SwayInterp.Value : 0.f);
	cl_wpn_sway_scale->SetValue(bSway ? Vars::Visuals::Viewmodel::SwayScale.Value : 0.f);
}

void CMisc::TauntKartControl(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	if (Vars::Misc::Automation::TauntControl.Value && pLocal->IsTaunting() && pLocal->m_bAllowMoveDuringTaunt())
	{
		if (pLocal->m_bTauntForceMoveForward())
		{
			if (pCmd->buttons & IN_BACK)
				pCmd->viewangles.x = 91.f;
			else if (!(pCmd->buttons & IN_FORWARD))
				pCmd->viewangles.x = 90.f;
		}
		if (pCmd->buttons & IN_MOVELEFT)
			pCmd->sidemove = pCmd->viewangles.x == 90.f ? -450.f : -pLocal->m_flTauntForceMoveForwardSpeed();
		else if (pCmd->buttons & IN_MOVERIGHT)
			pCmd->sidemove = pCmd->viewangles.x == 90.f ? 450.f : pLocal->m_flTauntForceMoveForwardSpeed();
	}
	else if (Vars::Misc::Automation::KartControl.Value && pLocal->InCond(TF_COND_HALLOWEEN_KART))
	{
		bool bChoke = I::ClientState->chokedcommands < 3 && F::Ticks.CanChoke(true);
		float flForward = fabsf(pCmd->forwardmove), flSide = pCmd->sidemove * (!bChoke ? 0.f : pCmd->forwardmove < 0.f ? -1 : 1);

		Vec3 vForward, vRight; Math::AngleVectors(pCmd->viewangles, &vForward, &vRight, nullptr);
		vForward.Normalize2D(), vRight.Normalize2D();

		pCmd->viewangles.x = 90.f;
		G::SilentAngles = true;

		if (!(pCmd->buttons & (IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT)))
			return;

		if (pCmd->forwardmove < 0.f)
			pCmd->viewangles.x = 91.f;
		else if (pCmd->forwardmove > 0.f || flSide)
			pCmd->viewangles.x = 10.f;
		pCmd->forwardmove = 0.f;

		if (!flForward && !flSide)
			return;

		pCmd->forwardmove = 450.f;
		if (flSide)
		{
			Vec3 vWishDir = Math::VectorAngles({ vForward.x * flForward + vRight.x * flSide, vForward.y * flForward + vRight.y * flSide, 0.f });
			pCmd->viewangles.y = vWishDir.y;
			G::PSilentAngles = true;
		}
	}
}

void CMisc::FastMovement(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	if (!pLocal->m_hGroundEntity() || pLocal->InCond(TF_COND_HALLOWEEN_KART))
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

		Vec3 vDirection = pLocal->m_vecVelocity().ToAngle();
		vDirection.y = pCmd->viewangles.y - vDirection.y;
		Vec3 vNegatedDirection = vDirection.FromAngle() * -flSpeed;
		pCmd->forwardmove = vNegatedDirection.x;
		pCmd->sidemove = vNegatedDirection.y;

		break;
	}
	case 1:
	{
		const bool bDucking = pLocal->IsDucking() || (pCmd->buttons & IN_DUCK);
		if ((bDucking ? !Vars::Misc::Movement::DuckSpeed.Value : !Vars::Misc::Movement::FastAccelerate.Value)
			|| F::AntiCheatCompatibility.Active()
			|| G::Attacking == 1 || F::Ticks.m_bDoubletap || F::Ticks.m_bRecharge)
			return;

		if (!(pCmd->buttons & (IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT))
			&& (!bDucking || (!pCmd->forwardmove && !pCmd->sidemove)))
			return;

		bool bChoke = !I::ClientState->chokedcommands && F::Ticks.CanChoke(true);
		if (!bChoke)
			return;

		if (bDucking)
		{
			pCmd->forwardmove *= -1.f;
			pCmd->sidemove *= -1.f;
			pCmd->viewangles.x = 91.f;
		}

		Vec3 vMove = { pCmd->forwardmove, pCmd->sidemove, 0.f };
		Vec3 vAngMoveReverse = Math::VectorAngles(-vMove);
		pCmd->forwardmove = -vMove.Length();
		pCmd->sidemove = 0.f;
		pCmd->viewangles.y = fmodf(pCmd->viewangles.y - vAngMoveReverse.y, 360.f);
		pCmd->viewangles.z = 270.f;
		G::PSilentAngles = true;
		m_bDuckSpeedActive = bDucking;

		break;
	}
	}
}

void CMisc::AutoPeek(CTFPlayer* pLocal, CUserCmd* pCmd, bool bPost)
{
	static bool bReturning = false;

	if (!bPost)
	{
		if (Vars::AutoPeek::Enabled.Value)
		{
			Vec3 vLocalPos = pLocal->m_vecOrigin();

			if (bReturning)
			{
				if (vLocalPos.DistTo2D(m_vPeekReturnPos) < 8.f)
				{
					bReturning = false;
					return;
				}

				SDK::WalkTo(pCmd, pLocal, m_vPeekReturnPos);
				pCmd->buttons &= ~IN_JUMP;
			}
			else if (!pLocal->m_hGroundEntity())
				m_bPeekPlaced = false;

			if (!m_bPeekPlaced)
			{
				m_vPeekReturnPos = vLocalPos;
				m_bPeekPlaced = true;
			}
			else
			{
				static Timer tTimer = {};
				if (tTimer.Run(0.7f))
					H::Particles.DispatchParticleEffect("ping_circle", m_vPeekReturnPos, {});
			}
		}
		else
			m_bPeekPlaced = bReturning = false;
	}
	else if (G::Attacking && m_bPeekPlaced)
		bReturning = true;
}

void CMisc::EdgeJump(CTFPlayer* pLocal, CUserCmd* pCmd, bool bPost)
{
	if (!Vars::Misc::Movement::EdgeJump.Value)
		return;

	static bool bStaticGround = false;
	if (!bPost)
		bStaticGround = pLocal->m_hGroundEntity();
	else if (bStaticGround && !pLocal->m_hGroundEntity())
		pCmd->buttons |= IN_JUMP;
}

void CMisc::Event(IGameEvent* pEvent, uint32_t uHash)
{
	switch (uHash)
	{
	case FNV1A::Hash32Const("client_disconnect"):
	case FNV1A::Hash32Const("client_beginconnect"):
	case FNV1A::Hash32Const("game_newmap"):
		m_vChatSpamLines.clear();
		m_vKillSayLines.clear();
		m_iCurrentChatSpamIndex = 0;
		m_bAutoBalanceTeamChangePending = false;
		ResetBuyBot();

		m_bEdgeBug = m_bEdgeBugCrouch = false;
		m_iEdgeBugMoveStage = EBStageEnum::Normal;
		m_iEdgeBugTicksLeft = m_iEdgeBugTicksTotal = 0;
		m_flEdgeBugStartYaw = m_flEdgeBugYawDelta = 0.f;
		m_vEdgeBugMove.Zero();
		m_vEdgebugPath.clear();
		break;
	case FNV1A::Hash32Const("player_spawn"):
	{
		if (I::EngineClient->GetPlayerForUserID(pEvent->GetInt("userid")) != I::EngineClient->GetLocalPlayer())
			return;

		m_bPeekPlaced = false;

		m_bEdgeBug = m_bEdgeBugCrouch = false;
		m_iEdgeBugMoveStage = EBStageEnum::Normal;
		m_iEdgeBugTicksLeft = m_iEdgeBugTicksTotal = 0;
		m_flEdgeBugStartYaw = m_flEdgeBugYawDelta = 0.f;
		m_vEdgeBugMove.Zero();
		m_vEdgebugPath.clear();
		break;
	case FNV1A::Hash32Const("player_death"):
	{
		const int iLocalPlayer = I::EngineClient->GetLocalPlayer();
		const int iAttacker = I::EngineClient->GetPlayerForUserID(pEvent->GetInt("attacker"));
		const int iVictim = I::EngineClient->GetPlayerForUserID(pEvent->GetInt("userid"));

		if (iVictim == iLocalPlayer
			&& m_bAutoBalanceTeamChangePending
			&& Vars::Misc::Automation::AntiAutobalance.Value == Vars::Misc::Automation::AntiAutobalanceEnum::RetryOnDeath)
		{
			auto pResource = H::Entities.GetResource();
			if (pResource)
			{
				int iRedPlayers = 0, iBluePlayers = 0;
				for (int n = 1; n <= I::EngineClient->GetMaxClients(); n++)
				{
					if (!pResource->m_bValid(n) || !pResource->m_bConnected(n))
						continue;

					switch (pResource->m_iTeam(n))
					{
					case TF_TEAM_RED: iRedPlayers++; break;
					case TF_TEAM_BLUE: iBluePlayers++; break;
					}
				}

				const int iLocalTeam = pResource->m_iTeam(iLocalPlayer);
				if (iLocalTeam == TF_TEAM_RED || iLocalTeam == TF_TEAM_BLUE)
				{
					const int iOurPlayers = iLocalTeam == TF_TEAM_RED ? iRedPlayers : iBluePlayers;
					const int iEnemyPlayers = iLocalTeam == TF_TEAM_RED ? iBluePlayers : iRedPlayers;
					if (iOurPlayers - iEnemyPlayers > 2)
						I::EngineClient->ClientCmd_Unrestricted("retry");
				}
			}
		}

		if (iAttacker == iLocalPlayer && iAttacker != iVictim)
		{
			player_info_t pi;
			if (I::EngineClient->GetPlayerInfo(iVictim, &pi))
				m_sLastKilledName = pi.name;

			DoKillSay(iVictim);
		}

		if (!Vars::Misc::Automation::AutoTaunt.Value)
			break;

		if (iAttacker != iLocalPlayer || iAttacker == iVictim)
			break;

		const int iChance = std::clamp(Vars::Misc::Automation::AutoTauntChance.Value, 0, 100);
		if (!iChance)
			break;

		const auto pLocal = H::Entities.GetLocal();
		if (!pLocal || !pLocal->IsAlive())
			break;

		if (pLocal->IsTaunting() || pLocal->InCond(TF_COND_HALLOWEEN_KART))
			break;

		if (SDK::RandomInt(1, 100) > iChance)
			break;

		if (!Vars::Misc::Automation::AutoTauntSlot.Value)
		{
weapon_taunt:
			I::EngineClient->ClientCmd_Unrestricted("taunt");
			break;
		}

		// Check if we actually have a taunt at that slot
		{
			auto pInventoryManager = I::TFInventoryManager();
			if (!pInventoryManager)
				goto weapon_taunt;

			auto pLocalInventory = pInventoryManager->GetLocalInventory();
			if (!pLocalInventory)
				goto weapon_taunt;

			auto pEconItemView = pLocalInventory->GetItemInLoadout(pLocal->m_iClass(), Vars::Misc::Automation::AutoTauntSlot.Value + 10);
			if (!pEconItemView)
				goto weapon_taunt;

			auto pItemDefinition = pEconItemView->GetStaticData();
			if (!pItemDefinition || !pItemDefinition->m_pTauntData())
				goto weapon_taunt;
		}

		I::EngineClient->ClientCmd_Unrestricted(std::format("taunt {}", Vars::Misc::Automation::AutoTauntSlot.Value).c_str());
		break;
	}
	case FNV1A::Hash32Const("vote_maps_changed"):
		if (Vars::Misc::Automation::AutoVoteMap.Value)
		{
			I::EngineClient->ClientCmd_Unrestricted(std::format("next_map_vote {}", Vars::Misc::Automation::AutoVoteMapOption.Value).c_str());
		}
		break;
	}
	}
}

int CMisc::AntiBackstab(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	if (!Vars::Misc::Automation::AntiBackstab.Value || !G::SendPacket || G::Attacking == 1 || !pLocal || pLocal->m_MoveType() != MOVETYPE_WALK || pLocal->InCond(TF_COND_HALLOWEEN_KART))
		return 0;

	std::vector<std::pair<Vec3, CBaseEntity*>> vTargets = {};
	for (auto pEntity : H::Entities.GetGroup(EntityEnum::PlayerEnemy))
	{
		auto pPlayer = pEntity->As<CTFPlayer>();
		if (pPlayer->IsDormant() || !pPlayer->IsAlive() || pPlayer->IsAGhost() || pPlayer->InCond(TF_COND_STEALTHED))
			continue;

		auto pWeapon = pPlayer->m_hActiveWeapon()->As<CTFWeaponBase>();
		if (!pWeapon
			|| pWeapon->GetWeaponID() != TF_WEAPON_KNIFE
			&& !(G::PrimaryWeaponType == EWeaponType::MELEE && SDK::AttribHookValue(0, "crit_from_behind", pWeapon) > 0)
			&& !(pWeapon->GetWeaponID() == TF_WEAPON_FLAMETHROWER && SDK::AttribHookValue(0, "set_flamethrower_back_crit", pWeapon) == 1)
			|| F::PlayerUtils.IsIgnored(pPlayer->entindex()))
			continue;

		Vec3 vLocalPos = pLocal->GetCenter();
		Vec3 vTargetPos1 = pPlayer->GetCenter();
		Vec3 vTargetPos2 = vTargetPos1 + pPlayer->m_vecVelocity() * F::Backtrack.GetReal();
		float flDistance = std::max(std::max(SDK::MaxSpeed(pPlayer), SDK::MaxSpeed(pLocal)), pPlayer->m_vecVelocity().Length());
		float flDistanceSqr = powf(flDistance, 2);
		if ((vLocalPos.DistToSqr(vTargetPos1) > flDistanceSqr || !SDK::VisPosWorld(pLocal, pPlayer, vLocalPos, vTargetPos1))
			&& (vLocalPos.DistToSqr(vTargetPos2) > flDistanceSqr || !SDK::VisPosWorld(pLocal, pPlayer, vLocalPos, vTargetPos2)))
			continue;

		vTargets.emplace_back(vTargetPos2, pEntity);
	}
	if (vTargets.empty())
		return 0;

	std::sort(vTargets.begin(), vTargets.end(), [&](const auto& a, const auto& b) -> bool
	{
		return pLocal->GetCenter().DistToSqr(a.first) < pLocal->GetCenter().DistToSqr(b.first);
	});

	auto& pTargetPos = vTargets.front();
	switch (Vars::Misc::Automation::AntiBackstab.Value)
	{
	case Vars::Misc::Automation::AntiBackstabEnum::Yaw:
	{
		Vec3 vAngleTo = Math::CalcAngle(pLocal->m_vecOrigin(), pTargetPos.first);
		vAngleTo.x = pCmd->viewangles.x;
		SDK::FixMovement(pCmd, vAngleTo);
		pCmd->viewangles = vAngleTo;

		return 1;
	}
	case Vars::Misc::Automation::AntiBackstabEnum::Pitch:
	case Vars::Misc::Automation::AntiBackstabEnum::Fake:
	{
		bool bCheater = F::PlayerUtils.HasTag(pTargetPos.second->entindex(), F::PlayerUtils.TagToIndex(CHEATER_TAG));
		// if the closest spy is a cheater, assume auto stab is being used, otherwise don't do anything if target is in front
		if (!bCheater)
		{
			auto fTargetIsBehind = [&]()
			{
				const float flCompDist = PLAYER_ORIGIN_COMPRESSION / 2;
				const float flSqCompDist = 0.0884f;

				Vec3 vToTarget = (pLocal->m_vecOrigin() - pTargetPos.first).To2D();
				const float flDist = vToTarget.Normalize();
				if (flDist < flSqCompDist)
					return true;

				const float flExtra = 2.f * flCompDist / flDist; // account for origin compression
				float flPosVsTargetViewMinDot = 0.f - 0.0031f - flExtra;

				Vec3 vTargetForward; Math::AngleVectors(pCmd->viewangles, &vTargetForward);
				vTargetForward.Normalize2D();

				const float flPosVsTargetViewDot = vToTarget.Dot(vTargetForward); // Behind?

				return flPosVsTargetViewDot > flPosVsTargetViewMinDot;
			};

			if (!fTargetIsBehind())
				return 0;
		}

		if (!bCheater || Vars::Misc::Automation::AntiBackstab.Value == Vars::Misc::Automation::AntiBackstabEnum::Pitch)
		{
			pCmd->forwardmove *= -1;
			pCmd->viewangles.x = 269.f;
		}
		else
			pCmd->viewangles.x = 271.f;
		// may slip up some auto backstabs depending on mode, though we are still able to be stabbed

		return 2;
	}
	}

	return 0;
}

void CMisc::PingReducer()
{
	static Timer tTimer = {};
	if (!tTimer.Run(0.1f))
		return;

	auto pNetChan = reinterpret_cast<CNetChannel*>(I::EngineClient->GetNetChannelInfo());
	auto pResource = H::Entities.GetResource();
	if (!pNetChan || !pResource)
		return;

	static auto cl_cmdrate = H::ConVars.FindVar("cl_cmdrate");
	const int iCmdRate = cl_cmdrate->GetInt();
	const int Ping = pResource->m_iPing(I::EngineClient->GetLocalPlayer());
	const int iTarget = Vars::Misc::Exploits::PingReducer.Value && (Ping > Vars::Misc::Exploits::PingTarget.Value) ? -1 : iCmdRate;

	NET_SetConVar cmd("cl_cmdrate", std::to_string(m_iWishCmdrate = iTarget).c_str());
	pNetChan->SendNetMsg(cmd);
}

void CMisc::UnlockAchievements()
{
	const auto pAchievementMgr = I::EngineClient->GetAchievementMgr();
	if (pAchievementMgr)
	{
		I::SteamUserStats->RequestCurrentStats();
		for (int i = 0; i < pAchievementMgr->GetAchievementCount(); i++)
			pAchievementMgr->AwardAchievement(pAchievementMgr->GetAchievementByIndex(i)->GetAchievementID());
		I::SteamUserStats->StoreStats();
		I::SteamUserStats->RequestCurrentStats();
	}
}

void CMisc::UnlockItemAchievements()
{
	const auto pAchievementMgr = I::EngineClient->GetAchievementMgr();
	if (pAchievementMgr)
	{
		const std::unordered_set<int> vItemAchievementIDs(
			Vars::Misc::Automation::GetItemAchievementIDs().begin(),
			Vars::Misc::Automation::GetItemAchievementIDs().end());

		I::SteamUserStats->RequestCurrentStats();
		for (int i = 0; i < pAchievementMgr->GetAchievementCount(); i++)
		{
			if (const auto pAchievement = pAchievementMgr->GetAchievementByIndex(i))
			{
				const int iAchievementID = pAchievement->GetAchievementID();
				if (vItemAchievementIDs.contains(iAchievementID))
					pAchievementMgr->AwardAchievement(iAchievementID);
			}
		}
		I::SteamUserStats->StoreStats();
		I::SteamUserStats->RequestCurrentStats();
	}
}

void CMisc::LockAchievements()
{
	const auto pAchievementMgr = I::EngineClient->GetAchievementMgr();
	if (pAchievementMgr)
	{
		I::SteamUserStats->RequestCurrentStats();
		for (int i = 0; i < pAchievementMgr->GetAchievementCount(); i++)
			I::SteamUserStats->ClearAchievement(pAchievementMgr->GetAchievementByIndex(i)->GetName());
		I::SteamUserStats->StoreStats();
		I::SteamUserStats->RequestCurrentStats();
	}
}

void CMisc::LockItemAchievements()
{
	const auto pAchievementMgr = I::EngineClient->GetAchievementMgr();
	if (pAchievementMgr)
	{
		const std::unordered_set<int> vItemAchievementIDs(
			Vars::Misc::Automation::GetItemAchievementIDs().begin(),
			Vars::Misc::Automation::GetItemAchievementIDs().end());

		I::SteamUserStats->RequestCurrentStats();
		for (int i = 0; i < pAchievementMgr->GetAchievementCount(); i++)
		{
			if (const auto pAchievement = pAchievementMgr->GetAchievementByIndex(i))
			{
				if (vItemAchievementIDs.contains(pAchievement->GetAchievementID()))
					I::SteamUserStats->ClearAchievement(pAchievement->GetName());
			}
		}
		I::SteamUserStats->StoreStats();
		I::SteamUserStats->RequestCurrentStats();
	}
}

void CMisc::AchievementSpam(CTFPlayer* pLocal)
{
	if (!Vars::Misc::Automation::AchievementSpam.Value || !pLocal || !pLocal->IsAlive())
	{
		m_eAchievementSpamState = AchievementSpamState::IDLE;
		return;
	}

	const auto pAchievementMgr = reinterpret_cast<IAchievementMgr * (*)()>(U::Memory.GetVirtual(I::EngineClient, 114))();
	if (!pAchievementMgr)
	{
		m_eAchievementSpamState = AchievementSpamState::IDLE;
		return;
	}

	switch (m_eAchievementSpamState)
	{
	case AchievementSpamState::IDLE:
	{
		if (!m_tAchievementSpamTimer.Run(20.0f))
			return;

		int specificAchievementID = Vars::Misc::Automation::AchievementSpamID.Value;

		IAchievement* pAchievement = nullptr;
		for (int i = 0; i < pAchievementMgr->GetAchievementCount(); i++)
		{
			IAchievement* pCurrentAchievement = pAchievementMgr->GetAchievementByIndex(i);
			if (pCurrentAchievement && pCurrentAchievement->GetAchievementID() == specificAchievementID)
			{
				pAchievement = pCurrentAchievement;
				break;
			}
		}

		if (!pAchievement || !pAchievement->GetName())
		{
			pAchievement = pAchievementMgr->GetAchievementByIndex(0);
			if (!pAchievement || !pAchievement->GetName())
				return;

			specificAchievementID = pAchievement->GetAchievementID();
			Vars::Misc::Automation::AchievementSpamID.Value = specificAchievementID;
		}

		m_iAchievementSpamID = specificAchievementID;
		m_sAchievementSpamName = pAchievement->GetName();
		m_eAchievementSpamState = AchievementSpamState::CLEARING;
		break;
	}
	case AchievementSpamState::CLEARING:
	{
		I::SteamUserStats->RequestCurrentStats();
		I::SteamUserStats->ClearAchievement(m_sAchievementSpamName.c_str());
		I::SteamUserStats->StoreStats();

		m_tAchievementDelayTimer.Update();
		m_eAchievementSpamState = AchievementSpamState::WAITING;
		break;
	}
	case AchievementSpamState::WAITING:
	{
		if (!m_tAchievementDelayTimer.Run(0.1f))
			return;

		m_eAchievementSpamState = AchievementSpamState::AWARDING;
		break;
	}
	case AchievementSpamState::AWARDING:
	{
		I::SteamUserStats->RequestCurrentStats();
		pAchievementMgr->AwardAchievement(m_iAchievementSpamID);
		I::SteamUserStats->StoreStats();

		m_eAchievementSpamState = AchievementSpamState::IDLE;
		break;
	}
	}
}

void CMisc::VoiceCommandSpam(CTFPlayer* pLocal)
{
	if (!Vars::Misc::Automation::VoiceCommandSpam.Value || !pLocal->IsAlive())
		return;

	static float flLastVoiceTime = 0.0f;
	float flCurrentTime = SDK::PlatFloatTime();
	if (flCurrentTime - flLastVoiceTime >= 6.5f) // 6500ms in seconds
	{
		flLastVoiceTime = flCurrentTime;

		switch (Vars::Misc::Automation::VoiceCommandSpam.Value)
		{
		case Vars::Misc::Automation::VoiceCommandSpamEnum::Random:
		{
			int iMenu = SDK::RandomInt(0, 2);
			int iCommand = SDK::RandomInt(0, 8);
			std::string sCmd = "voicemenu " + std::to_string(iMenu) + " " + std::to_string(iCommand);
			I::EngineClient->ClientCmd_Unrestricted(sCmd.c_str());
		}
		break;
		case Vars::Misc::Automation::VoiceCommandSpamEnum::Medic:
			I::EngineClient->ClientCmd_Unrestricted("voicemenu 0 0");
			break;
		case Vars::Misc::Automation::VoiceCommandSpamEnum::Thanks:
			I::EngineClient->ClientCmd_Unrestricted("voicemenu 0 1");
			break;
		case Vars::Misc::Automation::VoiceCommandSpamEnum::NiceShot:
			I::EngineClient->ClientCmd_Unrestricted("voicemenu 2 6");
			break;
		case Vars::Misc::Automation::VoiceCommandSpamEnum::Cheers:
			I::EngineClient->ClientCmd_Unrestricted("voicemenu 2 2");
			break;
		case Vars::Misc::Automation::VoiceCommandSpamEnum::Jeers:
			I::EngineClient->ClientCmd_Unrestricted("voicemenu 2 3");
			break;
		case Vars::Misc::Automation::VoiceCommandSpamEnum::GoGoGo:
			I::EngineClient->ClientCmd_Unrestricted("voicemenu 0 2");
			break;
		case Vars::Misc::Automation::VoiceCommandSpamEnum::MoveUp:
			I::EngineClient->ClientCmd_Unrestricted("voicemenu 0 3");
			break;
		case Vars::Misc::Automation::VoiceCommandSpamEnum::GoLeft:
			I::EngineClient->ClientCmd_Unrestricted("voicemenu 0 4");
			break;
		case Vars::Misc::Automation::VoiceCommandSpamEnum::GoRight:
			I::EngineClient->ClientCmd_Unrestricted("voicemenu 0 5");
			break;
		case Vars::Misc::Automation::VoiceCommandSpamEnum::Yes:
			I::EngineClient->ClientCmd_Unrestricted("voicemenu 0 6");
			break;
		case Vars::Misc::Automation::VoiceCommandSpamEnum::No:
			I::EngineClient->ClientCmd_Unrestricted("voicemenu 0 7");
			break;
		case Vars::Misc::Automation::VoiceCommandSpamEnum::Incoming:
			I::EngineClient->ClientCmd_Unrestricted("voicemenu 1 0");
			break;
		case Vars::Misc::Automation::VoiceCommandSpamEnum::Spy:
			I::EngineClient->ClientCmd_Unrestricted("voicemenu 1 1");
			break;
		case Vars::Misc::Automation::VoiceCommandSpamEnum::Sentry:
			I::EngineClient->ClientCmd_Unrestricted("voicemenu 1 2");
			break;
		case Vars::Misc::Automation::VoiceCommandSpamEnum::NeedTeleporter:
			I::EngineClient->ClientCmd_Unrestricted("voicemenu 1 3");
			break;
		case Vars::Misc::Automation::VoiceCommandSpamEnum::Pootis:
			I::EngineClient->ClientCmd_Unrestricted("voicemenu 1 4");
			break;
		case Vars::Misc::Automation::VoiceCommandSpamEnum::NeedSentry:
			I::EngineClient->ClientCmd_Unrestricted("voicemenu 1 5");
			break;
		case Vars::Misc::Automation::VoiceCommandSpamEnum::ActivateCharge:
			I::EngineClient->ClientCmd_Unrestricted("voicemenu 1 6");
			break;
		case Vars::Misc::Automation::VoiceCommandSpamEnum::Help:
			I::EngineClient->ClientCmd_Unrestricted("voicemenu 2 0");
			break;
		case Vars::Misc::Automation::VoiceCommandSpamEnum::BattleCry:
			I::EngineClient->ClientCmd_Unrestricted("voicemenu 2 1");
			break;
		case Vars::Misc::Automation::VoiceCommandSpamEnum::GoodJob:
			I::EngineClient->ClientCmd_Unrestricted("voicemenu 2 7");
			break;
		}
	}
}

void CMisc::AutoReport()
{
	if (!Vars::Misc::Automation::AutoReport.Value)
		return;

	static Timer tReportTimer{};
	if (!tReportTimer.Run(5.0f))
		return;

	int iLocalIdx = I::EngineClient->GetLocalPlayer();
	for (int i = 1; i <= I::EngineClient->GetMaxClients(); i++)
	{
		if (i == iLocalIdx)
			continue;

		if (auto uSteamId = F::PlayerUtils.GetAccountID(i))
		{
			if (H::Entities.IsFriend(i) ||
				H::Entities.InParty(i) ||
				F::PlayerUtils.IsIgnored(i) ||
				F::PlayerUtils.HasTag(i, F::PlayerUtils.TagToIndex(IGNORED_TAG)) ||
				F::PlayerUtils.HasTag(i, F::PlayerUtils.TagToIndex(FRIEND_TAG)))
				continue;

			uint64_t uSteamID64 = ((uint64_t)1 << 56) | ((uint64_t)1 << 52) | ((uint64_t)1 << 32) | uSteamId;
			S::ReportPlayerAccount.Call<bool>(uSteamID64, 1);
		}
	}
}

void CMisc::AutoRetry(CTFPlayer* pLocal)
{
	if (!Vars::Misc::Automation::AutoRetry.Value || !pLocal || !pLocal->IsAlive())
		return;

	const int max_health = pLocal->GetMaxHealth();
	const float health_percent = max_health ? static_cast<float>(pLocal->m_iHealth()) / max_health * 100.f : 100.f;
	if (health_percent >= Vars::Misc::Automation::AutoRetryHealth.Value)
		return;

	static Timer tRetryTimer{};
	if (!tRetryTimer.Run(5.0f))
		return;

	I::EngineClient->ClientCmd_Unrestricted("retry");
}


void CMisc::ChatSpam(CTFPlayer* pLocal)
{
	auto ResetChatTimer = [&]()
		{
			m_tChatSpamTimer.Update();
		};

	if (!Vars::Misc::Automation::ChatSpam::Enable.Value)
	{
		ResetChatTimer();
		return;
	}

	if (!pLocal->IsAlive() || !pLocal->IsInValidTeam() || pLocal->m_iClass() == TF_CLASS_UNDEFINED)
	{
		ResetChatTimer();
		return;
	}

	static Timer tReloadTimer{};
	auto EnsureChatLinesLoaded = [&]() -> bool
		{
			size_t uSize = m_vChatSpamLines.size();
			if (uSize > 0 && !tReloadTimer.Run(5.0f))
				return true;

			static const char* szDefaultContent =
				"This is a default message from cat_chatspam.txt\n"
				"Edit this file unibox/cat_chatspam.txt\n"
				"Each line will be sent as a separate message\n"
				"[unibox] Chat Spam is working!\n"
				"Put your chat spam lines in this file\n";

			if (LoadLines("cat_chatspam.txt", m_vChatSpamLines, szDefaultContent))
			{
				// Reset index if number of lines changed
				if (uSize != m_vChatSpamLines.size())
					m_iCurrentChatSpamIndex = 0;
				return true;
			}

			m_vChatSpamLines = {
				"Put your chat spam lines in unibox/cat_chatspam.txt",
				"ChatSpam is running but couldn't find unibox/cat_chatspam.txt",
				"[unibox] Chat Spam is working!"
			};
			m_iCurrentChatSpamIndex = 0;
			return false;
		};

	if (!EnsureChatLinesLoaded())
		return;

	float flSpamInterval = Vars::Misc::Automation::ChatSpam::Interval.Value;
	if (flSpamInterval < 0.2f)
		flSpamInterval = 0.2f;

	if (!m_tChatSpamTimer.Run(flSpamInterval))
		return;

	auto FetchNextChatLine = [&]() -> std::string
		{
			const size_t uLineCount = m_vChatSpamLines.size();
			if (!uLineCount)
				return {};

			if (Vars::Misc::Automation::ChatSpam::Randomize.Value)
			{
				int iMax = static_cast<int>(uLineCount) - 1;
				if (iMax < 0)
					iMax = 0;
				const int iRandomIndex = SDK::RandomInt(0, iMax);
				if (iRandomIndex >= 0 && iRandomIndex < static_cast<int>(uLineCount))
					return m_vChatSpamLines[iRandomIndex];
				return "[ChatSpam]";
			}

			if (m_iCurrentChatSpamIndex < 0 || m_iCurrentChatSpamIndex >= static_cast<int>(uLineCount))
				m_iCurrentChatSpamIndex = 0;

			const std::string& sLine = m_vChatSpamLines[m_iCurrentChatSpamIndex];
			m_iCurrentChatSpamIndex = (m_iCurrentChatSpamIndex + 1) % static_cast<int>(uLineCount);
			return sLine;
		};

	std::string sChatLine = FetchNextChatLine();
	if (sChatLine.empty())
		return;

	sChatLine = ReplaceTags(sChatLine);

	if (sChatLine.length() > 150)
		sChatLine.resize(150);

	std::string sChatCommand;
	if (Vars::Misc::Automation::ChatSpam::TeamChat.Value)
		sChatCommand = "say_team \"" + sChatLine + "\"";
	else
		sChatCommand = "say \"" + sChatLine + "\"";

	SDK::Output("ChatSpam", std::format("Sending: {}", sChatCommand).c_str(), {}, OUTPUT_CONSOLE | OUTPUT_DEBUG);
	I::EngineClient->ClientCmd_Unrestricted(sChatCommand.c_str());
}

void CMisc::EnsureChatUtilsDoc()
{
	static bool bWritten = false;
	if (bWritten)
		return;
	bWritten = true;

	const std::string sPath = F::Configs.m_sConfigPath + "chatutils_readme.txt";
	if (std::filesystem::exists(sPath))
		return;

	static const char* szDoc =
		"================================================================\n"
		"  ChatUtils Documentation\n"
		"================================================================\n"
		"\n"
		"All chat-related text features read plain .txt files from this\n"
		"cfg folder. Each file is auto-generated with examples the first\n"
		"time the corresponding feature runs. Lines starting with // and\n"
		"empty lines are ignored. Edit, save, and the file is reloaded\n"
		"automatically (within ~5 seconds).\n"
		"\n"
		"----------------------------------------------------------------\n"
		"  Files\n"
		"----------------------------------------------------------------\n"
		"\n"
		"  cat_chatspam.txt   Lines sent on a timer by Chat Spam.\n"
		"                     One message per line.\n"
		"\n"
		"  killsay.txt        Lines sent when YOU kill another player.\n"
		"                     One message per line, picked at random.\n"
		"\n"
		"  votekick.txt       Replies fired when a vote kick starts.\n"
		"                     Prefix each line with `F1:` or `F2:`.\n"
		"                       F1: used when a friend/you call the vote\n"
		"                       F2: used when someone votes on a friend/you\n"
		"\n"
		"  autoreply.txt      Trigger-based replies in public chat.\n"
		"                     Format:  trigger1, trigger2 : reply1, reply2\n"
		"                     Triggers are matched case-insensitively as\n"
		"                     substrings; one reply is chosen at random.\n"
		"\n"
		"----------------------------------------------------------------\n"
		"  Tags\n"
		"----------------------------------------------------------------\n"
		"\n"
		"  {killer}        Your in-game name (KillSay).\n"
		"  {victim}        Name of the player you just killed (KillSay).\n"
		"  {target}        Vote-kick target / autoreply sender.\n"
		"  {triggername}   Same thing as {target}.\n"
		"  {initiator}     Player who called the current vote.\n"
		"  {enemyteam}     `RED` or `BLU` - the opposite of your team.\n"
		"  {friendlyteam}  `RED` or `BLU` - your team.\n"
		"  {lastkilled}    Name of the last player you killed. (Global)\n"
		"  {highestscore}  Name of the player with the highest score.\n"
		"  {random}        Random non-bot player on the server.\n"
		"  {friend}        Random player tagged FRIEND.\n"
		"  {ignored}       Random player tagged IGNORED.\n"
		"\n"
		"Tags only resolve when the relevant data is available - e.g.\n"
		"{killer}/{victim} are empty outside KillSay, {target} is empty\n"
		"outside vote replies / auto-replies.\n"
		"\n"
		"----------------------------------------------------------------\n"
		"  PS\n"
		"----------------------------------------------------------------\n"
		"\n"
		"  - Messages are truncated to 150 characters after tag expansion.\n"
		"  - Kill Say honours the Chance slider in the menu.\n"
		"  - Use Team Chat toggles in the menu to switch say -> say_team.\n"
		"\n"
		"================================================================\n";

	std::ofstream file(sPath);
	if (file.good())
		file << szDoc;
}

void CMisc::DoKillSay(int iVictim)
{
	if (!Vars::Misc::Automation::KillSay::Enable.Value)
		return;

	const int iChance = std::clamp(Vars::Misc::Automation::KillSay::Chance.Value, 0, 100);
	if (!iChance || SDK::RandomInt(1, 100) > iChance)
		return;

	static Timer tReloadTimer{};
	if (m_vKillSayLines.empty() || tReloadTimer.Run(5.0f))
	{
		static const char* szDefaultContent =
			"// Kill Say configuration\n"
			"// Each non-empty, non-// line is a possible message sent when you get a kill.\n"
			"// Supports tags: {killer}, {victim}, {enemyteam}, {friendlyteam}, {random}, {friend}, {ignored}, {highestscore}, {lastkilled}\n"
			"SO FUHIN EASY!!! GET GOOD, GET UNIBOX!\n"
			"{victim} YOURE SO TRASH!\n"
			"WHAT??? YOU DIED TO ME, {killer}??? HOW COULD YOU BE THIS BAD??? {enemyteam} IS CARRYING YOU SO BAD!!!\n"
			"{friendlyteam} IS SUPERIOR, JUST LIKE {killer} AND UNIBOX! UNLIKE {victim} AND {enemyteam}!\n"
			"I AM THE FUCKING SPECTRE\n";
		LoadLines("killsay.txt", m_vKillSayLines, szDefaultContent);
	}

	if (m_vKillSayLines.empty())
		return;

	std::string sVictim;
	{
		player_info_t pi;
		if (I::EngineClient->GetPlayerInfo(iVictim, &pi))
			sVictim = pi.name;
	}

	std::string sKiller;
	{
		player_info_t pi;
		if (I::EngineClient->GetPlayerInfo(I::EngineClient->GetLocalPlayer(), &pi))
			sKiller = pi.name;
	}

	const int iIndex = SDK::RandomInt(0, static_cast<int>(m_vKillSayLines.size()) - 1);
	std::string sLine = ReplaceTags(m_vKillSayLines[iIndex], "", "", sKiller, sVictim);

	if (sLine.length() > 150)
		sLine.resize(150);

	std::string sCommand = Vars::Misc::Automation::KillSay::TeamChat.Value
		? "say_team \"" + sLine + "\""
		: "say \"" + sLine + "\"";

	SDK::Output("KillSay", std::format("Sending: {}", sCommand).c_str(), {}, OUTPUT_CONSOLE | OUTPUT_DEBUG);
	I::EngineClient->ClientCmd_Unrestricted(sCommand.c_str());
}

void CMisc::AutoMvmReadyUp()
{
	if (!Vars::Misc::MannVsMachine::AutoMvmReadyUp.Value)
		return;

	auto pLocal = H::Entities.GetLocal();
	if (!pLocal)
		return;

	auto pGameRules = I::TFGameRules();
	if (!pGameRules)
		return;

	if (!pGameRules->m_bPlayingMannVsMachine() ||
		!pGameRules->m_bInWaitingForPlayers() ||
		pGameRules->m_iRoundState() != GR_STATE_BETWEEN_RNDS)
		return;

	const int iLocalIndex = pLocal->entindex();
	if (iLocalIndex < 0 || iLocalIndex >= 100)
		return;

	if (!pGameRules->IsPlayerReady(iLocalIndex))
		I::EngineClient->ClientCmd_Unrestricted("tournament_player_readystate 1");
}

void CMisc::BuyBotJoinClass(int iClass)
{
	static const std::array<const char*, 10> aClassNames = { "", "scout", "sniper", "soldier", "demoman", "medic", "heavyweapons", "pyro", "spy", "engineer" };
	if (iClass <= TF_CLASS_UNDEFINED || iClass >= TF_CLASS_COUNT || iClass == TF_CLASS_CIVILIAN)
		return;

	float flCurTime = I::GlobalVars->curtime;
	if (m_flBuybotClassClock > flCurTime)
		return;

	I::EngineClient->ClientCmd_Unrestricted(std::format("joinclass {}", aClassNames[iClass]).c_str());
	I::EngineClient->ClientCmd_Unrestricted("menuclosed");
	m_flBuybotClassClock = flCurTime + 1.0f;
}

bool CMisc::BuyBotWalkAwayFromStation(CTFPlayer* pLocal, CUserCmd* pCmd, const Vec3& vStation)
{
	if (!pLocal || !pCmd || vStation.IsZero())
		return false;

	if (m_bBuybotUsingNav)
		F::NavEngine.CancelPath();

	Vec3 vDirection = pLocal->GetAbsOrigin() - vStation;
	vDirection.z = 0.0f;
	if (vDirection.IsZero())
		vDirection = { 1.0f, 0.0f, 0.0f };
	vDirection.Normalize();

	SDK::WalkTo(pCmd, pLocal, pLocal->GetAbsOrigin() + vDirection * 320.0f);
	m_bBuybotUsingNav = false;
	m_flBuybotStationPathStart = 0.0f;
	return true;
}

void CMisc::ExecBuyBot(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	if (!Vars::Misc::MannVsMachine::BuyBot.Value)
	{
		if (m_bBuybotUsingNav)
			F::NavEngine.CancelPath();
		ResetBuyBot();
		return;
	}

	auto pGameRules = I::TFGameRules();
	if (!pGameRules || !pGameRules->m_bPlayingMannVsMachine())
	{
		if (m_bBuybotUsingNav)
			F::NavEngine.CancelPath();
		ResetBuyBot();
		return;
	}

	if (Vars::Misc::MannVsMachine::MaxCash.Value > 0 && pLocal->m_nCurrency() >= Vars::Misc::MannVsMachine::MaxCash.Value)
		m_bBuybotCashLimitReached = true;

	const Vec3 vLocalOrigin = pLocal->GetAbsOrigin();
	bool bFoundStation = false;
	float flBestDist = FLT_MAX;
	Vec3 vBestStation = {};

	for (const auto& tTrigger : G::TriggerStorage)
	{
		if (tTrigger.m_eType != TriggerTypeEnum::UpgradeStation)
			continue;

		Vec3 vStation = tTrigger.m_vCenter.IsZero() ? tTrigger.m_vOrigin : tTrigger.m_vCenter;
		float flDist = vLocalOrigin.DistToSqr(vStation);
		if (flDist >= flBestDist)
			continue;

		flBestDist = flDist;
		vBestStation = vStation;
		bFoundStation = true;
	}

	int iDesiredClass = 0;
	if (Vars::Misc::MannVsMachine::BuyBotAutoClass.Value)
	{
		if (!m_bBuybotCashLimitReached)
			iDesiredClass = TF_CLASS_MEDIC;
		else if (pLocal->m_iClass() == TF_CLASS_MEDIC)
		{
			iDesiredClass = Vars::Misc::MannVsMachine::BuyBotClass.Value;
			if (iDesiredClass == TF_CLASS_MEDIC)
				iDesiredClass = TF_CLASS_HEAVY;
		}
	}

	if (iDesiredClass > TF_CLASS_UNDEFINED && iDesiredClass < TF_CLASS_COUNT && pLocal->m_iClass() != iDesiredClass)
	{
		if (pLocal->m_bInUpgradeZone() && BuyBotWalkAwayFromStation(pLocal, pCmd, vBestStation))
			return;

		BuyBotJoinClass(iDesiredClass);
		return;
	}

	const bool bWaitingForMedicClass = Vars::Misc::MannVsMachine::BuyBotAutoClass.Value && !m_bBuybotCashLimitReached && pLocal->m_iClass() != TF_CLASS_MEDIC;
	if (m_bBuybotFinishedUpgrades)
	{
		if (m_bBuybotUsingNav)
			F::NavEngine.CancelPath();
		m_flBuybotStationPathStart = 0.0f;
		m_bBuybotUsingNav = false;
		return;
	}

	if (bFoundStation)
	{
		if (m_vBuybotStationTarget.IsZero() || m_vBuybotStationTarget.DistToSqr(vBestStation) > 4096.0f)
		{
			m_vBuybotStationTarget = vBestStation;
			m_flBuybotStationPathStart = I::GlobalVars->curtime;
			m_bBuybotUsingNav = false;
		}

		if (!pLocal->m_bInUpgradeZone() && pLocal->IsAlive() && !pLocal->IsAGhost() && pLocal->m_MoveType() == MOVETYPE_WALK && !pLocal->IsSwimming() && !pLocal->IsTaunting())
		{
			const float flCurTime = I::GlobalVars->curtime;
			if (m_flBuybotStationPathStart == 0.0f)
				m_flBuybotStationPathStart = flCurTime;

			if (flCurTime - m_flBuybotStationPathStart >= 12.0f)
			{
				if (!m_bBuybotUsingNav || m_flBuybotNavClock <= flCurTime || m_vBuybotStationTarget.DistToSqr(vBestStation) > 4096.0f)
				{
					F::NavEngine.NavTo(vBestStation, PriorityListEnum::Forced, true, !F::NavEngine.IsPathing());
					m_flBuybotNavClock = flCurTime + 0.5f;
				}
				m_bBuybotUsingNav = true;
			}
			else if (pCmd)
				SDK::WalkTo(pCmd, pLocal, vBestStation);
		}
	}
	else
	{
		m_flBuybotStationPathStart = 0.0f;
		m_bBuybotUsingNav = false;
		m_vBuybotStationTarget = {};
	}

	if (!pLocal->m_bInUpgradeZone())
		return;

	if (m_bBuybotUsingNav)
		F::NavEngine.CancelPath();

	m_flBuybotStationPathStart = 0.0f;
	m_bBuybotUsingNav = false;

	static auto tfMvmRespec = H::ConVars.FindVar("tf_mvm_respec_enabled");
	float flCurTime = I::GlobalVars->curtime;
	if (m_flBuybotClock > flCurTime)
		return;

	if (bWaitingForMedicClass)
		return;

	if (m_bBuybotCashLimitReached)
	{
		if (pLocal->m_iClass() == TF_CLASS_MEDIC)
		{
			m_flBuybotClock = flCurTime + 0.2f;
			return;
		}

		constexpr int iMaxUpgradeIndex = 128;
		constexpr int iMaxUpgradeLevels = 10;
		const int aSlots[] = { 0, -1 };
		const int iSlot = aSlots[m_iBuybotUpgradeSlotStep % std::size(aSlots)];

		I::EngineClient->ServerCmdKeyValues(new KeyValues("MvM_UpgradesBegin"));
		for (int iLevel = 0; iLevel < iMaxUpgradeLevels; iLevel++)
		{
			KeyValues* kv = new KeyValues("MVM_Upgrade");
			KeyValues* sub = kv->FindKey("Upgrade", true);
			sub->SetInt("itemslot", iSlot);
			sub->SetInt("upgrade", m_iBuybotUpgradeIndex);
			sub->SetInt("count", 1);
			I::EngineClient->ServerCmdKeyValues(kv);
		}
		{
			KeyValues* kv = new KeyValues("MvM_UpgradesDone");
			kv->SetInt("num_upgrades", iMaxUpgradeLevels);
			I::EngineClient->ServerCmdKeyValues(kv);
		}

		m_iBuybotUpgradeIndex++;
		if (m_iBuybotUpgradeIndex >= iMaxUpgradeIndex)
		{
			m_iBuybotUpgradeIndex = 0;
			m_iBuybotUpgradeSlotStep++;
			if (m_iBuybotUpgradeSlotStep >= static_cast<int>(std::size(aSlots)))
			{
				m_bBuybotFinishedUpgrades = true;
				m_bBuybotUsingNav = false;
				m_vBuybotStationTarget = {};
			}
		}

		m_flBuybotClock = flCurTime + 0.05f;
		return;
	}

	m_iBuybotUpgradeSlotStep = 0;
	m_iBuybotUpgradeIndex = 0;

	if (tfMvmRespec->GetInt() != 1)
		return;

	switch (m_iBuybotStep)
	{
	case 1:
		I::EngineClient->ServerCmdKeyValues(new KeyValues("MvM_UpgradesBegin"));
		{
			KeyValues* kv = new KeyValues("MVM_Upgrade");
			KeyValues* sub = kv->FindKey("Upgrade", true);
			sub->SetInt("itemslot", 1);
			sub->SetInt("Upgrade", 19);
			sub->SetInt("count", 1);
			I::EngineClient->ServerCmdKeyValues(kv);
		}
		{
			KeyValues* kv = new KeyValues("MVM_Upgrade");
			KeyValues* sub = kv->FindKey("Upgrade", true);
			sub->SetInt("itemslot", 1);
			sub->SetInt("Upgrade", 19);
			sub->SetInt("count", 1);
			I::EngineClient->ServerCmdKeyValues(kv);
		}
		{
			KeyValues* kv = new KeyValues("MvM_UpgradesDone");
			kv->SetInt("num_upgrades", 2);
			I::EngineClient->ServerCmdKeyValues(kv);
		}
		break;
	case 2:
		I::EngineClient->ServerCmdKeyValues(new KeyValues("MvM_UpgradesBegin"));
		{
			KeyValues* kv = new KeyValues("MVM_Upgrade");
			KeyValues* sub = kv->FindKey("Upgrade", true);
			sub->SetInt("itemslot", 1);
			sub->SetInt("Upgrade", 19);
			sub->SetInt("count", -1);
			I::EngineClient->ServerCmdKeyValues(kv);
		}
		{
			KeyValues* kv = new KeyValues("MVM_Upgrade");
			KeyValues* sub = kv->FindKey("Upgrade", true);
			sub->SetInt("itemslot", 1);
			sub->SetInt("Upgrade", 19);
			sub->SetInt("count", 1);
			I::EngineClient->ServerCmdKeyValues(kv);
		}
		I::EngineClient->ServerCmdKeyValues(new KeyValues("MVM_Respec"));
		{
			KeyValues* kv = new KeyValues("MvM_UpgradesDone");
			kv->SetInt("num_upgrades", -1);
			I::EngineClient->ServerCmdKeyValues(kv);
		}
		break;
	case 3:
		I::EngineClient->ServerCmdKeyValues(new KeyValues("MvM_UpgradesBegin"));
		{
			KeyValues* kv = new KeyValues("MVM_Upgrade");
			KeyValues* sub = kv->FindKey("Upgrade", true);
			sub->SetInt("itemslot", 1);
			sub->SetInt("Upgrade", 19);
			sub->SetInt("count", 1);
			I::EngineClient->ServerCmdKeyValues(kv);
		}
		{
			KeyValues* kv = new KeyValues("MVM_Upgrade");
			KeyValues* sub = kv->FindKey("Upgrade", true);
			sub->SetInt("itemslot", 1);
			sub->SetInt("Upgrade", 19);
			sub->SetInt("count", 1);
			I::EngineClient->ServerCmdKeyValues(kv);
		}
		{
			KeyValues* kv = new KeyValues("MVM_Upgrade");
			KeyValues* sub = kv->FindKey("Upgrade", true);
			sub->SetInt("itemslot", 1);
			sub->SetInt("Upgrade", 19);
			sub->SetInt("count", -1);
			I::EngineClient->ServerCmdKeyValues(kv);
		}
		{
			KeyValues* kv = new KeyValues("MVM_Upgrade");
			KeyValues* sub = kv->FindKey("Upgrade", true);
			sub->SetInt("itemslot", 1);
			sub->SetInt("Upgrade", 19);
			sub->SetInt("count", -1);
			I::EngineClient->ServerCmdKeyValues(kv);
		}
		{
			KeyValues* kv = new KeyValues("MvM_UpgradesDone");
			kv->SetInt("num_upgrades", 0);
			I::EngineClient->ServerCmdKeyValues(kv);
		}
		break;
	}
	m_iBuybotStep = m_iBuybotStep % 3 + 1;
	m_flBuybotClock = flCurTime + 0.2f;
}

void CMisc::ResetBuyBot()
{
	m_iBuybotStep = 1;
	m_iBuybotUpgradeSlotStep = 0;
	m_iBuybotUpgradeIndex = 0;
	m_flBuybotClock = 0.0f;
	m_flBuybotStationPathStart = 0.0f;
	m_flBuybotNavClock = 0.0f;
	m_flBuybotClassClock = 0.0f;
	m_flBuybotRetryClock = 0.0f;
	m_bBuybotUsingNav = false;
	m_bBuybotCashLimitReached = false;
	m_bBuybotFinishedUpgrades = false;
	m_vBuybotStationTarget = {};
}

void CMisc::OnBuyBotClassChangeBlocked()
{
	if (!Vars::Misc::MannVsMachine::BuyBot.Value || !Vars::Misc::MannVsMachine::BuyBotAutoClass.Value)
		return;

	auto pGameRules = I::TFGameRules();
	if (!pGameRules || !pGameRules->m_bPlayingMannVsMachine() || pGameRules->m_iRoundState() == GR_STATE_RND_RUNNING)
		return;

	auto pObjectiveResource = H::Entities.GetObjectiveResource();
	if (pObjectiveResource && !pObjectiveResource->m_bMannVsMachineBetweenWaves() && pGameRules->m_iRoundState() != GR_STATE_BETWEEN_RNDS)
		return;

	const float flCurTime = I::GlobalVars->curtime;
	if (m_flBuybotRetryClock > flCurTime)
		return;

	I::EngineClient->ClientCmd_Unrestricted("retry");
	m_flBuybotRetryClock = flCurTime + 10.0f;
}

void CMisc::MicSpam()
{
	static bool bShouldRestore = false;
	static Timer tRecordTimer = {};

	if (Vars::Misc::Automation::Micspam.Value)
	{
		if (I::EngineClient->IsInGame() && tRecordTimer.Run(10.0f))
		{
			I::EngineClient->ClientCmd_Unrestricted("+voicerecord");
			I::EngineClient->ClientCmd_Unrestricted("voice_avggain 1");
#ifdef TEXTMODE
			I::EngineClient->ClientCmd_Unrestricted("volume 0");
			I::EngineClient->ClientCmd_Unrestricted("voice_enable 1");
			I::EngineClient->ClientCmd_Unrestricted("voice_loopback 0");
#endif
		}

		bShouldRestore = true;
	}
	else if (bShouldRestore)
	{
		if (S::Voice_IsRecording.Call<bool>())
		{
			I::EngineClient->ClientCmd_Unrestricted("-voicerecord");
#ifdef TEXTMODE
			I::EngineClient->ClientCmd_Unrestricted("volume 0");
			I::EngineClient->ClientCmd_Unrestricted("voice_enable 0");
			I::EngineClient->ClientCmd_Unrestricted("voice_loopback 0");
#endif
		}

		bShouldRestore = false;
	}
}

CMisc::ProfileDumpResult_t CMisc::DumpProfiles(bool bAnnounce)
{
	ProfileDumpResult_t tResult{};
	auto pResource = H::Entities.GetResource();
	if (!pResource)
	{
		if (bAnnounce)
			SDK::Output("ProfileScraper", "Player resource unavailable");
		return tResult;
	}
	tResult.m_bResourceAvailable = true;

	auto SanitizeName = [](const char* sRaw) -> std::string
		{
			if (!sRaw)
				return {};

			std::string sClean;
			sClean.reserve(std::strlen(sRaw));
			for (unsigned char c : std::string_view{ sRaw })
			{
				if (c < 32 || c > 126)
					continue;
				if (c == ',')
					return {};
				sClean.push_back(static_cast<char>(c));
			}
			return sClean;
		};

	struct ProfileEntry_t
	{
		uint32_t m_uAccountID = 0;
		std::string m_sName = {};
	};

	std::vector<ProfileEntry_t> vProfiles;
	std::unordered_set<uint32_t> setSessionAccounts;
	vProfiles.reserve(I::EngineClient->GetMaxClients());
	setSessionAccounts.reserve(I::EngineClient->GetMaxClients());

	const int iLocalPlayer = I::EngineClient->GetLocalPlayer();
	for (int n = 1; n <= I::EngineClient->GetMaxClients(); n++)
	{
		if (n == iLocalPlayer)
			continue;

		if (!pResource->m_bValid(n) || !pResource->m_bConnected(n) || pResource->IsFakePlayer(n))
			continue;

		tResult.m_uCandidateCount++;

		const uint32_t uAccountID = pResource->m_iAccountID(n);
		if (!uAccountID)
		{
			tResult.m_uSkippedInvalid++;
			continue;
		}

		const char* pszName = pResource->GetName(n);
		if (!pszName)
		{
			tResult.m_uSkippedInvalid++;
			continue;
		}

		if (std::strchr(pszName, ','))
		{
			tResult.m_uSkippedComma++;
			continue;
		}

		std::string sClean = SanitizeName(pszName);
		if (sClean.empty() || sClean == "  " || sClean == "ERRORNAME")
		{
			tResult.m_uSkippedInvalid++;
			continue;
		}

		if (!setSessionAccounts.emplace(uAccountID).second)
		{
			tResult.m_uSkippedSessionDuplicate++;
			continue;
		}

		vProfiles.push_back(ProfileEntry_t{ uAccountID, std::move(sClean) });
	}

	if (!tResult.m_uCandidateCount)
	{
		if (bAnnounce)
			SDK::Output("ProfileScraper", "No player profiles found");
		return tResult;
	}

	if (vProfiles.empty())
	{
		if (bAnnounce)
		{
			const char* pszReason = tResult.m_uSkippedComma ? "All player names contained commas" : "No valid player profiles to save";
			SDK::Output("ProfileScraper", pszReason);
		}
		return tResult;
	}

	auto sPath = std::filesystem::current_path() / "unibox" / "profiles.csv";
	std::error_code ec;
	std::filesystem::create_directories(sPath.parent_path(), ec);

	std::unordered_set<uint64_t> setExistingIDs;
	setExistingIDs.reserve(vProfiles.size() * 2);

	bool bAppendNewline = false;
	if (std::filesystem::exists(sPath))
	{
		std::ifstream input(sPath);
		if (input)
		{
			tResult.m_bFileOpened = true;
			std::string sLine;
			while (std::getline(input, sLine))
			{
				if (sLine.empty())
					continue;
				auto iComma = sLine.find(',');
				if (iComma == std::string::npos)
					continue;
				try
				{
					uint64_t uExisting = std::stoull(sLine.substr(0, iComma));
					setExistingIDs.emplace(uExisting);
				}
				catch (...)
				{
					continue;
				}
			}

			input.clear();
			input.seekg(0, std::ios::end);
			bAppendNewline = input.tellg() > 0;
		}
		else if (bAnnounce)
			SDK::Output("ProfileScraper", std::format("Failed to read existing profiles from {}", sPath.string()).c_str());
	}

	struct ProfileLine_t
	{
		uint64_t m_uSteamID64 = 0;
		std::string m_sName = {};
	};

	std::vector<ProfileLine_t> vNewProfiles;
	vNewProfiles.reserve(vProfiles.size());
	for (const auto& tEntry : vProfiles)
	{
		const uint64_t uSteamID64 = CSteamID(tEntry.m_uAccountID, k_EUniversePublic, k_EAccountTypeIndividual).ConvertToUint64();
		if (setExistingIDs.contains(uSteamID64))
		{
			tResult.m_uSkippedFileDuplicate++;
			continue;
		}

		setExistingIDs.emplace(uSteamID64);
		vNewProfiles.push_back({ uSteamID64, tEntry.m_sName });
	}

	tResult.m_uAppendedCount = vNewProfiles.size();
	tResult.m_outputPath = sPath;

	if (!vNewProfiles.empty())
	{
		std::ofstream file(sPath, std::ios::app);
		if (!file)
		{
			if (bAnnounce)
				SDK::Output("ProfileScraper", std::format("Failed to open {}", sPath.string()).c_str());
			return tResult;
		}

		tResult.m_bFileOpened = true;
		if (bAppendNewline)
			file << '\n';

		for (size_t i = 0; i < vNewProfiles.size(); i++)
		{
			if (i)
				file << '\n';
			file << vNewProfiles[i].m_uSteamID64 << ',' << vNewProfiles[i].m_sName;
		}

		if (!file.good())
		{
			if (bAnnounce)
				SDK::Output("ProfileScraper", "Failed to write profiles");
			return tResult;
		}

		tResult.m_bSuccess = true;
	}
	else if (bAnnounce)
	{
		const size_t uDuplicateCount = tResult.m_uSkippedSessionDuplicate + tResult.m_uSkippedFileDuplicate;
		SDK::Output("ProfileScraper", std::format("No new profiles to save ({} duplicates skipped, {} comma filtered)",
			uDuplicateCount,
			tResult.m_uSkippedComma).c_str());
	}

	auto CaptureAvatar = [](uint32_t uAccountID, std::vector<uint8_t>& vBgra, uint32_t& uWidth, uint32_t& uHeight) -> bool
		{
			if (!I::SteamFriends || !I::SteamUtils)
				return false;

			const CSteamID steamID(uAccountID, k_EUniversePublic, k_EAccountTypeIndividual);
			I::SteamFriends->RequestUserInformation(steamID, true);
			const int nAvatar = I::SteamFriends->GetMediumFriendAvatar(steamID);
			if (nAvatar <= 0)
				return false;

			if (!I::SteamUtils->GetImageSize(nAvatar, &uWidth, &uHeight) || !uWidth || !uHeight)
				return false;

			std::vector<uint8_t> vRgba(static_cast<size_t>(uWidth) * static_cast<size_t>(uHeight) * 4);
			if (!I::SteamUtils->GetImageRGBA(nAvatar, vRgba.data(), static_cast<int>(vRgba.size())))
				return false;

			vBgra.resize(vRgba.size());
			for (uint32_t y = 0; y < uHeight; y++)
			{
				for (uint32_t x = 0; x < uWidth; x++)
				{
					const size_t idx = (static_cast<size_t>(y) * uWidth + x) * 4;
					vBgra[idx + 0] = vRgba[idx + 2];
					vBgra[idx + 1] = vRgba[idx + 1];
					vBgra[idx + 2] = vRgba[idx + 0];
					vBgra[idx + 3] = vRgba[idx + 3];
				}
			}
			return true;
		};

	if (I::SteamFriends && I::SteamUtils)
	{
		for (const auto& tEntry : vProfiles)
		{
			std::vector<uint8_t> vBgra;
			uint32_t uWidth = 0, uHeight = 0;
			if (!CaptureAvatar(tEntry.m_uAccountID, vBgra, uWidth, uHeight))
			{
				tResult.m_uAvatarMissed++;
				continue;
			}

			std::filesystem::path sAvatarPath;
			if (CSteamProfileCache::SaveAvatarToDisk(tEntry.m_uAccountID, vBgra, uWidth, uHeight, &sAvatarPath, false))
			{
				tResult.m_uAvatarsSaved++;
				if (tResult.m_avatarFolder.empty())
					tResult.m_avatarFolder = sAvatarPath.parent_path();
			}
			else
			{
				tResult.m_uAvatarFailed++;
			}
		}
	}
	else if (bAnnounce)
	{
		SDK::Output("ProfileScraper", "Steam avatar interfaces unavailable; skipped avatar scraping.");
	}

	if (bAnnounce)
	{
		const size_t uDuplicateCount = tResult.m_uSkippedSessionDuplicate + tResult.m_uSkippedFileDuplicate;
		SDK::Output("ProfileScraper", std::format(
			"Saved {} new profiles to {} ({} duplicates skipped, {} comma filtered). Avatars: {} saved, {} unavailable, {} failed.",
			tResult.m_uAppendedCount,
			sPath.string(),
			uDuplicateCount,
			tResult.m_uSkippedComma,
			tResult.m_uAvatarsSaved,
			tResult.m_uAvatarMissed,
			tResult.m_uAvatarFailed).c_str());

		if (!tResult.m_avatarFolder.empty())
			SDK::Output("ProfileScraper", std::format("Avatar output directory: {}", tResult.m_avatarFolder.string()).c_str());
	}

	return tResult;
}

std::string CMisc::ReplaceTags(std::string sMsg, std::string sTarget, std::string sInitiator, std::string sKiller, std::string sVictim)
{
	auto ReplaceAll = [&](std::string& str, const std::string& from, const std::string& to)
		{
			if (from.empty()) return;
			size_t start_pos = 0;
			while ((start_pos = str.find(from, start_pos)) != std::string::npos)
			{
				str.replace(start_pos, from.length(), to);
				start_pos += to.length();
			}
		};

	if (!sTarget.empty())
	{
		ReplaceAll(sMsg, "{target}", sTarget);
		ReplaceAll(sMsg, "{triggername}", sTarget);
	}

	if (!sInitiator.empty())
		ReplaceAll(sMsg, "{initiator}", sInitiator);

	if (!sKiller.empty())
		ReplaceAll(sMsg, "{killer}", sKiller);

	if (!sVictim.empty())
		ReplaceAll(sMsg, "{victim}", sVictim);

	if (sMsg.find("{enemyteam}") != std::string::npos || sMsg.find("{friendlyteam}") != std::string::npos)
	{
		auto pLocal = H::Entities.GetLocal();
		if (pLocal)
		{
			int iLocalTeam = pLocal->m_iTeamNum();
			ReplaceAll(sMsg, "{enemyteam}", iLocalTeam == TF_TEAM_RED ? "BLU" : "RED");
			ReplaceAll(sMsg, "{friendlyteam}", iLocalTeam == TF_TEAM_RED ? "RED" : "BLU");
		}
	}

	if (sMsg.find("{lastkilled}") != std::string::npos)
		ReplaceAll(sMsg, "{lastkilled}", m_sLastKilledName.empty() ? "unknown" : m_sLastKilledName);

	if (sMsg.find("{highestscore}") != std::string::npos)
	{
		int iHighestScore = -1;
		std::string sHighestScoreName = "unknown";
		auto pResource = H::Entities.GetResource();
		if (pResource)
		{
			for (int i = 1; i <= I::EngineClient->GetMaxClients(); i++)
			{
				if (!pResource->m_bValid(i)) continue;
				int iScore = pResource->m_iTotalScore(i);
				if (iScore > iHighestScore)
				{
					iHighestScore = iScore;
					sHighestScoreName = pResource->GetName(i);
				}
			}
		}
		ReplaceAll(sMsg, "{highestscore}", sHighestScoreName);
	}

	auto GetRandomPlayer = [&](bool bFriend, bool bIgnored) -> std::string
		{
			std::vector<std::string> vCandidates;
			auto pResource = H::Entities.GetResource();
			if (pResource)
			{
				for (int i = 1; i <= I::EngineClient->GetMaxClients(); i++)
				{
					if (!pResource->m_bValid(i) || pResource->IsFakePlayer(i)) continue;

					bool isFriend = F::PlayerUtils.HasTag(i, F::PlayerUtils.TagToIndex(FRIEND_TAG));
					bool isIgnored = F::PlayerUtils.HasTag(i, F::PlayerUtils.TagToIndex(IGNORED_TAG));

					if (bFriend && !isFriend) continue;
					if (bIgnored && !isIgnored) continue;

					vCandidates.push_back(pResource->GetName(i));
				}
			}
			if (vCandidates.empty()) return "unknown";
			return vCandidates[SDK::RandomInt(0, static_cast<int>(vCandidates.size()) - 1)];
		};

	if (sMsg.find("{random}") != std::string::npos)
		ReplaceAll(sMsg, "{random}", GetRandomPlayer(false, false));

	if (sMsg.find("{friend}") != std::string::npos)
		ReplaceAll(sMsg, "{friend}", GetRandomPlayer(true, false));

	if (sMsg.find("{ignored}") != std::string::npos)
		ReplaceAll(sMsg, "{ignored}", GetRandomPlayer(false, true));

	return sMsg;
}

void CMisc::OnVoteStart(int iCaller, int iTarget, const std::string& sTarget)
{
	if (!Vars::Misc::Automation::ChatSpam::VoteKickReply.Value)
		return;

	static Timer tReloadTimer{};
	if ((m_vF1Messages.empty() && m_vF2Messages.empty()) || tReloadTimer.Run(5.0f))
	{
		m_vF1Messages.clear();
		m_vF2Messages.clear();

		static const char* szDefaultContent =
			"// Vote Kick Reply Configuration\n"
			"// Format: F1: message (supports {target}, {initiator}, {enemyteam}, {friendlyteam})\n"
			"// Format: F2: message (supports {target}, {initiator}, {enemyteam}, {friendlyteam})\n"
			"F1: {initiator} called a vote on {target}! Go {friendlyteam}!\n"
			"F2: {initiator} is trying to kick {target}! Don't let {enemyteam} win!\n";

		std::vector<std::string> vLines;
		if (LoadLines("votekick.txt", vLines, szDefaultContent))
		{
			for (const auto& line : vLines)
			{
				if (line.find("F1:") == 0)
				{
					std::string msg = line.substr(3);
					if (msg.find_first_not_of(" \t") != std::string::npos)
						msg.erase(0, msg.find_first_not_of(" \t"));
					if (!msg.empty()) m_vF1Messages.push_back(msg);
				}
				else if (line.find("F2:") == 0)
				{
					std::string msg = line.substr(3);
					if (msg.find_first_not_of(" \t") != std::string::npos)
						msg.erase(0, msg.find_first_not_of(" \t"));
					if (!msg.empty()) m_vF2Messages.push_back(msg);
				}
			}
		}
	}

	bool bTargetIsFriend = H::Entities.IsFriend(iTarget) || H::Entities.InParty(iTarget) || F::PlayerUtils.IsIgnored(iTarget);
	bool bCallerIsFriend = H::Entities.IsFriend(iCaller) || H::Entities.InParty(iCaller) || F::PlayerUtils.IsIgnored(iCaller);
	bool bTargetIsLocal = iTarget == I::EngineClient->GetLocalPlayer();
	bool bCallerIsLocal = iCaller == I::EngineClient->GetLocalPlayer();

	std::string sReply = "";
	std::string sInitiator = "";
	auto pResource = H::Entities.GetResource();
	if (pResource && pResource->m_bValid(iCaller))
		sInitiator = pResource->GetName(iCaller);

	if ((bTargetIsFriend || bTargetIsLocal) && !bCallerIsLocal)
	{
		if (!m_vF2Messages.empty())
		{
			int index = SDK::RandomInt(0, static_cast<int>(m_vF2Messages.size()) - 1);
			sReply = m_vF2Messages[index];
		}
	}
	else if ((bCallerIsFriend || bCallerIsLocal) && !bTargetIsLocal)
	{
		if (!m_vF1Messages.empty())
		{
			int index = SDK::RandomInt(0, static_cast<int>(m_vF1Messages.size()) - 1);
			sReply = m_vF1Messages[index];
		}
	}

	if (!sReply.empty())
	{
		sReply = ReplaceTags(sReply, sTarget, sInitiator);
		I::EngineClient->ClientCmd_Unrestricted(std::format("say {}", sReply).c_str());
	}
}

bool CMisc::LoadLines(const char* szFileName, std::vector<std::string>& vLines, const char* szDefaultContent)
{
	vLines.clear();

	std::string sPath = F::Configs.m_sConfigPath + szFileName;

	if (!std::filesystem::exists(sPath) && szDefaultContent)
	{
		std::ofstream newFile(sPath);
		if (newFile.good())
			newFile << szDefaultContent;
	}

	std::ifstream file(sPath);
	if (!file.good())
		return false;

	std::string line;
	while (std::getline(file, line))
	{
		if (line.empty() || line.find("//") == 0)
			continue;

		vLines.push_back(line);
	}

	return !vLines.empty();
}

std::vector<std::string> CMisc::ParseTokens(std::string str, char delimiter)
{
	std::vector<std::string> tokens;
	size_t pos = 0;
	std::string token;
	while ((pos = str.find(delimiter)) != std::string::npos)
	{
		token = str.substr(0, pos);
		if (token.find_first_not_of(" \t") != std::string::npos)
		{
			token.erase(0, token.find_first_not_of(" \t"));
			token.erase(token.find_last_not_of(" \t") + 1);
			if (!token.empty())
				tokens.push_back(token);
		}
		str.erase(0, pos + 1);
	}
	token = str;
	if (token.find_first_not_of(" \t") != std::string::npos)
	{
		token.erase(0, token.find_first_not_of(" \t"));
		token.erase(token.find_last_not_of(" \t") + 1);
		if (!token.empty())
			tokens.push_back(token);
	}
	return tokens;
}

void CMisc::ChargeBot(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	if (!Vars::Misc::Movement::ChargeBot.Value)
		return;

	auto pWeapon = H::Entities.GetWeapon();
	if (!pWeapon || pWeapon->GetSlot() != SLOT_MELEE)
		return;

	if (Vars::Misc::Movement::ChargeBotDelay.Value)
	{
		if (!m_bChargeBotReady)
		{
			if (!m_tChargeBotTimer.Run(Vars::Misc::Movement::ChargeBotDelay.Value))
				return;
			m_bChargeBotReady = true;
		}
	}
	else
		m_bChargeBotReady = true;

	if (!m_bChargeBotReady)
		return;

	CBaseEntity* pBestTarget = nullptr;
	float flBestScore = FLT_MAX;

	for (auto pEntity : H::Entities.GetGroup(EntityEnum::PlayerEnemy))
	{
		auto pPlayer = pEntity->As<CTFPlayer>();
		if (!pPlayer || !pPlayer->IsAlive() || pPlayer->IsAGhost() || pPlayer->IsDormant()
			|| F::AimbotGlobal.ShouldIgnore(pPlayer, pLocal, pWeapon))
			continue;

		Vec3 vDelta = pPlayer->GetCenter() - pLocal->GetCenter();
		float flDist = vDelta.Length();
		if (flDist > 500.f)
			continue;

		if (fabsf(vDelta.z) > 200.f)
			continue;

		float flScore = flDist;
		if (flScore < flBestScore)
		{
			flBestScore = flScore;
			pBestTarget = pPlayer;
		}
	}

	if (!pBestTarget)
	{
		m_bChargeBotReady = false;
		return;
	}

	Vec3 vAngleTo = Math::CalcAngle(pLocal->GetShootPos(), pBestTarget->GetCenter());
	float flYawDelta = Math::NormalizeAngle(vAngleTo.y - pCmd->viewangles.y);

	float flMaxTurn = Vars::Misc::Movement::ChargeBotTurnRate.Value;
	flYawDelta = std::clamp(flYawDelta, -flMaxTurn, flMaxTurn);

	pCmd->viewangles.y += flYawDelta;
	G::PSilentAngles = true;

	if (flBestScore < 72.f)
	{
		pCmd->buttons |= IN_ATTACK;
		m_bChargeBotReady = false;
	}

	CGameTrace trace = {};
	CTraceFilterWorldAndPropsOnly filter(pLocal);
	Vec3 vForward; Math::AngleVectors(pCmd->viewangles, &vForward);
	SDK::TraceHull(pLocal->m_vecOrigin(), pLocal->m_vecOrigin() + vForward * 100.f,
		pLocal->m_vecMins() * 0.9f, pLocal->m_vecMaxs() * 0.9f,
		MASK_PLAYERSOLID, &filter, &trace);
	if (trace.DidHit() && trace.fraction < 0.5f)
	{
		pCmd->viewangles.y -= flYawDelta * 2.f;
		G::PSilentAngles = true;
	}
}

void CMisc::AutoAirStickyPogo(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	if (!Vars::Misc::Movement::AutoAirStickyPogo.Value)
		return;

	auto pWeapon = H::Entities.GetWeapon();
	if (!pWeapon || pWeapon->GetWeaponID() != TF_WEAPON_PIPEBOMBLAUNCHER)
		return;

	if (pLocal->m_fFlags() & FL_ONGROUND)
		return;

	if (pLocal->m_MoveType() != MOVETYPE_WALK)
		return;

	if (!(pCmd->buttons & IN_JUMP))
		return;

	if (!m_tStickyPogoTimer.Run(Vars::Misc::Movement::AutoAirStickyPogoDelay.Value))
		return;

	auto& vProjectiles = H::Entities.GetGroup(EntityEnum::LocalStickies);
	if (vProjectiles.empty())
		return;

	CTFGrenadePipebombProjectile* pBestSticky = nullptr;
	float flBestDist = FLT_MAX;
	Vec3 vLocalOrigin = pLocal->GetCenter();

	for (auto pProjectile : vProjectiles)
	{
		auto pSticky = pProjectile->As<CTFGrenadePipebombProjectile>();
		if (!pSticky || !pSticky->m_hOriginalLauncher())
			continue;

		if (pSticky->m_iType() != TF_GL_MODE_REMOTE_DETONATE)
			continue;

		Vec3 vDelta = pSticky->m_vecOrigin() - vLocalOrigin;
		if (vDelta.z > -20.f || vDelta.z < -300.f)
			continue;

		float flDist2D = Vec2(vDelta.x, vDelta.y).Length();
		if (flDist2D > 150.f)
			continue;

		float flDist = vDelta.Length();
		if (flDist < flBestDist)
		{
			flBestDist = flDist;
			pBestSticky = pSticky;
		}
	}

	if (!pBestSticky)
		return;

	pCmd->buttons |= IN_ATTACK2;
}

void CMisc::OnChatMessage(int iEntIndex, const std::string& sName, const std::string& sMsg)
{
	if (iEntIndex == I::EngineClient->GetLocalPlayer())
		return;

	if (Vars::Misc::Automation::ChatSpam::AutoReply.Value)
	{
		static Timer tReloadTimer{};
		if (m_vAutoReplies.empty() || tReloadTimer.Run(5.0f))
		{
			m_vAutoReplies.clear();

			static const char* szDefaultContent =
				"// Auto Reply Configuration\n"
				"// Format: trigger1, trigger2 : response1, response2\n"
				"// Example:\n"
				"hello, hi : hi there, hello!\n"
				"bot, hacker : I am not a bot, I am just good\n";

			std::vector<std::string> vLines;
			if (LoadLines("autoreply.txt", vLines, szDefaultContent))
			{
				for (const auto& line : vLines)
				{
					size_t delimiterPos = line.find(':');
					if (delimiterPos != std::string::npos)
					{
						std::string triggersStr = line.substr(0, delimiterPos);
						std::string repliesStr = line.substr(delimiterPos + 1);

						AutoReply_t entry;
						entry.vTriggers = ParseTokens(triggersStr, ',');
						entry.vReplies = ParseTokens(repliesStr, ',');

						if (!entry.vTriggers.empty() && !entry.vReplies.empty())
							m_vAutoReplies.push_back(entry);
					}
				}
			}
		}

		auto StripColors = [](std::string str) -> std::string
			{
				std::string out = "";
				for (size_t i = 0; i < str.length(); i++)
				{
					if (str[i] > 0 && str[i] < 32) continue;
					out += str[i];
				}
				return out;
			};

		std::string sCleanMsg = StripColors(sMsg);
		std::transform(sCleanMsg.begin(), sCleanMsg.end(), sCleanMsg.begin(), ::tolower);

		for (const auto& entry : m_vAutoReplies)
		{
			bool bTriggered = false;
			for (const auto& trigger : entry.vTriggers)
			{
				std::string sLowTrigger = trigger;
				std::transform(sLowTrigger.begin(), sLowTrigger.end(), sLowTrigger.begin(), ::tolower);

				if (sCleanMsg.find(sLowTrigger) != std::string::npos)
				{
					bTriggered = true;
					break;
				}
			}

			if (bTriggered)
			{
				if (!entry.vReplies.empty())
				{
					int index = SDK::RandomInt(0, static_cast<int>(entry.vReplies.size()) - 1);
					std::string sReply = ReplaceTags(entry.vReplies[index], sName);
					I::EngineClient->ClientCmd_Unrestricted(std::format("say {}", sReply).c_str());
					break;
				}
			}
		}
	}

}
