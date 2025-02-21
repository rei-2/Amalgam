#include "AntiAim.h"

#include "../../TickHandler/TickHandler.h"
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
	if (!pLocal || !pLocal->IsAlive() || pLocal->IsAGhost() || pLocal->IsTaunting() || pLocal->m_MoveType() != MOVETYPE_WALK || pLocal->InCond(TF_COND_HALLOWEEN_KART)
		|| G::Attacking == 1 || F::AutoRocketJump.IsRunning() || F::Ticks.m_bDoubletap // this m_bDoubletap check can probably be removed if we fix tickbase correctly
		|| pWeapon && pWeapon->m_iItemDefinitionIndex() == Soldier_m_TheBeggarsBazooka && pCmd->buttons & IN_ATTACK && !(G::LastUserCmd->buttons & IN_ATTACK))
		return false;

	if (pLocal->InCond(TF_COND_SHIELD_CHARGE) || pCmd->buttons & IN_ATTACK2 && (pLocal->m_bShieldEquipped() && pLocal->m_flChargeMeter() == 100.f
		|| pWeapon && pWeapon->GetWeaponID() == TF_WEAPON_LUNCHBOX && G::PrimaryWeaponType == EWeaponType::PROJECTILE && pWeapon->HasPrimaryAmmoForShot()))
		return false;

	return true;
}



