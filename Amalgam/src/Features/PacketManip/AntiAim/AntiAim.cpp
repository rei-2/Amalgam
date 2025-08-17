#include "AntiAim.h"

#include "../../Ticks/Ticks.h"
#include "../../Players/PlayerUtils.h"
#include "../../Misc/Misc.h"
#include "../../Aimbot/AutoRocketJump/AutoRocketJump.h"

bool CAntiAim::AntiAimOn()
{
	return Vars::AntiAim::Enabled.Value
		&& (Vars::AntiAim::PitchReal.Value
		|| Vars::AntiAim::PitchFake.Value
		|| Vars::AntiAim::YawReal.Value
		|| Vars::AntiAim::YawFake.Value
		|| Vars::AntiAim::RealYawBase.Value
		|| Vars::AntiAim::FakeYawBase.Value
		|| Vars::AntiAim::RealYawOffset.Value
		|| Vars::AntiAim::FakeYawOffset.Value);
}

bool CAntiAim::YawOn()
{
	return Vars::AntiAim::Enabled.Value
		&& (Vars::AntiAim::YawReal.Value
		|| Vars::AntiAim::YawFake.Value
		|| Vars::AntiAim::RealYawBase.Value
		|| Vars::AntiAim::FakeYawBase.Value
		|| Vars::AntiAim::RealYawOffset.Value
		|| Vars::AntiAim::FakeYawOffset.Value);
}

bool CAntiAim::ShouldRun(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd)
{
	if (!pLocal->IsAlive() || pLocal->IsAGhost() || pLocal->IsTaunting() || pLocal->m_MoveType() != MOVETYPE_WALK || pLocal->InCond(TF_COND_HALLOWEEN_KART)
		|| G::Attacking == 1 || F::AutoRocketJump.IsRunning() || F::Ticks.m_bDoubletap // this m_bDoubletap check can probably be removed if we fix tickbase correctly
		|| pWeapon && pWeapon->m_iItemDefinitionIndex() == Soldier_m_TheBeggarsBazooka && pCmd->buttons & IN_ATTACK && !(G::LastUserCmd->buttons & IN_ATTACK))
		return false;

	if (pLocal->InCond(TF_COND_SHIELD_CHARGE) || pCmd->buttons & IN_ATTACK2 && pLocal->m_bShieldEquipped() && pLocal->m_flChargeMeter() == 100.f)
		return false;

	return true;
}



void CAntiAim::FakeShotAngles(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd)
{
	if (!Vars::AntiAim::InvalidShootPitch.Value || G::Attacking != 1 || G::PrimaryWeaponType != EWeaponType::HITSCAN || pLocal->m_MoveType() != MOVETYPE_WALK)
		return;

	switch (pWeapon ? pWeapon->GetWeaponID() : 0)
	{
	case TF_WEAPON_MEDIGUN:
	case TF_WEAPON_LASER_POINTER:
		return;
	}

	G::SilentAngles = true;
	if (!Vars::Aimbot::General::NoSpread.Value)
	{	// messes with nospread accuracy
		pCmd->viewangles.x = 180 - pCmd->viewangles.x;
		pCmd->viewangles.y += 180;
	}
	else
		pCmd->viewangles.x += 360 * (vFakeAngles.x < 0 ? -1 : 1);
}

static inline float EdgeDistance(CTFPlayer* pEntity, float flYaw, float flOffset)
{
	Vec3 vForward, vRight; Math::AngleVectors({ 0, flYaw, 0 }, &vForward, &vRight, nullptr);
	Vec3 vCenter = pEntity->GetCenter();

	CGameTrace trace = {};
	CTraceFilterWorldAndPropsOnly filter = {};
	SDK::Trace(vCenter, vCenter + vRight * flOffset, MASK_SHOT | CONTENTS_GRATE, &filter, &trace);
	F::AntiAim.vEdgeTrace.emplace_back(trace.startpos, trace.endpos);
	SDK::Trace(trace.endpos, trace.endpos + vForward * 300.f, MASK_SHOT | CONTENTS_GRATE, &filter, &trace);
	F::AntiAim.vEdgeTrace.emplace_back(trace.startpos, trace.endpos);

	return trace.fraction;
}

static inline int GetEdge(CTFPlayer* pEntity, const float flYaw)
{
	float flSize = pEntity->GetSize().y;
	float flEdgeLeftDist = EdgeDistance(pEntity, flYaw, -flSize);
	float flEdgeRightDist = EdgeDistance(pEntity, flYaw, flSize);

	return flEdgeLeftDist > flEdgeRightDist ? -1 : 1;
}

static inline int GetJitter(uint32_t uHash)
{
	static std::unordered_map<uint32_t, bool> mJitter = {};

	if (!I::ClientState->chokedcommands)
		mJitter[uHash] = !mJitter[uHash];
	return mJitter[uHash] ? 1 : -1;
}

