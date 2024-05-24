// what is this for?

#include "../SDK/SDK.h"

MAKE_SIGNATURE(CTFMatchSummary_OnTick, "client.dll", "55 8B EC 83 EC ? 53 57 8B F9 E8 ? ? ? ? E8", 0x0);

// Credits: mfed
MAKE_HOOK(CTFMatchSummary_OnTick, S::CTFMatchSummary_OnTick(), int, __fastcall,
    void* ecx)
{
    if (!reinterpret_cast<DWORD*>(ecx) + 750) { return CALL_ORIGINAL(ecx); } //  m_iCurrentState == MS_STATE_INITIAL
    DWORD* flags = reinterpret_cast<DWORD*>(I::TFGCClientSystem + 1488);
    if (flags && *flags & 2)
        *flags &= ~2;
    return CALL_ORIGINAL(ecx);
}