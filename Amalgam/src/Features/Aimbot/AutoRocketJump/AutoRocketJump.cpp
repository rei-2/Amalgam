#include "AutoRocketJump.h"

#include "../../Simulation/ProjectileSimulation/ProjectileSimulation.h"
#include "../../Simulation/MovementSimulation/MovementSimulation.h"

void CAutoRocketJump::ManageAngle(CTFWeaponBase* pWeapon, CUserCmd* pCmd, Vec3& viewAngles)
{
	Vec3 wishVel = { pCmd->forwardmove, pCmd->sidemove, 0.f }, wishAng;
	Math::VectorAngles(wishVel, wishAng);

	const bool bMoving = wishVel.Length2D() > 200.f;

	float v_x = 0.f;
	float v_y = bMoving ? viewAngles.y - wishAng.y : viewAngles.y;
	if (pWeapon->m_iItemDefinitionIndex() == Soldier_m_TheOriginal)
	{
		v_x = bMoving ? 70.f : 89.f;
		v_y -= 180.f;
	}
	else
	{
		v_x = bMoving ? 75.f : 89.f;
		v_y -= bMoving ? 133.f : 81.5f;
	}
	viewAngles = { v_x, v_y, 0 };
}

void CAutoRocketJump::Run(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd)
{
	if (!pLocal || !pWeapon || !pCmd || !pLocal->IsAlive() || pLocal->IsAGhost() || I::EngineVGui->IsGameUIVisible() || I::MatSystemSurface->IsCursorVisible())
	{
		iFrame = -1;
		return;
	}

	bool bValidWeapon = false;
	{
		switch (pWeapon->m_iWeaponID())
		{
		case TF_WEAPON_ROCKETLAUNCHER:
		case TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT:
		case TF_WEAPON_PARTICLE_CANNON: bValidWeapon = true;
		}
	}
	if (bValidWeapon && (Vars::Misc::Movement::AutoRocketJump.Value || Vars::Misc::Movement::AutoCTap.Value))
		pCmd->buttons &= ~IN_ATTACK2; // fix for retarded issue
	if (!bValidWeapon)
	{
		iFrame = -1;
		return;
	}

	const bool bCurrGrounded = pLocal->OnSolid();

	// doesn't seem 100% consistent, unsure if it's fps related, user error, or what
	if (iFrame == -1 && (pWeapon->m_iItemDefinitionIndex() == Soldier_m_TheBeggarsBazooka ? G::IsAttacking : G::CanPrimaryAttack))
	{
		const bool bReloading = pWeapon->IsInReload();
		Vec3 viewAngles = pCmd->viewangles;
		if (Vars::Misc::Movement::AutoRocketJump.Value)
			ManageAngle(pWeapon, pCmd, viewAngles);

		bool bWillHit = false;
		if (Vars::Misc::Movement::AutoRocketJump.Value || Vars::Misc::Movement::AutoCTap.Value)
		{
			PlayerStorage localStorage;
			ProjectileInfo projInfo = {};
			if (F::MoveSim.Initialize(pLocal, localStorage, false) && F::ProjSim.GetInfo(pLocal, pWeapon, viewAngles, projInfo) && F::ProjSim.Initialize(projInfo))
			{
				F::ProjSim.RunTick(projInfo); // run an initial time because dumb
				for (int n = 1; n < 10; n++)
				{
					Vec3 Old = F::ProjSim.GetOrigin();
					F::ProjSim.RunTick(projInfo);
					Vec3 New = F::ProjSim.GetOrigin();

					F::MoveSim.RunTick(localStorage);

					CGameTrace trace = {};
					CTraceFilterProjectile filter = {};
					filter.pSkip = pLocal;
					SDK::Trace(Old, New, MASK_SOLID, &filter, &trace);
					if (trace.DidHit())
					{
						auto WillHit = [](CTFPlayer* pLocal, const Vec3& vOrigin, const Vec3& vPoint)
							{
								const Vec3 vOriginal = pLocal->GetAbsOrigin();
								pLocal->SetAbsOrigin(vOrigin);
								Vec3 vPos = {}; reinterpret_cast<CCollisionProperty*>(pLocal->GetCollideable())->CalcNearestPoint(vPoint, &vPos);
								pLocal->SetAbsOrigin(vOriginal);

								return vPoint.DistTo(vPos) < 120.f && SDK::VisPos(pLocal, pLocal, vPoint, vOrigin + pLocal->m_vecViewOffset(), MASK_SHOT);
							};

						bWillHit = WillHit(pLocal, localStorage.m_MoveData.m_vecAbsOrigin, trace.endpos);
						iDelay = std::max(n + (n > Vars::Misc::Movement::ApplyAbove.Value ? Vars::Misc::Movement::TimingOffset.Value : 0), 0);

						if (bWillHit)
						{
							SDK::Output("Auto jump", std::format("Ticks to hit: {} ({})", iDelay, n).c_str(), { 255, 0, 0, 255 }, Vars::Debug::Logging.Value);
							if (Vars::Debug::Info.Value)
							{
								G::LinesStorage.clear(); G::BoxesStorage.clear();
								G::LinesStorage.push_back({ {{ pLocal->GetShootPos(), {} }, { trace.endpos, {} }}, I::GlobalVars->curtime + 5.f, Vars::Colors::ProjectileColor.Value, true });
								Vec3 angles; Math::VectorAngles(trace.plane.normal, angles);
								G::BoxesStorage.push_back({ trace.endpos, { -1.f, -1.f, -1.f }, { 1.f, 1.f, 1.f }, angles, I::GlobalVars->curtime + 5.f, Vars::Colors::ProjectileColor.Value, {}, true });
							}
						}

						break;
					}
				}
			}
			F::MoveSim.Restore(localStorage);
		}

		if (bWillHit)
		{
			if (bCurrGrounded && bCurrGrounded == bLastGrounded && !pLocal->IsDucking())
			{
				if (Vars::Misc::Movement::AutoRocketJump.Value)
				{
					iFrame = 0;
					bFull = true;
				}
				else if (Vars::Misc::Movement::AutoCTap.Value)
					iFrame = 0;
			}
			else if (!bCurrGrounded && pCmd->buttons & IN_DUCK)
			{
				if (pWeapon->m_iItemDefinitionIndex() != Soldier_m_TheBeggarsBazooka)
					pCmd->buttons |= IN_ATTACK;

				if (Vars::Misc::Movement::AutoRocketJump.Value && !bReloading)
				{
					G::SilentAngles = true; // would use G::PSilentAngles but that would mess with timing
					pCmd->viewangles = viewAngles;
				}
			}

			if (iFrame != -1 && bReloading && pWeapon->m_iItemDefinitionIndex() != Soldier_m_TheBeggarsBazooka)
			{
				iFrame = -1;
				bFull = false;
				pCmd->buttons |= IN_ATTACK;
			}
		}

		if (iFrame == -1 && pWeapon->m_iWeaponID() == TF_WEAPON_PARTICLE_CANNON && G::Buttons & IN_ATTACK2)
			pCmd->buttons |= IN_ATTACK2;
	}

	if (iFrame != -1)
	{
		iFrame++;
		G::IsAttacking = true; // even if we aren't attacking, prevent other stuff from messing with timing, e.g. antiaim

		if (iFrame == 1)
		{
			if (pWeapon->m_iItemDefinitionIndex() != Soldier_m_TheBeggarsBazooka)
				pCmd->buttons |= IN_ATTACK;

			if (bFull)
			{
				G::SilentAngles = true; // would use G::PSilentAngles but that would mess with timing
				ManageAngle(pWeapon, pCmd, pCmd->viewangles);
			}
		}

		if (iDelay > 1)
		{
			switch (iFrame - iDelay + 1)
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

		if (iFrame == iDelay + (iDelay > 1 ? 1 : 3))
		{
			iFrame = -1;
			bFull = false;
		}
	}

	bLastGrounded = bCurrGrounded;
}