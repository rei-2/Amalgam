#include "../SDK/SDK.h"

#include "../Features/Simulation/ProjectileSimulation/ProjectileSimulation.h"

MAKE_SIGNATURE(CParticleProperty_Create_Name, "client.dll", "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 56 48 83 EC ? 48 8B 59 ? 49 8B F1", 0x0);
MAKE_SIGNATURE(CParticleProperty_Create_Point, "client.dll", "44 89 4C 24 ? 44 89 44 24 ? 53", 0x0);
MAKE_SIGNATURE(CParticleProperty_AddControlPoint_Pointer, "client.dll", "48 89 5C 24 ? 48 89 6C 24 ? 56 57 41 55 41 56 41 57 48 83 EC ? 4C 8B BC 24", 0x0);
MAKE_SIGNATURE(CWeaponMedigun_UpdateEffects_CreateName_Call1, "client.dll", "49 8B CC F3 0F 11 74 24 ? 48 8B D8", 0x0);
MAKE_SIGNATURE(CWeaponMedigun_UpdateEffects_CreateName_Call2, "client.dll", "41 8B 14 24 48 8B D8", 0x0);
MAKE_SIGNATURE(CWeaponMedigun_ManageChargeEffect_CreateName_Call, "client.dll", "48 89 86 ? ? ? ? 48 89 BE ? ? ? ? 48 83 BE", 0x0);

