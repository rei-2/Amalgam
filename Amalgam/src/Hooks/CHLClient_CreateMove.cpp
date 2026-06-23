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
#include "../Features/Ticks/Ticks.h"
#include "../Features/Visuals/Visuals.h"
#include "../Features/Visuals/FakeAngle/FakeAngle.h"
#include "../Features/Spectate/Spectate.h"
#include "../Features/AntiCheatCompatibility/AntiCheatCompatibility.h"

static no_inline void UpdateInfo(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd)
{
	G::PSilentAngles = G::SilentAngles = G::Attacking = G::Throwing = false;
	G::LastUserCmd = G::CurrentUserCmd ? G::CurrentUserCmd : pCmd;
	G::CurrentUserCmd = pCmd;
	G::OriginalCmd = *pCmd;

	if (!pWeapon)
		return;

	SDK::CanAttack(pLocal, pWeapon, pCmd, G::CanPrimaryAttack, G::CanSecondaryAttack, G::Reloading);
	G::Attacking = SDK::IsAttacking(pLocal, pWeapon, pCmd);
	G::PrimaryWeaponType = SDK::GetWeaponType(pWeapon, &G::SecondaryWeaponType);
	G::CanHeadshot = pWeapon->CanHeadshot() || pWeapon->AmbassadorCanHeadshot(TICKS_TO_TIME(pLocal->m_nTickBase()));
}

MAKE_HOOK(CHLClient_CreateMove, U::Memory.GetVirtual(I::Client, 21), void,
	void* rcx, int sequence_number, float input_sample_frametime, bool active)
{
	DEBUG_RETURN(CHLClient_CreateMove, rcx, sequence_number, input_sample_frametime, active);

	CALL_ORIGINAL(rcx, sequence_number, input_sample_frametime, active);

	auto pLocal = H::Entities.GetLocal();
	auto pWeapon = H::Entities.GetWeapon();
	if (!pLocal)
		return;

	bool* pSendPacket = reinterpret_cast<bool*>(uintptr_t(_AddressOfReturnAddress()) + 0x20);
	CUserCmd* pCmd = &I::Input->m_pCommands[sequence_number % MULTIPLAYER_BACKUP];

	I::Prediction->Update(I::ClientState->m_nDeltaTick, I::ClientState->m_nDeltaTick > 0, I::ClientState->last_command_ack, I::ClientState->lastoutgoingcommand + I::ClientState->chokedcommands);

	UpdateInfo(pLocal, pWeapon, pCmd);
		F::Spectate.CreateMove(pCmd);
		F::Backtrack.CreateMove(pCmd);
		F::Misc.RunPre(pLocal, pCmd);
	F::Ticks.Start(pLocal, pCmd);
		F::Aimbot.Run(pLocal, pWeapon, pCmd);
	F::Ticks.End(pLocal, pCmd);
		F::CritHack.Run(pLocal, pWeapon, pCmd);
		F::NoSpread.Run(pLocal, pWeapon, pCmd);
		F::Misc.RunPost(pLocal, pCmd);
		F::PacketManip.Run(pLocal, pWeapon, pCmd, pSendPacket);
		F::Ticks.CreateMove(pLocal, pWeapon, pCmd, pSendPacket);
		F::AntiAim.Run(pLocal, pWeapon, pCmd, *pSendPacket);
		F::AntiCheatCompatibility.CreateMove(pCmd, pSendPacket);
		F::Visuals.CreateMove(pLocal, pWeapon);
		F::Visuals.LocalAnimations(pLocal, pWeapon, pCmd, *pSendPacket);
	F::EnginePrediction.End(pLocal, pCmd);
		F::Resolver.CreateMove();
		F::NoSpreadHitscan.AskForPlayerPerf();
	G::Choking = !*pSendPacket, G::LastUserCmd = pCmd;
}