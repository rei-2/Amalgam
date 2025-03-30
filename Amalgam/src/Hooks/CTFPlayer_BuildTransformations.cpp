#include "../SDK/SDK.h"

MAKE_SIGNATURE(CTFPlayer_BuildTransformations, "client.dll", "48 8B C4 48 89 58 ? 48 89 68 ? 57 41 54", 0x0);

MAKE_HOOK(CTFPlayer_BuildTransformations, S::CTFPlayer_BuildTransformations(), void,
	void* rcx, CStudioHdr* hdr, Vector* pos, Quaternion q[], const matrix3x4& cameraTransform, int boneMask, void* boneComputed)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::CTFPlayer_BuildTransformations[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx, hdr, pos, q, cameraTransform, boneMask, boneComputed);
#endif

	auto pPlayer = reinterpret_cast<CTFPlayer*>(rcx);
	auto iOriginal = pPlayer->m_fFlags();
	pPlayer->m_fFlags() &= ~FL_DUCKING;

	CALL_ORIGINAL(rcx, hdr, pos, q, cameraTransform, boneMask, boneComputed);

	pPlayer->m_fFlags() = iOriginal;
}