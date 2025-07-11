#include "CTFWeaponBase.h"

#include "../../SDK.h"

bool CTFWeaponBase::HasPrimaryAmmoForShot()
{
	if (IsEnergyWeapon())
		return m_flEnergy() > 0.f;

	int iClip = m_iClip1();
	auto pOwner = m_hOwnerEntity().Get();
	return (iClip == -1 && pOwner ? pOwner->As<CTFPlayer>()->GetAmmoCount(m_iPrimaryAmmoType()) : iClip) >= GetAmmoPerShot();
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

float CTFWeaponBase::GetDamage(bool bAttribHookValue)
{
	if (auto pWeaponInfo = GetWeaponInfo())
	{
		if (!bAttribHookValue)
			return pWeaponInfo->GetWeaponData(0).m_nDamage;
		return SDK::AttribHookValue(pWeaponInfo->GetWeaponData(0).m_nDamage, "mult_dmg", this);
	}
	return 0.f;
}

float CTFWeaponBase::GetFireRate(bool bAttribHookValue)
{
	if (auto pWeaponInfo = GetWeaponInfo())
	{
		if (!bAttribHookValue)
			return pWeaponInfo->GetWeaponData(0).m_flTimeFireDelay;
		return SDK::AttribHookValue(pWeaponInfo->GetWeaponData(0).m_flTimeFireDelay, "mult_postfiredelay", this);
	}
	return 0.315f;
}

int CTFWeaponBase::GetBulletsPerShot(bool bAttribHookValue)
{
	if (auto pWeaponInfo = GetWeaponInfo())
	{
		if (!bAttribHookValue)
			return pWeaponInfo->GetWeaponData(0).m_nBulletsPerShot;
		return SDK::AttribHookValue(pWeaponInfo->GetWeaponData(0).m_nBulletsPerShot, "mult_bullets_per_shot", this);
	}
	return 1;
}

int CTFWeaponBase::GetAmmoPerShot(bool bAttribHookValue)
{
	if (auto pWeaponInfo = GetWeaponInfo())
	{
		if (!bAttribHookValue)
			return pWeaponInfo->GetWeaponData(0).m_iAmmoPerShot;
		int iAmmoPerShot = SDK::AttribHookValue(0, "mod_ammo_per_shot", this);
		return iAmmoPerShot > 0 ? iAmmoPerShot : pWeaponInfo->GetWeaponData(0).m_iAmmoPerShot;
	}
	return 1;
}

bool CTFWeaponBase::IsRapidFire()
{
	if (auto pWeaponInfo = GetWeaponInfo())
		return pWeaponInfo->GetWeaponData(0).m_bUseRapidFireCrits;
	return false;
}

float CTFWeaponBase::GetRange()
{
	switch (GetWeaponID())
	{
	case TF_WEAPON_MEDIGUN: return 450.f;
	case TF_WEAPON_MECHANICAL_ARM: return 256.f;
	}

	if (auto pWeaponInfo = GetWeaponInfo())
		return pWeaponInfo->GetWeaponData(0).m_flRange;
	return 8192.f;
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
	auto GetMainMult = [&]()
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

	float flMult = SDK::AttribHookValue(GetMainMult(), "mult_dmg", this);
	if (m_flChargedDamage() == 150.f)
		flMult = SDK::AttribHookValue(flMult, "sniper_full_charge_damage_bonus", this);
	return flMult;
}

float CTFSniperRifle::GetBodyshotMult(CTFPlayer* pTarget)
{
	auto GetMainMult = [&]()
		{
			auto pOwner = m_hOwnerEntity()->As<CTFPlayer>();
			if (pOwner && pOwner->IsCritBoosted())
				return 3.f;

			if (pOwner && pOwner->IsMiniCritBoosted()
				|| pTarget && (pTarget->InCond(TF_COND_URINE) || pTarget->InCond(TF_COND_MARKEDFORDEATH)))
				return 1.36f;

			return 1.f;
		};

	float flMult = SDK::AttribHookValue(GetMainMult(), "mult_dmg", this);
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



#define ReturnTexture(string) { static CHudTexture* pTexture = H::Draw.GetIcon(string); return pTexture; }
CHudTexture* CTFWeaponBase::GetWeaponIcon() // wow this is stupid
{
	switch (m_iItemDefinitionIndex())
	{
	case 13:
	case 200:
	case 669:
	case 799:
	case 808:
	case 888:
	case 897:
	case 906:
	case 915:
	case 964:
	case 973:
	case 15002:
	case 15015:
	case 15021:
	case 15029:
	case 15036:
	case 15053:
	case 15065:
	case 15069:
	case 15106:
	case 15107:
	case 15108:
	case 15131:
	case 15151:
	case 15157:
		ReturnTexture("d_scattergun");
	case 45:
	case 1078:
		ReturnTexture("d_force_a_nature");
	case 220:
		ReturnTexture("d_shortstop");
	case 448:
		ReturnTexture("d_soda_popper");
	case 772:
		ReturnTexture("d_pep_brawlerblaster");
	case 1103:
		ReturnTexture("d_back_scatter");
	case 22:
	case 23:
	case 209:
	case 15013:
	case 15018:
	case 15035:
	case 15041:
	case 15046:
	case 15056:
	case 15060:
	case 15061:
	case 15100:
	case 15101:
	case 15102:
	case 15126:
	case 15148:
		ReturnTexture("d_pistol");
	case 46:
	case 1145:
		ReturnTexture("d_taunt_scout"); // Bonk has no icon but there is a taunt kill that says bonk so we'll use that
	case 160:
	case 294:
		ReturnTexture("d_maxgun");
	case 449:
		ReturnTexture("d_the_winger");
	case 773:
		ReturnTexture("d_pep_pistol");
	case 812:
	case 833:
		ReturnTexture("d_guillotine");
	case 30666:
		ReturnTexture("d_the_capper");
	case 0:
	case 190:
	case 660:
		ReturnTexture("d_bat");
	case 44:
		ReturnTexture("d_sandman");
	case 221:
	case 999:
		ReturnTexture("d_holymackerel");
	case 317:
		ReturnTexture("d_candy_cane");
	case 325:
		ReturnTexture("d_boston_basher");
	case 349:
		ReturnTexture("d_lava_bat");
	case 355:
		ReturnTexture("d_warfan");
	case 450:
		ReturnTexture("d_atomizer");
	case 452:
		ReturnTexture("d_scout_sword");
	case 264:
	case 1071:
		ReturnTexture("d_fryingpan");
	case 474:
		ReturnTexture("d_nonnonviolent_protest");
	case 572:
		ReturnTexture("d_unarmed_combat");
	case 648:
		ReturnTexture("d_wrap_assassin");
	case 939:
		ReturnTexture("d_skull");
	case 880:
		ReturnTexture("d_freedom_staff");
	case 954:
		ReturnTexture("d_memory_maker");
	case 1013:
		ReturnTexture("d_ham_shank");
	case 1123:
		ReturnTexture("d_necro_smasher");
	case 1127:
		ReturnTexture("d_crossing_guard");
	case 30667:
		ReturnTexture("d_batsaber");
	case 30758:
		ReturnTexture("d_prinny_machete");
	case 18:
	case 205:
	case 658:
	case 800:
	case 809:
	case 889:
	case 898:
	case 907:
	case 916:
	case 965:
	case 974:
	case 15006:
	case 15014:
	case 15028:
	case 15043:
	case 15052:
	case 15057:
	case 15081:
	case 15104:
	case 15105:
	case 15130:
	case 15150:
	case 237: // rocket jumper :|
		ReturnTexture("d_tf_projectile_rocket");
	case 127:
		ReturnTexture("d_rocketlauncher_directhit");
	case 228:
	case 1085:
		ReturnTexture("d_blackbox");
	case 414:
		ReturnTexture("d_liberty_launcher");
	case 441:
		ReturnTexture("d_cow_mangler");
	case 513:
		ReturnTexture("d_quake_rl");
	case 730:
		ReturnTexture("d_dumpster_device");
	case 1104:
		ReturnTexture("d_airstrike");
	case 10:
	case 199:
	case 15003:
	case 15016:
	case 15044:
	case 15047:
	case 15085:
	case 15109:
	case 15132:
	case 15133:
	case 15152:
	case 1141:
	case 12:
	case 11:
	case 9:
		ReturnTexture("d_shotgun_soldier");
	case 415:
		ReturnTexture("d_reserve_shooter");
	case 442:
		ReturnTexture("d_righteous_bison");
	case 1153:
		ReturnTexture("d_panic_attack");
	case 6:
	case 196:
		ReturnTexture("d_shovel");
	case 128:
	case 775:
		ReturnTexture("d_pickaxe");
	case 154:
		ReturnTexture("d_paintrain");
	case 357:
		ReturnTexture("d_demokatana");
	case 416:
		ReturnTexture("d_market_gardener");
	case 447:
		ReturnTexture("d_disciplinary_action");
	case 21:
	case 208:
	case 659:
	case 798:
	case 807:
	case 887:
	case 896:
	case 905:
	case 914:
	case 963:
	case 972:
	case 15005:
	case 15017:
	case 15030:
	case 15034:
	case 15049:
	case 15054:
	case 15066:
	case 15067:
	case 15068:
	case 15089:
	case 15090:
	case 15115:
	case 15141:
		ReturnTexture("d_flamethrower");
	case 40:
	case 1146:
		ReturnTexture("d_backburner");
	case 215:
		ReturnTexture("d_degreaser");
	case 594:
		ReturnTexture("d_phlogistinator");
	case 741:
		ReturnTexture("d_rainblower");
	case 1178:
		ReturnTexture("d_dragons_fury");
	case 39:
	case 1081:
		ReturnTexture("d_flaregun");
	case 740:
		ReturnTexture("d_scorch_shot");
	case 595:
		ReturnTexture("d_manmelter");
	case 351:
		ReturnTexture("d_detonator");
	case 1179:
		ReturnTexture("d_rocketpack");
	case 2:
	case 192:
		ReturnTexture("d_fireaxe");
	case 38:
	case 1000:
		ReturnTexture("d_axtinguisher");
	case 153:
		ReturnTexture("d_sledgehammer");
	case 214:
		ReturnTexture("d_powerjack");
	case 326:
		ReturnTexture("d_back_scratcher");
	case 348:
		ReturnTexture("d_lava_axe");
	case 457:
		ReturnTexture("d_mailbox");
	case 466:
		ReturnTexture("d_the_maul");
	case 593:
		ReturnTexture("d_thirddegree");
	case 739:
		ReturnTexture("d_lollichop");
	case 813:
	case 834:
		ReturnTexture("d_annihilator");
	case 1181:
		ReturnTexture("d_hot_hand");
	case 19:
	case 206:
	case 15077:
	case 15079:
	case 15091:
	case 15092:
	case 15116:
	case 15117:
	case 15142:
	case 15158:
	case 1007:
		ReturnTexture("d_tf_projectile_pipe");
	case 308:
		ReturnTexture("d_loch_n_load");
	case 996:
		ReturnTexture("d_loose_cannon_explosion");
	case 1151:
		ReturnTexture("d_iron_bomber");
	case 20:
	case 207:
	case 661:
	case 797:
	case 806:
	case 886:
	case 895:
	case 904:
	case 913:
	case 962:
	case 971:
	case 265:
	case 15009:
	case 15012:
	case 15024:
	case 15038:
	case 15045:
	case 15048:
	case 15082:
	case 15083:
	case 15084:
	case 15113:
	case 15137:
	case 15138:
	case 15155:
		ReturnTexture("d_tf_projectile_pipe_remote");
	case 130:
		ReturnTexture("d_sticky_resistance");
	case 131:
	case 1144:
		ReturnTexture("d_demoshield");
	case 1099:
		ReturnTexture("d_tide_turner");
	case 406:
		ReturnTexture("d_splendid_screen");
	case 1150:
		ReturnTexture("d_quickiebomb_launcher");
	case 1:
	case 191:
		ReturnTexture("d_bottle");
	case 132:
	case 1082:
		ReturnTexture("d_sword");
	case 172:
		ReturnTexture("d_battleaxe");
	case 266:
		ReturnTexture("d_headtaker");
	case 307:
		ReturnTexture("d_ullapool_caber_explosion");
	case 327:
		ReturnTexture("d_claidheamohmor");
	case 404:
		ReturnTexture("d_persian_persuader");
	case 482:
		ReturnTexture("d_nessieclub");
	case 609:
		ReturnTexture("d_scotland_shard");
	case 15:
	case 202:
	case 654:
	case 793:
	case 802:
	case 882:
	case 891:
	case 900:
	case 909:
	case 958:
	case 967:
	case 15004:
	case 15020:
	case 15026:
	case 15031:
	case 15040:
	case 15055:
	case 15086:
	case 15087:
	case 15088:
	case 15098:
	case 15099:
	case 15123:
	case 15124:
	case 15125:
	case 15147:
		ReturnTexture("d_minigun");
	case 41:
		ReturnTexture("d_natascha");
	case 312:
		ReturnTexture("d_brass_beast");
	case 424:
		ReturnTexture("d_tomislav");
	case 811:
	case 832:
		ReturnTexture("d_long_heatmaker");
	case 298:
	case 850:
		ReturnTexture("d_iron_curtain");
	case 425:
		ReturnTexture("d_family_business");
	case 5:
	case 195:
		ReturnTexture("d_fists");
	case 43:
		ReturnTexture("d_gloves");
	case 239:
	case 1084:
	case 1184:
		ReturnTexture("d_gloves_running_urgently");
	case 310:
		ReturnTexture("d_warrior_spirit");
	case 331:
		ReturnTexture("d_steel_fists");
	case 426:
		ReturnTexture("d_eviction_notice");
	case 587:
		ReturnTexture("d_apocofists");
	case 656:
		ReturnTexture("d_holiday_punch");
	case 1100:
		ReturnTexture("d_bread_bite");
	case 141:
	case 1004:
		ReturnTexture("d_frontier_kill");
	case 527:
		ReturnTexture("d_widowmaker");
	case 588:
		ReturnTexture("d_pomson");
	case 997:
		ReturnTexture("d_rescue_ranger");
	case 140:
	case 1086:
		ReturnTexture("d_wrangler_kill");
	case 528:
		ReturnTexture("d_short_circuit");
	case 30668:
		ReturnTexture("d_giger_counter");
	case 7:
	case 197:
	case 662:
	case 795:
	case 804:
	case 884:
	case 893:
	case 902:
	case 911:
	case 960:
	case 969:
	case 15073:
	case 15074:
	case 15075:
	case 15139:
	case 15140:
	case 15114:
	case 15156:
	case 169:
		ReturnTexture("d_wrench");
	case 142:
		ReturnTexture("d_robot_arm_kill");
	case 155:
		ReturnTexture("d_southern_comfort_kill");
	case 329:
		ReturnTexture("d_wrench_jag");
	case 589:
		ReturnTexture("d_eureka_effect");
	case 17:
	case 204:
		ReturnTexture("d_syringegun_medic");
	case 36:
		ReturnTexture("d_blutsauger");
	case 305:
	case 1079:
		ReturnTexture("d_crusaders_crossbow");
	case 412:
		ReturnTexture("d_proto_syringe");
	case 8:
	case 198:
	case 1143:
		ReturnTexture("d_bonesaw");
	case 37:
	case 1003:
		ReturnTexture("d_ubersaw");
	case 173:
		ReturnTexture("d_battleneedle");
	case 304:
		ReturnTexture("d_amputator");
	case 413:
		ReturnTexture("d_solemn_vow");
	case 14:
	case 201:
	case 664:
	case 792:
	case 801:
	case 881:
	case 890:
	case 899:
	case 908:
	case 957:
	case 966:
	case 15000:
	case 15007:
	case 15019:
	case 15023:
	case 15033:
	case 15059:
	case 15070:
	case 15071:
	case 15072:
	case 15111:
	case 15112:
	case 15135:
	case 15136:
	case 851:
		ReturnTexture("d_headshot");
	case 56:
	case 1005:
	case 1092:
		ReturnTexture("d_huntsman");
	case 30665:
		ReturnTexture("d_shooting_star");
	case 1098:
		ReturnTexture("d_the_classic");
	case 752:
		ReturnTexture("d_pro_rifle");
	case 526:
		ReturnTexture("d_machina");
	case 402:
		ReturnTexture("d_bazaar_bargain");
	case 230:
		ReturnTexture("d_sydney_sleeper");
	case 16:
	case 203:
	case 1149:
	case 15001:
	case 15022:
	case 15032:
	case 15037:
	case 15058:
	case 15076:
	case 15110:
	case 15134:
	case 15153:
		ReturnTexture("d_smg");
	case 751:
		ReturnTexture("d_pro_smg");
	case 3:
	case 193:
		ReturnTexture("d_club");
	case 171:
		ReturnTexture("d_tribalkukri");
	case 232:
		ReturnTexture("d_bushwacka");
	case 401:
		ReturnTexture("d_shahanshah");
	case 24:
	case 210:
	case 1142:
	case 15011:
	case 15027:
	case 15042:
	case 15051:
	case 15062:
	case 15063:
	case 15064:
	case 15103:
	case 15128:
	case 15129:
	case 15149:
		ReturnTexture("d_revolver");
	case 61:
	case 1006:
		ReturnTexture("d_ambassador");
	case 161:
		ReturnTexture("d_samrevolver");
	case 224:
		ReturnTexture("d_letranger");
	case 460:
		ReturnTexture("d_enforcer");
	case 525:
		ReturnTexture("d_diamondback");
	case 735:
	case 736:
	case 1080:
		ReturnTexture("d_obj_attachment_sapper");
	case 933:
		ReturnTexture("d_psapper");
	case 1102:
		ReturnTexture("d_snack_attack");
	case 810:
	case 831:
		ReturnTexture("d_recorder");
	case 225:
		ReturnTexture("d_eternal_reward");
	case 356:
		ReturnTexture("d_kunai");
	case 461:
		ReturnTexture("d_big_earner");
	case 574:
		ReturnTexture("d_voodoo_pin");
	case 638:
		ReturnTexture("d_sharp_dresser");
	case 649:
		ReturnTexture("d_spy_cicle");
	case 4:
	case 194:
	case 665:
	case 794:
	case 803:
	case 883:
	case 892:
	case 901:
	case 910:
	case 959:
	case 968:
	case 15094:
	case 15095:
	case 15096:
	case 15118:
	case 15119:
	case 15143:
	case 15144:
		ReturnTexture("d_knife");
	case 727:
		ReturnTexture("d_black_rose");
	case 27:
		ReturnTexture("hud_spy_disguise_menu_icon");
	}
	ReturnTexture("d_skull");
}