MAKE_HOOK(CParticleProperty_Create_Name, S::CParticleProperty_Create_Name(), void*,
	void* rcx, const char* pszParticleName, ParticleAttachment_t iAttachType, const char* pszAttachmentName)
{
#ifdef DEBUG_HOOKS
    if (!Vars::Hooks::CParticleProperty_Create[DEFAULT_BIND])
        return CALL_ORIGINAL(rcx, pszParticleName, iAttachType, pszAttachmentName);
#endif

    static const auto dwUpdateEffects1 = S::CWeaponMedigun_UpdateEffects_CreateName_Call1();
    static const auto dwUpdateEffects2 = S::CWeaponMedigun_UpdateEffects_CreateName_Call2();
    static const auto dwManageChargeEffect = S::CWeaponMedigun_ManageChargeEffect_CreateName_Call();
    const auto dwRetAddr = uintptr_t(_ReturnAddress());

    bool bUpdateEffects = dwRetAddr == dwUpdateEffects1 || dwRetAddr == dwUpdateEffects2, bManageChargeEffect = dwRetAddr == dwManageChargeEffect;
    if (bUpdateEffects || bManageChargeEffect)
    {
        auto pLocal = H::Entities.GetLocal();
        if (!pLocal)
            return CALL_ORIGINAL(rcx, pszParticleName, iAttachType, pszAttachmentName);

        /* // probably not needed
        auto pWeapon = pLocal->GetWeaponFromSlot(SLOT_SECONDARY);
        if (!pWeapon || pWeapon->GetWeaponID() != TF_WEAPON_MEDIGUN)
            return CALL_ORIGINAL(rcx, pszParticleName, iAttachType, pszAttachmentName);
        */

        auto pModel = pLocal->GetRenderedWeaponModel();
        if (!pModel || rcx != &pModel->m_Particles())
            return CALL_ORIGINAL(rcx, pszParticleName, iAttachType, pszAttachmentName);

        bool bBlue = pLocal->m_iTeamNum() == TF_TEAM_BLUE;
        if (bUpdateEffects)
        {
            switch (FNV1A::Hash32(Vars::Visuals::Effects::MedigunBeam.Value.c_str()))
            {
            case FNV1A::Hash32Const("Default"): break;
            case FNV1A::Hash32Const("None"): return nullptr;
            case FNV1A::Hash32Const("Uber"): pszParticleName = bBlue ? "medicgun_beam_blue_invun" : "medicgun_beam_red_invun"; break;
            case FNV1A::Hash32Const("Dispenser"): pszParticleName = bBlue ? "dispenser_heal_blue" : "dispenser_heal_red"; break;
            case FNV1A::Hash32Const("Passtime"): pszParticleName = "passtime_beam"; break;
            case FNV1A::Hash32Const("Bombonomicon"): pszParticleName = "bombonomicon_spell_trail"; break;
            case FNV1A::Hash32Const("White"): pszParticleName = "medicgun_beam_machinery_stage3"; break;
            case FNV1A::Hash32Const("Orange"): pszParticleName = "medicgun_beam_red_trail_stage3"; break;
            default: pszParticleName = Vars::Visuals::Effects::MedigunBeam.Value.c_str();
            }
        }
        else if (bManageChargeEffect)
        {
            switch (FNV1A::Hash32(Vars::Visuals::Effects::MedigunCharge.Value.c_str()))
            {
            case FNV1A::Hash32Const("Default"): break;
            case FNV1A::Hash32Const("None"): return nullptr;
            case FNV1A::Hash32Const("Electrocuted"): pszParticleName = bBlue ? "electrocuted_blue" : "electrocuted_red"; break;
            case FNV1A::Hash32Const("Halloween"): pszParticleName = "ghost_pumpkin"; break;
            case FNV1A::Hash32Const("Fireball"): pszParticleName = bBlue ? "spell_fireball_small_trail_blue" : "spell_fireball_small_trail_red"; break;
            case FNV1A::Hash32Const("Teleport"): pszParticleName = bBlue ? "spell_teleport_blue" : "spell_teleport_red"; break;
            case FNV1A::Hash32Const("Burning"): pszParticleName = "superrare_burning1"; break;
            case FNV1A::Hash32Const("Scorching"): pszParticleName = "superrare_burning2"; break;
            case FNV1A::Hash32Const("Purple energy"): pszParticleName = "superrare_purpleenergy"; break;
            case FNV1A::Hash32Const("Green energy"): pszParticleName = "superrare_greenenergy"; break;
            case FNV1A::Hash32Const("Nebula"): pszParticleName = "unusual_invasion_nebula"; break;
            case FNV1A::Hash32Const("Purple stars"): pszParticleName = "unusual_star_purple_parent"; break;
            case FNV1A::Hash32Const("Green stars"): pszParticleName = "unusual_star_green_parent"; break;
            case FNV1A::Hash32Const("Sunbeams"): pszParticleName = "superrare_beams1"; break;
            case FNV1A::Hash32Const("Spellbound"): pszParticleName = "unusual_spellbook_circle_purple"; break;
            case FNV1A::Hash32Const("Purple sparks"): pszParticleName = "unusual_robot_orbiting_sparks2"; break;
            case FNV1A::Hash32Const("Yellow sparks"): pszParticleName = "unusual_robot_orbiting_sparks"; break;
            case FNV1A::Hash32Const("Green zap"): pszParticleName = "unusual_zap_green"; break;
            case FNV1A::Hash32Const("Yellow zap"): pszParticleName = "unusual_zap_yellow"; break;
            case FNV1A::Hash32Const("Plasma"): pszParticleName = "superrare_plasma1"; break;
            case FNV1A::Hash32Const("Frostbite"): pszParticleName = "unusual_eotl_frostbite"; break;
            case FNV1A::Hash32Const("Time warp"): pszParticleName = bBlue ? "unusual_robot_time_warp2" : "unusual_robot_time_warp"; break;
            case FNV1A::Hash32Const("Purple souls"): pszParticleName = "unusual_souls_purple_parent"; break;
            case FNV1A::Hash32Const("Green souls"): pszParticleName = "unusual_souls_green_parent"; break;
            case FNV1A::Hash32Const("Bubbles"): pszParticleName = "unusual_bubbles"; break;
            case FNV1A::Hash32Const("Hearts"): pszParticleName = "unusual_hearts_bubbling"; break;
            default: pszParticleName = Vars::Visuals::Effects::MedigunCharge.Value.c_str();
            }
        }
    }

	return CALL_ORIGINAL(rcx, pszParticleName, iAttachType, pszAttachmentName);
}

