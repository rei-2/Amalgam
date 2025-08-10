#include "../SDK/SDK.h"

#include "../Features/Backtrack/Backtrack.h"
#include "../Features/NoSpread/NoSpreadHitscan/NoSpreadHitscan.h"
#include "../Features/Resolver/Resolver.h"

MAKE_SIGNATURE(FX_FireBullets, "client.dll", "48 89 5C 24 ? 48 89 74 24 ? 4C 89 4C 24 ? 55", 0x0);
MAKE_SIGNATURE(CTFWeaponBaseGun_FireBullet_FireBullets_Call, "client.dll", "0F 28 7C 24 ? 4C 8D 9C 24 ? ? ? ? 49 8B 5B ? 49 8B 6B ? 49 8B 73 ? 41 0F 28 73 ? 49 8B E3", 0x0);

MAKE_HOOK(FX_FireBullets, S::FX_FireBullets(), void,
	CTFWeaponBase* pWpn, int iPlayer, const Vec3& vecOrigin, const Vec3& vecAngles, int iWeapon, int iMode, int iSeed, float flSpread, float flDamage, bool bCritical)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::FX_FireBullets[DEFAULT_BIND])
		return CALL_ORIGINAL(pWpn, iPlayer, vecOrigin, vecAngles, iWeapon, iMode, iSeed, flSpread, flDamage, bCritical);
#endif

	static const auto dwDesired = S::CTFWeaponBaseGun_FireBullet_FireBullets_Call();
	const auto dwRetAddr = uintptr_t(_ReturnAddress());

	if (iPlayer != I::EngineClient->GetLocalPlayer())
		F::Backtrack.ReportShot(iPlayer);
	else if (Vars::Aimbot::General::NoSpread.Value && dwRetAddr == dwDesired)
		iSeed = F::NoSpreadHitscan.m_iSeed;

	return CALL_ORIGINAL(pWpn, iPlayer, vecOrigin, vecAngles, iWeapon, iMode, iSeed, flSpread, flDamage, bCritical);
}

#ifdef SEEDPRED_DEBUG
MAKE_SIGNATURE(FX_FireBullets_Server, "server.dll", "48 89 5C 24 ? 4C 89 4C 24 ? 55 56 41 54", 0x0);
MAKE_SIGNATURE(CBasePlayer_ProcessUsercmds, "server.dll", "40 53 55 56 57 41 54 48 83 EC ? 4C 89 6C 24", 0x0);

MAKE_HOOK(FX_FireBullets_Server, S::FX_FireBullets_Server(), void,
	CTFWeaponBase* pWpn, int iPlayer, const Vec3& vecOrigin, const Vec3& vecAngles, int iWeapon, int iMode, int iSeed, float flSpread, float flDamage, bool bCritical)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::FX_FireBullets[DEFAULT_BIND])
		return CALL_ORIGINAL(pWpn, iPlayer, vecOrigin, vecAngles, iWeapon, iMode, iSeed, flSpread, flDamage, bCritical);
#endif

	if (Vars::Aimbot::General::NoSpread.Value)
		SDK::Output("FX_FireBullets", std::format("{}", iSeed).c_str(), { 0, 255, 0 });
	return CALL_ORIGINAL(pWpn, iPlayer, vecOrigin, vecAngles, iWeapon, iMode, iSeed, flSpread, flDamage, bCritical);
}

MAKE_HOOK(CBasePlayer_ProcessUsercmds, S::CBasePlayer_ProcessUsercmds(), void,
	void* rcx, CUserCmd* cmds, int numcmds, int totalcmds, int dropped_packets, bool paused)
{
	bool bInAttack = false;
	for (int i = totalcmds - 1; i >= 0; i--)
	{
		CUserCmd* pCmd = &cmds[totalcmds - 1 - i];
		if (pCmd && pCmd->buttons & IN_ATTACK)
			bInAttack = true;
	}
	if (bInAttack)
	{
		double dFloatTime = SDK::PlatFloatTime();
		float flTime = float(SDK::PlatFloatTime() * 1000.0);
		int iSeed = *reinterpret_cast<int*>((char*)&flTime) & 255;
		SDK::Output("ProcessUsercmds", std::format("{}: {}", dFloatTime, iSeed).c_str(), { 0, 255, 0, 100 });
	}

	return CALL_ORIGINAL(rcx, cmds, numcmds, totalcmds, dropped_packets, paused);
}
#endif