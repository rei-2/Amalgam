#include "AutoRocketJump.h"

#include "../../Simulation/ProjectileSimulation/ProjectileSimulation.h"
#include "../../Simulation/MovementSimulation/MovementSimulation.h"

bool CAutoRocketJump::SetAngles(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd)
{
	m_vAngles = pCmd->viewangles;
	if (!Vars::Misc::Movement::AutoRocketJump.Value)
		return true;

	ProjectileInfo tProjInfo = {};
	if (!F::ProjSim.GetInfo(pLocal, pWeapon, {}, tProjInfo, ProjSimEnum::NoRandomAngles)
		|| !F::ProjSim.Initialize(tProjInfo, false))
		return false;

	Vec3 vOrigin = pLocal->m_vecOrigin();
	Vec3 vLocalPos = pLocal->GetShootPos();
	Vec3 vOffset = tProjInfo.m_vPos - vLocalPos; vOffset.x = 0; //vOffset.y *= -1;
	float flVelocity = F::ProjSim.GetVelocity().Length();

	Vec3 vPoint;
	if (pLocal->IsOnGround())
	{
		float flOffset = sqrtf(2 * powf(vOffset.y, 2.f) + powf(vOffset.z, 2.f));
		Vec3 vWishVel = { pCmd->forwardmove, pCmd->sidemove, 0.f }, vDir;
		if (vWishVel.IsZero())
			vDir = { 0.f, m_vAngles.y, 0.f };
		else
		{
			Vec3 vWishAng = Math::VectorAngles(vWishVel);
			vDir = { 0.f, m_vAngles.y - vWishAng.y, 0.f };
		}
		Vec3 vForward; Math::AngleVectors(vDir, &vForward);
		vPoint = pLocal->m_vecOrigin() - vForward * flOffset;
	}
	else
	{
		//float flOffset = pLocal->m_vecMaxs().x;
		float flOffset = sqrtf(2 * powf(vOffset.y, 2.f) + powf(vOffset.z, 2.f));
		bool bShouldReturn = true;
		MoveStorage tStorage;
		if (F::MoveSim.Initialize(pLocal, tStorage, false))
		{
			for (int n = 1; n < 10; n++)
			{
				F::MoveSim.RunTick(tStorage);
				if (!pLocal->IsOnGround() || pLocal->IsSwimming())
					continue;

				Vec3 vForward = tStorage.m_MoveData.m_vecVelocity.Normalized2D();
				vPoint = tStorage.m_MoveData.m_vecAbsOrigin - vForward * flOffset; //- Vec3(0, 0, 20);
				bShouldReturn = false;
				break;
			}
		}
		F::MoveSim.Restore(tStorage);
		if (bShouldReturn)
			return false;
	}

	float flPitch, flYaw;
	{	// basic trajectory pass
		Vec3 vAngleTo = Math::CalcAngle(vLocalPos, vPoint);
		m_vAngles.x = flPitch = vAngleTo.x, m_vAngles.y = flYaw = vAngleTo.y;
	}

	if (!F::ProjSim.GetInfo(pLocal, pWeapon, { flPitch, flYaw, 0 }, tProjInfo, ProjSimEnum::Trace | ProjSimEnum::NoRandomAngles))
		return false;

	{	// correct yaw
		Vec3 vShootPos = (tProjInfo.m_vPos - vLocalPos).To2D();
		Vec3 vTarget = vPoint - vLocalPos;
		Vec3 vForward; Math::AngleVectors(tProjInfo.m_vAng, &vForward); vForward.Normalize2D();
		float flA = 1.f;
		float flB = 2 * (vShootPos.x * vForward.x + vShootPos.y * vForward.y);
		float flC = vShootPos.Length2DSqr() - vTarget.Length2DSqr();
		auto vSolutions = Math::SolveQuadratic(flA, flB, flC);
		if (!vSolutions.empty())
		{
			vShootPos += vForward * vSolutions.front();
			m_vAngles.y = flYaw - (RAD2DEG(atan2(vShootPos.y, vShootPos.x)) - flYaw);
			flYaw = RAD2DEG(atan2(vShootPos.y, vShootPos.x));
		}
	}

	{	// correct pitch
		Vec3 vShootPos = Math::RotatePoint(tProjInfo.m_vPos - vLocalPos, {}, { 0, -flYaw, 0 }); vShootPos.y = 0;
		Vec3 vTarget = Math::RotatePoint(vPoint - vLocalPos, {}, { 0, -flYaw, 0 });
		Vec3 vForward; Math::AngleVectors(tProjInfo.m_vAng - Vec3(0, flYaw, 0), &vForward); vForward.y = 0; vForward.Normalize();
		float flA = 1.f;
		float flB = 2 * (vShootPos.x * vForward.x + vShootPos.z * vForward.z);
		float flC = (powf(vShootPos.x, 2) + powf(vShootPos.z, 2)) - (powf(vTarget.x, 2) + powf(vTarget.z, 2));
		auto vSolutions = Math::SolveQuadratic(flA, flB, flC);
		if (!vSolutions.empty())
		{
			vShootPos += vForward * vSolutions.front();
			m_vAngles.x = flPitch - (RAD2DEG(atan2(-vShootPos.z, vShootPos.x)) - flPitch);
		}
	}

	return true;
}

