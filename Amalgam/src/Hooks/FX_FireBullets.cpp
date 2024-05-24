#include "../SDK/SDK.h"

#include "../Features/Backtrack/Backtrack.h"
#include "../Features/NoSpread/NoSpreadHitscan/NoSpreadHitscan.h"
#include "../Features/Resolver/Resolver.h"

MAKE_SIGNATURE(FX_FireBullets, "client.dll", "48 89 5C 24 ? 48 89 74 24 ? 4C 89 4C 24 ? 55", 0x0);
MAKE_SIGNATURE(CTFWeaponBaseGun_FireBullet_Call, "client.dll", "0F 28 7C 24 ? 4C 8D 9C 24 ? ? ? ? 49 8B 5B ? 49 8B 6B ? 49 8B 73 ? 41 0F 28 73 ? 49 8B E3", 0x0);

MAKE_HOOK(FX_FireBullets, S::FX_FireBullets(), void, __cdecl,
	void* pWpn, int iPlayer, const Vec3& vecOrigin, const Vec3& vecAngles, int iWeapon, int iMode, int iSeed, float flSpread, float flDamage, bool bCritical)
{
	static auto dwDesired = S::CTFWeaponBaseGun_FireBullet_Call();
	const auto dwRetAddr = std::uintptr_t(_ReturnAddress());

	if (iPlayer != I::EngineClient->GetLocalPlayer())
	{
		F::Backtrack.ReportShot(iPlayer);
		F::Resolver.FXFireBullet(iPlayer, vecAngles);
	}

	if (dwRetAddr != dwDesired)
		return;

	if (Vars::Aimbot::General::NoSpread.Value)
		iSeed = F::NoSpreadHitscan.iSeed;

	return CALL_ORIGINAL(pWpn, iPlayer, vecOrigin, vecAngles, iWeapon, iMode, iSeed, flSpread, flDamage, bCritical);
}