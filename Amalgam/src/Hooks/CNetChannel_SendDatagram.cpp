#include "../SDK/SDK.h"

#include "../Features/Backtrack/Backtrack.h"

MAKE_SIGNATURE(CNetChannel_SendDatagram, "engine.dll", "40 55 57 41 56 48 8D AC 24", 0x0);

MAKE_HOOK(CNetChannel_SendDatagram, S::CNetChannel_SendDatagram(), int, __fastcall,
	CNetChannel* pNetChan, bf_write* datagram)
{
	if (!pNetChan || datagram)
		return CALL_ORIGINAL(pNetChan, datagram);

	F::Backtrack.flFakeLatency = std::min(F::Backtrack.flFakeLatency + TICKS_TO_TIME(1), F::Backtrack.GetFake());
	if (!Vars::Backtrack::Enabled.Value || !Vars::Backtrack::Latency.Value || !H::Entities.GetLocal())
		return CALL_ORIGINAL(pNetChan, datagram);

	const int nInSequenceNr = pNetChan->m_nInSequenceNr, nInReliableState = pNetChan->m_nInReliableState;
	F::Backtrack.AdjustPing(pNetChan);
	const int iReturn = CALL_ORIGINAL(pNetChan, datagram);
	pNetChan->m_nInSequenceNr = nInSequenceNr, pNetChan->m_nInReliableState = nInReliableState;

	return iReturn;
}