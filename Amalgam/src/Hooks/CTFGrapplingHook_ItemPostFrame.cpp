#include "../SDK/SDK.h"

MAKE_SIGNATURE(CTFGrapplingHook_ItemPostFrame, "client.dll", "40 53 57 48 83 EC ? 48 8B D9 E8 ? ? ? ? 48 8B F8 48 85 C0 0F 84 ? ? ? ? 8B 88", 0x0);
MAKE_SIGNATURE(CTFGrapplingHook_PrimaryAttack, "client.dll", "48 89 5C 24 ? 48 89 7C 24 ? 55 48 8D 6C 24 ? 48 81 EC ? ? ? ? 48 8B D9 E8 ? ? ? ? 8B 93", 0x0);
MAKE_SIGNATURE(CTFGrapplingHook_ItemPostFrame_Fired, "client.dll", "40 38 35 ? ? ? ? 75 ? 48 8B 03", 0x0);

static bool s_bOriginalFired = false;

MAKE_HOOK(CTFGrapplingHook_ItemPostFrame, S::CTFGrapplingHook_ItemPostFrame(), void,
	void* rcx)
{
	static bool& bFired = *reinterpret_cast<bool*>(U::Memory.RelToAbs(S::CTFGrapplingHook_ItemPostFrame_Fired()));
	s_bOriginalFired = bFired;
	bFired = false; // this var breaks prediction for this weapon pretty hard otherwise
	CALL_ORIGINAL(rcx);
}

MAKE_HOOK(CTFGrapplingHook_PrimaryAttack, S::CTFGrapplingHook_PrimaryAttack(), void,
	void* rcx)
{
	bool bOriginalFirstTimePredicted = I::Prediction->m_bFirstTimePredicted;
	I::Prediction->m_bFirstTimePredicted = !s_bOriginalFired; // don't spam deny sound
	CALL_ORIGINAL(rcx);
	I::Prediction->m_bFirstTimePredicted = bOriginalFirstTimePredicted;
}