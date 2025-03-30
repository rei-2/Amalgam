#include "../SDK/SDK.h"

#include "../Features/Backtrack/Backtrack.h"

MAKE_SIGNATURE(CNetChannel_SendDatagram, "engine.dll", "40 55 57 41 56 48 8D AC 24", 0x0);

MAKE_HOOK(CNetChannel_SendDatagram, S::CNetChannel_SendDatagram(), int,
	CNetChannel* pNetChan, bf_write* datagram)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::CNetChannel_SendDatagram[DEFAULT_BIND])
		return CALL_ORIGINAL(pNetChan, datagram);
#endif

	if (datagram)
		return CALL_ORIGINAL(pNetChan, datagram);

	F::Backtrack.AdjustPing(pNetChan);
	const int iReturn = CALL_ORIGINAL(pNetChan, datagram);
	F::Backtrack.RestorePing(pNetChan);
	return iReturn;
}