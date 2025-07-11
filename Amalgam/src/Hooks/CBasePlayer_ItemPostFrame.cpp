#include "../SDK/SDK.h"

MAKE_SIGNATURE(CBasePlayer_ItemPostFrame, "client.dll", "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B 39 48 8B F1 FF 97", 0x0);
MAKE_SIGNATURE(GetAnimationEvent, "server.dll", "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 0F 29 74 24 ? 0F 28 F3 49 8B F0", 0x0);

struct animevent_t
{
	int				event;
	const char*		options;
	float			cycle;
	float			eventtime;
	int				type;
	CBaseAnimating* pSource;
};

typedef enum
{
	AE_INVALID = -1,
	AE_EMPTY,
	AE_NPC_LEFTFOOT,
	AE_NPC_RIGHTFOOT,
	AE_NPC_BODYDROP_LIGHT,
	AE_NPC_BODYDROP_HEAVY,
	AE_NPC_SWISHSOUND,
	AE_NPC_180TURN,
	AE_NPC_ITEM_PICKUP,
	AE_NPC_WEAPON_DROP,
	AE_NPC_WEAPON_SET_SEQUENCE_NAME,
	AE_NPC_WEAPON_SET_SEQUENCE_NUMBER,
	AE_NPC_WEAPON_SET_ACTIVITY,
	AE_NPC_HOLSTER,
	AE_NPC_DRAW,
	AE_NPC_WEAPON_FIRE,
	AE_CL_PLAYSOUND,
	AE_SV_PLAYSOUND,
	AE_CL_STOPSOUND,
	AE_START_SCRIPTED_EFFECT,
	AE_STOP_SCRIPTED_EFFECT,
	AE_CLIENT_EFFECT_ATTACH,
	AE_MUZZLEFLASH,
	AE_NPC_MUZZLEFLASH,
	AE_THUMPER_THUMP,
	AE_AMMOCRATE_PICKUP_AMMO,
	AE_NPC_RAGDOLL,
	AE_NPC_ADDGESTURE,
	AE_NPC_RESTARTGESTURE,
	AE_NPC_ATTACK_BROADCAST,
	AE_NPC_HURT_INTERACTION_PARTNER,
	AE_NPC_SET_INTERACTION_CANTDIE,
	AE_SV_DUSTTRAIL,
	AE_CL_CREATE_PARTICLE_EFFECT,
	AE_RAGDOLL,
	AE_CL_ENABLE_BODYGROUP,
	AE_CL_DISABLE_BODYGROUP,
	AE_CL_BODYGROUP_SET_VALUE,
	AE_CL_BODYGROUP_SET_VALUE_CMODEL_WPN,
	AE_WPN_PRIMARYATTACK,
	AE_WPN_INCREMENTAMMO,
	AE_WPN_HIDE,
	AE_WPN_UNHIDE,
	AE_WPN_PLAYWPNSOUND,
	AE_RD_ROBOT_POP_PANELS_OFF,
	AE_TAUNT_ENABLE_MOVE,
	AE_TAUNT_DISABLE_MOVE,
	AE_CL_REMOVE_PARTICLE_EFFECT,
	LAST_SHARED_ANIMEVENT,
} Animevent;

MAKE_HOOK(CBasePlayer_ItemPostFrame, S::CBasePlayer_ItemPostFrame(), void,
	void* rcx)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::CBasePlayer_ItemPostFrame[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx);
#endif

	auto pLocal = reinterpret_cast<CTFPlayer*>(rcx);
	auto pWeapon = H::Entities.GetWeapon();
	if (!pWeapon)
		return CALL_ORIGINAL(rcx);

	switch (pWeapon->GetWeaponID())
	{
	case TF_WEAPON_ROCKETLAUNCHER:
	case TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT:
	case TF_WEAPON_PARTICLE_CANNON:
		if (!pWeapon->IsInReload())
			return CALL_ORIGINAL(rcx);
		break;
	default:
		return CALL_ORIGINAL(rcx);
	}

	if (pWeapon->m_iItemDefinitionIndex() == Soldier_m_TheBeggarsBazooka)
	{
		if (I::GlobalVars->curtime < pLocal->m_flNextAttack())
			return CALL_ORIGINAL(rcx);
	}
	else
	{
		// not perfect but seems to work fine enough for casual use
		auto pViewmodel = pLocal->m_hViewModel()->As<CBaseAnimating>();
		if (!pViewmodel)
			return CALL_ORIGINAL(rcx);

		auto pStudio = pViewmodel->GetModelPtr();
		if (!pStudio)
			return CALL_ORIGINAL(rcx);

		float flReloadTime = pViewmodel->SequenceDuration();
		float flReloadSpeed = 1.f / pViewmodel->m_flPlaybackRate();

		float flLastCycle = (I::GlobalVars->curtime - pWeapon->m_flReloadPriorNextFire() - TICK_INTERVAL) / (flReloadTime * flReloadSpeed);
		float flCurrCycle = (I::GlobalVars->curtime - pWeapon->m_flReloadPriorNextFire()) / (flReloadTime * flReloadSpeed);

		animevent_t event; int index = 0;
		index = S::GetAnimationEvent.Call<int>(pStudio, pViewmodel->m_nSequence(), &event, flLastCycle, flCurrCycle, index);
		if (!index || event.event != AE_WPN_INCREMENTAMMO)
			return CALL_ORIGINAL(rcx);
	}

	CALL_ORIGINAL(rcx);
	pWeapon->IncrementAmmo();
	pWeapon->m_bReloadedThroughAnimEvent() = true;
}