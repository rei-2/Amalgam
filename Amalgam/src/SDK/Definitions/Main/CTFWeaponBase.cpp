#include "CTFWeaponBase.h"

#include "../../SDK.h"

MAKE_SIGNATURE(CTFPlayerSharedUtils_GetEconItemViewByLoadoutSlot, "client.dll", "48 89 6C 24 ? 56 41 54 41 55 41 56 41 57 48 83 EC", 0x0);
MAKE_SIGNATURE(CEconItemView_GetItemName, "client.dll", "40 53 48 83 EC ? 48 8B D9 C6 81 ? ? ? ? ? E8 ? ? ? ? 48 8B 8B", 0x0);

bool CTFWeaponBase::HasPrimaryAmmoForShot()
{
	if (IsEnergyWeapon())
		return m_flEnergy() > 0.f;

	int iClip = m_iClip1();
	auto pOwner = m_hOwnerEntity().Get();
	return (iClip == WEAPON_NOCLIP && pOwner ? pOwner->As<CTFPlayer>()->GetAmmoCount(m_iPrimaryAmmoType()) : iClip) >= GetAmmoPerShot();
}

bool CTFWeaponBase::CanPrimaryAttack()
{
	auto pOwner = m_hOwnerEntity()->As<CTFPlayer>();
	if (!pOwner)
		return false;

	float flCurTime = TICKS_TO_TIME(pOwner->m_nTickBase());
	return m_flNextPrimaryAttack() <= flCurTime && pOwner->m_flNextAttack() <= flCurTime;
}

bool CTFWeaponBase::CanSecondaryAttack()
{
	auto pOwner = m_hOwnerEntity()->As<CTFPlayer>();
	if (!pOwner)
		return false;

	float flCurTime = TICKS_TO_TIME(pOwner->m_nTickBase());
	return m_flNextSecondaryAttack() <= flCurTime && pOwner->m_flNextAttack() <= flCurTime;
}

bool CTFWeaponBase::CanFireCriticalShot(bool bIsHeadshot)
{
	auto pOwner = m_hOwnerEntity()->As<CTFPlayer>();
	if (!pOwner)
		return false;

	int& iFOV = pOwner->m_iFOV(), nFovBackup = iFOV;
	iFOV = 70;
	bool bReturn = U::Memory.CallVirtual<428, bool>(this, bIsHeadshot, nullptr);
	iFOV = nFovBackup;
	return bReturn;
}

bool CTFWeaponBase::CanHeadshot()
{
	return GetDamageType() & DMG_USE_HITLOCATIONS && CanFireCriticalShot(true);
}

bool CTFWeaponBase::AmbassadorCanHeadshot(float flCurTime)
{
	if (GetClassID() == ETFClassID::CTFRevolver && SDK::AttribHookValue(0, "set_weapon_mode", this) == 1)
		return flCurTime - m_flLastFireTime() > 1.f;
	return false;
}

bool CTFWeaponBase::IsInReload()
{
	return m_bInReload() || m_iReloadMode() != 0;
}

//#define WEAPONDATA_USES_WEAPONMODE // would probably just be better to have a mode override

float CTFWeaponBase::GetDamage(bool bAttribHookValue)
{
	if (auto pWeaponInfo = m_pWeaponInfo())
	{
		if (!bAttribHookValue)
#ifdef WEAPONDATA_USES_WEAPONMODE
			return pWeaponInfo->GetWeaponData(m_iWeaponMode()).m_nDamage;
		return SDK::AttribHookValue(pWeaponInfo->GetWeaponData(m_iWeaponMode()).m_nDamage, "mult_dmg", this);
#else
			return pWeaponInfo->GetWeaponData(TF_WEAPON_PRIMARY_MODE).m_nDamage;
		return SDK::AttribHookValue(pWeaponInfo->GetWeaponData(TF_WEAPON_PRIMARY_MODE).m_nDamage, "mult_dmg", this);
#endif
	}
	return 0.f;
}

float CTFWeaponBase::GetFireRate(bool bAttribHookValue)
{
	if (auto pWeaponInfo = m_pWeaponInfo())
	{
		if (!bAttribHookValue)
#ifdef WEAPONDATA_USES_WEAPONMODE
			return pWeaponInfo->GetWeaponData(m_iWeaponMode()).m_flTimeFireDelay;
		return SDK::AttribHookValue(pWeaponInfo->GetWeaponData(m_iWeaponMode()).m_flTimeFireDelay, "mult_postfiredelay", this);
#else
			return pWeaponInfo->GetWeaponData(TF_WEAPON_PRIMARY_MODE).m_flTimeFireDelay;
		return SDK::AttribHookValue(pWeaponInfo->GetWeaponData(TF_WEAPON_PRIMARY_MODE).m_flTimeFireDelay, "mult_postfiredelay", this);
#endif
	}
	return 0.315f;
}

