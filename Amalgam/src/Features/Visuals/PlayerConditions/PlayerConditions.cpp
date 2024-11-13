#include "PlayerConditions.h"

std::vector<std::wstring> CPlayerConditions::Get(CTFPlayer* pEntity)
{
	std::vector<std::wstring> vConditions = {};

	{
		if (pEntity->InCond(TF_COND_CRITBOOSTED))
			vConditions.emplace_back(L"KRITS");
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
			vConditions.emplace_back(L"CRITS");

		if (pEntity->InCond(TF_COND_ENERGY_BUFF) ||
			pEntity->InCond(TF_COND_NOHEALINGDAMAGEBUFF))
			vConditions.emplace_back(L"MINI CRITS");

		if (pEntity->InCond(TF_COND_MINICRITBOOSTED_ON_KILL))
			vConditions.emplace_back(L"MINI CRITS ON KILL");

		if (pEntity->InCond(TF_COND_OFFENSEBUFF))
			vConditions.emplace_back(L"OFFENSE BUFF");

		if (pEntity->InCond(TF_COND_TMPDAMAGEBONUS))
			vConditions.emplace_back(L"DAMAGE BONUS");

		if (pEntity->InCond(TF_COND_INVULNERABLE) ||
			pEntity->InCond(TF_COND_INVULNERABLE_HIDE_UNLESS_DAMAGED) ||
			pEntity->InCond(TF_COND_INVULNERABLE_USER_BUFF) ||
			pEntity->InCond(TF_COND_INVULNERABLE_CARD_EFFECT))
			vConditions.emplace_back(L"INVULNERABLE");

		if (pEntity->InCond(TF_COND_INVULNERABLE_WEARINGOFF))
			vConditions.emplace_back(L"INVULNERABLE-");

		if (pEntity->InCond(TF_COND_DEFENSEBUFF))
			vConditions.emplace_back(L"DEFENSE BUFF");

		if (pEntity->InCond(TF_COND_DEFENSEBUFF_HIGH))
			vConditions.emplace_back(L"DEFENSE BUFF+");

		if (pEntity->InCond(TF_COND_DEFENSEBUFF_NO_CRIT_BLOCK))
			vConditions.emplace_back(L"DEFENSE BUFF-");

		if (pEntity->InCond(TF_COND_RADIUSHEAL) ||
			pEntity->InCond(TF_COND_HEALTH_BUFF) ||
			pEntity->InCond(TF_COND_RADIUSHEAL_ON_DAMAGE) ||
			pEntity->InCond(TF_COND_HALLOWEEN_HELL_HEAL))
			vConditions.emplace_back(L"HEAL");

		if (pEntity->InCond(TF_COND_MEGAHEAL) ||
			pEntity->InCond(TF_COND_HALLOWEEN_QUICK_HEAL))
			vConditions.emplace_back(L"HEAL+");

		if (pEntity->InCond(TF_COND_REGENONDAMAGEBUFF))
			vConditions.emplace_back(L"HEALTH ON DAMAGE");

		if (pEntity->InCond(TF_COND_HEALTH_OVERHEALED))
			vConditions.emplace_back(L"OVERHEAL");



		if (pEntity->InCond(TF_COND_MEDIGUN_UBER_BULLET_RESIST) ||
			pEntity->InCond(TF_COND_MEDIGUN_SMALL_BULLET_RESIST) ||
			pEntity->InCond(TF_COND_BULLET_IMMUNE))
			vConditions.emplace_back(L"BULLET");

		if (pEntity->InCond(TF_COND_MEDIGUN_UBER_BLAST_RESIST) ||
			pEntity->InCond(TF_COND_MEDIGUN_SMALL_BLAST_RESIST) ||
			pEntity->InCond(TF_COND_BLAST_IMMUNE))
			vConditions.emplace_back(L"BLAST");

		if (pEntity->InCond(TF_COND_MEDIGUN_UBER_FIRE_RESIST) ||
			pEntity->InCond(TF_COND_MEDIGUN_SMALL_FIRE_RESIST) ||
			pEntity->InCond(TF_COND_FIRE_IMMUNE))
			vConditions.emplace_back(L"FIRE");



		if (pEntity->m_bFeignDeathReady())
			vConditions.emplace_back(L"DR");

		if (pEntity->InCond(TF_COND_FEIGN_DEATH))
			vConditions.emplace_back(L"FEIGN");

		if (pEntity->InCond(TF_COND_STEALTHED))
			vConditions.emplace_back(L"CLOAK");

		if (pEntity->InCond(TF_COND_STEALTHED_USER_BUFF))
			vConditions.emplace_back(L"STEALTH");

		if (pEntity->InCond(TF_COND_STEALTHED_USER_BUFF_FADING))
			vConditions.emplace_back(L"STEALTH+");

		if (pEntity->InCond(TF_COND_DISGUISED))
			vConditions.emplace_back(L"DISGUISE");

		if (pEntity->InCond(TF_COND_DISGUISING))
			vConditions.emplace_back(L"DISGUISING");

		if (pEntity->InCond(TF_COND_DISGUISE_WEARINGOFF))
			vConditions.emplace_back(L"UNDISGUISING");

		if (pEntity->InCond(TF_COND_STEALTHED_BLINK))
			vConditions.emplace_back(L"BLINK");



		if (pEntity->InCond(TF_COND_SPEED_BOOST) ||
			pEntity->InCond(TF_COND_HALLOWEEN_SPEED_BOOST))
			vConditions.emplace_back(L"SPEED BOOST");

		if (pEntity->InCond(TF_COND_SODAPOPPER_HYPE))
			vConditions.emplace_back(L"HYPE");

		if (pEntity->InCond(TF_COND_SNIPERCHARGE_RAGE_BUFF))
			vConditions.emplace_back(L"FOCUS");

		if (pEntity->InCond(TF_COND_AIMING))
			vConditions.emplace_back(L"SLOWED");

		if (pEntity->InCond(TF_COND_ZOOMED))
			vConditions.emplace_back(L"ZOOM");

		if (pEntity->InCond(TF_COND_SHIELD_CHARGE))
			vConditions.emplace_back(L"CHARGING");

		if (pEntity->InCond(TF_COND_DEMO_BUFF))
			vConditions.emplace_back(L"EYELANDER");

		if (pEntity->InCond(TF_COND_PHASE))
			vConditions.emplace_back(L"BONK");

		if (pEntity->InCond(TF_COND_AFTERBURN_IMMUNE))
			vConditions.emplace_back(L"NO AFTERBURN");

		if (pEntity->InCond(TF_COND_BLASTJUMPING))
			vConditions.emplace_back(L"BLASTJUMP");

		if (pEntity->InCond(TF_COND_ROCKETPACK))
			vConditions.emplace_back(L"ROCKETPACK");

		if (pEntity->InCond(TF_COND_PARACHUTE_ACTIVE) ||
			pEntity->InCond(TF_COND_PARACHUTE_DEPLOYED))
			vConditions.emplace_back(L"PARACHUTE");

		if (pEntity->InCond(TF_COND_OBSCURED_SMOKE))
			vConditions.emplace_back(L"DODGE");



		if (pEntity->InCond(TF_COND_STUNNED) ||
			pEntity->InCond(TF_COND_MVM_BOT_STUN_RADIOWAVE))
			vConditions.emplace_back(L"STUN");

		if (pEntity->InCond(TF_COND_MARKEDFORDEATH) ||
			pEntity->InCond(TF_COND_MARKEDFORDEATH_SILENT) ||
			pEntity->InCond(TF_COND_PASSTIME_PENALTY_DEBUFF))
			vConditions.emplace_back(L"MARKED FOR DEATH");

		if (pEntity->InCond(TF_COND_URINE))
			vConditions.emplace_back(L"JARATE");

		if (pEntity->InCond(TF_COND_MAD_MILK))
			vConditions.emplace_back(L"MILK");

		if (pEntity->InCond(TF_COND_GAS))
			vConditions.emplace_back(L"GAS");

		if (pEntity->InCond(TF_COND_BURNING) ||
			pEntity->InCond(TF_COND_BURNING_PYRO))
			vConditions.emplace_back(L"BURN");

		if (pEntity->InCond(TF_COND_BLEEDING) ||
			pEntity->InCond(TF_COND_GRAPPLINGHOOK_BLEEDING))
			vConditions.emplace_back(L"BLEED");

		if (pEntity->InCond(TF_COND_KNOCKED_INTO_AIR))
			vConditions.emplace_back(L"AIRBLAST");

		if (pEntity->InCond(TF_COND_AIR_CURRENT))
			vConditions.emplace_back(L"AIR");

		if (pEntity->InCond(TF_COND_LOST_FOOTING))
			vConditions.emplace_back(L"SLIDE");

		if (pEntity->InCond(TF_COND_HEALING_DEBUFF))
			vConditions.emplace_back(L"HEAL DEBUFF");

		if (pEntity->InCond(TF_COND_CANNOT_SWITCH_FROM_MELEE) ||
			pEntity->InCond(TF_COND_MELEE_ONLY))
			vConditions.emplace_back(L"ONLY MELEE");



		if (pEntity->InCond(TF_COND_HALLOWEEN_GIANT))
			vConditions.emplace_back(L"GIANT");

		if (pEntity->InCond(TF_COND_HALLOWEEN_TINY))
			vConditions.emplace_back(L"TINY");

		if (pEntity->InCond(TF_COND_HALLOWEEN_BOMB_HEAD))
			vConditions.emplace_back(L"BOMB");

		if (pEntity->InCond(TF_COND_BALLOON_HEAD))
			vConditions.emplace_back(L"BALLOON");

		if (pEntity->InCond(TF_COND_HALLOWEEN_GHOST_MODE))
			vConditions.emplace_back(L"GHOST");

		if (pEntity->InCond(TF_COND_HALLOWEEN_KART))
			vConditions.emplace_back(L"KART");

		if (pEntity->InCond(TF_COND_HALLOWEEN_KART_DASH))
			vConditions.emplace_back(L"DASH");

		if (pEntity->InCond(TF_COND_SWIMMING_CURSE) ||
			pEntity->InCond(TF_COND_SWIMMING_NO_EFFECTS))
			vConditions.emplace_back(L"SWIM");

		if (pEntity->InCond(TF_COND_HALLOWEEN_KART_CAGE))
			vConditions.emplace_back(L"CAGE");



	//	if (pEntity->InCond(TF_COND_HASRUNE))
	//		vConditions.emplace_back(L"RUNE");

		if (pEntity->InCond(TF_COND_POWERUPMODE_DOMINANT))
			vConditions.emplace_back(L"DOMINANT");

		if (pEntity->InCond(TF_COND_RUNE_STRENGTH))
			vConditions.emplace_back(L"STRENGTH");

		if (pEntity->InCond(TF_COND_RUNE_HASTE))
			vConditions.emplace_back(L"HASTE");

		if (pEntity->InCond(TF_COND_RUNE_REGEN))
			vConditions.emplace_back(L"REGEN");

		if (pEntity->InCond(TF_COND_RUNE_RESIST))
			vConditions.emplace_back(L"RESIST");

		if (pEntity->InCond(TF_COND_RUNE_VAMPIRE))
			vConditions.emplace_back(L"VAMPIRE");

		if (pEntity->InCond(TF_COND_RUNE_REFLECT))
			vConditions.emplace_back(L"REFLECT");

		if (pEntity->InCond(TF_COND_RUNE_PRECISION))
			vConditions.emplace_back(L"PRECISION");

		if (pEntity->InCond(TF_COND_RUNE_AGILITY))
			vConditions.emplace_back(L"AGILITY");

		if (pEntity->InCond(TF_COND_RUNE_KNOCKOUT))
			vConditions.emplace_back(L"KNOCKOUT");

		if (pEntity->InCond(TF_COND_RUNE_IMBALANCE))
			vConditions.emplace_back(L"IMBALANCE");

		if (pEntity->InCond(TF_COND_RUNE_KING))
			vConditions.emplace_back(L"KING");

		if (pEntity->InCond(TF_COND_RUNE_PLAGUE))
			vConditions.emplace_back(L"PLAGUE");

		if (pEntity->InCond(TF_COND_RUNE_SUPERNOVA))
			vConditions.emplace_back(L"SUPERNOVA");

		if (pEntity->InCond(TF_COND_PLAGUE))
			vConditions.emplace_back(L"PLAGUED");

		if (pEntity->InCond(TF_COND_KING_BUFFED))
			vConditions.emplace_back(L"KING BUFF");



		if (pEntity->InCond(TF_COND_HALLOWEEN_IN_HELL))
			vConditions.emplace_back(L"HELL");

		if (pEntity->InCond(TF_COND_PURGATORY))
			vConditions.emplace_back(L"PURGATORY");

		if (pEntity->InCond(TF_COND_PASSTIME_INTERCEPTION))
			vConditions.emplace_back(L"INTERCEPTION");

		if (pEntity->InCond(TF_COND_TEAM_GLOWS))
			vConditions.emplace_back(L"TEAM GLOWS");



		if (pEntity->InCond(TF_COND_PREVENT_DEATH))
			vConditions.emplace_back(L"PREVENT DEATH");

		if (pEntity->InCond(TF_COND_GRAPPLINGHOOK))
			vConditions.emplace_back(L"GRAPPLE");

		if (pEntity->InCond(TF_COND_GRAPPLINGHOOK_SAFEFALL))
			vConditions.emplace_back(L"SAFEFALL");

		if (pEntity->InCond(TF_COND_GRAPPLINGHOOK_LATCHED))
			vConditions.emplace_back(L"LATCHED");

		if (pEntity->InCond(TF_COND_GRAPPLED_TO_PLAYER))
			vConditions.emplace_back(L"TO PLAYER");

		if (pEntity->InCond(TF_COND_GRAPPLED_BY_PLAYER))
			vConditions.emplace_back(L"BY PLAYER");

		if (pEntity->InCond(TF_COND_TELEPORTED))
			vConditions.emplace_back(L"TELEPORT");

		if (pEntity->InCond(TF_COND_SELECTED_TO_TELEPORT))
			vConditions.emplace_back(L"TELEPORTING");

		if (pEntity->InCond(TF_COND_MEDIGUN_DEBUFF))
			vConditions.emplace_back(L"MEDIGUN DEBUFF");

		if (pEntity->InCond(TF_COND_SAPPED))
			vConditions.emplace_back(L"SAPPED");

		if (pEntity->InCond(TF_COND_DISGUISED_AS_DISPENSER))
			vConditions.emplace_back(L"DISPENSER");

		if (pEntity->InCond(TF_COND_TAUNTING))
			vConditions.emplace_back(L"TAUNT");

		if (pEntity->InCond(TF_COND_HALLOWEEN_THRILLER))
			vConditions.emplace_back(L"THRILLER");

		if (pEntity->InCond(TF_COND_FREEZE_INPUT))
			vConditions.emplace_back(L"FREEZE INPUT");

		if (pEntity->InCond(TF_COND_REPROGRAMMED))
			vConditions.emplace_back(L"REPROGRAMMED");

	//	if (pEntity->InCond(TF_COND_COMPETITIVE_WINNER))
	//		vConditions.emplace_back(L"WIN");

	//	if (pEntity->InCond(TF_COND_COMPETITIVE_LOSER))
	//		vConditions.emplace_back(L"LOSS");
	}
	
	return vConditions;
}