MAKE_HOOK(CParticleProperty_Create_Point, S::CParticleProperty_Create_Point(), void*,
	void* rcx, const char* pszParticleName, ParticleAttachment_t iAttachType, int iAttachmentPoint, Vector vecOriginOffset)
{
#ifdef DEBUG_HOOKS
    if (!Vars::Hooks::CParticleProperty_Create[DEFAULT_BIND])
        return CALL_ORIGINAL(rcx, pszParticleName, iAttachType, iAttachmentPoint, vecOriginOffset);
#endif

    if (FNV1A::Hash32(Vars::Visuals::Effects::ProjectileTrail.Value.c_str()) != FNV1A::Hash32Const("Default"))
    {
        switch (FNV1A::Hash32(pszParticleName))
        {
        // any trails we want to replace
        case FNV1A::Hash32Const("peejar_trail_blu"):
        case FNV1A::Hash32Const("peejar_trail_red"):
        case FNV1A::Hash32Const("peejar_trail_blu_glow"):
        case FNV1A::Hash32Const("peejar_trail_red_glow"):
        case FNV1A::Hash32Const("stunballtrail_blue"):
        case FNV1A::Hash32Const("stunballtrail_red"):
        case FNV1A::Hash32Const("rockettrail"):
        case FNV1A::Hash32Const("rockettrail_airstrike"):
        case FNV1A::Hash32Const("drg_cow_rockettrail_normal_blue"):
        case FNV1A::Hash32Const("drg_cow_rockettrail_normal"):
        case FNV1A::Hash32Const("drg_cow_rockettrail_charged_blue"):
        case FNV1A::Hash32Const("drg_cow_rockettrail_charged"):
        case FNV1A::Hash32Const("rockettrail_RocketJumper"):
        case FNV1A::Hash32Const("rockettrail_underwater"):
        case FNV1A::Hash32Const("halloween_rockettrail"):
        case FNV1A::Hash32Const("eyeboss_projectile"):
        case FNV1A::Hash32Const("drg_bison_projectile"):
        case FNV1A::Hash32Const("flaregun_trail_blue"):
        case FNV1A::Hash32Const("flaregun_trail_red"):
        case FNV1A::Hash32Const("scorchshot_trail_blue"):
        case FNV1A::Hash32Const("scorchshot_trail_red"):
        case FNV1A::Hash32Const("drg_manmelter_projectile"):
        case FNV1A::Hash32Const("pipebombtrail_blue"):
        case FNV1A::Hash32Const("pipebombtrail_red"):
        case FNV1A::Hash32Const("stickybombtrail_blue"):
        case FNV1A::Hash32Const("stickybombtrail_red"):
        case FNV1A::Hash32Const("healshot_trail_blue"):
        case FNV1A::Hash32Const("healshot_trail_red"):
        case FNV1A::Hash32Const("flaming_arrow"):
        case FNV1A::Hash32Const("spell_fireball_small_trail_blue"):
        case FNV1A::Hash32Const("spell_fireball_small_trail_red"):
        {
            auto pLocal = H::Entities.GetLocal();
            if (!pLocal)
                return CALL_ORIGINAL(rcx, pszParticleName, iAttachType, iAttachmentPoint, vecOriginOffset);

            bool bValid = false;
            for (auto pEntity : H::Entities.GetGroup(EGroupType::WORLD_PROJECTILES))
            {
                auto pOwner = F::ProjSim.GetEntities(pEntity).second;
                if (bValid = pLocal == pOwner && rcx == &pEntity->m_Particles())
                    break;
            }
            if (!bValid)
                return CALL_ORIGINAL(rcx, pszParticleName, iAttachType, iAttachmentPoint, vecOriginOffset);

            bool bBlue = pLocal->m_iTeamNum() == TF_TEAM_BLUE;
            switch (FNV1A::Hash32(Vars::Visuals::Effects::ProjectileTrail.Value.c_str()))
            {
            case FNV1A::Hash32Const("None"): return nullptr;
            case FNV1A::Hash32Const("Rocket"): pszParticleName = "rockettrail"; break;
            case FNV1A::Hash32Const("Critical"): pszParticleName = bBlue ? "critical_rocket_blue" : "critical_rocket_red"; break;
            case FNV1A::Hash32Const("Energy"): pszParticleName = bBlue ? "drg_cow_rockettrail_normal_blue" : "drg_cow_rockettrail_normal"; break;
            case FNV1A::Hash32Const("Charged"): pszParticleName = bBlue ? "drg_cow_rockettrail_charged_blue" : "drg_cow_rockettrail_charged"; break;
            case FNV1A::Hash32Const("Ray"): pszParticleName = "drg_manmelter_projectile"; break;
            case FNV1A::Hash32Const("Fireball"): pszParticleName = bBlue ? "spell_fireball_small_trail_blue" : "spell_fireball_small_trail_red"; break;
            case FNV1A::Hash32Const("Teleport"): pszParticleName = bBlue ? "spell_teleport_blue" : "spell_teleport_red"; break;
            case FNV1A::Hash32Const("Fire"): pszParticleName = "flamethrower"; break;
            case FNV1A::Hash32Const("Flame"): pszParticleName = "flying_flaming_arrow"; break;
            case FNV1A::Hash32Const("Sparks"): pszParticleName = bBlue ? "critical_rocket_bluesparks" : "critical_rocket_redsparks"; break;
            case FNV1A::Hash32Const("Flare"): pszParticleName = bBlue ? "flaregun_trail_blue" : "flaregun_trail_red"; break;
            case FNV1A::Hash32Const("Trail"): pszParticleName = bBlue ? "stickybombtrail_blue" : "stickybombtrail_red"; break;
            case FNV1A::Hash32Const("Health"): pszParticleName = bBlue ? "healshot_trail_blue" : "healshot_trail_red"; break;
            case FNV1A::Hash32Const("Smoke"): pszParticleName = "rockettrail_airstrike_line"; break;
            case FNV1A::Hash32Const("Bubbles"): pszParticleName = bBlue ? "pyrovision_scorchshot_trail_blue" : "pyrovision_scorchshot_trail_red"; break;
            case FNV1A::Hash32Const("Halloween"): pszParticleName = "halloween_rockettrail"; break;
            case FNV1A::Hash32Const("Monoculus"): pszParticleName = "eyeboss_projectile"; break;
            case FNV1A::Hash32Const("Sparkles"): pszParticleName = bBlue ? "burningplayer_rainbow_blue" : "burningplayer_rainbow_red"; break;
            case FNV1A::Hash32Const("Rainbow"): pszParticleName = "flamethrower_rainbow"; break;
            default: pszParticleName = Vars::Visuals::Effects::ProjectileTrail.Value.c_str();
            }
            break;
        }
        /*
        // any additional trails
        case FNV1A::Hash32Const("stunballtrail_blue_crit"):
        case FNV1A::Hash32Const("stunballtrail_red_crit"):
        case FNV1A::Hash32Const("critical_rocket_blue"):
        case FNV1A::Hash32Const("critical_rocket_red"):
        case FNV1A::Hash32Const("critical_rocket_bluesparks"):
        case FNV1A::Hash32Const("critical_rocket_redsparks"):
        case FNV1A::Hash32Const("flaregun_trail_crit_blue"):
        case FNV1A::Hash32Const("flaregun_trail_crit_red"):
        case FNV1A::Hash32Const("critical_pipe_blue"):
        case FNV1A::Hash32Const("critical_pipe_red"):
        case FNV1A::Hash32Const("critical_grenade_blue"):
        case FNV1A::Hash32Const("critical_grenade_red"):
        */
        case FNV1A::Hash32Const("rockettrail_airstrike_line"): return nullptr;
        }
    }

	return CALL_ORIGINAL(rcx, pszParticleName, iAttachType, iAttachmentPoint, vecOriginOffset);
}

MAKE_HOOK(CParticleProperty_AddControlPoint_Pointer, S::CParticleProperty_AddControlPoint_Pointer(), void,
    void* rcx, void* pEffect, int iPoint, CBaseEntity* pEntity, ParticleAttachment_t iAttachType, const char* pszAttachmentName, Vector vecOriginOffset)
{
    if (!pEffect)
        return; // crash fix

    CALL_ORIGINAL(rcx, pEffect, iPoint, pEntity, iAttachType, pszAttachmentName, vecOriginOffset);
}