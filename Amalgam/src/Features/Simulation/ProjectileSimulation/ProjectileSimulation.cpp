#include "ProjectileSimulation.h"

#include "../../EnginePrediction/EnginePrediction.h"
#include "../../CritHack/CritHack.h"

bool CProjectileSimulation::GetInfoMain(CTFPlayer* pPlayer, CTFWeaponBase* pWeapon, Vec3 vAngles, ProjectileInfo& out, int iFlags, float flAutoCharge)
{
	if (!pPlayer || !pPlayer->IsAlive() || pPlayer->IsAGhost() || pPlayer->IsTaunting() || !pWeapon)
		return false;

	bool bTrace = iFlags & ProjSimEnum::Trace;
	bool bQuick = iFlags & ProjSimEnum::Quick;

	static auto sv_gravity = U::ConVars.FindVar("sv_gravity");
	static auto cl_flipviewmodels = U::ConVars.FindVar("cl_flipviewmodels");

	bool bDucking = pPlayer->m_fFlags() & FL_DUCKING;
	float flGravity = (sv_gravity ? sv_gravity->GetFloat() : 800.f) / 800.f;

	Vec3 vPos, vAngle;

	if (!bQuick && G::CurrentUserCmd)
	{
		switch (pWeapon->GetWeaponID())
		{
		case TF_WEAPON_ROCKETLAUNCHER:
		case TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT:
		case TF_WEAPON_PARTICLE_CANNON:
		case TF_WEAPON_RAYGUN:
		case TF_WEAPON_DRG_POMSON:
		case TF_WEAPON_GRENADELAUNCHER:
		case TF_WEAPON_PIPEBOMBLAUNCHER:
		case TF_WEAPON_CANNON:
		case TF_WEAPON_FLAREGUN:
		case TF_WEAPON_FLAREGUN_REVENGE:
		case TF_WEAPON_COMPOUND_BOW:
		case TF_WEAPON_CROSSBOW:
		case TF_WEAPON_SHOTGUN_BUILDING_RESCUE:
		case TF_WEAPON_SYRINGEGUN_MEDIC:
		case TF_WEAPON_GRAPPLINGHOOK:
		{
			float flOldCurrentTime = I::GlobalVars->curtime;
			I::GlobalVars->curtime = TICKS_TO_TIME(pPlayer->m_nTickBase());

			int iCmdNum = iFlags & ProjSimEnum::PredictCmdNum ? F::CritHack.PredictCmdNum(pPlayer, pWeapon, G::CurrentUserCmd) : G::CurrentUserCmd->command_number;
			SDK::RandomSeed(SDK::SeedFileLineHash(MD5_PseudoRandom(iCmdNum) & 0x7FFFFFFF, "SelectWeightedSequence", 0));
			for (int i = 0; i < 6; ++i)
				SDK::RandomFloat();

			Vec3 vAngAdd = pWeapon->GetSpreadAngles() - I::EngineClient->GetViewAngles();
			switch (pWeapon->GetWeaponID())
			{
			case TF_WEAPON_COMPOUND_BOW:
				// done after the projectile is created and not before, position may be a bit off
				if (pWeapon->As<CTFPipebombLauncher>()->m_flChargeBeginTime() > 0.f && I::GlobalVars->curtime - pWeapon->As<CTFPipebombLauncher>()->m_flChargeBeginTime() > 5.0f)
				{
					vAngAdd.x += -6.f + SDK::RandomInt() / float(0x7FFF) * 12.f;
					vAngAdd.y += -6.f + SDK::RandomInt() / float(0x7FFF) * 12.f;
				}
				break;
			case TF_WEAPON_SYRINGEGUN_MEDIC:
				vAngAdd.x += SDK::RandomFloat(-1.5f, 1.5f);
				vAngAdd.y += SDK::RandomFloat(-1.5f, 1.5f);
			}
			if (!(iFlags & ProjSimEnum::NoRandomAngles)) // don't do angle stuff for aimbot, nospread will pick that up
				vAngles += vAngAdd;

			I::GlobalVars->curtime = flOldCurrentTime;
		}
		}
	}

	if (Vars::Visuals::Trajectory::Overwrite.Value)
	{
		SDK::GetProjectileFireSetup(pPlayer, vAngles, { Vars::Visuals::Trajectory::OffX.Value, Vars::Visuals::Trajectory::OffY.Value, Vars::Visuals::Trajectory::OffZ.Value }, vPos, vAngle, !bTrace ? true : Vars::Visuals::Trajectory::Pipes.Value, bQuick);
		out = { TF_PROJECTILE_NONE, vPos, vAngle, { Vars::Visuals::Trajectory::Hull.Value, Vars::Visuals::Trajectory::Hull.Value, Vars::Visuals::Trajectory::Hull.Value }, Vars::Visuals::Trajectory::Speed.Value, Vars::Visuals::Trajectory::Gravity.Value, Vars::Visuals::Trajectory::NoSpin.Value, Vars::Visuals::Trajectory::LifeTime.Value };
		return true;
	}

	switch (pWeapon->GetWeaponID())
	{
	case TF_WEAPON_ROCKETLAUNCHER:
	case TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT:
	{
		SDK::GetProjectileFireSetup(pPlayer, vAngles, { 23.5f, int(SDK::AttribHookValue(0, "centerfire_projectile", pWeapon)) == 1 ? 0.f : 12.f, bDucking ? 8.f : -3.f }, vPos, vAngle, !bTrace ? true : false, bQuick);
		float flSpeed = pPlayer->InCond(TF_COND_RUNE_PRECISION) ? 3000.f : SDK::AttribHookValue(1100.f, "mult_projectile_speed", pWeapon);
		out = { TF_PROJECTILE_ROCKET, vPos, vAngle, { 0.f, 0.f, 0.f }, flSpeed, 0.f, true };
		return true;
	}
	case TF_WEAPON_PARTICLE_CANNON:
	case TF_WEAPON_RAYGUN:
	case TF_WEAPON_DRG_POMSON:
	{
		bool bCowMangler = pWeapon->GetWeaponID() == TF_WEAPON_PARTICLE_CANNON;

		SDK::GetProjectileFireSetup(pPlayer, vAngles, { 23.5f, 8.f, bDucking ? 8.f : -3.f }, vPos, vAngle, !bTrace ? true : false, bQuick);
		if (pWeapon->GetWeaponID() == TF_WEAPON_DRG_POMSON)
			vPos.z -= 13.f;
		float flSpeed = bCowMangler ? 1100.f : 1200.f;
		out = { TF_PROJECTILE_ENERGY_RING, vPos, vAngle, bCowMangler ? Vec3() : Vec3(1.f, 1.f, 1.f), flSpeed, 0.f, true };
		return true;
	}
	case TF_WEAPON_GRENADELAUNCHER: // vphysics projectiles affected by server start gravity
	case TF_WEAPON_CANNON:
	{
		bool bCannon = pWeapon->GetWeaponID() == TF_WEAPON_CANNON;
		float flMortar = bCannon ? SDK::AttribHookValue(0.f, "grenade_launcher_mortar_mode", pWeapon) : 0.f;

		SDK::GetProjectileFireSetup(pPlayer, vAngles, { 16.f, 8.f, -6.f }, vPos, vAngle, true, bQuick);
		float flSpeed = pPlayer->InCond(TF_COND_RUNE_PRECISION) ? 3000.f : SDK::AttribHookValue(1200.f, "mult_projectile_speed", pWeapon);
		float flLifetime = flMortar
			? pWeapon->As<CTFGrenadeLauncher>()->m_flDetonateTime() > 0.f ? pWeapon->As<CTFGrenadeLauncher>()->m_flDetonateTime() - I::GlobalVars->curtime : flMortar
			: SDK::AttribHookValue(2.2f, "fuse_mult", pWeapon);
		out = { bCannon ? TF_PROJECTILE_CANNONBALL : TF_PROJECTILE_PIPEBOMB, vPos, vAngle, { 6.f, 6.f, 6.f }, flSpeed, 1.f, pWeapon->m_iItemDefinitionIndex() == Demoman_m_TheLochnLoad, flLifetime };
		return true;
	}
	case TF_WEAPON_PIPEBOMBLAUNCHER:
	{
		SDK::GetProjectileFireSetup(pPlayer, vAngles, { 16.f, 8.f, -6.f }, vPos, vAngle, true, bQuick);
		float flCharge = flAutoCharge > 0.f && pWeapon->m_iItemDefinitionIndex() != Demoman_s_StickyJumper
			? SDK::AttribHookValue(4.f, "stickybomb_charge_rate", pWeapon) * flAutoCharge
			: (pWeapon->As<CTFPipebombLauncher>()->m_flChargeBeginTime() > 0.f ? I::GlobalVars->curtime - pWeapon->As<CTFPipebombLauncher>()->m_flChargeBeginTime() : 0.f);
		float flSpeed = Math::RemapValClamped(flCharge, 0.f, SDK::AttribHookValue(4.f, "stickybomb_charge_rate", pWeapon), 900.f, 2400.f);
		out = { TF_PROJECTILE_PIPEBOMB_REMOTE, vPos, vAngle, { 6.f, 6.f, 6.f }, flSpeed, 1.f, false };
		return true;
	}
	case TF_WEAPON_FLAREGUN:
	{
		SDK::GetProjectileFireSetup(pPlayer, vAngles, { 23.5f, 12.f, bDucking ? 8.f : -3.f }, vPos, vAngle, !bTrace ? true : false, bQuick);
		out = { TF_PROJECTILE_FLARE, vPos, vAngle, { 1.f, 1.f, 1.f }, SDK::AttribHookValue(2000.f, "mult_projectile_speed", pWeapon), 0.3f * flGravity, true};
		return true;
	}
	case TF_WEAPON_FLAREGUN_REVENGE:
	{
		SDK::GetProjectileFireSetup(pPlayer, vAngles, { 23.5f, 12.f, bDucking ? 8.f : -3.f }, vPos, vAngle, !bTrace ? true : false, bQuick);
		out = { TF_PROJECTILE_FLARE, vPos, vAngle, { 1.f, 1.f, 1.f }, 3000.f, 0.45f * flGravity, true };
		return true;
	}
	case TF_WEAPON_COMPOUND_BOW:
	{
		SDK::GetProjectileFireSetup(pPlayer, vAngles, { 23.5f, 8.f, -3.f }, vPos, vAngle, !bTrace ? true : false, bQuick);
		float flCharge = pWeapon->As<CTFPipebombLauncher>()->m_flChargeBeginTime() > 0.f ? I::GlobalVars->curtime - pWeapon->As<CTFPipebombLauncher>()->m_flChargeBeginTime() : 0.f;
		float flSpeed = Math::RemapValClamped(flCharge, 0.f, 1.f, 1800.f, 2600.f);
		flGravity = Math::RemapValClamped(flCharge, 0.f, 1.f, 0.5f, 0.1f) * flGravity;
		out = { TF_PROJECTILE_ARROW, vPos, vAngle, { 1.f, 1.f, 1.f }, flSpeed, flGravity, true, 10.f /*arrows have some lifetime check for whatever reason*/ };
		return true;
	}
	case TF_WEAPON_CROSSBOW:
	case TF_WEAPON_SHOTGUN_BUILDING_RESCUE:
	{
		SDK::GetProjectileFireSetup(pPlayer, vAngles, { 23.5f, 8.f, -3.f }, vPos, vAngle, !bTrace ? true : false, bQuick);
		out = { TF_PROJECTILE_ARROW, vPos, vAngle, pWeapon->GetWeaponID() == TF_WEAPON_CROSSBOW ? Vec3(3.f, 3.f, 3.f) : Vec3(1.f, 1.f, 1.f), 2400.f, 0.2f * flGravity, true, 10.f /*arrows have some lifetime check for whatever reason*/ };
		return true;
	}
	case TF_WEAPON_SYRINGEGUN_MEDIC:
	{
		SDK::GetProjectileFireSetup(pPlayer, vAngles, { 16.f, 6.f, -8.f }, vPos, vAngle, !bTrace ? true : false, bQuick);
		out = { TF_PROJECTILE_SYRINGE, vPos, vAngle, { 1.f, 1.f, 1.f }, 1000.f, 0.3f * flGravity, true };
		return true;
	}
	case TF_WEAPON_FLAMETHROWER: // this inherits player velocity, possibly account for
	{
		static auto tf_flamethrower_boxsize = U::ConVars.FindVar("tf_flamethrower_boxsize");
		const float flHull = tf_flamethrower_boxsize ? tf_flamethrower_boxsize->GetFloat() : 12.f;
		bool bFlipped = cl_flipviewmodels ? cl_flipviewmodels->GetBool() : false;

		SDK::GetProjectileFireSetup(pPlayer, vAngles, { 40.f, bFlipped ? -5.f : 5.f /*doesn't flip*/, 0.f}, vPos, vAngle, true, bQuick);
		out = { TF_PROJECTILE_FLAME_ROCKET, vPos, vAngle, { flHull, flHull, flHull }, 1000.f, 0.f, true, 0.33f };
		return true;
	}
	case TF_WEAPON_FLAME_BALL:
	{
		bool bFlipped = cl_flipviewmodels ? cl_flipviewmodels->GetBool() : false;

		SDK::GetProjectileFireSetup(pPlayer, vAngles, { 70.f, bFlipped ? -7.f : 7.f /*doesn't flip*/, -9.f}, vPos, vAngle, !bTrace ? true : false, bQuick);
		out = { TF_PROJECTILE_BALLOFFIRE, vPos, vAngle, { 1.f, 1.f, 1.f /*damaging hull much bigger, shouldn't matter here*/ }, 3000.f, 0.f, true, 0.2f };
		return true;
	}
	case TF_WEAPON_CLEAVER:
	{
		SDK::GetProjectileFireSetup(pPlayer, vAngles, { 16.f, 8.f, -6.f }, vPos, vAngle, true, bQuick);
		out = { TF_PROJECTILE_CLEAVER, vPos, vAngle, { 1.f, 1.f, 10.f /*weird, probably still inaccurate*/ }, 3000.f, 1.f, !bTrace ? true : false, 2.2f };
		return true;
	}
	case TF_WEAPON_BAT_WOOD:
	case TF_WEAPON_BAT_GIFTWRAP:
	{
		static auto tf_scout_stunball_base_speed = U::ConVars.FindVar("tf_scout_stunball_base_speed");

		const bool bWrapAssassin = pWeapon->GetWeaponID() == TF_WEAPON_BAT_GIFTWRAP;
		
		SDK::GetProjectileFireSetup(pPlayer, vAngles, { 0.f, 0.f, 0.f }, vPos, vAngle, true, bQuick);
		Vec3 vForward; Math::AngleVectors(vAngle, &vForward);
		vPos = (bQuick ? pPlayer->GetAbsOrigin() : pPlayer->m_vecOrigin()) + (Vec3(0, 0, 50) + vForward * 32.f) * pPlayer->m_flModelScale(); // why?
		out = { bWrapAssassin ? TF_PROJECTILE_FESTIVE_ARROW : TF_PROJECTILE_THROWABLE, vPos, vAngle, { 3.f, 3.f, 3.f }, tf_scout_stunball_base_speed ? tf_scout_stunball_base_speed->GetInt() : 3000.f, 1.f, false, bWrapAssassin ? 2.3f : 100.f };
		return true;
	}
	case TF_WEAPON_JAR:
	case TF_WEAPON_JAR_MILK:
	{
		SDK::GetProjectileFireSetup(pPlayer, vAngles, { 16.f, 8.f, -6.f }, vPos, vAngle, true, bQuick);
		out = { TF_PROJECTILE_JAR, vPos, vAngle, { 3.f, 3.f, 3.f }, 1000.f, 1.f, false, 2.2f };
		return true;
	}
	case TF_WEAPON_JAR_GAS:
	{
		SDK::GetProjectileFireSetup(pPlayer, vAngles, { 16.f, 8.f, -6.f }, vPos, vAngle, true, bQuick);
		out = { TF_PROJECTILE_JAR_GAS, vPos, vAngle, { 3.f, 3.f, 3.f }, 2000.f, 1.f, false, 2.2f };
		return true;
	}
	case TF_WEAPON_GRAPPLINGHOOK:
	{
		SDK::GetProjectileFireSetup(pPlayer, vAngles, { 23.5f, -8.f, -3.f }, vPos, vAngle, false, bQuick);
		static auto tf_grapplinghook_projectile_speed = U::ConVars.FindVar("tf_grapplinghook_projectile_speed");
		static auto tf_grapplinghook_max_distance = U::ConVars.FindVar("tf_grapplinghook_max_distance");
		float flSpeed = tf_grapplinghook_projectile_speed ? tf_grapplinghook_projectile_speed->GetFloat() : 1500.f;
		if (pPlayer->InCond(TF_COND_RUNE_AGILITY))
		{
			switch (pPlayer->m_iClass())
			{
			case TF_CLASS_SOLDIER:
			case TF_CLASS_HEAVY: flSpeed = 2600.f; break;
			default: flSpeed = 3000.f;
			}
		}
		float flLifetime = (tf_grapplinghook_max_distance ? tf_grapplinghook_max_distance->GetFloat() : 2000.f) / flSpeed;
		out = { TF_PROJECTILE_GRAPPLINGHOOK, vPos, vAngle, { 1.2f, 1.2f, 1.2f }, flSpeed, 0.f, false, flLifetime };
		return true;
	}
	}

	switch (pWeapon->m_iItemDefinitionIndex())
	{
	case Heavy_s_RoboSandvich:
	case Heavy_s_Sandvich:
	case Heavy_s_FestiveSandvich:
	case Heavy_s_Fishcake:
	case Heavy_s_TheDalokohsBar:
	case Heavy_s_SecondBanana:
		SDK::GetProjectileFireSetup(pPlayer, vAngles, { 0.f, 0.f, -8.f }, vPos, vAngle, true, bQuick);
		vAngle -= Vec3(10, 0, 0);
		out = { TF_PROJECTILE_BREAD_MONSTER, vPos, vAngle, bQuick ? Vec3(4.f, 4.f, 4.f) : Vec3(17.f, 17.f, 17.f), 500.f, 1.f * flGravity, false };
		return true;
	}

	return false;
}

