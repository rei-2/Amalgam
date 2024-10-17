#include "AutoRocketJump.h"

#include "../../Simulation/ProjectileSimulation/ProjectileSimulation.h"
#include "../../Simulation/MovementSimulation/MovementSimulation.h"

void CAutoRocketJump::ManageAngle(CTFWeaponBase* pWeapon, CUserCmd* pCmd, Vec3& viewAngles)
{
	Vec3 vWishVel = { pCmd->forwardmove, pCmd->sidemove, 0.f };
	Vec3 vWishAng; Math::VectorAngles(vWishVel, vWishAng);

	const bool bMoving = vWishVel.Length2D() > 200.f;

	Vec3 vAngle = { 0.f, bMoving ? viewAngles.y - vWishAng.y : viewAngles.y, 0.f };
	if (pWeapon->m_iItemDefinitionIndex() == Soldier_m_TheOriginal)
	{
		vAngle.x = bMoving ? 70.f : 89.f;
		vAngle.y -= 180.f;
	}
	else
	{
		vAngle.x = bMoving ? 75.f : 89.f;
		vAngle.y -= bMoving ? 133.f : 81.5f;
	}
	viewAngles = vAngle;
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
		switch (pWeapon->GetWeaponID())
		{
		case TF_WEAPON_ROCKETLAUNCHER:
		case TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT:
		case TF_WEAPON_PARTICLE_CANNON: bValidWeapon = true;
		}
	}
	if (!bValidWeapon)
	{
		iFrame = -1;
		return;
	}
	else if (Vars::Misc::Movement::AutoRocketJump.Value || Vars::Misc::Movement::AutoCTap.Value)
		pCmd->buttons &= ~IN_ATTACK2; // fix for retarded issue

	static bool bStaticGrounded = false;
	bool bLastGrounded = bStaticGrounded, bCurrGrounded = bStaticGrounded = pLocal->m_hGroundEntity();

	const bool bReloading = pWeapon->IsInReload();
	if (iFrame == -1 && (pWeapon->m_iItemDefinitionIndex() == Soldier_m_TheBeggarsBazooka ? G::IsAttacking && !bReloading : G::CanPrimaryAttack))
	{
		Vec3 viewAngles = pCmd->viewangles;
		if (Vars::Misc::Movement::AutoRocketJump.Value)
			ManageAngle(pWeapon, pCmd, viewAngles);

		bool bWillHit = false;
		if (Vars::Misc::Movement::AutoRocketJump.Value || Vars::Misc::Movement::AutoCTap.Value)
		{
			PlayerStorage localStorage;
			ProjectileInfo projInfo = {};

			bool bProjSimSetup = F::ProjSim.GetInfo(pLocal, pWeapon, viewAngles, projInfo) && F::ProjSim.Initialize(projInfo);
			bool bMoveSimSetup = F::MoveSim.Initialize(pLocal, localStorage, false); // do move sim after to not mess with proj sim
			if (bMoveSimSetup && bProjSimSetup)
			{
				Vec3 vOriginal = F::ProjSim.GetOrigin();
				for (int n = 1; n < 10; n++)
				{
					Vec3 Old = F::ProjSim.GetOrigin();
					F::ProjSim.RunTick(projInfo);
					Vec3 New = F::ProjSim.GetOrigin();

					F::MoveSim.RunTick(localStorage);

					CGameTrace trace = {};
					CTraceFilterProjectile filter = {}; filter.pSkip = pLocal;
					SDK::Trace(Old, New, MASK_SOLID, &filter, &trace);
					if (trace.DidHit())
					{
						auto WillHit = [](CTFPlayer* pLocal, const Vec3& vOrigin, const Vec3& vPoint)
							{
								const Vec3 vOriginal = pLocal->GetAbsOrigin();
								pLocal->SetAbsOrigin(vOrigin);
								Vec3 vPos = {}; reinterpret_cast<CCollisionProperty*>(pLocal->GetCollideable())->CalcNearestPoint(vPoint, &vPos);
								pLocal->SetAbsOrigin(vOriginal);

								return vPoint.DistTo(vPos) < 120.f && SDK::VisPosWorld(pLocal, pLocal, vPoint, vOrigin + pLocal->m_vecViewOffset(), MASK_SHOT);
							};

						bWillHit = WillHit(pLocal, localStorage.m_MoveData.m_vecAbsOrigin, trace.endpos + trace.plane.normal);
						iDelay = std::max(n + (n > Vars::Misc::Movement::ApplyAbove.Value ? Vars::Misc::Movement::TimingOffset.Value : 0), 0);

						if (bWillHit)
						{
							SDK::Output("Auto jump", std::format("Ticks to hit: {} ({})", iDelay, n).c_str(), { 255, 0, 0, 255 }, Vars::Debug::Logging.Value);
							if (Vars::Debug::Info.Value)
							{
								G::LineStorage.clear(); G::BoxStorage.clear();
								G::LineStorage.push_back({ { vOriginal, trace.endpos }, I::GlobalVars->curtime + 5.f, {}, true});
								Vec3 angles; Math::VectorAngles(trace.plane.normal, angles);
								G::BoxStorage.push_back({ trace.endpos, { -1.f, -1.f, -1.f }, { 1.f, 1.f, 1.f }, angles, I::GlobalVars->curtime + 5.f, {}, { 0, 0, 0, 0 }, true });
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
				if (Vars::Misc::Movement::AutoRocketJump.Value || Vars::Misc::Movement::AutoCTap.Value)
					iFrame = 0;
				if (Vars::Misc::Movement::AutoRocketJump.Value)
					bFull = true;
			}
			else if (!bCurrGrounded && pCmd->buttons & IN_DUCK)
			{
				if (pWeapon->m_iItemDefinitionIndex() != Soldier_m_TheBeggarsBazooka)
					pCmd->buttons |= IN_ATTACK;

				if (Vars::Misc::Movement::AutoRocketJump.Value && !bReloading)
				{
					G::SilentAngles = true;
					pCmd->viewangles = viewAngles;
				}
			}

			if (iFrame != -1 && bReloading)
			{
				iFrame = -1;
				bFull = false;
				pCmd->buttons |= IN_ATTACK;
			}
		}

		if (iFrame == -1 && pWeapon->GetWeaponID() == TF_WEAPON_PARTICLE_CANNON && G::Buttons & IN_ATTACK2)
			pCmd->buttons |= IN_ATTACK2;
	}

	if (iFrame != -1)
	{
		iFrame++;

		if (iFrame == 1)
		{
			if (pWeapon->m_iItemDefinitionIndex() != Soldier_m_TheBeggarsBazooka)
				pCmd->buttons |= IN_ATTACK;

			if (bFull)
			{
				G::SilentAngles = true;
				ManageAngle(pWeapon, pCmd, pCmd->viewangles);
			}
		}

		if (iFrame <= Vars::Misc::Movement::ChokeCount.Value)
			G::PSilentAngles = true, G::SilentAngles = false; // only con to this is that you may not be able to choke depending on the ticks charged

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

		if (iFrame == iDelay + 3)
		{
			iFrame = -1;
			bFull = false;
		}
	}

	bRunning = iFrame != -1; // prevent stuff like anti-aim messing with timing
}