void CPlayerConditions::Draw(CTFPlayer* pLocal)
{
	if (!(Vars::Menu::Indicators.Value & Vars::Menu::IndicatorsEnum::Conditions) || !pLocal->IsAlive())
		return;

	int x = Vars::Menu::ConditionsDisplay.Value.x;
	int y = Vars::Menu::ConditionsDisplay.Value.y + 8;
	const auto& fFont = H::Fonts.GetFont(FONT_INDICATORS);
	const int nTall = fFont.m_nTall + H::Draw.Scale(1);

	EAlign align = ALIGN_TOP;
	if (x <= 100 + H::Draw.Scale(50, Scale_Round))
	{
		x -= H::Draw.Scale(42, Scale_Round);
		align = ALIGN_TOPLEFT;
	}
	else if (x >= H::Draw.m_nScreenW - 100 - H::Draw.Scale(50, Scale_Round))
	{
		x += H::Draw.Scale(42, Scale_Round);
		align = ALIGN_TOPRIGHT;
	}

	std::vector<std::wstring> vConditions = Get(pLocal);

	int iOffset = 0;
	for (const std::wstring& sCondition : vConditions)
	{
		H::Draw.StringOutlined(fFont, x, y + iOffset, Vars::Menu::Theme::Active.Value, Vars::Menu::Theme::Background.Value, align, sCondition.c_str());
		iOffset += nTall;
	}
}