void CAntiAim::FakeShotAngles(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	if (!pLocal || pLocal->m_MoveType() == MOVETYPE_WALK
		|| !Vars::AntiHack::AntiAim::InvalidShootPitch.Value || G::Attacking != 1 || G::PrimaryWeaponType != EWeaponType::HITSCAN)
		return;

	G::SilentAngles = true;
	pCmd->viewangles.x = -pCmd->viewangles.x + 180;
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

int CAntiAim::GetEdge(CTFPlayer* pEntity, const float flEdgeOrigYaw, bool bUpPitch)
{
	// we should probably make this dynamic for different head positions

	float flSize = pEntity->m_vecMaxs().y - pEntity->m_vecMins().y;
	float flEdgeLeftDist = EdgeDistance(pEntity, flEdgeOrigYaw, -flSize);
	float flEdgeRightDist = EdgeDistance(pEntity, flEdgeOrigYaw, flSize);

	if (flEdgeLeftDist > 299.f && flEdgeRightDist > 299.f)
		return bUpPitch ? 1 : -1;
	return (bUpPitch ? flEdgeLeftDist > flEdgeRightDist : flEdgeLeftDist < flEdgeRightDist) ? 1 : -1;
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

inline int GetJitter(uint32_t uHash)
{
	static std::unordered_map<uint32_t, bool> mJitter = {};

	if (!I::ClientState->chokedcommands)
		mJitter[uHash] = !mJitter[uHash];
	return mJitter[uHash] ? 1 : -1;
}

float CAntiAim::GetYawOffset(CTFPlayer* pEntity, bool bFake)
{
	const int iMode = bFake ? Vars::AntiHack::AntiAim::YawFake.Value : Vars::AntiHack::AntiAim::YawReal.Value;
	const bool bUpPitch = (bFake ? Vars::AntiHack::AntiAim::PitchFake.Value : Vars::AntiHack::AntiAim::PitchReal.Value) == Vars::AntiHack::AntiAim::PitchRealEnum::Up;
	int iJitter = GetJitter(FNV1A::Hash32Const("Yaw"));

	switch (iMode)
	{
	case Vars::AntiHack::AntiAim::YawEnum::Forward: return 0.f;
	case Vars::AntiHack::AntiAim::YawEnum::Left: return 90.f;
	case Vars::AntiHack::AntiAim::YawEnum::Right: return -90.f;
	case Vars::AntiHack::AntiAim::YawEnum::Backwards: return 180.f;
	case Vars::AntiHack::AntiAim::YawEnum::Edge: return (bFake ? Vars::AntiHack::AntiAim::FakeYawValue.Value : Vars::AntiHack::AntiAim::RealYawValue.Value) * GetEdge(pEntity, I::EngineClient->GetViewAngles().y, bUpPitch);
	case Vars::AntiHack::AntiAim::YawEnum::Jitter: return (bFake ? Vars::AntiHack::AntiAim::FakeYawValue.Value : Vars::AntiHack::AntiAim::RealYawValue.Value) * iJitter;
	case Vars::AntiHack::AntiAim::YawEnum::Spin: return fmod(I::GlobalVars->tickcount * Vars::AntiHack::AntiAim::SpinSpeed.Value + 180.f, 360.f) - 180.f;
	}
	return 0.f;
}

float CAntiAim::GetBaseYaw(CTFPlayer* pLocal, CUserCmd* pCmd, bool bFake)
{
	const int iMode = bFake ? Vars::AntiHack::AntiAim::FakeYawMode.Value : Vars::AntiHack::AntiAim::RealYawMode.Value;
	const float flOffset = bFake ? Vars::AntiHack::AntiAim::FakeYawOffset.Value : Vars::AntiHack::AntiAim::RealYawOffset.Value;
	switch (iMode) // 0 offset, 1 at player
	{
	case Vars::AntiHack::AntiAim::YawModeEnum::View: return pCmd->viewangles.y + flOffset;
	case Vars::AntiHack::AntiAim::YawModeEnum::Target:
	{
		float flSmallestAngleTo = 0.f; float flSmallestFovTo = 360.f;
		for (auto pEntity : H::Entities.GetGroup(EGroupType::PLAYERS_ENEMIES))
		{
			auto pPlayer = pEntity->As<CTFPlayer>();
			if (pPlayer->IsDormant() || !pPlayer->IsAlive() || pPlayer->IsAGhost() || F::PlayerUtils.IsIgnored(pPlayer->entindex()))
				continue;
			
			const Vec3 vAngleTo = Math::CalcAngle(pLocal->m_vecOrigin(), pPlayer->m_vecOrigin());
			const float flFOVTo = Math::CalcFov(I::EngineClient->GetViewAngles(), vAngleTo);

			if (flFOVTo < flSmallestFovTo)
			{
				flSmallestAngleTo = vAngleTo.y;
				flSmallestFovTo = flFOVTo;
			}
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

float CAntiAim::GetPitch(float flCurPitch)
{
	float flRealPitch = 0.f, flFakePitch = 0.f;
	int iJitter = GetJitter(FNV1A::Hash32Const("Pitch"));

	switch (Vars::AntiHack::AntiAim::PitchReal.Value)
	{
	case Vars::AntiHack::AntiAim::PitchRealEnum::Up: flRealPitch = -89.f; break;
	case Vars::AntiHack::AntiAim::PitchRealEnum::Down: flRealPitch = 89.f; break;
	case Vars::AntiHack::AntiAim::PitchRealEnum::Zero: flRealPitch = 0.f; break;
	case Vars::AntiHack::AntiAim::PitchRealEnum::Jitter: flRealPitch = -89.f * iJitter; break;
	case Vars::AntiHack::AntiAim::PitchRealEnum::ReverseJitter: flRealPitch = 89.f * iJitter; break;
	}

	switch (Vars::AntiHack::AntiAim::PitchFake.Value)
	{
	case Vars::AntiHack::AntiAim::PitchFakeEnum::Up: flFakePitch = -89.f; break;
	case Vars::AntiHack::AntiAim::PitchFakeEnum::Down: flFakePitch = 89.f; break;
	case Vars::AntiHack::AntiAim::PitchFakeEnum::Jitter: flFakePitch = -89.f * iJitter; break;
	case Vars::AntiHack::AntiAim::PitchFakeEnum::ReverseJitter: flFakePitch = 89.f * iJitter; break;
	}

	if (Vars::AntiHack::AntiAim::PitchReal.Value && Vars::AntiHack::AntiAim::PitchFake.Value)
		return flRealPitch + (flFakePitch > 0.f ? 360 : -360);
	else if (Vars::AntiHack::AntiAim::PitchReal.Value)
		return flRealPitch;
	else if (Vars::AntiHack::AntiAim::PitchFake.Value)
		return flFakePitch;
	else
		return flCurPitch;
}

void CAntiAim::MinWalk(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	if (!Vars::AntiHack::AntiAim::MinWalk.Value || !F::AntiAim.YawOn() || !pLocal->m_hGroundEntity() || pLocal->InCond(TF_COND_HALLOWEEN_KART))
		return;

	if (!pCmd->forwardmove && !pCmd->sidemove && pLocal->m_vecVelocity().Length2D() < 2.f)
	{
		static bool bVar = true;
		float flMove = (pLocal->IsDucking() ? 3 : 1) * ((bVar = !bVar) ? 1 : -1);
		Vec3 vDir = { flMove, flMove, 0 };

		Vec3 vMove = Math::RotatePoint(vDir, {}, { 0, -pCmd->viewangles.y, 0 });
		pCmd->forwardmove = vMove.x * (fmodf(fabsf(pCmd->viewangles.x), 180.f) > 90.f ? -1 : 1);
		pCmd->sidemove = -vMove.y;

		pLocal->m_vecVelocity() = { 1, 1 }; // a bit stupid but it's probably fine
	}
}



void CAntiAim::Run(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd, bool bSendPacket)
{
	vEdgeTrace.clear();

	G::AntiAim = AntiAimOn() && ShouldRun(pLocal, pWeapon, pCmd);

	int iAntiBackstab = F::Misc.AntiBackstab(pLocal, pCmd, bSendPacket);
	if (!iAntiBackstab)
		FakeShotAngles(pLocal, pCmd);

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

	MinWalk(pLocal, pCmd);
}