bool CProjectileSimulation::GetInfo(CTFPlayer* pPlayer, CTFWeaponBase* pWeapon, Vec3 vAngles, ProjectileInfo& out, int iFlags, float flAutoCharge)
{
	bool InitCheck = iFlags & ProjSimEnum::InitCheck;
	bool bQuick = iFlags & ProjSimEnum::Quick;

	const float flOldCurrentTime = I::GlobalVars->curtime;
	I::GlobalVars->curtime = TICKS_TO_TIME(pPlayer->m_nTickBase());
	bool bReturn = GetInfoMain(pPlayer, pWeapon, vAngles, out, iFlags, flAutoCharge);
	out.m_bQuick = bQuick;
	I::GlobalVars->curtime = flOldCurrentTime;

	if (!bReturn || !InitCheck)
		return bReturn;

	const Vec3 vStart = bQuick ? pPlayer->GetEyePosition() : pPlayer->GetShootPos();
	const Vec3 vEnd = out.m_vPos;

	CGameTrace trace = {};
	CTraceFilterWorldAndPropsOnly filter = {};
	SDK::TraceHull(vStart, vEnd, out.m_vHull * -1.f, out.m_vHull, MASK_SOLID, &filter, &trace);
	return !trace.DidHit();
}

bool CProjectileSimulation::Initialize(ProjectileInfo& info, bool bSimulate)
{
	if (!env)
		env = I::Physics->CreateEnvironment();

	if (!obj)
	{
		// it doesn't matter what the size is for non drag affected projectiles
		// pipes use the size below so it works out just fine
		CPhysCollide* col = I::PhysicsCollision->BBoxToCollide({ -2.f, -2.f, -2.f }, { 2.f, 2.f, 2.f });

		objectparams_t params = g_PhysDefaultObjectParams;
		params.damping = 0.f;
		params.rotdamping = 0.f;
		params.inertia = 0.f;
		params.rotInertiaLimit = 0.f;
		params.enableCollisions = false;

		obj = env->CreatePolyObject(col, 0, info.m_vPos, info.m_vAng, &params);

		obj->Wake();
	}

	if (!env || !obj)
		return false;

	//set drag
	{
		float flDrag = 0.f;
		Vec3 vDragBasis = {};
		Vec3 vAngDragBasis = {};

		// these values were dumped from the server by firing the projectiles with 0 0 0 angles
		// they are calculated in CPhysicsObject::RecomputeDragBases
		switch (info.m_iType)
		{
		case TF_PROJECTILE_NONE:
			flDrag = Vars::Visuals::Trajectory::Drag.Value;
			vDragBasis = { Vars::Visuals::Trajectory::DragBasisX.Value, Vars::Visuals::Trajectory::DragBasisY.Value, Vars::Visuals::Trajectory::DragBasisZ.Value };
			vAngDragBasis = { Vars::Visuals::Trajectory::AngDragBasisX.Value, Vars::Visuals::Trajectory::AngDragBasisY.Value, Vars::Visuals::Trajectory::AngDragBasisZ.Value };
			break;
		case TF_PROJECTILE_PIPEBOMB:
			flDrag = 1.f;
			vDragBasis = { 0.003902f, 0.009962f, 0.009962f };
			vAngDragBasis = { 0.003618f, 0.001514f, 0.001514f };
			break;
		case TF_PROJECTILE_PIPEBOMB_REMOTE:
		case TF_PROJECTILE_PIPEBOMB_PRACTICE:
			flDrag = 1.f;
			vDragBasis = { 0.007491f, 0.007491f, 0.007306f };
			vAngDragBasis = { 0.002777f, 0.002842f, 0.002812f };
			break;
		case TF_PROJECTILE_CANNONBALL:
			flDrag = 1.f;
			vDragBasis = { 0.020971f, 0.019420f, 0.020971f };
			vAngDragBasis = { 0.012997f, 0.013496f, 0.013714f };
			break;
		case TF_PROJECTILE_CLEAVER:
			flDrag = 1.f;
			vDragBasis = { 0.022287f, 0.005208f, 0.110697f };
			vAngDragBasis = { 0.013982f, 0.043243f, 0.003465f };
			break;
		case TF_PROJECTILE_THROWABLE:
			flDrag = 1.f;
			vDragBasis = { 0.009000f /*0.006645f*/, 0.006581f, 0.006710f};
			vAngDragBasis = { 0.002233f, 0.002246f, 0.002206f };
			break;
		case TF_PROJECTILE_FESTIVE_ARROW:
			flDrag = 1.f;
			vDragBasis = { 0.013500f /*0.010867f*/, 0.0108727f, 0.010804f };
			vAngDragBasis = { 0.002081f, 0.002162f, 0.002069f };
			break;
		case TF_PROJECTILE_JAR: // there are different drags for different models, though shouldn't matter too much here
			flDrag = 1.f;
			vDragBasis = { 0.005127f, 0.002925f, 0.004337f };
			vAngDragBasis = { 0.000641f, 0.001350f, 0.000717f };
			break;
		case TF_PROJECTILE_JAR_GAS:
			flDrag = 1.f;
			vDragBasis = { 0.026360f, 0.021780f, 0.058978f };
			vAngDragBasis = { 0.035050f, 0.031199f, 0.022922f };
		}

		obj->SetDragCoefficient(&flDrag, &flDrag);

		obj->m_dragBasis = vDragBasis;
		obj->m_angDragBasis = vAngDragBasis;
	}

	//set position and velocity
	{
		Vec3 vForward, vRight, vUp; Math::AngleVectors(info.m_vAng, &vForward, &vRight, &vUp);
		Vec3 vVelocity = vForward * info.m_flVelocity, vAngularVelocity;

		switch (info.m_iType)
		{
		case TF_PROJECTILE_NONE:
			vVelocity += vUp * Vars::Visuals::Trajectory::UpVelocity.Value;
			vAngularVelocity = { Vars::Visuals::Trajectory::AngVelocityX.Value, Vars::Visuals::Trajectory::AngVelocityY.Value, Vars::Visuals::Trajectory::AngVelocityZ.Value };
			break;
		case TF_PROJECTILE_PIPEBOMB:
		case TF_PROJECTILE_PIPEBOMB_REMOTE:
		case TF_PROJECTILE_PIPEBOMB_PRACTICE:
		case TF_PROJECTILE_CANNONBALL:
			if (!info.m_bQuick && G::CurrentUserCmd)
			{
				vVelocity += vUp * 200.f + vUp * SDK::RandomFloat(-10.f, 10.f) + vRight * SDK::RandomFloat(-10.f, 10.f);
				vAngularVelocity = { 600.f, float(SDK::RandomInt(-1200, 1200)), 0.f };
				break;
			}
			vVelocity += vUp * 200.f;
			vAngularVelocity = { 600.f, -1200.f, 0.f };
			break;
		case TF_PROJECTILE_CLEAVER:
			vVelocity = vForward * 10 + vUp;
			vVelocity.Normalize();
			vVelocity *= info.m_flVelocity;
			vAngularVelocity = { 0.f, 500.f, 0.f };
			break;
		case TF_PROJECTILE_THROWABLE:
		case TF_PROJECTILE_FESTIVE_ARROW:
			vVelocity = vForward * 10 + vUp;
			vVelocity.Normalize();
			vVelocity *= info.m_flVelocity;
			vAngularVelocity = { 0.f, 100.f, 0.f };
			break;
		case TF_PROJECTILE_JAR:
		case TF_PROJECTILE_JAR_GAS:
			vVelocity += vUp * 200.f;
			vAngularVelocity = { 300.f, 0.f, 0.f };
		}

		if (info.m_bNoSpin)
			vAngularVelocity.Zero();

		if (bSimulate && obj->m_dragBasis.IsZero()) // don't include vphysics projectiles
			vVelocity.z += 400.f * info.m_flGravity * TICK_INTERVAL; // i don't know why this makes it more accurate but it does

		obj->SetPosition(info.m_vPos, info.m_vAng, true);
		obj->SetVelocity(&vVelocity, &vAngularVelocity);
	}

	//set env params
	{
		float flMaxVelocity = 1000000.f;
		float vMaxAngularVelocity = 1000000.f;

		//only pipes need k_flMaxVelocity and k_flMaxAngularVelocity
		switch (info.m_iType)
		{
		case TF_PROJECTILE_NONE:
			if (Vars::Visuals::Trajectory::MaxVelocity.Value)
				flMaxVelocity = Vars::Visuals::Trajectory::MaxVelocity.Value;
			if (Vars::Visuals::Trajectory::MaxAngularVelocity.Value)
				vMaxAngularVelocity = Vars::Visuals::Trajectory::MaxAngularVelocity.Value;
			break;
		case TF_PROJECTILE_PIPEBOMB:
		case TF_PROJECTILE_PIPEBOMB_REMOTE:
		case TF_PROJECTILE_PIPEBOMB_PRACTICE:
		case TF_PROJECTILE_CANNONBALL:
		case TF_PROJECTILE_CLEAVER:
		case TF_PROJECTILE_THROWABLE:
		case TF_PROJECTILE_FESTIVE_ARROW:
			flMaxVelocity = k_flMaxVelocity;
			vMaxAngularVelocity = k_flMaxAngularVelocity;
		}

		physics_performanceparams_t params = {}; params.Defaults();
		params.maxVelocity = flMaxVelocity;
		params.maxAngularVelocity = vMaxAngularVelocity;

		env->SetPerformanceSettings(&params);
		env->SetAirDensity(2.f);
		env->SetGravity({ 0.f, 0.f, -(800.f * info.m_flGravity) });

		env->ResetSimulationClock(); // not needed?
	}

	RunTick(info, false); // simulate an initial time because dumb

	return true;
}

