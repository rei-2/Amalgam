#include "PacketManip.h"

#include "../Visuals/FakeAngle/FakeAngle.h"

static inline bool AntiAimCheck(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd)
{
	return F::AntiAim.YawOn() && F::AntiAim.ShouldRun(pLocal, pWeapon, pCmd) && I::ClientState->chokedcommands < 3 && !G::Recharge;
}

void CPacketManip::Run(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd, bool* pSendPacket)
{
	F::FakeAngle.bDrawChams = Vars::CL_Move::Fakelag::Fakelag.Value || F::AntiAim.AntiAimOn();

	*pSendPacket = true;
	F::FakeLag.Run(pLocal, pWeapon, pCmd, pSendPacket);
	if (AntiAimCheck(pLocal, pWeapon, pCmd))
		*pSendPacket = false;
}