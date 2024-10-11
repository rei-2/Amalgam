#include "AntiAim.h"

#include "../../Players/PlayerUtils.h"
#include "../../Misc/Misc.h"
#include "../../Aimbot/AutoRocketJump/AutoRocketJump.h"

bool CAntiAim::AntiAimOn()
{
	return Vars::AntiHack::AntiAim::Enabled.Value
		&& (Vars::AntiHack::AntiAim::PitchReal.Value
		|| Vars::AntiHack::AntiAim::PitchFake.Value
		|| Vars::AntiHack::AntiAim::YawReal.Value
		|| Vars::AntiHack::AntiAim::YawFake.Value
		|| Vars::AntiHack::AntiAim::RealYawMode.Value
		|| Vars::AntiHack::AntiAim::FakeYawMode.Value
		|| Vars::AntiHack::AntiAim::RealYawOffset.Value
		|| Vars::AntiHack::AntiAim::FakeYawOffset.Value);
}

bool CAntiAim::YawOn()
{
	return Vars::AntiHack::AntiAim::Enabled.Value
		&& (Vars::AntiHack::AntiAim::YawReal.Value
		|| Vars::AntiHack::AntiAim::YawFake.Value
		|| Vars::AntiHack::AntiAim::RealYawMode.Value
		|| Vars::AntiHack::AntiAim::FakeYawMode.Value
		|| Vars::AntiHack::AntiAim::RealYawOffset.Value
		|| Vars::AntiHack::AntiAim::FakeYawOffset.Value);
}

bool CAntiAim::ShouldRun(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd)
{
	if (!pLocal)
		return false;

	if (pLocal->IsCharging() || pCmd->buttons & IN_ATTACK2 && (pLocal->m_bShieldEquipped() && pLocal->m_flChargeMeter() == 100.f
		|| pWeapon && pWeapon->GetWeaponID() == TF_WEAPON_LUNCHBOX && G::PrimaryWeaponType == EWeaponType::PROJECTILE && pWeapon->HasPrimaryAmmoForShot()))
		return false;

	if (!pLocal->IsAlive() || pLocal->IsAGhost() || pLocal->IsTaunting() || pLocal->IsInBumperKart()
		|| G::IsAttacking || F::AutoRocketJump.IsRunning()
		|| pWeapon && pWeapon->m_iItemDefinitionIndex() == Soldier_m_TheBeggarsBazooka && pCmd->buttons & IN_ATTACK && !(G::LastUserCmd->buttons & IN_ATTACK))
		return false;

	return true;
}



void CAntiAim::FakeShotAngles(CUserCmd* pCmd)
{
	if (!Vars::AntiHack::AntiAim::InvalidShootPitch.Value || !G::IsAttacking || G::PrimaryWeaponType != EWeaponType::HITSCAN)
		return;

	G::SilentAngles = true;
	pCmd->viewangles.x = CalculateCustomRealPitch(-pCmd->viewangles.x) + 180;
	pCmd->viewangles.y += 180;
}

float CAntiAim::EdgeDistance(CTFPlayer* pEntity, float flEdgeRayYaw, float flOffset)
{
	Vec3 vForward, vRight; Math::AngleVectors({ 0, flEdgeRayYaw, 0 }, &vForward, &vRight, nullptr);

	Vec3 vCenter = pEntity->GetCenter() + vRight * flOffset;
	Vec3 vEndPos = vCenter + vForward * 300.f;

	CGameTrace trace = {};
	CTraceFilterWorldAndPropsOnly filter = {};
	SDK::Trace(vCenter, vEndPos, MASK_SHOT | CONTENTS_GRATE, &filter, &trace);

	vEdgeTrace.push_back({ vCenter, trace.endpos });

	return (trace.startpos - trace.endpos).Length2D();
}

bool CAntiAim::GetEdge(CTFPlayer* pEntity, const float flEdgeOrigYaw, bool bUpPitch)
{
	// we should probably make this dynamic for different head positions

	float flSize = pEntity->m_vecMaxs().y - pEntity->m_vecMins().y;
	float flEdgeLeftDist = EdgeDistance(pEntity, flEdgeOrigYaw, -flSize);
	float flEdgeRightDist = EdgeDistance(pEntity, flEdgeOrigYaw, flSize);

	if (flEdgeLeftDist > 299.f && flEdgeRightDist > 299.f)
		return bUpPitch;
	return bUpPitch ? flEdgeLeftDist > flEdgeRightDist : flEdgeLeftDist < flEdgeRightDist;
}

void CAntiAim::RunOverlapping(CTFPlayer* pEntity, CUserCmd* pCmd, float& flRealYaw, bool bFake, float flEpsilon)
{
	if (!Vars::AntiHack::AntiAim::AntiOverlap.Value || bFake)
		return;

	float flFakeYaw = GetBaseYaw(pEntity, pCmd, true) + GetYawOffset(pEntity, true);
	const float flYawDiff = RAD2DEG(Math::AngleDiffRad(DEG2RAD(flRealYaw), DEG2RAD(flFakeYaw)));
	if (fabsf(flYawDiff) < flEpsilon)
		flRealYaw += flYawDiff > 0 ? flEpsilon : -flEpsilon;
}