int CTFWeaponBase::GetBulletsPerShot(bool bAttribHookValue)
{
	if (auto pWeaponInfo = m_pWeaponInfo())
	{
		if (!bAttribHookValue)
#ifdef WEAPONDATA_USES_WEAPONMODE
			return pWeaponInfo->GetWeaponData(m_iWeaponMode()).m_nBulletsPerShot;
		return SDK::AttribHookValue(pWeaponInfo->GetWeaponData(m_iWeaponMode()).m_nBulletsPerShot, "mult_bullets_per_shot", this);
#else
			return pWeaponInfo->GetWeaponData(TF_WEAPON_PRIMARY_MODE).m_nBulletsPerShot;
		return SDK::AttribHookValue(pWeaponInfo->GetWeaponData(TF_WEAPON_PRIMARY_MODE).m_nBulletsPerShot, "mult_bullets_per_shot", this);
#endif
	}
	return 1;
}

int CTFWeaponBase::GetAmmoPerShot(bool bAttribHookValue)
{
	if (auto pWeaponInfo = m_pWeaponInfo())
	{
		if (!bAttribHookValue)
#ifdef WEAPONDATA_USES_WEAPONMODE
			return pWeaponInfo->GetWeaponData(m_iWeaponMode()).m_iAmmoPerShot;
		int iAmmoPerShot = SDK::AttribHookValue(0, "mod_ammo_per_shot", this);
		return iAmmoPerShot > 0 ? iAmmoPerShot : pWeaponInfo->GetWeaponData(m_iWeaponMode()).m_iAmmoPerShot;
#else
			return pWeaponInfo->GetWeaponData(TF_WEAPON_PRIMARY_MODE).m_iAmmoPerShot;
		int iAmmoPerShot = SDK::AttribHookValue(0, "mod_ammo_per_shot", this);
		return iAmmoPerShot > 0 ? iAmmoPerShot : pWeaponInfo->GetWeaponData(TF_WEAPON_PRIMARY_MODE).m_iAmmoPerShot;
#endif
	}
	return 1;
}

bool CTFWeaponBase::IsRapidFire()
{
	if (auto pWeaponInfo = m_pWeaponInfo())
#ifdef WEAPONDATA_USES_WEAPONMODE
		return pWeaponInfo->GetWeaponData(m_iWeaponMode()).m_bUseRapidFireCrits;
#else
		return pWeaponInfo->GetWeaponData(TF_WEAPON_PRIMARY_MODE).m_bUseRapidFireCrits;
#endif
	return false;
}

float CTFWeaponBase::GetSmackDelay()
{
	if (auto pWeaponInfo = m_pWeaponInfo())
#ifdef WEAPONDATA_USES_WEAPONMODE
		return pWeaponInfo->GetWeaponData(m_iWeaponMode()).m_flSmackDelay;
#else
		return pWeaponInfo->GetWeaponData(TF_WEAPON_PRIMARY_MODE).m_flSmackDelay;
#endif
	return 0.2f;
}

float CTFWeaponBase::GetRange()
{
	if (auto pWeaponInfo = m_pWeaponInfo())
#ifdef WEAPONDATA_USES_WEAPONMODE
		return pWeaponInfo->GetWeaponData(m_iWeaponMode()).m_flRange;
#else
		return pWeaponInfo->GetWeaponData(TF_WEAPON_PRIMARY_MODE).m_flRange;
#endif
	return 8192.f;
}

const wchar_t* CTFWeaponBase::GetWeaponName()
{
	auto pAttributeManager = U::Memory.CallVirtual<1, void*>(uintptr_t(this) + 3096);
	auto pCurItemData = reinterpret_cast<void*>(uintptr_t(pAttributeManager) + 144);
	return S::CEconItemView_GetItemName.Call<const wchar_t*>(pCurItemData);
}

