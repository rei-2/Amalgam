#include "PlayerConditions.h"

std::vector<std::string> CPlayerConditions::Get(CTFPlayer* pEntity)
{
	std::vector<std::string> vConditions = {};

	{
		if (pEntity->InCond(TF_COND_CRITBOOSTED) ||
			pEntity->InCond(TF_COND_CRITBOOSTED_PUMPKIN) ||
			pEntity->InCond(TF_COND_CRITBOOSTED_USER_BUFF) ||
			pEntity->InCond(TF_COND_CRITBOOSTED_DEMO_CHARGE) ||
			pEntity->InCond(TF_COND_CRITBOOSTED_FIRST_BLOOD) ||
			pEntity->InCond(TF_COND_CRITBOOSTED_BONUS_TIME) ||
			pEntity->InCond(TF_COND_CRITBOOSTED_CTF_CAPTURE) ||
			pEntity->InCond(TF_COND_CRITBOOSTED_ON_KILL) ||
			pEntity->InCond(TF_COND_CRITBOOSTED_RAGE_BUFF) ||
			pEntity->InCond(TF_COND_CRITBOOSTED_CARD_EFFECT) ||
			pEntity->InCond(TF_COND_CRITBOOSTED_RUNE_TEMP))
			vConditions.emplace_back("CRITS");

		if (pEntity->InCond(TF_COND_ENERGY_BUFF) ||
			pEntity->InCond(TF_COND_NOHEALINGDAMAGEBUFF))
			vConditions.emplace_back("MINI CRITS");

		if (pEntity->InCond(TF_COND_MINICRITBOOSTED_ON_KILL))
			vConditions.emplace_back("MINI CRITS ON KILL");

		if (pEntity->InCond(TF_COND_OFFENSEBUFF))
			vConditions.emplace_back("OFFENSE BUFF");

		if (pEntity->InCond(TF_COND_TMPDAMAGEBONUS))
			vConditions.emplace_back("DAMAGE BONUS");

		if (pEntity->InCond(TF_COND_INVULNERABLE) ||
			pEntity->InCond(TF_COND_INVULNERABLE_HIDE_UNLESS_DAMAGED) ||
			pEntity->InCond(TF_COND_INVULNERABLE_USER_BUFF) ||
			pEntity->InCond(TF_COND_INVULNERABLE_CARD_EFFECT))
		{
			if (pEntity->InCond(TF_COND_INVULNERABLE_WEARINGOFF))
				vConditions.emplace_back("INVULNERABLE-"); // this cond may sometimes be residual
			else
				vConditions.emplace_back("INVULNERABLE");
		}

		if (pEntity->InCond(TF_COND_DEFENSEBUFF))
			vConditions.emplace_back("DEFENSE BUFF");

		if (pEntity->InCond(TF_COND_DEFENSEBUFF_HIGH))
			vConditions.emplace_back("DEFENSE BUFF+");

		if (pEntity->InCond(TF_COND_DEFENSEBUFF_NO_CRIT_BLOCK))
			vConditions.emplace_back("DEFENSE BUFF-");

		if (pEntity->InCond(TF_COND_MEGAHEAL) ||
			pEntity->InCond(TF_COND_HALLOWEEN_QUICK_HEAL))
			vConditions.emplace_back("HEAL+");
		else if (pEntity->InCond(TF_COND_RADIUSHEAL) ||
			pEntity->InCond(TF_COND_HEALTH_BUFF) ||
			pEntity->InCond(TF_COND_RADIUSHEAL_ON_DAMAGE) ||
			pEntity->InCond(TF_COND_HALLOWEEN_HELL_HEAL))
			vConditions.emplace_back("HEAL");

		if (pEntity->InCond(TF_COND_REGENONDAMAGEBUFF))
			vConditions.emplace_back("HEALTH ON DAMAGE");

		if (pEntity->InCond(TF_COND_HEALTH_OVERHEALED))
			vConditions.emplace_back("OVERHEAL");



		if (pEntity->InCond(TF_COND_MEDIGUN_UBER_BULLET_RESIST) ||
			pEntity->InCond(TF_COND_MEDIGUN_SMALL_BULLET_RESIST) ||
			pEntity->InCond(TF_COND_BULLET_IMMUNE))
			vConditions.emplace_back("BULLET");

		if (pEntity->InCond(TF_COND_MEDIGUN_UBER_BLAST_RESIST) ||
			pEntity->InCond(TF_COND_MEDIGUN_SMALL_BLAST_RESIST) ||
			pEntity->InCond(TF_COND_BLAST_IMMUNE))
			vConditions.emplace_back("BLAST");

		if (pEntity->InCond(TF_COND_MEDIGUN_UBER_FIRE_RESIST) ||
			pEntity->InCond(TF_COND_MEDIGUN_SMALL_FIRE_RESIST) ||
			pEntity->InCond(TF_COND_FIRE_IMMUNE))
			vConditions.emplace_back("FIRE");



		if (pEntity->m_bFeignDeathReady())
			vConditions.emplace_back("DR");

		if (pEntity->InCond(TF_COND_FEIGN_DEATH))
			vConditions.emplace_back("FEIGN");

		if (pEntity->InCond(TF_COND_STEALTHED))
			vConditions.emplace_back("CLOAK");

		if (pEntity->InCond(TF_COND_STEALTHED_USER_BUFF))
			vConditions.emplace_back("STEALTH");

		if (pEntity->InCond(TF_COND_STEALTHED_USER_BUFF_FADING))
			vConditions.emplace_back("STEALTH+");

		if (pEntity->InCond(TF_COND_DISGUISED))
			vConditions.emplace_back("DISGUISE");

		if (pEntity->InCond(TF_COND_DISGUISING))
			vConditions.emplace_back("DISGUISING");

		if (pEntity->InCond(TF_COND_DISGUISE_WEARINGOFF))
			vConditions.emplace_back("UNDISGUISING");

		if (pEntity->InCond(TF_COND_STEALTHED_BLINK))
			vConditions.emplace_back("BLINK");



		if (pEntity->InCond(TF_COND_SPEED_BOOST) ||
			pEntity->InCond(TF_COND_HALLOWEEN_SPEED_BOOST))
			vConditions.emplace_back("SPEED BOOST");

		if (pEntity->InCond(TF_COND_SODAPOPPER_HYPE))
			vConditions.emplace_back("HYPE");

		if (pEntity->InCond(TF_COND_SNIPERCHARGE_RAGE_BUFF))
			vConditions.emplace_back("FOCUS");

		if (pEntity->InCond(TF_COND_AIMING))
			vConditions.emplace_back("SLOWED");

		if (pEntity->InCond(TF_COND_ZOOMED))
			vConditions.emplace_back("ZOOM");

		if (pEntity->InCond(TF_COND_SHIELD_CHARGE))
			vConditions.emplace_back("CHARGING");

		if (pEntity->InCond(TF_COND_DEMO_BUFF))
			vConditions.emplace_back("EYELANDER");

		if (pEntity->InCond(TF_COND_PHASE))
			vConditions.emplace_back("BONK");

		if (pEntity->InCond(TF_COND_AFTERBURN_IMMUNE))
			vConditions.emplace_back("NO AFTERBURN");

		if (pEntity->InCond(TF_COND_BLASTJUMPING))
			vConditions.emplace_back("BLASTJUMP");

		if (pEntity->InCond(TF_COND_ROCKETPACK))
			vConditions.emplace_back("ROCKETPACK");

		if (pEntity->InCond(TF_COND_PARACHUTE_ACTIVE) ||
			pEntity->InCond(TF_COND_PARACHUTE_DEPLOYED))
			vConditions.emplace_back("PARACHUTE");

		if (pEntity->InCond(TF_COND_OBSCURED_SMOKE))
			vConditions.emplace_back("DODGE");



		if (pEntity->InCond(TF_COND_STUNNED) && !(pEntity->m_iStunFlags() & (TF_STUN_CONTROLS | TF_STUN_LOSER_STATE)))
			vConditions.emplace_back("SLOWED");
		else if (pEntity->InCond(TF_COND_STUNNED) ||
			pEntity->InCond(TF_COND_MVM_BOT_STUN_RADIOWAVE))
			vConditions.emplace_back("STUN");

		if (pEntity->InCond(TF_COND_MARKEDFORDEATH) ||
			pEntity->InCond(TF_COND_MARKEDFORDEATH_SILENT) ||
			pEntity->InCond(TF_COND_PASSTIME_PENALTY_DEBUFF))
			vConditions.emplace_back("MARKED FOR DEATH");

		if (pEntity->InCond(TF_COND_URINE))
			vConditions.emplace_back("JARATE");

		if (pEntity->InCond(TF_COND_MAD_MILK))
			vConditions.emplace_back("MILK");

		if (pEntity->InCond(TF_COND_GAS))
			vConditions.emplace_back("GAS");

		if (pEntity->InCond(TF_COND_BURNING) ||
			pEntity->InCond(TF_COND_BURNING_PYRO))
			vConditions.emplace_back("BURN");

		if (pEntity->InCond(TF_COND_BLEEDING) ||
			pEntity->InCond(TF_COND_GRAPPLINGHOOK_BLEEDING))
			vConditions.emplace_back("BLEED");

		if (pEntity->InCond(TF_COND_KNOCKED_INTO_AIR))
			vConditions.emplace_back("AIRBLAST");

		if (pEntity->InCond(TF_COND_AIR_CURRENT))
			vConditions.emplace_back("AIR");

		if (pEntity->InCond(TF_COND_LOST_FOOTING))
			vConditions.emplace_back("SLIDE");

		if (pEntity->InCond(TF_COND_HEALING_DEBUFF))
			vConditions.emplace_back("HEAL DEBUFF");

		if (pEntity->InCond(TF_COND_CANNOT_SWITCH_FROM_MELEE) ||
			pEntity->InCond(TF_COND_MELEE_ONLY))
			vConditions.emplace_back("ONLY MELEE");



		if (pEntity->InCond(TF_COND_HALLOWEEN_GIANT))
			vConditions.emplace_back("GIANT");

		if (pEntity->InCond(TF_COND_HALLOWEEN_TINY))
			vConditions.emplace_back("TINY");

		if (pEntity->InCond(TF_COND_HALLOWEEN_BOMB_HEAD))
			vConditions.emplace_back("BOMB");

		if (pEntity->InCond(TF_COND_BALLOON_HEAD))
			vConditions.emplace_back("BALLOON");

		if (pEntity->InCond(TF_COND_HALLOWEEN_GHOST_MODE))
			vConditions.emplace_back("GHOST");

		if (pEntity->InCond(TF_COND_HALLOWEEN_KART))
			vConditions.emplace_back("KART");

		if (pEntity->InCond(TF_COND_HALLOWEEN_KART_DASH))
			vConditions.emplace_back("DASH");

		if (pEntity->InCond(TF_COND_SWIMMING_CURSE) ||
			pEntity->InCond(TF_COND_SWIMMING_NO_EFFECTS))
			vConditions.emplace_back("SWIM");

		if (pEntity->InCond(TF_COND_HALLOWEEN_KART_CAGE))
			vConditions.emplace_back("CAGE");



	//	if (pEntity->InCond(TF_COND_HASRUNE))
	//		vConditions.emplace_back("RUNE");

		if (pEntity->InCond(TF_COND_POWERUPMODE_DOMINANT))
			vConditions.emplace_back("DOMINANT");

		if (pEntity->InCond(TF_COND_RUNE_STRENGTH))
			vConditions.emplace_back("STRENGTH");

		if (pEntity->InCond(TF_COND_RUNE_HASTE))
			vConditions.emplace_back("HASTE");

		if (pEntity->InCond(TF_COND_RUNE_REGEN))
			vConditions.emplace_back("REGEN");

		if (pEntity->InCond(TF_COND_RUNE_RESIST))
			vConditions.emplace_back("RESIST");

		if (pEntity->InCond(TF_COND_RUNE_VAMPIRE))
			vConditions.emplace_back("VAMPIRE");

		if (pEntity->InCond(TF_COND_RUNE_REFLECT))
			vConditions.emplace_back("REFLECT");

		if (pEntity->InCond(TF_COND_RUNE_PRECISION))
			vConditions.emplace_back("PRECISION");

		if (pEntity->InCond(TF_COND_RUNE_AGILITY))
			vConditions.emplace_back("AGILITY");

		if (pEntity->InCond(TF_COND_RUNE_KNOCKOUT))
			vConditions.emplace_back("KNOCKOUT");

		if (pEntity->InCond(TF_COND_RUNE_IMBALANCE))
			vConditions.emplace_back("IMBALANCE");

		if (pEntity->InCond(TF_COND_RUNE_KING))
			vConditions.emplace_back("KING");

		if (pEntity->InCond(TF_COND_RUNE_PLAGUE))
			vConditions.emplace_back("PLAGUE");

		if (pEntity->InCond(TF_COND_RUNE_SUPERNOVA))
			vConditions.emplace_back("SUPERNOVA");

		if (pEntity->InCond(TF_COND_PLAGUE))
			vConditions.emplace_back("PLAGUED");

		if (pEntity->InCond(TF_COND_KING_BUFFED))
			vConditions.emplace_back("KING BUFF");



		if (pEntity->InCond(TF_COND_HALLOWEEN_IN_HELL))
			vConditions.emplace_back("HELL");

		if (pEntity->InCond(TF_COND_PURGATORY))
			vConditions.emplace_back("PURGATORY");

		if (pEntity->InCond(TF_COND_PASSTIME_INTERCEPTION))
			vConditions.emplace_back("INTERCEPTION");

		if (pEntity->InCond(TF_COND_TEAM_GLOWS))
			vConditions.emplace_back("TEAM GLOWS");



		if (pEntity->InCond(TF_COND_PREVENT_DEATH))
			vConditions.emplace_back("PREVENT DEATH");

		if (pEntity->InCond(TF_COND_GRAPPLINGHOOK))
			vConditions.emplace_back("GRAPPLE");

		if (pEntity->InCond(TF_COND_GRAPPLINGHOOK_SAFEFALL))
			vConditions.emplace_back("SAFEFALL");

		if (pEntity->InCond(TF_COND_GRAPPLINGHOOK_LATCHED))
			vConditions.emplace_back("LATCHED");

		if (pEntity->InCond(TF_COND_GRAPPLED_TO_PLAYER))
			vConditions.emplace_back("TO PLAYER");

		if (pEntity->InCond(TF_COND_GRAPPLED_BY_PLAYER))
			vConditions.emplace_back("BY PLAYER");

		if (pEntity->InCond(TF_COND_TELEPORTED))
			vConditions.emplace_back("TELEPORT");

		if (pEntity->InCond(TF_COND_SELECTED_TO_TELEPORT))
			vConditions.emplace_back("TELEPORTING");

		if (pEntity->InCond(TF_COND_MEDIGUN_DEBUFF))
			vConditions.emplace_back("MEDIGUN DEBUFF");

		if (pEntity->InCond(TF_COND_SAPPED))
			vConditions.emplace_back("SAPPED");

		if (pEntity->InCond(TF_COND_DISGUISED_AS_DISPENSER))
			vConditions.emplace_back("DISPENSER");

		if (pEntity->InCond(TF_COND_TAUNTING))
			vConditions.emplace_back("TAUNT");

		if (pEntity->InCond(TF_COND_HALLOWEEN_THRILLER))
			vConditions.emplace_back("THRILLER");

		if (pEntity->InCond(TF_COND_FREEZE_INPUT))
			vConditions.emplace_back("FREEZE INPUT");

		if (pEntity->InCond(TF_COND_REPROGRAMMED))
			vConditions.emplace_back("REPROGRAMMED");

		//if (pEntity->InCond(TF_COND_COMPETITIVE_WINNER))
		//	vConditions.emplace_back("WIN");

		//if (pEntity->InCond(TF_COND_COMPETITIVE_LOSER))
		//	vConditions.emplace_back("LOSS");
	}
	
	return vConditions;
}

void CPlayerConditions::Draw(CTFPlayer* pLocal)
{
	if (!(Vars::Menu::Indicators.Value & Vars::Menu::IndicatorsEnum::Conditions))
		return;

	auto pTarget = pLocal;
	switch (pLocal->m_iObserverMode())
	{
	case OBS_MODE_FIRSTPERSON:
	case OBS_MODE_THIRDPERSON:
		pTarget = pLocal->m_hObserverTarget().Get()->As<CTFPlayer>();
	}
	if (!pTarget || !pTarget->IsPlayer() || !pTarget->IsAlive())
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

	std::vector<std::string> vConditions = Get(pTarget);

	int iOffset = 0;
	for (const std::string& sCondition : vConditions)
	{
		H::Draw.StringOutlined(fFont, x, y + iOffset, Vars::Menu::Theme::Active.Value, Vars::Menu::Theme::Background.Value, align, sCondition.c_str());
		iOffset += nTall;
	}
}