float CAntiAim::GetYawOffset(CTFPlayer* pEntity, bool bFake)
{
	const int iMode = bFake ? Vars::AntiHack::AntiAim::YawFake.Value : Vars::AntiHack::AntiAim::YawReal.Value;
	const bool bUpPitch = bFake ? Vars::AntiHack::AntiAim::PitchFake.Value == 1 : Vars::AntiHack::AntiAim::PitchReal.Value == 1;
	switch (iMode)
	{
		case 0: return 0.f;
		case 1: return 135.f;
		case 2: return -135.f;
		case 3: return 180.f;
		case 4: return (GetEdge(pEntity, I::EngineClient->GetViewAngles().y, bUpPitch) ? 1 : -1) * (bFake ? -135 : 135);
		case 5: return (bFake ? Vars::AntiHack::AntiAim::FakeJitter.Value : Vars::AntiHack::AntiAim::RealJitter.Value) * (I::GlobalVars->tickcount % 2 ? -1 : 1);
		case 6: return fmod(I::GlobalVars->tickcount * Vars::AntiHack::AntiAim::SpinSpeed.Value + 180.f, 360.f) - 180.f;
	}
	return 0.f;
}

float CAntiAim::GetBaseYaw(CTFPlayer* pLocal, CUserCmd* pCmd, bool bFake)
{
	const int iMode = bFake ? Vars::AntiHack::AntiAim::FakeYawMode.Value : Vars::AntiHack::AntiAim::RealYawMode.Value;
	const float flOffset = bFake ? Vars::AntiHack::AntiAim::FakeYawOffset.Value : Vars::AntiHack::AntiAim::RealYawOffset.Value;
	switch (iMode) // 0 offset, 1 at player
	{
		case 0: return pCmd->viewangles.y + flOffset;
		case 1:
		{
			float flSmallestAngleTo = 0.f; float flSmallestFovTo = 360.f;
			for (auto pEntity : H::Entities.GetGroup(EGroupType::PLAYERS_ENEMIES))
			{
				auto pPlayer = pEntity->As<CTFPlayer>();
				if (!pPlayer->IsAlive() || pPlayer->IsDormant())
					continue;

				PlayerInfo_t pi{};
				if (I::EngineClient->GetPlayerInfo(pPlayer->entindex(), &pi) && F::PlayerUtils.IsIgnored(pi.friendsID))
					continue;
			
				const Vec3 vAngleTo = Math::CalcAngle(pLocal->GetAbsOrigin(), pPlayer->GetAbsOrigin());
				const float flFOVTo = Math::CalcFov(I::EngineClient->GetViewAngles(), vAngleTo);

				if (flFOVTo < flSmallestFovTo) { flSmallestAngleTo = vAngleTo.y; flSmallestFovTo = flFOVTo; }
			}
			return (flSmallestFovTo == 360.f ? pCmd->viewangles.y + flOffset : flSmallestAngleTo + flOffset);
		}
	}
	return pCmd->viewangles.y;
}

float CAntiAim::GetYaw(CTFPlayer* pLocal, CUserCmd* pCmd, bool bFake)
{
	float flYaw = GetBaseYaw(pLocal, pCmd, bFake) + GetYawOffset(pLocal, bFake);
	RunOverlapping(pLocal, pCmd, flYaw, bFake);
	return flYaw;
}

float CAntiAim::CalculateCustomRealPitch(float flWishPitch)
{
	switch (Vars::AntiHack::AntiAim::PitchFake.Value)
	{
	case 1: return flWishPitch - 360;
	case 2: return flWishPitch + 360;
	case 3: return flWishPitch - 360 * (I::GlobalVars->tickcount % 2 ? -1 : 1);
	case 4: return flWishPitch + 360 * (I::GlobalVars->tickcount % 2 ? -1 : 1);
	default: return flWishPitch;
	}
}

float CAntiAim::GetPitch(float flCurPitch)
{
	switch (Vars::AntiHack::AntiAim::PitchReal.Value)
	{
	case 1: return CalculateCustomRealPitch(-89.f);
	case 2: return CalculateCustomRealPitch(89.f);
	case 3: return CalculateCustomRealPitch(0.f);
	case 4: return CalculateCustomRealPitch(-89.f * (I::GlobalVars->tickcount % 2 ? -1 : 1));
	case 5: return CalculateCustomRealPitch(89.f * (I::GlobalVars->tickcount % 2 ? -1 : 1));
	}

	switch (Vars::AntiHack::AntiAim::PitchFake.Value)
	{
	case 1: return -89.f;
	case 2: return 89.f;
	case 3: return -89.f * (I::GlobalVars->tickcount % 2 ? -1 : 1);
	case 4: return 89.f * (I::GlobalVars->tickcount % 2 ? -1 : 1);
	}

	return flCurPitch;
}



void CAntiAim::Run(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd, bool bSendPacket)
{
	vEdgeTrace.clear();

	G::AntiAim = AntiAimOn() && ShouldRun(pLocal, pWeapon, pCmd);

	int iAntiBackstab = F::Misc.AntiBackstab(pLocal, pCmd, bSendPacket);
	if (!iAntiBackstab)
		FakeShotAngles(pCmd);

	if (!G::AntiAim)
	{
		vRealAngles = { pCmd->viewangles.x, pCmd->viewangles.y };
		vFakeAngles = { pCmd->viewangles.x, pCmd->viewangles.y };
		return;
	}

	Vec2& vAngles = bSendPacket ? vFakeAngles : vRealAngles;
	vAngles.x = iAntiBackstab != 2 ? GetPitch(pCmd->viewangles.x) : pCmd->viewangles.x;
	vAngles.y = !iAntiBackstab ? GetYaw(pLocal, pCmd, bSendPacket) : pCmd->viewangles.y;

	SDK::FixMovement(pCmd, vAngles);
	pCmd->viewangles.x = vAngles.x;
	pCmd->viewangles.y = vAngles.y;
}