void CAutoRocketJump::Run(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd)
{
	if (!pWeapon || !pLocal->IsAlive() || pLocal->IsAGhost() || I::EngineVGui->IsGameUIVisible())
	{
		m_iFrame = -1;
		return;
	}

	static bool bStaticGrounded = false;
	bool bLastGrounded = bStaticGrounded;
	bool bCurrGrounded = bStaticGrounded = pLocal->m_hGroundEntity();
	if (m_iFrame == -1)
	{
		bool bDuck = pLocal->IsDucking();
		if ((bCurrGrounded ? bDuck || I::GlobalVars->curtime < pLocal->m_flDuckTimer() : !bDuck))
			return;
	}

	bool bValidWeapon = false;
	{
		switch (pWeapon->GetWeaponID())
		{
		case TF_WEAPON_ROCKETLAUNCHER:
		case TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT:
		case TF_WEAPON_PARTICLE_CANNON: bValidWeapon = true;
		}
	}
	if (!bValidWeapon)
	{
		m_iFrame = -1;
		return;
	}
	else if (Vars::Misc::Movement::AutoRocketJump.Value || Vars::Misc::Movement::AutoCTap.Value)
		pCmd->buttons &= ~IN_ATTACK2; // fix for retarded issue

	if (m_iFrame == -1 && (pWeapon->m_iItemDefinitionIndex() == Soldier_m_TheBeggarsBazooka ? G::Attacking == 1 : G::CanPrimaryAttack || G::Reloading)
		&& SetAngles(pLocal, pWeapon, pCmd))
	{
		bool bWillHit = false;
		if (Vars::Misc::Movement::AutoRocketJump.Value || Vars::Misc::Movement::AutoCTap.Value)
		{
			MoveStorage tStorage;
			ProjectileInfo tProjInfo = {};

			bool bProjSimSetup = F::ProjSim.GetInfo(pLocal, pWeapon, m_vAngles, tProjInfo, ProjSimEnum::Trace | ProjSimEnum::InitCheck | ProjSimEnum::NoRandomAngles) && F::ProjSim.Initialize(tProjInfo);
			bool bMoveSimSetup = F::MoveSim.Initialize(pLocal, tStorage, false); // do move sim after to not mess with proj sim
			if (bMoveSimSetup && bProjSimSetup)
			{
				Vec3 vOriginal = F::ProjSim.GetOrigin();
				for (int n = 1; n < 10; n++)
				{
					Vec3 Old = F::ProjSim.GetOrigin();
					F::MoveSim.RunTick(tStorage);
					F::ProjSim.RunTick(tProjInfo);
					Vec3 New = F::ProjSim.GetOrigin();

					CGameTrace trace = {};
					CTraceFilterCollideable filter = {};
					filter.pSkip = pLocal;
					SDK::Trace(Old, New, MASK_SOLID, &filter, &trace);
					if (trace.DidHit())
					{
						if (!pLocal->IsOnGround() || pLocal->IsSwimming())
							break;

						auto WillHit = [](CTFPlayer* pLocal, const Vec3& vOrigin, const Vec3& vPoint)
							{
								const Vec3 vOriginal = pLocal->GetAbsOrigin();
								pLocal->SetAbsOrigin(vOrigin);
								Vec3 vPos; reinterpret_cast<CCollisionProperty*>(pLocal->GetCollideable())->CalcNearestPoint(vPoint, &vPos);
								pLocal->SetAbsOrigin(vOriginal);

								return vPoint.DistTo(vPos) < 120.f && SDK::VisPosWorld(pLocal, pLocal, vPoint, vOrigin + pLocal->m_vecViewOffset(), MASK_SHOT);
							};

						bWillHit = WillHit(pLocal, tStorage.m_MoveData.m_vecAbsOrigin, trace.endpos + trace.plane.normal);
						m_iDelay = std::max(n + (n > Vars::Misc::Movement::ApplyAbove.Value ? Vars::Misc::Movement::TimingOffset.Value : 0), 0);

						if (bWillHit && G::CanPrimaryAttack)
						{
							SDK::Output("Auto jump", std::format("Ticks to hit: {} ({})", m_iDelay, n).c_str(), { 255, 0, 0 }, Vars::Debug::Logging.Value);
							if (Vars::Debug::Info.Value)
							{
								//G::LineStorage.clear(); G::BoxStorage.clear();
								Vec3 vAngles = Math::VectorAngles(trace.plane.normal);
								G::BoxStorage.emplace_back(trace.endpos + trace.plane.normal, Vec3(-1.f, -1.f, -1.f), Vec3(1.f, 1.f, 1.f), vAngles, I::GlobalVars->curtime + 5.f, Color_t(), Color_t(0, 0, 0, 0), true);
								G::LineStorage.emplace_back(std::pair<Vec3, Vec3>(tStorage.m_MoveData.m_vecAbsOrigin + pLocal->m_vecViewOffset(), trace.endpos + trace.plane.normal), I::GlobalVars->curtime + 5.f, Color_t(), true);
							}
						}

						break;
					}

					// this will just prevent shooting speedshots when projectile isn't fast enough
					//if (!bCurrGrounded && !bLastGround && bCurGround)
					//	break;
				}
			}
			F::MoveSim.Restore(tStorage);
		}

		if (bWillHit)
		{
			if (bCurrGrounded && bCurrGrounded == bLastGrounded)
			{
				if (Vars::Misc::Movement::AutoRocketJump.Value || Vars::Misc::Movement::AutoCTap.Value)
					m_iFrame = 0;
				if (Vars::Misc::Movement::AutoRocketJump.Value)
					m_bFull = true;
			}
			else if (!bCurrGrounded)
			{
				if (pWeapon->m_iItemDefinitionIndex() != Soldier_m_TheBeggarsBazooka)
					pCmd->buttons |= IN_ATTACK;

				if (Vars::Misc::Movement::AutoRocketJump.Value && G::CanPrimaryAttack)
				{
					G::SilentAngles = true;
					pCmd->viewangles = m_vAngles;
				}
			}

			if (m_iFrame != -1 && G::Reloading)
			{
				m_iFrame = -1;
				m_bFull = false;
				pCmd->buttons |= IN_ATTACK;
			}
		}

		if (m_iFrame == -1 && pWeapon->GetWeaponID() == TF_WEAPON_PARTICLE_CANNON && G::OriginalCmd.buttons & IN_ATTACK2)
			pCmd->buttons |= IN_ATTACK2;
	}

	if (m_iFrame != -1)
	{
		m_iFrame++;
		pCmd->buttons &= ~IN_JUMP;

		if (m_iFrame == 1)
		{
			if (pWeapon->m_iItemDefinitionIndex() != Soldier_m_TheBeggarsBazooka)
				pCmd->buttons |= IN_ATTACK;

			if (m_bFull)
			{
				G::SilentAngles = bCurrGrounded;
				pCmd->viewangles = m_vAngles;
			}
		}

		if (m_iFrame <= Vars::Misc::Movement::ChokeCount.Value)
			G::PSilentAngles = true, G::SilentAngles = false; // only con to this is that you may not be able to choke depending on the ticks charged

		if (m_iDelay > 1)
		{
			switch (m_iFrame - m_iDelay + 1)
			{
			case 0:
				pCmd->buttons |= IN_DUCK;
				break;
			case 1:
				pCmd->buttons |= IN_JUMP;
			}
		}
		else // won't ctap in time
			pCmd->buttons |= IN_DUCK | IN_JUMP;

		if (m_iFrame == m_iDelay + 3)
		{
			m_iFrame = -1;
			m_bFull = false;
		}
	}

	m_bRunning = m_iFrame != -1; // prevent stuff like antiaim messing with timing
}