#define ReturnTexture(string) { static CHudTexture* pTexture = H::Draw.GetIcon(string); return pTexture; }
CHudTexture* CTFWeaponBase::GetWeaponIcon() // wow this is stupid
{
	switch (m_iItemDefinitionIndex())
	{
	case Scout_m_Scattergun:
	case Scout_m_ScattergunR:
	case Scout_m_FestiveScattergun:
	case Scout_m_SilverBotkillerScattergunMkI:
	case Scout_m_GoldBotkillerScattergunMkI:
	case Scout_m_RustBotkillerScattergunMkI:
	case Scout_m_BloodBotkillerScattergunMkI:
	case Scout_m_CarbonadoBotkillerScattergunMkI:
	case Scout_m_DiamondBotkillerScattergunMkI:
	case Scout_m_SilverBotkillerScattergunMkII:
	case Scout_m_GoldBotkillerScattergunMkII:
	case Scout_m_NightTerror:
	case Scout_m_TartanTorpedo:
	case Scout_m_CountryCrusher:
	case Scout_m_BackcountryBlaster:
	case Scout_m_SpruceDeuce:
	case Scout_m_CurrentEvent:
	case Scout_m_MacabreWeb:
	case Scout_m_Nutcracker:
	case Scout_m_BlueMew:
	case Scout_m_FlowerPower:
	case Scout_m_ShottoHell:
	case Scout_m_CoffinNail:
	case Scout_m_KillerBee:
	case Scout_m_Corsair: ReturnTexture("d_scattergun");
	case Scout_m_ForceANature:
	case Scout_m_FestiveForceANature: ReturnTexture("d_force_a_nature");
	case Scout_m_TheShortstop: ReturnTexture("d_shortstop");
	case Scout_m_TheSodaPopper: ReturnTexture("d_soda_popper");
	case Scout_m_BabyFacesBlaster: ReturnTexture("d_pep_brawlerblaster");
	case Scout_m_TheBackScatter: ReturnTexture("d_back_scatter");
	case Scout_s_ScoutsPistol:
	case Scout_s_PistolR:
	case Engi_s_EngineersPistol:
	case Scout_s_RedRockRoscoe:
	case Scout_s_HickoryHolepuncher:
	case Scout_s_BlackDahlia:
	case Scout_s_MacabreWeb:
	case Scout_s_Nutcracker:
	case Scout_s_BlueMew:
	case Scout_s_ShottoHell:
	case Scout_s_Blitzkrieg:
	case Engi_s_HomemadeHeater:
	case Engi_s_LocalHero:
	case Engi_s_SandstoneSpecial:
	case Engi_s_BrainCandy:
	case Engi_s_DressedToKill: ReturnTexture("d_pistol");
	case Scout_s_BonkAtomicPunch:
	case Scout_s_FestiveBonk: ReturnTexture("d_taunt_scout"); // Bonk has no icon but there is a taunt kill that says bonk so we'll use that
	case Scout_s_Lugermorph:
	case Scout_s_VintageLugermorph: ReturnTexture("d_maxgun");
	case Scout_s_TheWinger: ReturnTexture("d_the_winger");
	case Scout_s_PrettyBoysPocketPistol: ReturnTexture("d_pep_pistol");
	case Scout_s_TheFlyingGuillotine:
	case Scout_s_TheFlyingGuillotineG: ReturnTexture("d_guillotine");
	case Scout_s_TheCAPPER: ReturnTexture("d_the_capper");
	case Scout_t_Bat:
	case Scout_t_BatR:
	case Scout_t_FestiveBat: ReturnTexture("d_bat");
	case Scout_t_TheSandman: ReturnTexture("d_sandman");
	case Scout_t_TheHolyMackerel:
	case Scout_t_FestiveHolyMackerel: ReturnTexture("d_holymackerel");
	case Scout_t_TheCandyCane: ReturnTexture("d_candy_cane");
	case Scout_t_TheBostonBasher: ReturnTexture("d_boston_basher");
	case Scout_t_SunonaStick: ReturnTexture("d_lava_bat");
	case Scout_t_TheFanOWar: ReturnTexture("d_warfan");
	case Scout_t_TheAtomizer: ReturnTexture("d_atomizer");
	case Scout_t_ThreeRuneBlade: ReturnTexture("d_scout_sword");
	case Scout_t_UnarmedCombat: ReturnTexture("d_unarmed_combat");
	case Scout_t_TheWrapAssassin: ReturnTexture("d_wrap_assassin");
	case Scout_t_Batsaber: ReturnTexture("d_batsaber");
	case Soldier_m_RocketLauncher:
	case Soldier_m_RocketLauncherR:
	case Soldier_m_FestiveRocketLauncher:
	case Soldier_m_RocketJumper:
	case Soldier_m_SilverBotkillerRocketLauncherMkI:
	case Soldier_m_GoldBotkillerRocketLauncherMkI:
	case Soldier_m_RustBotkillerRocketLauncherMkI:
	case Soldier_m_BloodBotkillerRocketLauncherMkI:
	case Soldier_m_CarbonadoBotkillerRocketLauncherMkI:
	case Soldier_m_DiamondBotkillerRocketLauncherMkI:
	case Soldier_m_SilverBotkillerRocketLauncherMkII:
	case Soldier_m_GoldBotkillerRocketLauncherMkII:
	case Soldier_m_WoodlandWarrior:
	case Soldier_m_SandCannon:
	case Soldier_m_AmericanPastoral:
	case Soldier_m_SmalltownBringdown:
	case Soldier_m_ShellShocker:
	case Soldier_m_AquaMarine:
	case Soldier_m_Autumn:
	case Soldier_m_BlueMew:
	case Soldier_m_BrainCandy:
	case Soldier_m_CoffinNail: // shared with Soldier_m_CoffinNail
	case Soldier_m_HighRollers:
	case Soldier_m_Warhawk: ReturnTexture("d_tf_projectile_rocket");
	case Soldier_m_TheDirectHit: ReturnTexture("d_rocketlauncher_directhit");
	case Soldier_m_TheBlackBox:
	case Soldier_m_FestiveBlackBox: ReturnTexture("d_blackbox");
	case Soldier_m_TheLibertyLauncher: ReturnTexture("d_liberty_launcher");
	case Soldier_m_TheCowMangler5000: ReturnTexture("d_cow_mangler");
	case Soldier_m_TheOriginal: ReturnTexture("d_quake_rl");
	case Soldier_m_TheBeggarsBazooka: ReturnTexture("d_dumpster_device");
	case Soldier_m_TheAirStrike: ReturnTexture("d_airstrike");
	case Soldier_s_SoldiersShotgun:
	case Soldier_s_ShotgunR:
	case Soldier_s_FestiveShotgun:
	case Pyro_s_PyrosShotgun:
	case Heavy_s_HeavysShotgun:
	case Engi_m_EngineersShotgun:
	case Soldier_s_BackwoodsBoomstick:
	case Soldier_s_RusticRuiner:
	case Soldier_s_CivicDuty:
	case Soldier_s_LightningRod:
	case Soldier_s_Autumn:
	case Soldier_s_FlowerPower:
	case Soldier_s_CoffinNail:
	case Soldier_s_DressedtoKill:
	case Soldier_s_RedBear: ReturnTexture("d_shotgun_soldier");
	case Soldier_s_TheReserveShooter: ReturnTexture("d_reserve_shooter");
	case Soldier_s_TheRighteousBison: ReturnTexture("d_righteous_bison");
	case Soldier_s_PanicAttack: ReturnTexture("d_panic_attack");
	case Soldier_t_Shovel:
	case Soldier_t_ShovelR: ReturnTexture("d_shovel");
	case Soldier_t_TheEqualizer:
	case Soldier_t_TheEscapePlan: ReturnTexture("d_pickaxe");
	case Soldier_t_ThePainTrain: ReturnTexture("d_paintrain");
	case Soldier_t_TheHalfZatoichi: ReturnTexture("d_demokatana");
	case Soldier_t_TheMarketGardener: ReturnTexture("d_market_gardener");
	case Soldier_t_TheDisciplinaryAction: ReturnTexture("d_disciplinary_action");
	case Pyro_m_FlameThrower:
	case Pyro_m_FlameThrowerR:
	case Pyro_m_FestiveFlameThrower:
	case Pyro_m_SilverBotkillerFlameThrowerMkI:
	case Pyro_m_GoldBotkillerFlameThrowerMkI:
	case Pyro_m_RustBotkillerFlameThrowerMkI:
	case Pyro_m_BloodBotkillerFlameThrowerMkI:
	case Pyro_m_CarbonadoBotkillerFlameThrowerMkI:
	case Pyro_m_DiamondBotkillerFlameThrowerMkI:
	case Pyro_m_SilverBotkillerFlameThrowerMkII:
	case Pyro_m_GoldBotkillerFlameThrowerMkII:
	case Pyro_m_ForestFire:
	case Pyro_m_BarnBurner:
	case Pyro_m_BovineBlazemaker:
	case Pyro_m_EarthSkyandFire:
	case Pyro_m_FlashFryer:
	case Pyro_m_TurbineTorcher:
	case Pyro_m_Autumn:
	case Pyro_m_PumpkinPatch:
	case Pyro_m_Nutcracker:
	case Pyro_m_Balloonicorn:
	case Pyro_m_Rainbow:
	case Pyro_m_CoffinNail:
	case Pyro_m_Warhawk: ReturnTexture("d_flamethrower");
	case Pyro_m_TheBackburner:
	case Pyro_m_FestiveBackburner: ReturnTexture("d_backburner");
	case Pyro_m_TheDegreaser: ReturnTexture("d_degreaser");
	case Pyro_m_ThePhlogistinator: ReturnTexture("d_phlogistinator");
	case Pyro_m_TheRainblower: ReturnTexture("d_rainblower");
	case Pyro_m_DragonsFury: ReturnTexture("d_dragons_fury");
	case Pyro_s_TheFlareGun:
	case Pyro_s_FestiveFlareGun: ReturnTexture("d_flaregun");
	case Pyro_s_TheDetonator: ReturnTexture("d_detonator");
	case Pyro_s_TheManmelter: ReturnTexture("d_manmelter");
	case Pyro_s_TheScorchShot: ReturnTexture("d_scorch_shot");
	case Pyro_s_ThermalThruster: ReturnTexture("d_rocketpack");
	case Pyro_t_FireAxe:
	case Pyro_t_FireAxeR: ReturnTexture("d_fireaxe");
	case Pyro_t_TheAxtinguisher:
	case Pyro_t_TheFestiveAxtinguisher: ReturnTexture("d_axtinguisher");
	case Pyro_t_Homewrecker: ReturnTexture("d_sledgehammer");
	case Pyro_t_ThePowerjack: ReturnTexture("d_powerjack");
	case Pyro_t_TheBackScratcher: ReturnTexture("d_back_scratcher");
	case Pyro_t_SharpenedVolcanoFragment: ReturnTexture("d_lava_axe");
	case Pyro_t_ThePostalPummeler: ReturnTexture("d_mailbox");
	case Pyro_t_TheMaul: ReturnTexture("d_the_maul");
	case Pyro_t_TheThirdDegree: ReturnTexture("d_thirddegree");
	case Pyro_t_TheLollichop: ReturnTexture("d_lollichop");
	case Pyro_t_NeonAnnihilator:
	case Pyro_t_NeonAnnihilatorG: ReturnTexture("d_annihilator");
	case Pyro_t_HotHand: ReturnTexture("d_hot_hand");
	case Demoman_m_GrenadeLauncher:
	case Demoman_m_GrenadeLauncherR:
	case Demoman_m_FestiveGrenadeLauncher:
	case Demoman_m_Autumn:
	case Demoman_m_MacabreWeb:
	case Demoman_m_Rainbow:
	case Demoman_m_SweetDreams:
	case Demoman_m_CoffinNail:
	case Demoman_m_TopShelf:
	case Demoman_m_Warhawk:
	case Demoman_m_ButcherBird: ReturnTexture("d_tf_projectile_pipe");
	case Demoman_m_TheLochnLoad: ReturnTexture("d_loch_n_load");
	case Demoman_m_TheLooseCannon: ReturnTexture("d_loose_cannon_explosion");
	case Demoman_m_TheIronBomber: ReturnTexture("d_iron_bomber");
	case Demoman_s_StickybombLauncher:
	case Demoman_s_StickybombLauncherR:
	case Demoman_s_FestiveStickybombLauncher:
	case Demoman_s_StickyJumper:
	case Demoman_s_SilverBotkillerStickybombLauncherMkI:
	case Demoman_s_GoldBotkillerStickybombLauncherMkI:
	case Demoman_s_RustBotkillerStickybombLauncherMkI:
	case Demoman_s_BloodBotkillerStickybombLauncherMkI:
	case Demoman_s_CarbonadoBotkillerStickybombLauncherMkI:
	case Demoman_s_DiamondBotkillerStickybombLauncherMkI:
	case Demoman_s_SilverBotkillerStickybombLauncherMkII:
	case Demoman_s_GoldBotkillerStickybombLauncherMkII:
	case Demoman_s_SuddenFlurry:
	case Demoman_s_CarpetBomber:
	case Demoman_s_BlastedBombardier:
	case Demoman_s_RooftopWrangler:
	case Demoman_s_LiquidAsset:
	case Demoman_s_PinkElephant:
	case Demoman_s_Autumn:
	case Demoman_s_PumpkinPatch:
	case Demoman_s_MacabreWeb:
	case Demoman_s_SweetDreams:
	case Demoman_s_CoffinNail:
	case Demoman_s_DressedtoKill:
	case Demoman_s_Blitzkrieg: ReturnTexture("d_tf_projectile_pipe_remote");
	case Demoman_s_TheScottishResistance: ReturnTexture("d_sticky_resistance");
	case Demoman_s_TheCharginTarge:
	case Demoman_s_FestiveTarge: ReturnTexture("d_demoshield");
	case Demoman_s_TheSplendidScreen: ReturnTexture("d_splendid_screen");
	case Demoman_s_TheTideTurner: ReturnTexture("d_tide_turner");
	case Demoman_s_TheQuickiebombLauncher: ReturnTexture("d_quickiebomb_launcher");
	case Demoman_t_Bottle:
	case Demoman_t_BottleR: ReturnTexture("d_bottle");
	case Demoman_t_TheEyelander:
	case Demoman_t_FestiveEyelander: ReturnTexture("d_sword");
	case Demoman_t_TheScotsmansSkullcutter: ReturnTexture("d_battleaxe");
	case Demoman_t_HorselessHeadlessHorsemannsHeadtaker: ReturnTexture("d_headtaker");
	case Demoman_t_UllapoolCaber: ReturnTexture("d_ullapool_caber_explosion");
	case Demoman_t_TheClaidheamhMor: ReturnTexture("d_claidheamohmor");
	case Demoman_t_ThePersianPersuader: ReturnTexture("d_persian_persuader");
	case Demoman_t_NessiesNineIron: ReturnTexture("d_nessieclub");
	case Demoman_t_TheScottishHandshake: ReturnTexture("d_scotland_shard");
	case Heavy_m_Minigun:
	case Heavy_m_MinigunR:
	case Heavy_m_FestiveMinigun:
	case Heavy_m_SilverBotkillerMinigunMkI:
	case Heavy_m_GoldBotkillerMinigunMkI:
	case Heavy_m_RustBotkillerMinigunMkI:
	case Heavy_m_BloodBotkillerMinigunMkI:
	case Heavy_m_CarbonadoBotkillerMinigunMkI:
	case Heavy_m_DiamondBotkillerMinigunMkI:
	case Heavy_m_SilverBotkillerMinigunMkII:
	case Heavy_m_GoldBotkillerMinigunMkII:
	case Heavy_m_KingoftheJungle:
	case Heavy_m_IronWood:
	case Heavy_m_AntiqueAnnihilator:
	case Heavy_m_WarRoom:
	case Heavy_m_CitizenPain:
	case Heavy_m_BrickHouse:
	case Heavy_m_MacabreWeb:
	case Heavy_m_PumpkinPatch:
	case Heavy_m_Nutcracker:
	case Heavy_m_BrainCandy:
	case Heavy_m_MisterCuddles:
	case Heavy_m_CoffinNail:
	case Heavy_m_DressedtoKill:
	case Heavy_m_TopShelf:
	case Heavy_m_ButcherBird: ReturnTexture("d_minigun");
	case Heavy_m_Natascha: ReturnTexture("d_natascha");
	case Heavy_m_IronCurtain:
	case Heavy_m_Deflector_mvm: ReturnTexture("d_iron_curtain");
	case Heavy_m_TheBrassBeast: ReturnTexture("d_brass_beast");
	case Heavy_m_Tomislav: ReturnTexture("d_tomislav");
	case Heavy_m_TheHuoLongHeater:
	case Heavy_m_TheHuoLongHeaterG: ReturnTexture("d_long_heatmaker");
	case Heavy_s_TheFamilyBusiness: ReturnTexture("d_family_business");
	case Heavy_t_Fists:
	case Heavy_t_FistsR: ReturnTexture("d_fists");
	case Heavy_t_TheKillingGlovesofBoxing: ReturnTexture("d_gloves");
	case Heavy_t_GlovesofRunningUrgently:
	case Heavy_t_FestiveGlovesofRunningUrgently:
	case Heavy_t_GlovesofRunningUrgentlyMvM: ReturnTexture("d_gloves_running_urgently");
	case Heavy_t_WarriorsSpirit: ReturnTexture("d_warrior_spirit");
	case Heavy_t_FistsofSteel: ReturnTexture("d_steel_fists");
	case Heavy_t_TheEvictionNotice: ReturnTexture("d_eviction_notice");
	case Heavy_t_ApocoFists: ReturnTexture("d_apocofists");
	case Heavy_t_TheHolidayPunch: ReturnTexture("d_holiday_punch");
	case Heavy_t_TheBreadBite: ReturnTexture("d_bread_bite");
	case Engi_m_TheFrontierJustice:
	case Engi_m_FestiveFrontierJustice: ReturnTexture("d_frontier_kill");
	case Engi_m_TheWidowmaker: ReturnTexture("d_widowmaker");
	case Engi_m_ThePomson6000: ReturnTexture("d_pomson");
	case Engi_m_TheRescueRanger: ReturnTexture("d_rescue_ranger");
	case Engi_s_TheWrangler:
	case Engi_s_FestiveWrangler: ReturnTexture("d_wrangler_kill");
	case Engi_s_TheShortCircuit: ReturnTexture("d_short_circuit");
	case Engi_s_TheGigarCounter: ReturnTexture("d_giger_counter");
	case Engi_t_Wrench:
	case Engi_t_WrenchR:
	case Engi_t_FestiveWrench:
	case Engi_t_GoldenWrench:
	case Engi_t_SilverBotkillerWrenchMkI:
	case Engi_t_GoldBotkillerWrenchMkI:
	case Engi_t_RustBotkillerWrenchMkI:
	case Engi_t_BloodBotkillerWrenchMkI:
	case Engi_t_CarbonadoBotkillerWrenchMkI:
	case Engi_t_DiamondBotkillerWrenchMkI:
	case Engi_t_SilverBotkillerWrenchMkII:
	case Engi_t_GoldBotkillerWrenchMkII:
	case Engi_t_Nutcracker:
	case Engi_t_Autumn:
	case Engi_t_Boneyard:
	case Engi_t_DressedtoKill:
	case Engi_t_TopShelf:
	case Engi_t_TorquedtoHell:
	case Engi_t_Airwolf: ReturnTexture("d_wrench");
	case Engi_t_TheGunslinger: ReturnTexture("d_robot_arm_kill");
	case Engi_t_TheSouthernHospitality: ReturnTexture("d_southern_comfort_kill");
	case Engi_t_TheJag: ReturnTexture("d_wrench_jag");
	case Engi_t_TheEurekaEffect: ReturnTexture("d_eureka_effect");
	case Medic_m_SyringeGun:
	case Medic_m_SyringeGunR: ReturnTexture("d_syringegun_medic");
	case Medic_m_TheBlutsauger: ReturnTexture("d_blutsauger");
	case Medic_m_CrusadersCrossbow:
	case Medic_m_FestiveCrusadersCrossbow: ReturnTexture("d_crusaders_crossbow");
	case Medic_m_TheOverdose: ReturnTexture("d_proto_syringe");
	case Medic_t_Bonesaw:
	case Medic_t_BonesawR:
	case Medic_t_FestiveBonesaw: ReturnTexture("d_bonesaw");
	case Medic_t_TheUbersaw:
	case Medic_t_FestiveUbersaw: ReturnTexture("d_ubersaw");
	case Medic_t_TheVitaSaw: ReturnTexture("d_battleneedle");
	case Medic_t_Amputator: ReturnTexture("d_amputator");
	case Medic_t_TheSolemnVow: ReturnTexture("d_solemn_vow");
	case Sniper_m_SniperRifle:
	case Sniper_m_SniperRifleR:
	case Sniper_m_FestiveSniperRifle:
	case Sniper_m_TheAWPerHand:
	case Sniper_m_SilverBotkillerSniperRifleMkI:
	case Sniper_m_GoldBotkillerSniperRifleMkI:
	case Sniper_m_RustBotkillerSniperRifleMkI:
	case Sniper_m_BloodBotkillerSniperRifleMkI:
	case Sniper_m_CarbonadoBotkillerSniperRifleMkI:
	case Sniper_m_DiamondBotkillerSniperRifleMkI:
	case Sniper_m_SilverBotkillerSniperRifleMkII:
	case Sniper_m_GoldBotkillerSniperRifleMkII:
	case Sniper_m_NightOwl:
	case Sniper_m_PurpleRange:
	case Sniper_m_LumberFromDownUnder:
	case Sniper_m_ShotintheDark:
	case Sniper_m_Bogtrotter:
	case Sniper_m_Thunderbolt:
	case Sniper_m_PumpkinPatch:
	case Sniper_m_Boneyard:
	case Sniper_m_Wildwood:
	case Sniper_m_Balloonicorn:
	case Sniper_m_Rainbow:
	case Sniper_m_CoffinNail:
	case Sniper_m_DressedtoKill: ReturnTexture("d_headshot");
	case Sniper_m_TheHuntsman:
	case Sniper_m_FestiveHuntsman:
	case Sniper_m_TheFortifiedCompound: ReturnTexture("d_huntsman");
	case Sniper_m_TheSydneySleeper: ReturnTexture("d_sydney_sleeper");
	case Sniper_m_TheBazaarBargain: ReturnTexture("d_bazaar_bargain");
	case Sniper_m_TheMachina: ReturnTexture("d_machina");
	case Sniper_m_TheHitmansHeatmaker: ReturnTexture("d_pro_rifle");
	case Sniper_m_TheClassic: ReturnTexture("d_the_classic");
	case Sniper_m_ShootingStar: ReturnTexture("d_shooting_star");
	case Sniper_s_SMG:
	case Sniper_s_SMGR:
	case Sniper_s_FestiveSMG:
	case Sniper_s_WoodsyWidowmaker:
	case Sniper_s_PlaidPotshotter:
	case Sniper_s_TreadplateTormenter:
	case Sniper_s_TeamSprayer:
	case Sniper_s_LowProfile:
	case Sniper_s_Wildwood:
	case Sniper_s_BlueMew:
	case Sniper_s_HighRollers:
	case Sniper_s_Blitzkrieg: ReturnTexture("d_smg");
	case Sniper_s_TheCleanersCarbine: ReturnTexture("d_pro_smg");
	case Sniper_t_Kukri:
	case Sniper_t_KukriR: ReturnTexture("d_club");
	case Sniper_t_TheTribalmansShiv: ReturnTexture("d_tribalkukri");
	case Sniper_t_TheBushwacka: ReturnTexture("d_bushwacka");
	case Sniper_t_TheShahanshah: ReturnTexture("d_shahanshah");
	case Spy_m_Revolver:
	case Spy_m_RevolverR:
	case Spy_m_FestiveRevolver:
	case Spy_m_PsychedelicSlugger:
	case Spy_m_OldCountry:
	case Spy_m_Mayor:
	case Spy_m_DeadReckoner:
	//case Spy_m_Boneyard: // shared with Spy_t_Boneyard
	case Spy_m_Wildwood:
	case Spy_m_MacabreWeb:
	case Spy_m_FlowerPower:
	case Spy_m_TopShelf:
	//case Spy_m_CoffinNail: // shared with Soldier_m_CoffinNail
	case Spy_m_Blitzkrieg: ReturnTexture("d_revolver");
	case Spy_m_TheAmbassador:
	case Spy_m_FestiveAmbassador: ReturnTexture("d_ambassador");
	case Spy_m_BigKill: ReturnTexture("d_samrevolver");
	case Spy_m_LEtranger: ReturnTexture("d_letranger");
	case Spy_m_TheEnforcer: ReturnTexture("d_enforcer");
	case Spy_m_TheDiamondback: ReturnTexture("d_diamondback");
	case Spy_s_Sapper:
	case Spy_s_SapperR:
	case Spy_s_FestiveSapper: ReturnTexture("d_obj_attachment_sapper");
	case Spy_s_TheRedTapeRecorder:
	case Spy_s_TheRedTapeRecorderG: ReturnTexture("d_recorder");
	case Spy_s_TheApSapG: ReturnTexture("d_psapper");
	case Spy_s_TheSnackAttack: ReturnTexture("d_snack_attack");
	case Spy_t_Knife:
	case Spy_t_KnifeR:
	case Spy_t_FestiveKnife:
	case Spy_t_SilverBotkillerKnifeMkI:
	case Spy_t_GoldBotkillerKnifeMkI:
	case Spy_t_RustBotkillerKnifeMkI:
	case Spy_t_BloodBotkillerKnifeMkI:
	case Spy_t_CarbonadoBotkillerKnifeMkI:
	case Spy_t_DiamondBotkillerKnifeMkI:
	case Spy_t_SilverBotkillerKnifeMkII:
	case Spy_t_GoldBotkillerKnifeMkII:
	case Spy_t_Boneyard:
	case Spy_t_BlueMew:
	case Spy_t_BrainCandy:
	case Spy_t_StabbedtoHell:
	case Spy_t_DressedtoKill:
	case Spy_t_TopShelf:
	case Spy_t_Blitzkrieg:
	case Spy_t_Airwolf: ReturnTexture("d_knife");
	case Spy_t_YourEternalReward: ReturnTexture("d_eternal_reward");
	case Spy_t_ConniversKunai: ReturnTexture("d_kunai");
	case Spy_t_TheBigEarner: ReturnTexture("d_big_earner");
	case Spy_t_TheWangaPrick: ReturnTexture("d_voodoo_pin");
	case Spy_t_TheSharpDresser: ReturnTexture("d_sharp_dresser");
	case Spy_t_TheSpycicle: ReturnTexture("d_spy_cicle");
	case Spy_t_TheBlackRose: ReturnTexture("d_black_rose");
	case Spy_d_DisguiseKitPDA: ReturnTexture("hud_spy_disguise_menu_icon");
	case Misc_t_FryingPan:
	case Misc_t_GoldFryingPan: ReturnTexture("d_fryingpan");
	case Misc_t_TheConscientiousObjector: ReturnTexture("d_nonnonviolent_protest");
	case Misc_t_TheBatOuttaHell: ReturnTexture("d_skull");
	case Misc_t_TheFreedomStaff: ReturnTexture("d_freedom_staff");
	case Misc_t_TheMemoryMaker: ReturnTexture("d_memory_maker");
	case Misc_t_TheHamShank: ReturnTexture("d_ham_shank");
	case Misc_t_TheNecroSmasher: ReturnTexture("d_necro_smasher");
	case Misc_t_TheCrossingGuard: ReturnTexture("d_crossing_guard");
	case Misc_t_PrinnyMachete: ReturnTexture("d_prinny_machete");
	}
	ReturnTexture("d_skull");
}



