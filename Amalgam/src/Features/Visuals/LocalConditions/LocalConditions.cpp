#include "LocalConditions.h"

std::vector<std::wstring> LCESP::GetPlayerConditions(CTFPlayer* pEntity) const
{
	std::vector<std::wstring> szCond{};

	if (Vars::Menu::Indicators.Value & (1 << 4))
	{
		if (pEntity->InCond(TF_COND_CRITBOOSTED))
			szCond.emplace_back(L"KRITS");
		else if (pEntity->InCond(TF_COND_CRITBOOSTED_PUMPKIN) ||
			pEntity->InCond(TF_COND_CRITBOOSTED_USER_BUFF) ||
			pEntity->InCond(TF_COND_CRITBOOSTED_DEMO_CHARGE) ||
			pEntity->InCond(TF_COND_CRITBOOSTED_FIRST_BLOOD) ||
			pEntity->InCond(TF_COND_CRITBOOSTED_BONUS_TIME) ||
			pEntity->InCond(TF_COND_CRITBOOSTED_CTF_CAPTURE) ||
			pEntity->InCond(TF_COND_CRITBOOSTED_ON_KILL) ||
			pEntity->InCond(TF_COND_CRITBOOSTED_RAGE_BUFF) ||
			pEntity->InCond(TF_COND_CRITBOOSTED_CARD_EFFECT) ||
			pEntity->InCond(TF_COND_CRITBOOSTED_RUNE_TEMP))
			szCond.emplace_back(L"CRITS");

		if (pEntity->InCond(TF_COND_ENERGY_BUFF) ||
			pEntity->InCond(TF_COND_NOHEALINGDAMAGEBUFF))
			szCond.emplace_back(L"MINI CRITS");

		if (pEntity->InCond(TF_COND_MINICRITBOOSTED_ON_KILL))
			szCond.emplace_back(L"MINI CRITS ON KILL");

		if (pEntity->InCond(TF_COND_OFFENSEBUFF))
			szCond.emplace_back(L"OFFENSE BUFF");

		if (pEntity->InCond(TF_COND_TMPDAMAGEBONUS))
			szCond.emplace_back(L"DAMAGE BONUS");

		if (pEntity->InCond(TF_COND_INVULNERABLE) ||
			pEntity->InCond(TF_COND_INVULNERABLE_HIDE_UNLESS_DAMAGED) ||
			pEntity->InCond(TF_COND_INVULNERABLE_USER_BUFF) ||
			pEntity->InCond(TF_COND_INVULNERABLE_CARD_EFFECT))
			szCond.emplace_back(L"INVULNERABLE");

		if (pEntity->InCond(TF_COND_INVULNERABLE_WEARINGOFF))
			szCond.emplace_back(L"INVULNERABLE-");

		if (pEntity->InCond(TF_COND_DEFENSEBUFF))
			szCond.emplace_back(L"DEFENSE BUFF");

		if (pEntity->InCond(TF_COND_DEFENSEBUFF_HIGH))
			szCond.emplace_back(L"DEFENSE BUFF+");

		if (pEntity->InCond(TF_COND_DEFENSEBUFF_NO_CRIT_BLOCK))
			szCond.emplace_back(L"DEFENSE BUFF-");

		if (pEntity->InCond(TF_COND_RADIUSHEAL) ||
			pEntity->InCond(TF_COND_HEALTH_BUFF) ||
			pEntity->InCond(TF_COND_RADIUSHEAL_ON_DAMAGE) ||
			pEntity->InCond(TF_COND_HALLOWEEN_HELL_HEAL))
			szCond.emplace_back(L"HEAL");

		if (pEntity->InCond(TF_COND_MEGAHEAL) ||
			pEntity->InCond(TF_COND_HALLOWEEN_QUICK_HEAL))
			szCond.emplace_back(L"HEAL+");

		if (pEntity->InCond(TF_COND_REGENONDAMAGEBUFF))
			szCond.emplace_back(L"HEALTH ON DAMAGE");

		if (pEntity->InCond(TF_COND_HEALTH_OVERHEALED))
			szCond.emplace_back(L"OVERHEAL");



		if (pEntity->InCond(TF_COND_MEDIGUN_UBER_BULLET_RESIST) ||
			pEntity->InCond(TF_COND_MEDIGUN_SMALL_BULLET_RESIST) ||
			pEntity->InCond(TF_COND_BULLET_IMMUNE))
			szCond.emplace_back(L"BULLET");

		if (pEntity->InCond(TF_COND_MEDIGUN_UBER_BLAST_RESIST) ||
			pEntity->InCond(TF_COND_MEDIGUN_SMALL_BLAST_RESIST) ||
			pEntity->InCond(TF_COND_BLAST_IMMUNE))
			szCond.emplace_back(L"BLAST");

		if (pEntity->InCond(TF_COND_MEDIGUN_UBER_FIRE_RESIST) ||
			pEntity->InCond(TF_COND_MEDIGUN_SMALL_FIRE_RESIST) ||
			pEntity->InCond(TF_COND_FIRE_IMMUNE))
			szCond.emplace_back(L"FIRE");



		if (pEntity->m_bFeignDeathReady())
			szCond.emplace_back(L"DR");

		if (pEntity->InCond(TF_COND_FEIGN_DEATH))
			szCond.emplace_back(L"FEIGN");

		if (pEntity->InCond(TF_COND_STEALTHED))
			szCond.emplace_back(L"CLOAK");

		if (pEntity->InCond(TF_COND_STEALTHED_USER_BUFF))
			szCond.emplace_back(L"STEALTH");

		if (pEntity->InCond(TF_COND_STEALTHED_USER_BUFF_FADING))
			szCond.emplace_back(L"STEALTH+");

		if (pEntity->InCond(TF_COND_DISGUISED))
			szCond.emplace_back(L"DISGUISE");

		if (pEntity->InCond(TF_COND_DISGUISING))
			szCond.emplace_back(L"DISGUISING");

		if (pEntity->InCond(TF_COND_DISGUISE_WEARINGOFF))
			szCond.emplace_back(L"UNDISGUISING");

		if (pEntity->InCond(TF_COND_STEALTHED_BLINK))
			szCond.emplace_back(L"BLINK");



		if (pEntity->InCond(TF_COND_SPEED_BOOST) ||
			pEntity->InCond(TF_COND_HALLOWEEN_SPEED_BOOST))
			szCond.emplace_back(L"SPEED BOOST");

		if (pEntity->InCond(TF_COND_SODAPOPPER_HYPE))
			szCond.emplace_back(L"HYPE");

		if (pEntity->InCond(TF_COND_SNIPERCHARGE_RAGE_BUFF))
			szCond.emplace_back(L"FOCUS");

		if (pEntity->InCond(TF_COND_AIMING))
			szCond.emplace_back(L"SLOWED");

		if (pEntity->InCond(TF_COND_ZOOMED))
			szCond.emplace_back(L"ZOOM");

		if (pEntity->InCond(TF_COND_SHIELD_CHARGE))
			szCond.emplace_back(L"CHARGING");

		if (pEntity->InCond(TF_COND_DEMO_BUFF))
			szCond.emplace_back(L"EYELANDER");

		if (pEntity->InCond(TF_COND_PHASE))
			szCond.emplace_back(L"BONK");

		if (pEntity->InCond(TF_COND_AFTERBURN_IMMUNE))
			szCond.emplace_back(L"NO AFTERBURN");

		if (pEntity->InCond(TF_COND_BLASTJUMPING))
			szCond.emplace_back(L"BLASTJUMP");

		if (pEntity->InCond(TF_COND_ROCKETPACK))
			szCond.emplace_back(L"ROCKETPACK");

		if (pEntity->InCond(TF_COND_PARACHUTE_ACTIVE) ||
			pEntity->InCond(TF_COND_PARACHUTE_DEPLOYED))
			szCond.emplace_back(L"PARACHUTE");

		if (pEntity->InCond(TF_COND_OBSCURED_SMOKE))
			szCond.emplace_back(L"DODGE");



		if (pEntity->InCond(TF_COND_STUNNED) ||
			pEntity->InCond(TF_COND_MVM_BOT_STUN_RADIOWAVE))
			szCond.emplace_back(L"STUN");

		if (pEntity->InCond(TF_COND_MARKEDFORDEATH) ||
			pEntity->InCond(TF_COND_MARKEDFORDEATH_SILENT) ||
			pEntity->InCond(TF_COND_PASSTIME_PENALTY_DEBUFF))
			szCond.emplace_back(L"MARKED FOR DEATH");

		if (pEntity->InCond(TF_COND_URINE))
			szCond.emplace_back(L"JARATE");

		if (pEntity->InCond(TF_COND_MAD_MILK))
			szCond.emplace_back(L"MILK");

		if (pEntity->InCond(TF_COND_GAS))
			szCond.emplace_back(L"GAS");

		if (pEntity->InCond(TF_COND_BURNING) ||
			pEntity->InCond(TF_COND_BURNING_PYRO))
			szCond.emplace_back(L"BURN");

		if (pEntity->InCond(TF_COND_BLEEDING) ||
			pEntity->InCond(TF_COND_GRAPPLINGHOOK_BLEEDING))
			szCond.emplace_back(L"BLEED");

		if (pEntity->InCond(TF_COND_KNOCKED_INTO_AIR))
			szCond.emplace_back(L"AIRBLAST");

		if (pEntity->InCond(TF_COND_AIR_CURRENT))
			szCond.emplace_back(L"AIR");

		if (pEntity->InCond(TF_COND_LOST_FOOTING))
			szCond.emplace_back(L"SLIDE");

		if (pEntity->InCond(TF_COND_HEALING_DEBUFF))
			szCond.emplace_back(L"HEAL DEBUFF");

		if (pEntity->InCond(TF_COND_CANNOT_SWITCH_FROM_MELEE) ||
			pEntity->InCond(TF_COND_MELEE_ONLY))
			szCond.emplace_back(L"ONLY MELEE");



		if (pEntity->InCond(TF_COND_HALLOWEEN_GIANT))
			szCond.emplace_back(L"GIANT");

		if (pEntity->InCond(TF_COND_HALLOWEEN_TINY))
			szCond.emplace_back(L"TINY");

		if (pEntity->InCond(TF_COND_HALLOWEEN_BOMB_HEAD))
			szCond.emplace_back(L"BOMB");

		if (pEntity->InCond(TF_COND_BALLOON_HEAD))
			szCond.emplace_back(L"BALLOON");

		if (pEntity->InCond(TF_COND_HALLOWEEN_GHOST_MODE))
			szCond.emplace_back(L"GHOST");

		if (pEntity->InCond(TF_COND_HALLOWEEN_KART))
			szCond.emplace_back(L"KART");

		if (pEntity->InCond(TF_COND_HALLOWEEN_KART_DASH))
			szCond.emplace_back(L"DASH");

		if (pEntity->InCond(TF_COND_SWIMMING_CURSE) ||
			pEntity->InCond(TF_COND_SWIMMING_NO_EFFECTS))
			szCond.emplace_back(L"SWIM");

		if (pEntity->InCond(TF_COND_HALLOWEEN_KART_CAGE))
			szCond.emplace_back(L"CAGE");



	//	if (pEntity->InCond(TF_COND_HASRUNE))
	//		szCond.emplace_back(L"RUNE");

		if (pEntity->InCond(TF_COND_POWERUPMODE_DOMINANT))
			szCond.emplace_back(L"DOMINANT");

		if (pEntity->InCond(TF_COND_RUNE_STRENGTH))
			szCond.emplace_back(L"STRENGTH");

		if (pEntity->InCond(TF_COND_RUNE_HASTE))
			szCond.emplace_back(L"HASTE");

		if (pEntity->InCond(TF_COND_RUNE_REGEN))
			szCond.emplace_back(L"REGEN");

		if (pEntity->InCond(TF_COND_RUNE_RESIST))
			szCond.emplace_back(L"RESIST");

		if (pEntity->InCond(TF_COND_RUNE_VAMPIRE))
			szCond.emplace_back(L"VAMPIRE");

		if (pEntity->InCond(TF_COND_RUNE_REFLECT))
			szCond.emplace_back(L"REFLECT");

		if (pEntity->InCond(TF_COND_RUNE_PRECISION))
			szCond.emplace_back(L"PRECISION");

		if (pEntity->InCond(TF_COND_RUNE_AGILITY))
			szCond.emplace_back(L"AGILITY");

		if (pEntity->InCond(TF_COND_RUNE_KNOCKOUT))
			szCond.emplace_back(L"KNOCKOUT");

		if (pEntity->InCond(TF_COND_RUNE_IMBALANCE))
			szCond.emplace_back(L"IMBALANCE");

		if (pEntity->InCond(TF_COND_RUNE_KING))
			szCond.emplace_back(L"KING");

		if (pEntity->InCond(TF_COND_RUNE_PLAGUE))
			szCond.emplace_back(L"PLAGUE");

		if (pEntity->InCond(TF_COND_RUNE_SUPERNOVA))
			szCond.emplace_back(L"SUPERNOVA");

		if (pEntity->InCond(TF_COND_PLAGUE))
			szCond.emplace_back(L"PLAGUED");

		if (pEntity->InCond(TF_COND_KING_BUFFED))
			szCond.emplace_back(L"KING BUFF");



		if (pEntity->InCond(TF_COND_HALLOWEEN_IN_HELL))
			szCond.emplace_back(L"HELL");

		if (pEntity->InCond(TF_COND_PURGATORY))
			szCond.emplace_back(L"PURGATORY");

		if (pEntity->InCond(TF_COND_PASSTIME_INTERCEPTION))
			szCond.emplace_back(L"INTERCEPTION");

		if (pEntity->InCond(TF_COND_TEAM_GLOWS))
			szCond.emplace_back(L"TEAM GLOWS");



		if (pEntity->InCond(TF_COND_PREVENT_DEATH))
			szCond.emplace_back(L"PREVENT DEATH");

		if (pEntity->InCond(TF_COND_GRAPPLINGHOOK))
			szCond.emplace_back(L"GRAPPLE");

		if (pEntity->InCond(TF_COND_GRAPPLINGHOOK_SAFEFALL))
			szCond.emplace_back(L"SAFEFALL");

		if (pEntity->InCond(TF_COND_GRAPPLINGHOOK_LATCHED))
			szCond.emplace_back(L"LATCHED");

		if (pEntity->InCond(TF_COND_GRAPPLED_TO_PLAYER))
			szCond.emplace_back(L"TO PLAYER");

		if (pEntity->InCond(TF_COND_GRAPPLED_BY_PLAYER))
			szCond.emplace_back(L"BY PLAYER");

		if (pEntity->InCond(TF_COND_TELEPORTED))
			szCond.emplace_back(L"TELEPORT");

		if (pEntity->InCond(TF_COND_SELECTED_TO_TELEPORT))
			szCond.emplace_back(L"TELEPORTING");

		if (pEntity->InCond(TF_COND_MEDIGUN_DEBUFF))
			szCond.emplace_back(L"MEDIGUN DEBUFF");

		if (pEntity->InCond(TF_COND_SAPPED))
			szCond.emplace_back(L"SAPPED");

		if (pEntity->InCond(TF_COND_DISGUISED_AS_DISPENSER))
			szCond.emplace_back(L"DISPENSER");

		if (pEntity->InCond(TF_COND_TAUNTING))
			szCond.emplace_back(L"TAUNT");

		if (pEntity->InCond(TF_COND_HALLOWEEN_THRILLER))
			szCond.emplace_back(L"THRILLER");

		if (pEntity->InCond(TF_COND_FREEZE_INPUT))
			szCond.emplace_back(L"FREEZE INPUT");

		if (pEntity->InCond(TF_COND_REPROGRAMMED))
			szCond.emplace_back(L"REPROGRAMMED");

	//	if (pEntity->InCond(TF_COND_COMPETITIVE_WINNER))
	//		szCond.emplace_back(L"WIN");

	//	if (pEntity->InCond(TF_COND_COMPETITIVE_LOSER))
	//		szCond.emplace_back(L"LOSS");
	}
	
	return szCond;
}