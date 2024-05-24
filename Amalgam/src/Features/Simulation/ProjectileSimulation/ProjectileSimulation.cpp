#include "ProjectileSimulation.h"

bool CProjectileSimulation::GetInfoMain(CTFPlayer* pPlayer, CTFWeaponBase* pWeapon, const Vec3& vAngles, ProjectileInfo& out, bool bTrace, bool bQuick, float flCharge)
{
	if (!pPlayer || !pPlayer->IsAlive() || pPlayer->IsAGhost() || pPlayer->IsTaunting() || !pWeapon)
		return false;

	static auto sv_gravity = U::ConVars.FindVar("sv_gravity");
	static auto cl_flipviewmodels = U::ConVars.FindVar("cl_flipviewmodels");

	const bool bDucking = pPlayer->m_fFlags() & FL_DUCKING;
	const float flGravity = sv_gravity ? sv_gravity->GetFloat() : 800.f;
	const bool bFlipped = cl_flipviewmodels ? cl_flipviewmodels->GetBool() : false;

	Vec3 pos, ang;

	if (Vars::Visuals::Trajectory::Overwrite.Value)
	{
		SDK::GetProjectileFireSetup(pPlayer, vAngles, { Vars::Visuals::Trajectory::OffX.Value, Vars::Visuals::Trajectory::OffY.Value, Vars::Visuals::Trajectory::OffZ.Value }, pos, ang, !bTrace ? true : Vars::Visuals::Trajectory::Pipes.Value, bQuick);
		out = { TF_PROJECTILE_NONE, pos, ang, { Vars::Visuals::Trajectory::Hull.Value, Vars::Visuals::Trajectory::Hull.Value, Vars::Visuals::Trajectory::Hull.Value }, Vars::Visuals::Trajectory::Speed.Value, Vars::Visuals::Trajectory::Gravity.Value, Vars::Visuals::Trajectory::NoSpin.Value, Vars::Visuals::Trajectory::LifeTime.Value };
		return true;
	}

	switch (pWeapon->m_iWeaponID())
	{
	case TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT:
	case TF_WEAPON_ROCKETLAUNCHER:
	{
		SDK::GetProjectileFireSetup(pPlayer, vAngles, { 23.5f, int(SDK::AttribHookValue(0, "centerfire_projectile", pWeapon)) == 1 ? 0.f : 12.f, bDucking ? 8.f : -3.f }, pos, ang, !bTrace ? true : false, bQuick);
		const float speed = pPlayer->IsPrecisionRune() ? 3000.f : SDK::AttribHookValue(1100.f, "mult_projectile_speed", pWeapon);
		out = { TF_PROJECTILE_ROCKET, pos, ang, { 0.f, 0.f, 0.f }, bQuick ? 3783722.f : speed, 0.f, true };
		return true;
	}
	case TF_WEAPON_PARTICLE_CANNON:
	case TF_WEAPON_RAYGUN:
	case TF_WEAPON_DRG_POMSON:
	{
		SDK::GetProjectileFireSetup(pPlayer, vAngles, { 23.5f, 8.f, bDucking ? 8.f : -3.f }, pos, ang, !bTrace ? true : false, bQuick);
		if (pWeapon->m_iWeaponID() == TF_WEAPON_DRG_POMSON)
			pos.z -= 13.f;
		const float speed = pWeapon->m_iWeaponID() == TF_WEAPON_PARTICLE_CANNON ? 1100.f : 1200.f;
		out = { TF_PROJECTILE_ENERGY_RING, pos, ang, { 1.f, 1.f, 1.f }, bQuick ? 3783722.f : speed, 0.f, true };
		return true;
	}
	case TF_WEAPON_GRENADELAUNCHER: // vphysics projectiles affected by server start gravity
	case TF_WEAPON_CANNON:
	{
		const bool bCannon = pWeapon->m_iWeaponID() == TF_WEAPON_CANNON;
		const float flMortar = bCannon ? SDK::AttribHookValue(0.f, "grenade_launcher_mortar_mode", pWeapon) : 0.f;

		SDK::GetProjectileFireSetup(pPlayer, vAngles, { 16.f, 8.f, -6.f }, pos, ang, true, bQuick);
		const float speed = pPlayer->IsPrecisionRune() ? 3000.f : SDK::AttribHookValue(1200.f, "mult_projectile_speed", pWeapon);
		const float lifetime = flMortar
			? pWeapon->As<CTFGrenadeLauncher>()->m_flDetonateTime() > 0.f ? pWeapon->As<CTFGrenadeLauncher>()->m_flDetonateTime() - I::GlobalVars->curtime : flMortar
			: SDK::AttribHookValue(2.2f, "fuse_mult", pWeapon);
		out = { bCannon ? TF_PROJECTILE_CANNONBALL : TF_PROJECTILE_PIPEBOMB, pos, ang, { 5.f, 5.f, 5.f }, speed, 1.f, pWeapon->m_iItemDefinitionIndex() == Demoman_m_TheLochnLoad, lifetime };
		return true;
	}
	case TF_WEAPON_PIPEBOMBLAUNCHER:
	{
		SDK::GetProjectileFireSetup(pPlayer, vAngles, { 16.f, 8.f, -6.f }, pos, ang, true, bQuick);
		const float charge = flCharge > 0.f && pWeapon->m_iItemDefinitionIndex() != Demoman_s_StickyJumper
			? SDK::AttribHookValue(4.f, "stickybomb_charge_rate", pWeapon) * flCharge
			: (pWeapon->As<CTFPipebombLauncher>()->m_flChargeBeginTime() > 0.f ? I::GlobalVars->curtime - pWeapon->As<CTFPipebombLauncher>()->m_flChargeBeginTime() : 0.f);
		const float speed = Math::RemapValClamped(charge, 0.f, SDK::AttribHookValue(4.f, "stickybomb_charge_rate", pWeapon), 900.f, 2400.f);
		out = { TF_PROJECTILE_PIPEBOMB_REMOTE, pos, ang, { 5.f, 5.f, 5.f }, speed, 1.f, false };
		return true;
	}
	case TF_WEAPON_FLAREGUN:
	{
		SDK::GetProjectileFireSetup(pPlayer, vAngles, { 23.5f, 12.f, bDucking ? 8.f : -3.f }, pos, ang, !bTrace ? true : false, bQuick);
		out = { TF_PROJECTILE_FLARE, pos, ang, { 1.f, 1.f, 1.f }, SDK::AttribHookValue(2000.f, "mult_projectile_speed", pWeapon), 0.000375f * flGravity /*0.3*/, true};
		return true;
	}
	case TF_WEAPON_FLAREGUN_REVENGE:
	{
		SDK::GetProjectileFireSetup(pPlayer, vAngles, { 23.5f, 12.f, bDucking ? 8.f : -3.f }, pos, ang, !bTrace ? true : false, bQuick);
		out = { TF_PROJECTILE_FLARE, pos, ang, { 1.f, 1.f, 1.f }, 3000.f, 0.0005625f * flGravity /*0.45*/, true };
		return true;
	}
	case TF_WEAPON_COMPOUND_BOW:
	{
		SDK::GetProjectileFireSetup(pPlayer, vAngles, { 23.5f, 8.f, -3.f }, pos, ang, !bTrace ? true : false, bQuick);
		const float charge = pWeapon->As<CTFPipebombLauncher>()->m_flChargeBeginTime() > 0.f ? I::GlobalVars->curtime - pWeapon->As<CTFPipebombLauncher>()->m_flChargeBeginTime() : 0.f;
		const float speed = Math::RemapValClamped(charge, 0.f, 1.f, 1800.f, 2600.f);
		const float gravity = Math::RemapValClamped(charge, 0.f, 1.f, 0.000625f, 0.000125f) * flGravity /*0.5, 0.1*/;
		out = { TF_PROJECTILE_ARROW, pos, ang, { 1.f, 1.f, 1.f }, speed, gravity, true, 10.f /*arrows have some lifetime check for whatever reason*/ };
		return true;
	}
	case TF_WEAPON_CROSSBOW:
	case TF_WEAPON_SHOTGUN_BUILDING_RESCUE:
	{
		SDK::GetProjectileFireSetup(pPlayer, vAngles, { 23.5f, 8.f, -3.f }, pos, ang, !bTrace ? true : false, bQuick);
		out = { TF_PROJECTILE_ARROW, pos, ang, pWeapon->m_iWeaponID() == TF_WEAPON_CROSSBOW ? Vec3(3.f, 3.f, 3.f) : Vec3(1.f, 1.f, 1.f), 2400.f, 0.00025f * flGravity /*0.2*/, true, 10.f /*arrows have some lifetime check for whatever reason*/ };
		return true;
	}
	case TF_WEAPON_SYRINGEGUN_MEDIC:
	{
		SDK::GetProjectileFireSetup(pPlayer, vAngles, { 16.f, 6.f, -8.f }, pos, ang, !bTrace ? true : false, bQuick);
		out = { TF_PROJECTILE_SYRINGE, pos, ang, { 1.f, 1.f, 1.f }, 1000.f, 0.000375f * flGravity /*0.3*/, true };
		return true;
	}
	case TF_WEAPON_FLAMETHROWER: // this inherits player velocity, possibly account for
	{
		static auto tf_flamethrower_boxsize = U::ConVars.FindVar("tf_flamethrower_boxsize");
		const float flHull = tf_flamethrower_boxsize ? tf_flamethrower_boxsize->GetFloat() : 12.f;

		SDK::GetProjectileFireSetup(pPlayer, vAngles, { 40.f, bFlipped ? -5.f : 5.f /*doesn't flip*/, 0.f}, pos, ang, true, bQuick);
		out = { TF_PROJECTILE_FLAME_ROCKET, pos, ang, { flHull, flHull, flHull }, 1000.f, 0.f, true, 0.33f };
		return true;
	}
	case TF_WEAPON_FLAME_BALL:
	{
		SDK::GetProjectileFireSetup(pPlayer, vAngles, { 70.f, bFlipped ? -7.f : 7.f /*doesn't flip*/, -9.f}, pos, ang, !bTrace ? true : false, bQuick);
		out = { TF_PROJECTILE_BALLOFFIRE, pos, ang, { 1.f, 1.f, 1.f /*damaging hull much bigger, shouldn't matter here*/ }, 3000.f, 0.f, true, 0.2f };
		return true;
	}
	case TF_WEAPON_CLEAVER:
	{
		SDK::GetProjectileFireSetup(pPlayer, vAngles, { 16.f, 8.f, -6.f }, pos, ang, true, bQuick);
		out = { TF_PROJECTILE_CLEAVER, pos, ang, { 1.f, 1.f, 10.f /*weird, probably still inaccurate*/ }, 3000.f, 2.f, !bTrace ? true : false, 1.55f };
		return true;
	}
	case TF_WEAPON_BAT_WOOD:
	case TF_WEAPON_BAT_GIFTWRAP:
	{
		const bool bWrapAssassin = pWeapon->m_iWeaponID() == TF_WEAPON_BAT_GIFTWRAP;
		
		SDK::GetProjectileFireSetup(pPlayer, vAngles, { 0.f, 0.f, 0.f }, pos, ang, true, bQuick);
		Vec3 forward; Math::AngleVectors(ang, &forward);
		pos = (bQuick ? pPlayer->GetAbsOrigin() : pPlayer->m_vecOrigin()) + (Vec3(0, 0, 50) + forward * 32.f) * pPlayer->m_flModelScale(); // why?
		out = { bWrapAssassin ? TF_PROJECTILE_FESTIVE_ARROW : TF_PROJECTILE_THROWABLE, pos, ang, { 3.f, 3.f, 3.f }, 2000.f, 1.f, false, bWrapAssassin ? 2.3f : 100.f };
		return true;
	}
	case TF_WEAPON_JAR:
	case TF_WEAPON_JAR_MILK:
	{
		SDK::GetProjectileFireSetup(pPlayer, vAngles, { 16.f, 8.f, -6.f }, pos, ang, true, bQuick);
		out = { TF_PROJECTILE_JAR, pos, ang, { 3.f, 3.f, 3.f }, 1000.f, 1.f, false, 2.2f };
		return true;
	}
	case TF_WEAPON_JAR_GAS:
	{
		SDK::GetProjectileFireSetup(pPlayer, vAngles, { 16.f, 8.f, -6.f }, pos, ang, true, bQuick);
		out = { TF_PROJECTILE_JAR_GAS, pos, ang, { 3.f, 3.f, 3.f }, 2000.f, 1.f, false, 2.2f };
		return true;
	}
	case TF_WEAPON_GRAPPLINGHOOK:
	{
		SDK::GetProjectileFireSetup(pPlayer, vAngles, { 23.5f, -8.f, -3.f }, pos, ang, false, bQuick);
		static auto tf_grapplinghook_projectile_speed = U::ConVars.FindVar("tf_grapplinghook_projectile_speed");
		static auto tf_grapplinghook_max_distance = U::ConVars.FindVar("tf_grapplinghook_max_distance");
		const float speed = tf_grapplinghook_projectile_speed ? tf_grapplinghook_projectile_speed->GetFloat() : 1500.f;
		const float lifetime = (tf_grapplinghook_max_distance ? tf_grapplinghook_max_distance->GetFloat() : 2000.f) / speed;
		out = { TF_PROJECTILE_GRAPPLINGHOOK, pos, ang, { 1.2f, 1.2f, 1.2f }, speed, 0.f, false, lifetime };
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
		SDK::GetProjectileFireSetup(pPlayer, vAngles, { 0.f, 0.f, -8.f }, pos, ang, true, bQuick);
		ang -= Vec3(10, 0, 0);
		out = { TF_PROJECTILE_BREAD_MONSTER, pos, ang, bQuick ? Vec3(4.f, 4.f, 4.f) : Vec3(17.f, 17.f, 17.f), 500.f, 0.00125f * flGravity /*1*/, false};
		return true;
	}

	return false;
}

bool CProjectileSimulation::GetInfo(CTFPlayer* pPlayer, CTFWeaponBase* pWeapon, const Vec3& vAngles, ProjectileInfo& out, bool bTrace, bool bQuick, float flCharge)
{
	const float flOldCurrentTime = I::GlobalVars->curtime;
	if (bQuick)
		I::GlobalVars->curtime = TICKS_TO_TIME(pPlayer->m_nTickBase());
	const bool bReturn = GetInfoMain(pPlayer, pWeapon, vAngles, out, bTrace, bQuick, flCharge);
	I::GlobalVars->curtime = flOldCurrentTime;

	if (!bReturn)
		return false;

	const Vec3 vStart = bQuick ? pPlayer->GetEyePosition() : pPlayer->GetShootPos();
	const Vec3 vEnd = out.m_vPos;

	CGameTrace trace = {};
	CTraceFilterProjectile filter = {};
	filter.pSkip = pPlayer;
	SDK::TraceHull(vStart, vEnd, out.m_vHull * -1.f, out.m_vHull, MASK_SOLID, &filter, &trace);
	return !trace.DidHit();
}

bool CProjectileSimulation::Initialize(const ProjectileInfo& info, bool bSimulate)
{
	if (!env)
		env = I::Physics->CreateEnvironment();

	if (!obj)
	{
		//it doesn't matter what the size is for non drag affected projectiles
		//pipes use the size below so it works out just fine
		auto col{ I::PhysicsCollision->BBoxToCollide({ -2.f, -2.f, -2.f }, { 2.f, 2.f, 2.f }) };

		auto params{ g_PhysDefaultObjectParams };

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
		float drag = 0.f;
		Vec3 drag_basis = {};
		Vec3 ang_drag_basis = {};

		//these values were dumped from the server by firing the projectiles with 0 0 0 angles
		//they are calculated in CPhysicsObject::RecomputeDragBases
		switch (info.m_iType)
		{
		case TF_PROJECTILE_NONE:
			drag = Vars::Visuals::Trajectory::Drag.Value;
			drag_basis = { Vars::Visuals::Trajectory::DragBasisX.Value, Vars::Visuals::Trajectory::DragBasisY.Value, Vars::Visuals::Trajectory::DragBasisZ.Value };
			ang_drag_basis = { Vars::Visuals::Trajectory::AngDragBasisX.Value, Vars::Visuals::Trajectory::AngDragBasisY.Value, Vars::Visuals::Trajectory::AngDragBasisZ.Value };
			break;
		case TF_PROJECTILE_PIPEBOMB:
			drag = 1.f;
			drag_basis = { 0.003902f, 0.009962f, 0.009962f };
			ang_drag_basis = { 0.003618f, 0.001514f, 0.001514f };
			break;
		case TF_PROJECTILE_PIPEBOMB_REMOTE:
		case TF_PROJECTILE_PIPEBOMB_PRACTICE:
			drag = 1.f;
			drag_basis = { 0.007491f, 0.007491f, 0.007306f };
			ang_drag_basis = { 0.002777f, 0.002842f, 0.002812f };
			break;
		case TF_PROJECTILE_CANNONBALL:
			drag = 1.f;
			drag_basis = { 0.020971f, 0.019420f, 0.020971f };
			ang_drag_basis = { 0.012997f, 0.013496f, 0.013714f };
			break;
		case TF_PROJECTILE_CLEAVER: // guesstimate
			drag = 1.f;
			drag_basis = { 0.020971f, 0.f, 0.f };
			break;
		case TF_PROJECTILE_THROWABLE: // guesstimate
			drag = 1.f;
			drag_basis = { 0.008500f, 0.f, 0.f };
			break;
		case TF_PROJECTILE_FESTIVE_ARROW: // guesstimate
			drag = 1.f;
			drag_basis = { 0.015500f, 0.f, 0.f };
			break;
		case TF_PROJECTILE_JAR: // guesstimate (there are different drags for different models, though shouldn't matter here)
			drag = 1.f;
			drag_basis = { 0.005500f, 0.005000f, 0.f };
			break;
		case TF_PROJECTILE_JAR_GAS: // who
			drag = 1.f;
			drag_basis = { 0.027101f, 0.067938f, 0.f };
		}

		obj->SetDragCoefficient(&drag, &drag);

		obj->m_dragBasis = drag_basis;
		obj->m_angDragBasis = ang_drag_basis;
	}

	//set position and velocity
	{
		Vec3 forward, up;

		Math::AngleVectors(info.m_vAng, &forward, nullptr, &up);

		Vec3 vel = { forward * info.m_flVelocity };
		Vec3 ang_vel;

		switch (info.m_iType)
		{
		case TF_PROJECTILE_NONE:
			vel += up * Vars::Visuals::Trajectory::UpVelocity.Value;
			ang_vel = { Vars::Visuals::Trajectory::AngVelocityX.Value, Vars::Visuals::Trajectory::AngVelocityY.Value, Vars::Visuals::Trajectory::AngVelocityZ.Value };
			break;
		case TF_PROJECTILE_PIPEBOMB:
		case TF_PROJECTILE_PIPEBOMB_REMOTE:
		case TF_PROJECTILE_PIPEBOMB_PRACTICE:
		case TF_PROJECTILE_CANNONBALL:
			vel += up * 200.f;
			ang_vel = { 600.f, -1200.f, 0.f };
			break;
		case TF_PROJECTILE_CLEAVER:
			vel += up * 300.f;
			ang_vel = { 300.f, 0.f, 0.f };
			break;
		case TF_PROJECTILE_THROWABLE:
		case TF_PROJECTILE_FESTIVE_ARROW:
			vel += up * 200.f;
			ang_vel = { 100.f, 0.f, 0.f };
			break;
		case TF_PROJECTILE_JAR:
		case TF_PROJECTILE_JAR_GAS:
			vel += up * 200.f;
			ang_vel = { 300.f, 0.f, 0.f };
		}

		if (info.m_bNoSpin)
			ang_vel.Zero();

		if (bSimulate && obj->m_dragBasis.IsZero()) // don't include vphysics projectiles
			vel.z += 400.f * info.m_flGravity * TICK_INTERVAL; // i don't know why this makes it more accurate but it does

		obj->SetPosition(info.m_vPos, info.m_vAng, true);
		obj->SetVelocity(&vel, &ang_vel);
	}

	//set env params
	{
		float max_vel = 1000000.f;
		float max_ang_vel = 1000000.f;

		//only pipes need k_flMaxVelocity and k_flMaxAngularVelocity
		switch (info.m_iType)
		{
		case TF_PROJECTILE_NONE:
			if (Vars::Visuals::Trajectory::MaxVelocity.Value)
				max_vel = Vars::Visuals::Trajectory::MaxVelocity.Value;
			if (Vars::Visuals::Trajectory::MaxAngularVelocity.Value)
				max_ang_vel = Vars::Visuals::Trajectory::MaxAngularVelocity.Value;
			break;
		case TF_PROJECTILE_PIPEBOMB:
		case TF_PROJECTILE_PIPEBOMB_REMOTE:
		case TF_PROJECTILE_PIPEBOMB_PRACTICE:
		case TF_PROJECTILE_CANNONBALL:
			max_vel = k_flMaxVelocity;
			max_ang_vel = k_flMaxAngularVelocity;
		}

		physics_performanceparams_t params{};
		params.Defaults();

		params.maxVelocity = max_vel;
		params.maxAngularVelocity = max_ang_vel;

		env->SetPerformanceSettings(&params);
		env->SetAirDensity(2.f);
		env->SetGravity({ 0.f, 0.f, -(800.f * info.m_flGravity) });

		env->ResetSimulationClock(); // not needed?
	}

	return true;
}

void CProjectileSimulation::RunTick(ProjectileInfo& info)
{
	if (!env)
		return;

	info.PredictionLines.push_back({ GetOrigin(), Math::GetRotatedPosition(GetOrigin(), Math::VelocityToAngles(GetVelocity() * Vec3(1, 1, 0)).Length2D() + 90, Vars::Visuals::Simulation::SeparatorLength.Value) });

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

	Vec3 out;
	obj->GetPosition(&out, nullptr);
	return out;
}

Vec3 CProjectileSimulation::GetVelocity()
{
	if (!obj)
		return {};

	Vec3 out;
	obj->GetVelocity(&out, nullptr);
	return out;
}