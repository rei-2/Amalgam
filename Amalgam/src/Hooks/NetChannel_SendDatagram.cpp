#include "../SDK/SDK.h"

#include "../Features/Backtrack/Backtrack.h"

MAKE_SIGNATURE(NetChannel_SendDatagram, "engine.dll", "40 55 57 41 56 48 8D AC 24", 0x0);

MAKE_HOOK(NetChannel_SendDatagram, S::NetChannel_SendDatagram(), int, __fastcall,
	CNetChannel* netChannel, bf_write* datagram)
{
	if (!netChannel || datagram)
		return CALL_ORIGINAL(netChannel, datagram);

	F::Backtrack.bFakeLatency = H::Entities.GetLocal() && Vars::Backtrack::Enabled.Value && Vars::Backtrack::Latency.Value;
	if (!F::Backtrack.bFakeLatency)
		return CALL_ORIGINAL(netChannel, datagram);

	const int nInSequenceNr = netChannel->m_nInSequenceNr, nInReliableState = netChannel->m_nInReliableState;
	F::Backtrack.AdjustPing(netChannel);
	const int original = CALL_ORIGINAL(netChannel, datagram);
	netChannel->m_nInSequenceNr = nInSequenceNr, netChannel->m_nInReliableState = nInReliableState;

	return original;
}