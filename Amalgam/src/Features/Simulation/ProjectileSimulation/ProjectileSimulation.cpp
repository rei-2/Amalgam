#include "ProjectileSimulation.h"

#include "../../EnginePrediction/EnginePrediction.h"
#include "../../CritHack/CritHack.h"

bool CProjectileSimulation::GetInfoMain(CTFPlayer* pPlayer, CTFWeaponBase* pWeapon, Vec3 vAngles, ProjectileInfo& tProjInfo, int iFlags, float flAutoCharge)
{
	if (!pPlayer || !pPlayer->IsAlive() || pPlayer->IsAGhost() || pPlayer->IsTaunting() || !pWeapon)
		return false;

	static auto sv_gravity = U::ConVars.FindVar("sv_gravity");
	float flGravity = sv_gravity->GetFloat() / 800.f;
	bool bDucking = pPlayer->m_fFlags() & FL_DUCKING;

	bool bTrace = iFlags & ProjSimEnum::Trace;
	bool bQuick = iFlags & ProjSimEnum::Quick;
	bool bMaxSpeed = iFlags & ProjSimEnum::MaxSpeed;

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
					vAngAdd.x += -6.f + float(SDK::RandomInt()) / 0x7FFF * 12.f;
					vAngAdd.y += -6.f + float(SDK::RandomInt()) / 0x7FFF * 12.f;
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

	if (Vars::Visuals::Trajectory::Override.Value)
	{
		SDK::GetProjectileFireSetup(pPlayer, vAngles, { Vars::Visuals::Trajectory::OffsetX.Value, Vars::Visuals::Trajectory::OffsetY.Value, Vars::Visuals::Trajectory::OffsetZ.Value }, vPos, vAngle, !bTrace ? true : Vars::Visuals::Trajectory::Pipes.Value, bQuick);
		tProjInfo = { pPlayer, pWeapon, FNV1A::Hash32Const("custom"), vPos, vAngle, { Vars::Visuals::Trajectory::Hull.Value, Vars::Visuals::Trajectory::Hull.Value, Vars::Visuals::Trajectory::Hull.Value }, Vars::Visuals::Trajectory::Speed.Value, Vars::Visuals::Trajectory::Gravity.Value, Vars::Visuals::Trajectory::LifeTime.Value };
		return true;
	}

	switch (pWeapon->GetWeaponID())
	{
	case TF_WEAPON_ROCKETLAUNCHER:
	case TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT:
	{
		SDK::GetProjectileFireSetup(pPlayer, vAngles, { 23.5f, int(SDK::AttribHookValue(0, "centerfire_projectile", pWeapon)) == 1 ? 0.f : 12.f, bDucking ? 8.f : -3.f }, vPos, vAngle, !bTrace ? true : false, bQuick);
		float flSpeed = pPlayer->InCond(TF_COND_RUNE_PRECISION) ? 3000.f : SDK::AttribHookValue(1100.f, "mult_projectile_speed", pWeapon);
		tProjInfo = { pPlayer, pWeapon, FNV1A::Hash32Const("models/weapons/w_models/w_rocket.mdl"), vPos, vAngle, { 0.f, 0.f, 0.f }, flSpeed, 0.f };
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
		tProjInfo = { pPlayer, pWeapon, FNV1A::Hash32Const("models/weapons/w_models/w_drg_ball.mdl"), vPos, vAngle, bCowMangler ? Vec3() : Vec3(1.f, 1.f, 1.f), flSpeed, 0.f };
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
			: SDK::AttribHookValue(2.f, "fuse_mult", pWeapon);
		auto uType = bCannon ? FNV1A::Hash32Const("models/weapons/w_models/w_cannonball.mdl") : FNV1A::Hash32Const("models/weapons/w_models/w_grenade_grenadelauncher.mdl");
		tProjInfo = { pPlayer, pWeapon, uType, vPos, vAngle, { 6.f, 6.f, 6.f }, flSpeed, 1.f, floorf(flLifetime / 0.195f + 1) * 0.195f };
		return true;
	}
	case TF_WEAPON_PIPEBOMBLAUNCHER:
	{
		SDK::GetProjectileFireSetup(pPlayer, vAngles, { 16.f, 8.f, -6.f }, vPos, vAngle, true, bQuick);
		float flCharge = flAutoCharge > 0.f && SDK::AttribHookValue(1, "mult_dmg", pWeapon)
			? SDK::AttribHookValue(4.f, "stickybomb_charge_rate", pWeapon) * flAutoCharge
			: (pWeapon->As<CTFPipebombLauncher>()->m_flChargeBeginTime() > 0.f ? I::GlobalVars->curtime - pWeapon->As<CTFPipebombLauncher>()->m_flChargeBeginTime() : 0.f);
		float flSpeed = bMaxSpeed ? 2400.f : Math::RemapVal(flCharge, 0.f, SDK::AttribHookValue(4.f, "stickybomb_charge_rate", pWeapon), 900.f, 2400.f);
		tProjInfo = { pPlayer, pWeapon, FNV1A::Hash32Const("models/weapons/w_models/w_stickybomb.mdl"), vPos, vAngle, { 6.f, 6.f, 6.f }, flSpeed, 1.f };
		return true;
	}
	case TF_WEAPON_FLAREGUN:
	{
		SDK::GetProjectileFireSetup(pPlayer, vAngles, { 23.5f, 12.f, bDucking ? 8.f : -3.f }, vPos, vAngle, !bTrace ? true : false, bQuick);
		tProjInfo = { pPlayer, pWeapon, FNV1A::Hash32Const("models/weapons/w_models/w_flaregun_shell.mdl"), vPos, vAngle, { 0.f, 0.f, 0.f }, SDK::AttribHookValue(2000.f, "mult_projectile_speed", pWeapon), 0.3f * flGravity };
		return true;
	}
	case TF_WEAPON_FLAREGUN_REVENGE:
	{
		SDK::GetProjectileFireSetup(pPlayer, vAngles, { 23.5f, 12.f, bDucking ? 8.f : -3.f }, vPos, vAngle, !bTrace ? true : false, bQuick);
		tProjInfo = { pPlayer, pWeapon, FNV1A::Hash32Const("models/weapons/w_models/w_flaregun_shell.mdl"), vPos, vAngle, { 0.f, 0.f, 0.f }, 3000.f, 0.45f * flGravity };
		return true;
	}
	case TF_WEAPON_COMPOUND_BOW:
	{
		SDK::GetProjectileFireSetup(pPlayer, vAngles, { 23.5f, 8.f, -3.f }, vPos, vAngle, !bTrace ? true : false, bQuick);
		float flCharge = pWeapon->As<CTFPipebombLauncher>()->m_flChargeBeginTime() > 0.f ? I::GlobalVars->curtime - pWeapon->As<CTFPipebombLauncher>()->m_flChargeBeginTime() : 0.f;
		float flSpeed = bMaxSpeed ? 2600.f : Math::RemapVal(flCharge, 0.f, 1.f, 1800.f, 2600.f);
		flGravity = Math::RemapVal(flCharge, 0.f, 1.f, 0.5f, 0.1f) * flGravity;
		tProjInfo = { pPlayer, pWeapon, FNV1A::Hash32Const("models/weapons/w_models/w_arrow.mdl"), vPos, vAngle, { 1.f, 1.f, 1.f }, flSpeed, flGravity, 10.f /*arrows have some lifetime check for whatever reason*/ };
		return true;
	}
	case TF_WEAPON_CROSSBOW:
	case TF_WEAPON_SHOTGUN_BUILDING_RESCUE:
	{
		bool bCrossbow = pWeapon->GetWeaponID() == TF_WEAPON_CROSSBOW;

		SDK::GetProjectileFireSetup(pPlayer, vAngles, { 23.5f, 8.f, -3.f }, vPos, vAngle, !bTrace ? true : false, bQuick);
		auto uType = bCrossbow ? FNV1A::Hash32Const("models/weapons/w_models/w_syringe_proj.mdl") : FNV1A::Hash32Const("models/weapons/w_models/w_repair_claw.mdl");
		tProjInfo = { pPlayer, pWeapon, uType, vPos, vAngle, pWeapon->GetWeaponID() == TF_WEAPON_CROSSBOW ? Vec3(3.f, 3.f, 3.f) : Vec3(1.f, 1.f, 1.f), 2400.f, 0.2f * flGravity, 10.f /*arrows have some lifetime check for whatever reason*/ };
		return true;
	}
	case TF_WEAPON_SYRINGEGUN_MEDIC:
	{
		SDK::GetProjectileFireSetup(pPlayer, vAngles, { 16.f, 6.f, -8.f }, vPos, vAngle, !bTrace ? true : false, bQuick);
		tProjInfo = { pPlayer, pWeapon, FNV1A::Hash32Const("models/weapons/w_models/w_syringe_proj.mdl"), vPos, vAngle, { 1.f, 1.f, 1.f }, 1000.f, 0.3f * flGravity };
		return true;
	}
	case TF_WEAPON_FLAMETHROWER:
	{
		static auto tf_flamethrower_boxsize = U::ConVars.FindVar("tf_flamethrower_boxsize");
		const float flHull = tf_flamethrower_boxsize->GetFloat();

		SDK::GetProjectileFireSetup(pPlayer, vAngles, { 40.f, 5.f, 0.f }, vPos, vAngle, true, bQuick, false);
		tProjInfo = { pPlayer, pWeapon, FNV1A::Hash32Const("particles/flamethrower.pcf"), vPos, vAngle, { flHull, flHull, flHull }, 1000.f, 0.f, 0.285f };
		return true;
	}
	case TF_WEAPON_FLAME_BALL:
	{
		SDK::GetProjectileFireSetup(pPlayer, vAngles, { 3.f, 7.f, -9.f }, vPos, vAngle, true, bQuick, false);
		tProjInfo = { pPlayer, pWeapon, FNV1A::Hash32Const("models/weapons/c_models/c_flameball/c_flameball.mdl"), vPos, vAngle, { 1.f, 1.f, 1.f /*damaging hull much bigger, shouldn't matter here*/ }, 3000.f, 0.f, 0.18f };
		return true;
	}
	case TF_WEAPON_CLEAVER:
	{
		SDK::GetProjectileFireSetup(pPlayer, vAngles, { 16.f, 8.f, -6.f }, vPos, vAngle, true, bQuick);
		tProjInfo = { pPlayer, pWeapon, FNV1A::Hash32Const("models/workshop_partner/weapons/c_models/c_sd_cleaver/c_sd_cleaver.mdl"), vPos, vAngle, { 1.f, 1.f, 10.f /*weird, probably still inaccurate*/ }, 3000.f, 1.f, 2.2f };
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
		auto uHash = bWrapAssassin ? FNV1A::Hash32Const("models/weapons/c_models/c_xms_festive_ornament.mdl") : FNV1A::Hash32Const("models/weapons/w_models/w_baseball.mdl");
		tProjInfo = { pPlayer, pWeapon, uHash, vPos, vAngle, { 3.f, 3.f, 3.f }, tf_scout_stunball_base_speed->GetFloat(), 1.f, bWrapAssassin ? 2.3f : 100.f };
		return true;
	}
	case TF_WEAPON_JAR:
	case TF_WEAPON_JAR_MILK:
	{
		SDK::GetProjectileFireSetup(pPlayer, vAngles, { 16.f, 8.f, -6.f }, vPos, vAngle, true, bQuick);
		uint32_t uType = uType = FNV1A::Hash32Const("models/weapons/c_models/urinejar.mdl");
		switch (pWeapon->m_iItemDefinitionIndex())
		{
		case Scout_s_MadMilk: uType = FNV1A::Hash32Const("models/workshop/weapons/c_models/c_madmilk/c_madmilk.mdl"); break;
		case Sniper_s_TheSelfAwareBeautyMark: uType = FNV1A::Hash32Const("models/weapons/c_models/c_breadmonster/c_breadmonster.mdl"); break;
		case Scout_s_MutatedMilk: uType = FNV1A::Hash32Const("models/weapons/c_models/c_breadmonster/c_breadmonster_milk.mdl"); break;
		}
		tProjInfo = { pPlayer, pWeapon, uType, vPos, vAngle, { 3.f, 3.f, 3.f }, 1000.f, 1.f, 2.2f };
		return true;
	}
	case TF_WEAPON_JAR_GAS:
	{
		SDK::GetProjectileFireSetup(pPlayer, vAngles, { 16.f, 8.f, -6.f }, vPos, vAngle, true, bQuick);
		tProjInfo = { pPlayer, pWeapon, FNV1A::Hash32Const("models/weapons/c_models/c_gascan/c_gascan.mdl"), vPos, vAngle, { 3.f, 3.f, 3.f }, 2000.f, 1.f, 2.2f };
		return true;
	}
	case TF_WEAPON_GRAPPLINGHOOK:
	{
		static auto tf_grapplinghook_projectile_speed = U::ConVars.FindVar("tf_grapplinghook_projectile_speed");
		static auto tf_grapplinghook_max_distance = U::ConVars.FindVar("tf_grapplinghook_max_distance");

		SDK::GetProjectileFireSetup(pPlayer, vAngles, { 23.5f, -8.f, -3.f }, vPos, vAngle, !bTrace ? true : false, bQuick);
		float flSpeed = tf_grapplinghook_projectile_speed->GetFloat();
		if (pPlayer->InCond(TF_COND_RUNE_AGILITY))
		{
			switch (pPlayer->m_iClass())
			{
			case TF_CLASS_SOLDIER:
			case TF_CLASS_HEAVY: flSpeed = 2600.f; break;
			default: flSpeed = 3000.f;
			}
		}
		float flLifetime = tf_grapplinghook_max_distance->GetFloat() / flSpeed;
		tProjInfo = { pPlayer, pWeapon, FNV1A::Hash32Const("models/weapons/c_models/c_grapple_proj/c_grapple_proj.mdl"), vPos, vAngle, { 1.2f, 1.2f, 1.2f }, flSpeed, 0.f, flLifetime };
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
		tProjInfo = { pPlayer, pWeapon, FNV1A::Hash32Const("models/weapons/c_models/c_sandwich/c_sandwich.mdl"), vPos, vAngle, { 17.f, 17.f, 7.f }, 500.f, 1.f * flGravity };
		return true;
	}

	return false;
}

bool CProjectileSimulation::GetInfo(CTFPlayer* pPlayer, CTFWeaponBase* pWeapon, Vec3 vAngles, ProjectileInfo& tProjInfo, int iFlags, float flAutoCharge)
{
	bool InitCheck = iFlags & ProjSimEnum::InitCheck;
	bool bQuick = iFlags & ProjSimEnum::Quick;

	const float flOldCurrentTime = I::GlobalVars->curtime;
	I::GlobalVars->curtime = TICKS_TO_TIME(pPlayer->m_nTickBase());
	bool bReturn = GetInfoMain(pPlayer, pWeapon, vAngles, tProjInfo, iFlags, flAutoCharge);
	I::GlobalVars->curtime = flOldCurrentTime;
	tProjInfo.m_bQuick = bQuick;

	if (!bReturn || !InitCheck)
		return bReturn;

	CGameTrace trace = {};
	CTraceFilterWorldAndPropsOnly filter = {};
	filter.pSkip = pPlayer;

	Vec3 vStart = bQuick ? pPlayer->GetEyePosition() : pPlayer->GetShootPos();
	Vec3 vEnd = tProjInfo.m_vPos;

	SDK::TraceHull(vStart, vEnd, tProjInfo.m_vHull * -1.f, tProjInfo.m_vHull, MASK_SOLID, &filter, &trace);
	return !trace.DidHit();
}

void CProjectileSimulation::GetInfo(CBaseEntity* pProjectile, ProjectileInfo& tProjInfo)
{
	auto paEntities = GetEntities(pProjectile);
	Vec3 vVelocity = GetVelocity(pProjectile);

	tProjInfo.m_pOwner = paEntities.second;
	tProjInfo.m_pWeapon = paEntities.first;
	tProjInfo.m_uType = FNV1A::Hash32(I::ModelInfoClient->GetModelName(pProjectile->GetModel()));

	tProjInfo.m_vPos = pProjectile->m_vecOrigin();
	tProjInfo.m_vAng = Math::VectorAngles(vVelocity);
	tProjInfo.m_vHull = pProjectile->m_vecMaxs();

	tProjInfo.m_flVelocity = vVelocity.Length();
	tProjInfo.m_flGravity = GetGravity(pProjectile, tProjInfo.m_pWeapon);
}

bool CProjectileSimulation::Initialize(ProjectileInfo& tProjInfo, bool bSimulate, bool bWorld)
{
	if (!m_pEnv)
		m_pEnv = I::Physics->CreateEnvironment();

	if (!m_pObj)
	{
		CPhysCollide* pCollide = I::PhysicsCollision->BBoxToCollide({ -2.f, -2.f, -2.f }, { 2.f, 2.f, 2.f });
		objectparams_t tParams = m_tPhysDefaultObjectParams;
		tParams.damping = 0.f;
		tParams.rotdamping = 0.f;
		tParams.inertia = 0.f;
		tParams.rotInertiaLimit = 0.f;
		tParams.enableCollisions = false;

		m_pObj = m_pEnv->CreatePolyObject(pCollide, 0, tProjInfo.m_vPos, tProjInfo.m_vAng, &tParams);
		m_pObj->Wake();
	}

	if (!m_pEnv || !m_pObj)
		return false;

	//set drag
	{
		float flDrag = 0.f;
		Vec3 vDragBasis = {};
		Vec3 vAngDragBasis = {};

		switch (tProjInfo.m_uType)
		{
		case FNV1A::Hash32Const("custom"):
			flDrag = Vars::Visuals::Trajectory::Drag.Value;
			vDragBasis = { Vars::Visuals::Trajectory::DragX.Value, Vars::Visuals::Trajectory::DragY.Value, Vars::Visuals::Trajectory::DragZ.Value };
			vAngDragBasis = { Vars::Visuals::Trajectory::AngularDragX.Value, Vars::Visuals::Trajectory::AngularDragY.Value, Vars::Visuals::Trajectory::AngularDragZ.Value };
			break;
		case FNV1A::Hash32Const("models/weapons/w_models/w_grenade_grenadelauncher.mdl"):
		case FNV1A::Hash32Const("models/workshop/weapons/c_models/c_quadball/w_quadball_grenade.mdl"):
			flDrag = 1.f;
			vDragBasis = { 0.003902f, 0.009962f, 0.009962f };
			vAngDragBasis = { 0.003618f, 0.001514f, 0.001514f };
			break;
		case FNV1A::Hash32Const("models/weapons/w_models/w_stickybomb.mdl"):
		case FNV1A::Hash32Const("models/workshop/weapons/c_models/c_kingmaker_sticky/w_kingmaker_stickybomb.mdl"):
		case FNV1A::Hash32Const("models/weapons/w_models/w_stickybomb_d.mdl"):
			flDrag = 1.f;
			vDragBasis = { 0.007491f, 0.007491f, 0.007306f };
			vAngDragBasis = { 0.002777f, 0.002842f, 0.002812f };
			break;
		case FNV1A::Hash32Const("models/weapons/w_models/w_stickybomb2.mdl"):
			flDrag = 1.f;
			vDragBasis = { 0.007394f, 0.007394f, 0.007100f };
			vAngDragBasis = { 0.002654f, 0.002717f, 0.002708f };
			break;
		case FNV1A::Hash32Const("models/weapons/w_models/w_cannonball.mdl"):
			flDrag = 1.f;
			vDragBasis = { 0.020971f, 0.019420f, 0.020971f };
			vAngDragBasis = { 0.012997f, 0.013496f, 0.013714f };
			break;
		case FNV1A::Hash32Const("models/workshop_partner/weapons/c_models/c_sd_cleaver/c_sd_cleaver.mdl"):
			flDrag = 1.f;
			vDragBasis = { 0.022287f, 0.005208f, 0.110697f };
			vAngDragBasis = { 0.013982f, 0.043243f, 0.003465f };
			break;
		case FNV1A::Hash32Const("models/weapons/w_models/w_baseball.mdl"):
			flDrag = 1.f;
			vDragBasis = { 0.009000f /*0.006645f*/, 0.006581f, 0.006710f};
			vAngDragBasis = { 0.002233f, 0.002246f, 0.002206f };
			break;
		case FNV1A::Hash32Const("models/weapons/c_models/c_xms_festive_ornament.mdl"):
			flDrag = 1.f;
			vDragBasis = { 0.013500f /*0.010867f*/, 0.0108727f, 0.010804f };
			vAngDragBasis = { 0.002081f, 0.002162f, 0.002069f };
			break;
		case FNV1A::Hash32Const("models/weapons/c_models/urinejar.mdl"):
			flDrag = 1.f;
			vDragBasis = { 0.005127f, 0.002925f, 0.004337f };
			vAngDragBasis = { 0.000641f, 0.001350f, 0.000717f };
			break;
		case FNV1A::Hash32Const("models/workshop/weapons/c_models/c_madmilk/c_madmilk.mdl"):
			flDrag = 1.f;
			vDragBasis = { 0.005514f, 0.002313f, 0.005558f };
			vAngDragBasis = { 0.000684f, 0.001439f, 0.000680f };
			break;
		case FNV1A::Hash32Const("models/weapons/c_models/c_breadmonster/c_breadmonster.mdl"):
			flDrag = 1.f;
			vDragBasis = { 0.002208f, 0.001640f, 0.002187f };
			vAngDragBasis = { 0.000799f, 0.001515f, 0.000879f };
			break;
		case FNV1A::Hash32Const("models/weapons/c_models/c_breadmonster/c_breadmonster_milk.mdl"):
			flDrag = 1.f;
			vDragBasis = { 0.002335f, 0.001960f, 0.001920f };
			vAngDragBasis = { 0.000672f, 0.001064f, 0.000747f };
			break;
		case FNV1A::Hash32Const("models/weapons/c_models/c_gascan/c_gascan.mdl"):
			flDrag = 1.f;
			vDragBasis = { 0.026360f, 0.021780f, 0.058978f };
			vAngDragBasis = { 0.035050f, 0.031199f, 0.022922f };
		}

		m_pObj->SetDragCoefficient(&flDrag, &flDrag);

		m_pObj->m_dragBasis = vDragBasis;
		m_pObj->m_angDragBasis = vAngDragBasis;
	}

	//set position and velocity
	{
		Vec3 vVelocity, vAngularVelocity;
		if (!bWorld)
		{
			Vec3 vAngle = tProjInfo.m_vAng;
			if (tProjInfo.m_uType == FNV1A::Hash32Const("models/weapons/c_models/c_sandwich/c_sandwich.mdl"))
				vAngle -= Vec3(10.f, 0.f, 0.f);

			Vec3 vForward, vRight, vUp; Math::AngleVectors(vAngle, &vForward, &vRight, &vUp);
			vVelocity = vForward * tProjInfo.m_flVelocity;

			switch (tProjInfo.m_uType)
			{
			case FNV1A::Hash32Const("custom"):
				vVelocity += vUp * Vars::Visuals::Trajectory::UpVelocity.Value;
				vAngularVelocity = { Vars::Visuals::Trajectory::AngularVelocityX.Value, Vars::Visuals::Trajectory::AngularVelocityY.Value, Vars::Visuals::Trajectory::AngularVelocityZ.Value };
				break;
			case FNV1A::Hash32Const("models/weapons/w_models/w_grenade_grenadelauncher.mdl"):
			case FNV1A::Hash32Const("models/workshop/weapons/c_models/c_quadball/w_quadball_grenade.mdl"):
			case FNV1A::Hash32Const("models/weapons/w_models/w_stickybomb.mdl"):
			case FNV1A::Hash32Const("models/weapons/w_models/w_stickybomb2.mdl"):
			case FNV1A::Hash32Const("models/weapons/w_models/w_cannonball.mdl"):
				if (!tProjInfo.m_bQuick && G::CurrentUserCmd)
				{
					vVelocity += vUp * 200.f + vUp * SDK::RandomFloat(-10.f, 10.f) + vRight * SDK::RandomFloat(-10.f, 10.f);
					if (!tProjInfo.m_pWeapon || !SDK::AttribHookValue(0, "grenade_no_spin", tProjInfo.m_pWeapon))
						vAngularVelocity = { 600.f, float(SDK::RandomInt(-1200, 1200)), 0.f };
					break;
				}
				vVelocity += vUp * 200.f;
				vAngularVelocity = { 600.f, -1200.f, 0.f };
				break;
			case FNV1A::Hash32Const("models/workshop_partner/weapons/c_models/c_sd_cleaver/c_sd_cleaver.mdl"):
				vVelocity = vForward * 10 + vUp;
				vVelocity.Normalize();
				vVelocity *= tProjInfo.m_flVelocity;
				vAngularVelocity = { 0.f, 500.f, 0.f };
				break;
			case FNV1A::Hash32Const("models/weapons/w_models/w_baseball.mdl"):
			case FNV1A::Hash32Const("models/weapons/c_models/c_xms_festive_ornament.mdl"):
				vVelocity = vForward * 10 + vUp;
				vVelocity.Normalize();
				vVelocity *= tProjInfo.m_flVelocity;
				vAngularVelocity = { 0.f, 100.f, 0.f };
				break;
			case FNV1A::Hash32Const("models/weapons/c_models/urinejar.mdl"):
			case FNV1A::Hash32Const("models/workshop/weapons/c_models/c_madmilk/c_madmilk.mdl"):
			case FNV1A::Hash32Const("models/weapons/c_models/c_breadmonster/c_breadmonster.mdl"):
			case FNV1A::Hash32Const("models/weapons/c_models/c_breadmonster/c_breadmonster_milk.mdl"):
			case FNV1A::Hash32Const("models/weapons/c_models/c_gascan/c_gascan.mdl"):
				vVelocity += vUp * 200.f;
				vAngularVelocity = { 300.f, 0.f, 0.f };
				break;
			case FNV1A::Hash32Const("particles/flamethrower.pcf"):
				if (bSimulate && tProjInfo.m_pOwner)
				{
					Vec3 vOwnerVelocity = tProjInfo.m_pOwner->m_vecVelocity();
					if (!vOwnerVelocity.IsZero())
					{
						float flOwnerVelocity = vOwnerVelocity.Length();

						float flDot = vForward.Dot(vOwnerVelocity) / flOwnerVelocity;
						vVelocity = vForward * (tProjInfo.m_flVelocity + flOwnerVelocity * flDot);
					}
				}
			}
		}
		else // in the case of adding projectiles that already exist in the world
		{
			Vec3 vForward, vRight, vUp; Math::AngleVectors(tProjInfo.m_vAng, &vForward, &vRight, &vUp);
			vVelocity = vForward * tProjInfo.m_flVelocity;

			switch (tProjInfo.m_uType)
			{
			case FNV1A::Hash32Const("models/weapons/w_models/w_grenade_grenadelauncher.mdl"):
			case FNV1A::Hash32Const("models/workshop/weapons/c_models/c_quadball/w_quadball_grenade.mdl"):
			case FNV1A::Hash32Const("models/weapons/w_models/w_stickybomb.mdl"):
			case FNV1A::Hash32Const("models/weapons/w_models/w_stickybomb2.mdl"):
			case FNV1A::Hash32Const("models/weapons/w_models/w_cannonball.mdl"):
				vAngularVelocity = { 600.f, -1200.f, 0.f };
				break;
			case FNV1A::Hash32Const("models/workshop_partner/weapons/c_models/c_sd_cleaver/c_sd_cleaver.mdl"):
				vAngularVelocity = { 0.f, 500.f, 0.f };
				break;
			case FNV1A::Hash32Const("models/weapons/w_models/w_baseball.mdl"):
			case FNV1A::Hash32Const("models/weapons/c_models/c_xms_festive_ornament.mdl"):
				vAngularVelocity = { 0.f, 100.f, 0.f };
				break;
			case FNV1A::Hash32Const("models/weapons/c_models/urinejar.mdl"):
			case FNV1A::Hash32Const("models/workshop/weapons/c_models/c_madmilk/c_madmilk.mdl"):
			case FNV1A::Hash32Const("models/weapons/c_models/c_breadmonster/c_breadmonster.mdl"):
			case FNV1A::Hash32Const("models/weapons/c_models/c_breadmonster/c_breadmonster_milk.mdl"):
			case FNV1A::Hash32Const("models/weapons/c_models/c_gascan/c_gascan.mdl"):
				vAngularVelocity = { 300.f, 0.f, 0.f };
				break;
			}
		}

		if (bSimulate && !F::ProjSim.m_pObj->IsDragEnabled() && m_pObj->m_dragBasis.IsZero()) // don't include vphysics projectiles
			vVelocity.z += 400.f * tProjInfo.m_flGravity * TICK_INTERVAL; // i don't know why this makes it more accurate but it does

		m_pObj->SetPosition(tProjInfo.m_vPos, tProjInfo.m_vAng, true);
		m_pObj->SetVelocity(&vVelocity, &vAngularVelocity);
	}

	//set m_pEnv params
	{
		float flMaxVelocity = 1000000.f;
		float vMaxAngularVelocity = 1000000.f;

		//only pipes need k_flMaxVelocity and k_flMaxAngularVelocity
		switch (tProjInfo.m_uType)
		{
		case FNV1A::Hash32Const("custom"):
			if (Vars::Visuals::Trajectory::MaxVelocity.Value)
				flMaxVelocity = Vars::Visuals::Trajectory::MaxVelocity.Value;
			if (Vars::Visuals::Trajectory::MaxAngularVelocity.Value)
				vMaxAngularVelocity = Vars::Visuals::Trajectory::MaxAngularVelocity.Value;
			break;
		case FNV1A::Hash32Const("models/weapons/w_models/w_grenade_grenadelauncher.mdl"):
		case FNV1A::Hash32Const("models/workshop/weapons/c_models/c_quadball/w_quadball_grenade.mdl"):
		case FNV1A::Hash32Const("models/weapons/w_models/w_stickybomb.mdl"):
		case FNV1A::Hash32Const("models/weapons/w_models/w_stickybomb2.mdl"):
		case FNV1A::Hash32Const("models/weapons/w_models/w_cannonball.mdl"):
		case FNV1A::Hash32Const("models/workshop_partner/weapons/c_models/c_sd_cleaver/c_sd_cleaver.mdl"):
		case FNV1A::Hash32Const("models/weapons/w_models/w_baseball.mdl"):
		case FNV1A::Hash32Const("models/weapons/c_models/c_xms_festive_ornament.mdl"):
			flMaxVelocity = k_flMaxVelocity;
			vMaxAngularVelocity = k_flMaxAngularVelocity;
		}

		physics_performanceparams_t params = {}; params.Defaults();
		params.maxVelocity = flMaxVelocity;
		params.maxAngularVelocity = vMaxAngularVelocity;

		m_pEnv->SetPerformanceSettings(&params);
		m_pEnv->SetAirDensity(2.f);
		m_pEnv->SetGravity({ 0.f, 0.f, -(800.f * tProjInfo.m_flGravity) });

		m_pEnv->ResetSimulationClock();
	}

	RunTick(tProjInfo, false); // simulate an initial time because dumb

	return true;
}

void CProjectileSimulation::RunTick(ProjectileInfo& tProjInfo, bool bPath) // bug: per frame projectile trace can cause inconsistencies?
{
	if (!m_pEnv)
		return;

	if (bPath)
		tProjInfo.m_vPath.push_back(GetOrigin());

	m_pEnv->Simulate(TICK_INTERVAL);

	/* // params.maxVelocity limits velocity uniformly
	Vec3 vVelocity, vAngular;
	m_pObj->GetVelocity(&vVelocity, &vAngular);
	static auto sv_maxvelocity = U::ConVars.FindVar("sv_maxvelocity");
	const float flMaxVel = sv_maxvelocity->GetFloat();
	vVelocity = { std::clamp(vVelocity.x, -flMaxVel, flMaxVel), std::clamp(vVelocity.y, -flMaxVel, flMaxVel), std::clamp(vVelocity.z, -flMaxVel, flMaxVel) };
	m_pObj->SetVelocity(&vVelocity, &vAngular);
	*/
}

Vec3 CProjectileSimulation::GetOrigin()
{
	if (!m_pObj)
		return {};

	Vec3 vOut;
	m_pObj->GetPosition(&vOut, nullptr);
	return vOut;
}

Vec3 CProjectileSimulation::GetVelocity()
{
	if (!m_pObj)
		return {};

	Vec3 vOut;
	m_pObj->GetVelocity(&vOut, nullptr);
	return vOut;
}

void CProjectileSimulation::SetupTrace(CTraceFilterCollideable& filter, int& nMask, CTFWeaponBase* pWeapon, int nTick, bool bQuick)
{
	switch (nTick)
	{
	case 0:
		switch (pWeapon->GetWeaponID())
		{
		case TF_WEAPON_RAYGUN:
			filter.iObject = OBJECT_DEFAULT;
			break;
		case TF_WEAPON_FLAMETHROWER:
		case TF_WEAPON_FLAME_BALL:
			nMask |= CONTENTS_WATER;
			break;
		case TF_WEAPON_DRG_POMSON:
		case TF_WEAPON_SYRINGEGUN_MEDIC:
		case TF_WEAPON_BAT_GIFTWRAP:
			if (bQuick)
				filter.iPlayer = PLAYER_ALL;
		}
		break;
	case 16:
		if (bQuick)
		{
			switch (pWeapon->GetWeaponID())
			{
			case TF_WEAPON_RAYGUN:
			case TF_WEAPON_GRAPPLINGHOOK:
				break;
			default:
				filter.iPlayer = PLAYER_ALL;
			}
		}
	}
}

void CProjectileSimulation::SetupTrace(CTraceFilterCollideable& filter, int& nMask, CBaseEntity* pProjectile)
{
	switch (pProjectile->GetClassID())
	{
	case ETFClassID::CTFProjectile_EnergyRing:
		filter.iObject = OBJECT_DEFAULT;
		break;
	case ETFClassID::CTFProjectile_BallOfFire:
		nMask |= CONTENTS_WATER;
	}
}