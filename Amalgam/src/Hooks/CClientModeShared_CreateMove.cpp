#include "../SDK/SDK.h"
#include "../Features/Aimbot/Aimbot.h"
#include "../Features/Backtrack/Backtrack.h"
#include "../Features/CritHack/CritHack.h"
#include "../Features/EnginePrediction/EnginePrediction.h"
#include "../Features/Misc/Misc.h"
#include "../Features/NoSpread/NoSpread.h"
#include "../Features/NoSpread/NoSpreadHitscan/NoSpreadHitscan.h"
#include "../Features/PacketManip/PacketManip.h"
#include "../Features/Resolver/Resolver.h"
#include "../Features/TickHandler/TickHandler.h"
#include "../Features/Visuals/Visuals.h"
#include "../Features/Visuals/FakeAngle/FakeAngle.h"
#include "../Features/Spectate/Spectate.h"

MAKE_SIGNATURE(IHasGenericMeter_GetMeterMultiplier, "client.dll", "F3 0F 10 81 ? ? ? ? C3 CC CC CC CC CC CC CC 48 85 D2", 0x0);

MAKE_HOOK(CClientModeShared_CreateMove, U::Memory.GetVFunc(I::ClientModeShared, 21), bool,
	CClientModeShared* rcx, float flInputSampleTime, CUserCmd* pCmd)
{
	G::Buttons = pCmd ? pCmd->buttons : G::Buttons;

	const bool bReturn = CALL_ORIGINAL(rcx, flInputSampleTime, pCmd);
	if (!pCmd || !pCmd->command_number)
		return bReturn;

	bool* pSendPacket = reinterpret_cast<bool*>(uintptr_t(_AddressOfReturnAddress()) + 0x128);

	G::PSilentAngles = G::SilentAngles = G::Attacking = G::Throwing = false;
	G::LastUserCmd = G::CurrentUserCmd ? G::CurrentUserCmd : pCmd;
	G::CurrentUserCmd = pCmd;

	I::Prediction->Update(I::ClientState->m_nDeltaTick, I::ClientState->m_nDeltaTick > 0, I::ClientState->last_command_ack, I::ClientState->lastoutgoingcommand + I::ClientState->chokedcommands);
	
	// correct tick_count for fakeinterp / nointerp
	pCmd->tick_count += TICKS_TO_TIME(F::Backtrack.m_flFakeInterp) - (Vars::Visuals::Removals::Interpolation.Value ? 0 : TICKS_TO_TIME(G::Lerp));
	if (G::Buttons & IN_DUCK) // lol
		pCmd->buttons |= IN_DUCK;

	auto pLocal = H::Entities.GetLocal();
	auto pWeapon = H::Entities.GetWeapon();
	if (pLocal && pWeapon)
	{	// Update Global Info
		{
			static int iStaticItemDefinitionIndex = 0;
			int iOldItemDefinitionIndex = iStaticItemDefinitionIndex;
			int iNewItemDefinitionIndex = iStaticItemDefinitionIndex = pWeapon->m_iItemDefinitionIndex();

			if (iNewItemDefinitionIndex != iOldItemDefinitionIndex || !pWeapon->m_iClip1() || !pLocal->IsAlive() || pLocal->IsTaunting() || pLocal->IsBonked() || pLocal->IsAGhost() || pLocal->IsInBumperKart())
				F::Ticks.m_iWait = 1;
		}

		G::CanPrimaryAttack = G::CanSecondaryAttack = G::Reloading = false;
		bool bCanAttack = pLocal->CanAttack();
		switch (SDK::GetRoundState())
		{
		case GR_STATE_BETWEEN_RNDS:
		case GR_STATE_GAME_OVER:
			bCanAttack = bCanAttack && !(pLocal->m_fFlags() & FL_FROZEN);
		}
		if (bCanAttack)
		{
			G::CanPrimaryAttack = pWeapon->CanPrimaryAttack();
			G::CanSecondaryAttack = pWeapon->CanSecondaryAttack();

			switch (pWeapon->GetWeaponID())
			{
			case TF_WEAPON_FLAME_BALL:
				if (G::CanPrimaryAttack)
				{
					// do this, otherwise it will be a tick behind
					float flFrametime = TICK_INTERVAL * 100;
					float flMeterMult = S::IHasGenericMeter_GetMeterMultiplier.Call<float>(pWeapon->m_pMeter());
					float flRate = SDK::AttribHookValue(1.f, "item_meter_charge_rate", pWeapon) - 1;
					float flMult = SDK::AttribHookValue(1.f, "mult_item_meter_charge_rate", pWeapon);
					float flTankPressure = pLocal->m_flTankPressure() + flFrametime * flMeterMult / (flRate * flMult);

					if (G::CanPrimaryAttack && flTankPressure < 100.f)
						G::CanPrimaryAttack = G::CanSecondaryAttack = false;
				}
				break;
			case TF_WEAPON_MINIGUN:
			{
				int iState = pWeapon->As<CTFMinigun>()->m_iWeaponState();
				if (iState != AC_STATE_FIRING && iState != AC_STATE_SPINNING || !pWeapon->HasPrimaryAmmoForShot())
					G::CanPrimaryAttack = false;
				break;
			}
			case TF_WEAPON_FLAREGUN_REVENGE:
				if (pCmd->buttons & IN_ATTACK2)
					G::CanPrimaryAttack = false;
				break;
			case TF_WEAPON_BAT_WOOD:
			case TF_WEAPON_BAT_GIFTWRAP:
				if (!pWeapon->HasPrimaryAmmoForShot())
					G::CanSecondaryAttack = false;
				break;
			case TF_WEAPON_MEDIGUN:
			case TF_WEAPON_BUILDER:
				break;
			case TF_WEAPON_PARTICLE_CANNON:
			{
				float flChargeBeginTime = pWeapon->As<CTFParticleCannon>()->m_flChargeBeginTime();
				if (flChargeBeginTime > 0)
				{
					float flTotalChargeTime = TICKS_TO_TIME(pLocal->m_nTickBase()) - flChargeBeginTime;
					if (flTotalChargeTime < TF_PARTICLE_MAX_CHARGE_TIME)
					{
						G::CanPrimaryAttack = G::CanSecondaryAttack = false;
						break;
					}
				}
				[[fallthrough]];
			}
			default:
				if (pWeapon->GetSlot() != SLOT_MELEE)
				{
					/*
					if (pWeapon->IsInReload())
						G::CanPrimaryAttack = pWeapon->HasPrimaryAmmoForShot();
					*/

					bool bAmmo = pWeapon->HasPrimaryAmmoForShot();
					bool bReload = pWeapon->IsInReload();
					if (!bAmmo && pWeapon->m_iItemDefinitionIndex() != Soldier_m_TheBeggarsBazooka)
						G::CanPrimaryAttack = G::CanSecondaryAttack = false;
					if (bReload && bAmmo && !G::CanPrimaryAttack)
						G::Reloading = true;
				}
			}
		}

		G::Attacking = SDK::IsAttacking(pLocal, pWeapon, pCmd);
		G::PrimaryWeaponType = SDK::GetWeaponType(pWeapon, &G::SecondaryWeaponType);
		G::CanHeadshot = pWeapon->CanHeadShot();
	}

	// Run Features
	F::Spectate.CreateMove(pLocal, pCmd);
	F::Misc.RunPre(pLocal, pCmd);
	F::Backtrack.Run(pCmd);

	F::EnginePrediction.Start(pLocal, pCmd);
	F::Aimbot.Run(pLocal, pWeapon, pCmd);
	F::EnginePrediction.End(pLocal, pCmd);

	F::PacketManip.Run(pLocal, pWeapon, pCmd, pSendPacket);
	F::Ticks.CreateMove(pLocal, pCmd);
	F::CritHack.Run(pLocal, pWeapon, pCmd);
	F::NoSpread.Run(pLocal, pWeapon, pCmd);
	F::Misc.RunPost(pLocal, pCmd, *pSendPacket);
	F::Resolver.CreateMove();
	F::Visuals.CreateMove(pLocal, pWeapon);

	{
		static bool bWasSet = false;
		const bool bOverchoking = I::ClientState->chokedcommands >= 21 || F::Ticks.m_iShiftedTicks + I::ClientState->chokedcommands == F::Ticks.m_iMaxShift + (F::AntiAim.YawOn() ? 3 : 0); // failsafe
		if (G::PSilentAngles && !bOverchoking)
			*pSendPacket = false, bWasSet = true;
		else if (bWasSet || bOverchoking)
			*pSendPacket = true, bWasSet = false;
	}
	F::Ticks.ManagePacket(pCmd, pSendPacket);
	F::AntiAim.Run(pLocal, pWeapon, pCmd, *pSendPacket);
	if (*pSendPacket)
		F::FakeAngle.Run(pLocal);

	if (pLocal)
	{
		static std::vector<Vec3> vAngles;
		vAngles.push_back(pCmd->viewangles);
		auto pAnimState = pLocal->GetAnimState();
		if (*pSendPacket && pAnimState)
		{
			float flOldFrametime = I::GlobalVars->frametime;
			I::GlobalVars->frametime = TICK_INTERVAL;
			for (auto& vAngle : vAngles)
			{
				if (pLocal->IsTaunting() && pLocal->m_bAllowMoveDuringTaunt())
					pLocal->m_flTauntYaw() = vAngle.y;
				pAnimState->m_flEyeYaw = vAngle.y;
				pAnimState->Update(vAngle.y, vAngle.x);
				pLocal->FrameAdvance(TICK_INTERVAL);
			}
			I::GlobalVars->frametime = flOldFrametime;
			vAngles.clear();
		}
	}

	G::Choking = !*pSendPacket;
	G::LastUserCmd = pCmd;
	F::NoSpreadHitscan.AskForPlayerPerf();

	//const bool bShouldSkip = G::PSilentAngles || G::SilentAngles || G::AntiAim || G::AvoidingBackstab;
	//return bShouldSkip ? false : CALL_ORIGINAL(rcx, edx, input_sample_frametime, pCmd);
	return false;
}