float CAntiAim::GetYawOffset(CTFPlayer* pEntity, bool bFake)
{
	const int iMode = bFake ? Vars::AntiAim::YawFake.Value : Vars::AntiAim::YawReal.Value;
	int iJitter = GetJitter(FNV1A::Hash32Const("Yaw"));

	switch (iMode)
	{
	case Vars::AntiAim::YawEnum::Forward: return 0.f;
	case Vars::AntiAim::YawEnum::Left: return 90.f;
	case Vars::AntiAim::YawEnum::Right: return -90.f;
	case Vars::AntiAim::YawEnum::Backwards: return 180.f;
	case Vars::AntiAim::YawEnum::Edge: return (bFake ? Vars::AntiAim::FakeYawValue.Value : Vars::AntiAim::RealYawValue.Value) * GetEdge(pEntity, I::EngineClient->GetViewAngles().y);
	case Vars::AntiAim::YawEnum::Jitter: return (bFake ? Vars::AntiAim::FakeYawValue.Value : Vars::AntiAim::RealYawValue.Value) * iJitter;
	case Vars::AntiAim::YawEnum::Spin: return fmod(I::GlobalVars->tickcount * Vars::AntiAim::SpinSpeed.Value + 180.f, 360.f) - 180.f;
	}
	return 0.f;
}

float CAntiAim::GetBaseYaw(CTFPlayer* pLocal, CUserCmd* pCmd, bool bFake)
{
	const int iMode = bFake ? Vars::AntiAim::FakeYawBase.Value : Vars::AntiAim::RealYawBase.Value;
	const float flOffset = bFake ? Vars::AntiAim::FakeYawOffset.Value : Vars::AntiAim::RealYawOffset.Value;
	switch (iMode) // 0 offset, 1 at player
	{
	case Vars::AntiAim::YawModeEnum::View: return pCmd->viewangles.y + flOffset;
	case Vars::AntiAim::YawModeEnum::Target:
	{
		float flSmallestAngleTo = 0.f; float flSmallestFovTo = 360.f;
		for (auto pEntity : H::Entities.GetGroup(EGroupType::PLAYERS_ENEMIES))
		{
			auto pPlayer = pEntity->As<CTFPlayer>();
			if (!pPlayer->IsAlive() || pPlayer->IsAGhost() || F::PlayerUtils.IsIgnored(pPlayer->entindex()))
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

void CAntiAim::RunOverlapping(CTFPlayer* pEntity, CUserCmd* pCmd, float& flYaw, bool bFake, float flEpsilon)
{
	if (!Vars::AntiAim::AntiOverlap.Value || bFake)
		return;

	float flFakeYaw = GetBaseYaw(pEntity, pCmd, true) + GetYawOffset(pEntity, true);
	const float flYawDiff = Math::NormalizeAngle(flYaw - flFakeYaw);
	if (fabsf(flYawDiff) < flEpsilon)
		flYaw += flYawDiff > 0 ? flEpsilon : -flEpsilon;
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

	switch (Vars::AntiAim::PitchReal.Value)
	{
	case Vars::AntiAim::PitchRealEnum::Up: flRealPitch = -89.f; break;
	case Vars::AntiAim::PitchRealEnum::Down: flRealPitch = 89.f; break;
	case Vars::AntiAim::PitchRealEnum::Zero: flRealPitch = 0.f; break;
	case Vars::AntiAim::PitchRealEnum::Jitter: flRealPitch = -89.f * iJitter; break;
	case Vars::AntiAim::PitchRealEnum::ReverseJitter: flRealPitch = 89.f * iJitter; break;
	}

	switch (Vars::AntiAim::PitchFake.Value)
	{
	case Vars::AntiAim::PitchFakeEnum::Up: flFakePitch = -89.f; break;
	case Vars::AntiAim::PitchFakeEnum::Down: flFakePitch = 89.f; break;
	case Vars::AntiAim::PitchFakeEnum::Jitter: flFakePitch = -89.f * iJitter; break;
	case Vars::AntiAim::PitchFakeEnum::ReverseJitter: flFakePitch = 89.f * iJitter; break;
	}

	if (Vars::AntiAim::PitchReal.Value && Vars::AntiAim::PitchFake.Value)
		return flRealPitch + (flFakePitch > 0.f ? 360 : -360);
	else if (Vars::AntiAim::PitchReal.Value)
		return flRealPitch;
	else if (Vars::AntiAim::PitchFake.Value)
		return flFakePitch;
	else
		return flCurPitch;
}

void CAntiAim::MinWalk(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	if (!Vars::AntiAim::MinWalk.Value || !YawOn() || !pLocal->m_hGroundEntity() || pLocal->InCond(TF_COND_HALLOWEEN_KART))
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
	G::AntiAim = AntiAimOn() && ShouldRun(pLocal, pWeapon, pCmd);

	int iAntiBackstab = F::Misc.AntiBackstab(pLocal, pCmd, bSendPacket);
	if (!iAntiBackstab)
		FakeShotAngles(pLocal, pWeapon, pCmd);

	if (!G::AntiAim)
	{
		vRealAngles = { pCmd->viewangles.x, pCmd->viewangles.y };
		vFakeAngles = { pCmd->viewangles.x, pCmd->viewangles.y };
		return;
	}

	vEdgeTrace.clear();

	Vec2& vAngles = bSendPacket ? vFakeAngles : vRealAngles;
	vAngles.x = iAntiBackstab != 2 ? GetPitch(pCmd->viewangles.x) : pCmd->viewangles.x;
	vAngles.y = !iAntiBackstab ? GetYaw(pLocal, pCmd, bSendPacket) : pCmd->viewangles.y;

	if (Vars::Misc::Game::AntiCheatCompatibility.Value)
		Math::ClampAngles(vAngles);
	SDK::FixMovement(pCmd, vAngles);
	pCmd->viewangles.x = vAngles.x;
	pCmd->viewangles.y = vAngles.y;

	MinWalk(pLocal, pCmd);
}