#include "../SDK/SDK.h"

MAKE_SIGNATURE(CHudCrosshair_GetDrawPosition, "client.dll", "48 8B C4 55 53 56 41 54 41 55", 0x0);

MAKE_HOOK(CHudCrosshair_GetDrawPosition, S::CHudCrosshair_GetDrawPosition(), void,
	float* pX, float* pY, bool* pbBehindCamera, Vec3 angleCrosshairOffset)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::CHudCrosshair_GetDrawPosition[DEFAULT_BIND])
		return CALL_ORIGINAL(pX, pY, pbBehindCamera, angleCrosshairOffset);
#endif

	if (!Vars::Visuals::Viewmodel::CrosshairAim.Value && !Vars::Visuals::Thirdperson::Crosshair.Value
		|| Vars::Visuals::UI::CleanScreenshots.Value && I::EngineClient->IsTakingScreenshot())
		return CALL_ORIGINAL(pX, pY, pbBehindCamera, angleCrosshairOffset);

	auto pLocal = H::Entities.GetLocal();
	if (!pLocal)
		return CALL_ORIGINAL(pX, pY, pbBehindCamera, angleCrosshairOffset);

	bool bSet = false;

	if (Vars::Visuals::Viewmodel::CrosshairAim.Value && pLocal->IsAlive() && G::AimPoint.m_iTickCount)
	{
		Vec3 vScreen;
		if (SDK::W2S(G::AimPoint.m_vOrigin, vScreen))
		{
			if (pX) *pX = vScreen.x;
			if (pY) *pY = vScreen.y;
			if (pbBehindCamera) *pbBehindCamera = false;
			bSet = true;
		}
	}

	if (Vars::Visuals::Thirdperson::Crosshair.Value && !bSet && I::Input->CAM_IsThirdPerson())
	{
		Vec3 vAngles = I::EngineClient->GetViewAngles();
		Vec3 vForward; Math::AngleVectors(vAngles, &vForward);

		Vec3 vStartPos = pLocal->GetEyePosition();
		Vec3 vEndPos = (vStartPos + vForward * 8192);

		CGameTrace trace = {};
		CTraceFilterHitscan filter = {}; filter.pSkip = pLocal;
		SDK::Trace(vStartPos, vEndPos, MASK_SHOT, &filter, &trace);

		Vec3 vScreen;
		if (SDK::W2S(trace.endpos, vScreen))
		{
			if (pX) *pX = vScreen.x;
			if (pY) *pY = vScreen.y;
			if (pbBehindCamera) *pbBehindCamera = false;
			bSet = true;
		}
	}

	if (!bSet)
		CALL_ORIGINAL(pX, pY, pbBehindCamera, angleCrosshairOffset);
}