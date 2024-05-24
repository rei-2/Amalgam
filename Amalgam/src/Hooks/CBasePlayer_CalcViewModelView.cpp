#include "../SDK/SDK.h"

MAKE_SIGNATURE(CBasePlayer_CalcViewModelView, "client.dll", "48 89 74 24 ? 55 41 56 41 57 48 8D AC 24", 0x0);

MAKE_HOOK(C_BasePlayer_CalcViewModelView, S::CBasePlayer_CalcViewModelView(), void, __fastcall,
	void* ecx, CBaseEntity* pOwner, const Vec3& vEyePosition, Vec3& vEyeAngles)
{
	Vec3 vOffset = { float(Vars::Visuals::Viewmodel::OffsetX.Value), float(Vars::Visuals::Viewmodel::OffsetY.Value), float(Vars::Visuals::Viewmodel::OffsetZ.Value) };
	bool bOffset = !vOffset.IsZero();

	if (!Vars::Visuals::Viewmodel::ViewmodelAim.Value && !bOffset && !Vars::Visuals::Viewmodel::Roll.Value
		|| Vars::Visuals::UI::CleanScreenshots.Value && I::EngineClient->IsTakingScreenshot())
		return CALL_ORIGINAL(ecx, pOwner, vEyePosition, vEyeAngles);

	bool bFlip = false;
	{
		static auto cl_flipviewmodels = U::ConVars.FindVar("cl_flipviewmodels");
		if (cl_flipviewmodels ? cl_flipviewmodels->GetBool() : false)
			bFlip = !bFlip;
		auto pWeapon = H::Entities.GetWeapon();
		if (pWeapon && pWeapon->m_iWeaponID() == TF_WEAPON_COMPOUND_BOW)
			bFlip = !bFlip;
	}

	if (Vars::Visuals::Viewmodel::ViewmodelAim.Value)
	{
		auto pLocal = H::Entities.GetLocal();
		if (pLocal && pLocal->IsAlive())
		{
			static Vec3 vAng = {};
			static int iTick = 0;

			if (!G::AimPosition.IsZero())
			{
				vAng = Math::CalcAngle(vEyePosition, G::AimPosition);
				iTick = I::GlobalVars->tickcount;
			}

			if (abs(iTick - I::GlobalVars->tickcount) < 32)
			{
				Vec3 vDiff = I::EngineClient->GetViewAngles() - vAng;
				if (bFlip)
					vDiff.y *= -1;
				vEyeAngles = I::EngineClient->GetViewAngles() - vDiff;
			}
		}
	}

	Vec3 vNewEyePosition = vEyePosition;
	if (bOffset)
	{
		Vec3 vForward = {}, vRight = {}, vUp = {};
		Math::AngleVectors(vEyeAngles, &vForward, &vRight, &vUp);
		vNewEyePosition += (vRight * vOffset.x * (bFlip ? -1 : 1)) + (vForward * vOffset.y) + (vUp * vOffset.z);
	}
	if (Vars::Visuals::Viewmodel::Roll.Value)
		vEyeAngles.z += Vars::Visuals::Viewmodel::Roll.Value * (bFlip ? -1 : 1);

	CALL_ORIGINAL(ecx, pOwner, vNewEyePosition, vEyeAngles);
}