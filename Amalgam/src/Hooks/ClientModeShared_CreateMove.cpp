#include "../SDK/SDK.h"
#include "../Features/Aimbot/Aimbot.h"
#include "../Features/Aimbot/AimbotProjectile/AimbotProjectile.h"
#include "../Features/Backtrack/Backtrack.h"
#include "../Features/CritHack/CritHack.h"
#include "../Features/EnginePrediction/EnginePrediction.h"
#include "../Features/Misc/Misc.h"
#include "../Features/NoSpread/NoSpread.h"
#include "../Features/PacketManip/PacketManip.h"
#include "../Features/Resolver/Resolver.h"
#include "../Features/TickHandler/TickHandler.h"
#include "../Features/Visuals/Visuals.h"
#include "../Features/Visuals/FakeAngle/FakeAngle.h"

MAKE_HOOK(ClientModeShared_CreateMove, U::Memory.GetVFunc(I::ClientModeShared, 21), bool, __fastcall,
	CClientModeShared* ecx, float flInputSampleTime, CUserCmd* pCmd)
{
	G::PSilentAngles = G::SilentAngles = G::IsAttacking = false;
	G::Buttons = pCmd ? pCmd->buttons : G::Buttons;
	const bool bReturn = CALL_ORIGINAL(ecx, flInputSampleTime, pCmd);
	if (!pCmd || !pCmd->command_number)
		return bReturn;

	auto ebp = *reinterpret_cast<uintptr_t*>(uintptr_t(_AddressOfReturnAddress()) - sizeof(void*));
	bool* pSendPacket = reinterpret_cast<bool*>(ebp + 0xA7);

	I::Prediction->Update(I::ClientState->m_nDeltaTick, I::ClientState->m_nDeltaTick > 0, I::ClientState->last_command_ack, I::ClientState->lastoutgoingcommand + I::ClientState->chokedcommands);

	G::CurrentUserCmd = pCmd;
	if (!G::LastUserCmd)
		G::LastUserCmd = pCmd;

	// correct tick_count for fakeinterp / nointerp
	pCmd->tick_count += TICKS_TO_TIME(F::Backtrack.flFakeInterp) - (Vars::Visuals::Removals::Interpolation.Value ? 0 : TICKS_TO_TIME(G::Lerp));
	if (G::Buttons & IN_DUCK) // lol
		pCmd->buttons |= IN_DUCK;

	auto pLocal = H::Entities.GetLocal();
	auto pWeapon = H::Entities.GetWeapon();
	if (pLocal && pWeapon)
	{	// Update Global Info
		int nOldDefIndex = G::WeaponDefIndex;
		G::WeaponDefIndex = pWeapon->m_iItemDefinitionIndex();
		if (G::WeaponDefIndex != nOldDefIndex || !pWeapon->m_iClip1() || !pLocal->IsAlive() || pLocal->IsTaunting() || pLocal->IsBonked() || pLocal->IsAGhost() || pLocal->IsInBumperKart())
			G::WaitForShift = 1;

		G::CanPrimaryAttack = G::CanSecondaryAttack = false;
		bool bCanAttack = pLocal->CanAttack();
		switch (SDK::GetRoundState())
		{
		case GR_STATE_BETWEEN_RNDS:
		case GR_STATE_GAME_OVER:
			bCanAttack = bCanAttack && !(pLocal->m_fFlags() & FL_FROZEN);
		}
		if (bCanAttack)
		{
			switch (pWeapon->m_iWeaponID())
			{
			case TF_WEAPON_FLAME_BALL:
				G::CanPrimaryAttack = G::CanSecondaryAttack = pLocal->m_flTankPressure() >= 100.f;
				break;
			case TF_WEAPON_BUILDER:
				G::CanPrimaryAttack = true;
				break;
			default:
				G::CanPrimaryAttack = pWeapon->CanPrimaryAttack(pLocal);
				G::CanSecondaryAttack = pWeapon->CanSecondaryAttack(pLocal);

				if (pWeapon->m_iSlot() == SLOT_MELEE)
					break;
				
				if (pWeapon->IsInReload())
					G::CanPrimaryAttack = pWeapon->HasPrimaryAmmoForShot();

				if (G::WeaponDefIndex != Soldier_m_TheBeggarsBazooka && pWeapon->m_iClip1() == 0)
					G::CanPrimaryAttack = false;

				if (pWeapon->m_bEnergyWeapon() && !pWeapon->m_flEnergy())
					G::CanPrimaryAttack = false;

				switch (pWeapon->m_iWeaponID())
				{
				case TF_WEAPON_MINIGUN:
					if (pWeapon->As<CTFMinigun>()->m_iWeaponState() != AC_STATE_FIRING && pWeapon->As<CTFMinigun>()->m_iWeaponState() != AC_STATE_SPINNING)
						G::CanPrimaryAttack = false;
					break;
				case TF_WEAPON_FLAREGUN_REVENGE:
					if (pCmd->buttons & IN_ATTACK2)
						G::CanPrimaryAttack = false;
				}
			}
		}

		G::IsAttacking = SDK::IsAttacking(pLocal, pWeapon, pCmd);
		G::WeaponType = SDK::GetWeaponType(pWeapon);
		G::CanHeadshot = pWeapon->CanHeadShot(pLocal);
	}

	const bool bSkip = F::AimbotProjectile.bLastTickCancel;
	if (bSkip)
	{
		pCmd->weaponselect = F::AimbotProjectile.bLastTickCancel;
		F::AimbotProjectile.bLastTickCancel = 0;
	}

	// Run Features
	F::Misc.RunPre(pLocal, pCmd);
	F::Backtrack.Run(pCmd);

	F::EnginePrediction.Start(pLocal, pCmd);
	{
		const bool bAimRan = bSkip ? false : F::Aimbot.Run(pLocal, pWeapon, pCmd);
		if (!bAimRan && G::CanPrimaryAttack && G::IsAttacking && !F::AimbotProjectile.bLastTickCancel)
			F::Visuals.ProjectileTrace(pLocal, pWeapon, false);
	}
	F::EnginePrediction.End(pLocal, pCmd);

	F::PacketManip.Run(pLocal, pCmd, pSendPacket);
	F::Ticks.MovePost(pLocal, pCmd);
	F::CritHack.Run(pLocal, pWeapon, pCmd);
	F::NoSpread.Run(pLocal, pWeapon, pCmd);
	F::Misc.RunPost(pLocal, pCmd, *pSendPacket);
	F::Resolver.CreateMove();

	{
		static bool bWasSet = false;
		const bool bOverchoking = I::ClientState->chokedcommands >= 21; // failsafe
		if (G::PSilentAngles && G::ShiftedTicks != G::MaxShift && !bOverchoking)
			*pSendPacket = false, bWasSet = true;
		else if (bWasSet || bOverchoking)
			*pSendPacket = true, bWasSet = false;
	}
	F::Misc.DoubletapPacket(pCmd, pSendPacket);
	F::AntiAim.Run(pLocal, pCmd, pSendPacket);
	G::Choking = !*pSendPacket;
	if (*pSendPacket)
		F::FakeAngle.Run(pLocal);

	G::ViewAngles = pCmd->viewangles;
	G::LastUserCmd = pCmd;

	//const bool bShouldSkip = G::PSilentAngles || G::SilentAngles || G::AntiAim || G::AvoidingBackstab;
	//return bShouldSkip ? false : CALL_ORIGINAL(ecx, edx, input_sample_frametime, pCmd);
	return false;
}