void CProjectileSimulation::RunTick(ProjectileInfo& info, bool bPath)
{
	if (!env)
		return;

	if (bPath)
		info.m_vPath.push_back(GetOrigin());

	env->Simulate(TICK_INTERVAL);

	/* // params.maxVelocity limits velocity uniformly
	Vec3 vVelocity, vAngular;
	obj->GetVelocity(&vVelocity, &vAngular);
	static auto sv_maxvelocity = U::ConVars.FindVar("sv_maxvelocity");
	const float flMaxVel = sv_maxvelocity ? sv_maxvelocity->GetFloat() : 3500.f;
	vVelocity = { std::clamp(vVelocity.x, -flMaxVel, flMaxVel), std::clamp(vVelocity.y, -flMaxVel, flMaxVel), std::clamp(vVelocity.z, -flMaxVel, flMaxVel) };
	obj->SetVelocity(&vVelocity, &vAngular);
	*/
}

Vec3 CProjectileSimulation::GetOrigin()
{
	if (!obj)
		return {};

	Vec3 vOut;
	obj->GetPosition(&vOut, nullptr);
	return vOut;
}

Vec3 CProjectileSimulation::GetVelocity()
{
	if (!obj)
		return {};

	Vec3 vOut;
	obj->GetVelocity(&vOut, nullptr);
	return vOut;
}