int CWeaponMedigun::GetMedigunType()
{
	return SDK::AttribHookValue(0, "set_weapon_mode", this);
}

MedigunChargeTypes CWeaponMedigun::GetChargeType()
{
	int iTmp = SDK::AttribHookValue(MEDIGUN_CHARGE_INVULN, "set_charge_type", this);
	if (GetMedigunType() == MEDIGUN_RESIST)
		iTmp += m_nChargeResistType();

	return MedigunChargeTypes(iTmp);
}

medigun_resist_types_t CWeaponMedigun::GetResistType()
{
	int nCurrentActiveResist = (GetChargeType() - MEDIGUN_CHARGE_BULLET_RESIST);
	nCurrentActiveResist = nCurrentActiveResist % MEDIGUN_NUM_RESISTS;
	return medigun_resist_types_t(nCurrentActiveResist);
}



int CTFPipebombLauncher::GetDetonateType()
{
	return SDK::AttribHookValue(0, "set_detonate_mode", this);
}



int CTFSniperRifle::GetRifleType()
{
	return SDK::AttribHookValue(0, "set_weapon_mode", this);
}

float CTFSniperRifle::GetHeadshotMult(CTFPlayer* pTarget)
{
	auto fGetMainMult = [&]()
	{
		auto pOwner = m_hOwnerEntity()->As<CTFPlayer>();
		if (pOwner && pOwner->IsCritBoosted())
			return 3.f;

		if (GetRifleType() == RIFLE_JARATE)
		{
			if (SDK::AttribHookValue(0, "jarate_duration", this) > 0)
				return 1.36f;

			if (pOwner && pOwner->IsMiniCritBoosted()
				|| pTarget && (pTarget->InCond(TF_COND_URINE) || pTarget->InCond(TF_COND_MARKEDFORDEATH)))
				return 1.36f;

			return 1.f;
		}

		return 3.f;
	};

	float flMult = SDK::AttribHookValue(fGetMainMult(), "mult_dmg", this);
	if (m_flChargedDamage() == 150.f)
		flMult = SDK::AttribHookValue(flMult, "sniper_full_charge_damage_bonus", this);
	return flMult;
}

float CTFSniperRifle::GetBodyshotMult(CTFPlayer* pTarget)
{
	auto fGetMainMult = [&]()
	{
		auto pOwner = m_hOwnerEntity()->As<CTFPlayer>();
		if (pOwner && pOwner->IsCritBoosted())
			return 3.f;

		if (pOwner && pOwner->IsMiniCritBoosted()
			|| pTarget && (pTarget->InCond(TF_COND_URINE) || pTarget->InCond(TF_COND_MARKEDFORDEATH)))
			return 1.36f;

		return 1.f;
	};

	float flMult = SDK::AttribHookValue(fGetMainMult(), "mult_dmg", this);
	flMult = SDK::AttribHookValue(flMult, "bodyshot_damage_modify", this);
	if (m_flChargedDamage() == 150.f)
		flMult = SDK::AttribHookValue(flMult, "sniper_full_charge_damage_bonus", this);
	return flMult;
}



int CTFGrenadeLauncher::GetDetonateType()
{
	return SDK::AttribHookValue(0, "set_detonate_mode", this);
}



int CTFFlareGun::GetFlareGunType()
{
	return SDK::AttribHookValue(0, "set_weapon_mode", this);
}