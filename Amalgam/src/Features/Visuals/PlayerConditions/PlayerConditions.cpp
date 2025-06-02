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
			vConditions.emplace_back("Crits");

		if (pEntity->InCond(TF_COND_ENERGY_BUFF) ||
			pEntity->InCond(TF_COND_NOHEALINGDAMAGEBUFF))
			vConditions.emplace_back("Mini crits");

		if (pEntity->InCond(TF_COND_MINICRITBOOSTED_ON_KILL))
			vConditions.emplace_back("Mini crits on kill");

		if (pEntity->InCond(TF_COND_OFFENSEBUFF))
			vConditions.emplace_back("Offense buff");

		if (pEntity->InCond(TF_COND_TMPDAMAGEBONUS))
			vConditions.emplace_back("Damage bonus");

		if (pEntity->InCond(TF_COND_INVULNERABLE) ||
			pEntity->InCond(TF_COND_INVULNERABLE_HIDE_UNLESS_DAMAGED) ||
			pEntity->InCond(TF_COND_INVULNERABLE_USER_BUFF) ||
			pEntity->InCond(TF_COND_INVULNERABLE_CARD_EFFECT))
		{
			if (pEntity->InCond(TF_COND_INVULNERABLE_WEARINGOFF))
				vConditions.emplace_back("Invulnerable-"); // this cond may sometimes be residual
			else
				vConditions.emplace_back("Invulnerable");
		}

		if (pEntity->InCond(TF_COND_DEFENSEBUFF))
			vConditions.emplace_back("Defense buff");

		if (pEntity->InCond(TF_COND_DEFENSEBUFF_HIGH))
			vConditions.emplace_back("Defense buff+");

		if (pEntity->InCond(TF_COND_DEFENSEBUFF_NO_CRIT_BLOCK))
			vConditions.emplace_back("Defense buff-");

		if (pEntity->InCond(TF_COND_MEGAHEAL) ||
			pEntity->InCond(TF_COND_HALLOWEEN_QUICK_HEAL))
			vConditions.emplace_back("Heal+");
		else if (pEntity->InCond(TF_COND_RADIUSHEAL) ||
			pEntity->InCond(TF_COND_HEALTH_BUFF) ||
			pEntity->InCond(TF_COND_RADIUSHEAL_ON_DAMAGE) ||
			pEntity->InCond(TF_COND_HALLOWEEN_HELL_HEAL))
			vConditions.emplace_back("Heal");

		if (pEntity->InCond(TF_COND_REGENONDAMAGEBUFF))
			vConditions.emplace_back("Health on damage");

		if (pEntity->InCond(TF_COND_HEALTH_OVERHEALED))
			vConditions.emplace_back("Overheal");



		if (pEntity->InCond(TF_COND_MEDIGUN_UBER_BULLET_RESIST) ||
			pEntity->InCond(TF_COND_BULLET_IMMUNE))
			vConditions.emplace_back("Bullet+");
		else if (pEntity->InCond(TF_COND_MEDIGUN_SMALL_BULLET_RESIST))
			vConditions.emplace_back("Bullet");

		if (pEntity->InCond(TF_COND_MEDIGUN_UBER_BLAST_RESIST) ||
			pEntity->InCond(TF_COND_BLAST_IMMUNE))
			vConditions.emplace_back("Blast+");
		else if (pEntity->InCond(TF_COND_MEDIGUN_SMALL_BLAST_RESIST))
			vConditions.emplace_back("Blast");

		if (pEntity->InCond(TF_COND_MEDIGUN_UBER_FIRE_RESIST) ||
			pEntity->InCond(TF_COND_FIRE_IMMUNE))
			vConditions.emplace_back("Fire+");
		else if (pEntity->InCond(TF_COND_MEDIGUN_SMALL_FIRE_RESIST))
			vConditions.emplace_back("Fire");



		if (pEntity->m_bFeignDeathReady())
			vConditions.emplace_back("Deadringer");

		if (pEntity->InCond(TF_COND_FEIGN_DEATH))
			vConditions.emplace_back("Feign");

		if (pEntity->InCond(TF_COND_STEALTHED))
			vConditions.emplace_back("Cloak");

		if (pEntity->InCond(TF_COND_STEALTHED_USER_BUFF))
			vConditions.emplace_back("Stealth");

		if (pEntity->InCond(TF_COND_STEALTHED_USER_BUFF_FADING))
			vConditions.emplace_back("Stealth+");

		if (pEntity->InCond(TF_COND_DISGUISED))
			vConditions.emplace_back("Disguise");

		if (pEntity->InCond(TF_COND_DISGUISING))
			vConditions.emplace_back("Disguising");

		if (pEntity->InCond(TF_COND_DISGUISE_WEARINGOFF))
			vConditions.emplace_back("Undisguising");

		if (pEntity->InCond(TF_COND_STEALTHED_BLINK))
			vConditions.emplace_back("Blink");



		if (pEntity->InCond(TF_COND_SPEED_BOOST) ||
			pEntity->InCond(TF_COND_HALLOWEEN_SPEED_BOOST))
			vConditions.emplace_back("Speed boost");

		if (pEntity->InCond(TF_COND_SODAPOPPER_HYPE))
			vConditions.emplace_back("Hype");

		if (pEntity->InCond(TF_COND_SNIPERCHARGE_RAGE_BUFF))
			vConditions.emplace_back("Focus");

		if (pEntity->InCond(TF_COND_AIMING))
			vConditions.emplace_back("Slowed");

		if (pEntity->InCond(TF_COND_ZOOMED))
			vConditions.emplace_back("Zoom");

		if (pEntity->InCond(TF_COND_SHIELD_CHARGE))
			vConditions.emplace_back("Charging");

		if (pEntity->InCond(TF_COND_DEMO_BUFF))
			vConditions.emplace_back("Eyelander");

		if (pEntity->InCond(TF_COND_PHASE))
			vConditions.emplace_back("Bonk");

		if (pEntity->InCond(TF_COND_AFTERBURN_IMMUNE))
			vConditions.emplace_back("No afterburn");

		if (pEntity->InCond(TF_COND_BLASTJUMPING))
			vConditions.emplace_back("Blastjump");

		if (pEntity->InCond(TF_COND_ROCKETPACK))
			vConditions.emplace_back("Rocketpack");

		if (pEntity->InCond(TF_COND_PARACHUTE_ACTIVE) ||
			pEntity->InCond(TF_COND_PARACHUTE_DEPLOYED))
			vConditions.emplace_back("Parachute");

		if (pEntity->InCond(TF_COND_OBSCURED_SMOKE))
			vConditions.emplace_back("Dodge");



		if (pEntity->InCond(TF_COND_STUNNED) && !(pEntity->m_iStunFlags() & (TF_STUN_CONTROLS | TF_STUN_LOSER_STATE)))
			vConditions.emplace_back("Slowed");
		else if (pEntity->InCond(TF_COND_STUNNED) ||
			pEntity->InCond(TF_COND_MVM_BOT_STUN_RADIOWAVE))
			vConditions.emplace_back("Stun");

		if (pEntity->InCond(TF_COND_MARKEDFORDEATH) ||
			pEntity->InCond(TF_COND_MARKEDFORDEATH_SILENT) ||
			pEntity->InCond(TF_COND_PASSTIME_PENALTY_DEBUFF))
			vConditions.emplace_back("Marked for death");

		if (pEntity->InCond(TF_COND_URINE))
			vConditions.emplace_back("Jarate");

		if (pEntity->InCond(TF_COND_MAD_MILK))
			vConditions.emplace_back("Milk");

		if (pEntity->InCond(TF_COND_GAS))
			vConditions.emplace_back("Gas");

		if (pEntity->InCond(TF_COND_BURNING) ||
			pEntity->InCond(TF_COND_BURNING_PYRO))
			vConditions.emplace_back("Burn");

		if (pEntity->InCond(TF_COND_BLEEDING) ||
			pEntity->InCond(TF_COND_GRAPPLINGHOOK_BLEEDING))
			vConditions.emplace_back("Bleed");

		if (pEntity->InCond(TF_COND_KNOCKED_INTO_AIR))
			vConditions.emplace_back("Airblast");

		if (pEntity->InCond(TF_COND_AIR_CURRENT))
			vConditions.emplace_back("Air");

		if (pEntity->InCond(TF_COND_LOST_FOOTING))
			vConditions.emplace_back("Slide");

		if (pEntity->InCond(TF_COND_HEALING_DEBUFF))
			vConditions.emplace_back("Heal debuff");

		if (pEntity->InCond(TF_COND_CANNOT_SWITCH_FROM_MELEE) ||
			pEntity->InCond(TF_COND_MELEE_ONLY))
			vConditions.emplace_back("Only melee");



		if (pEntity->InCond(TF_COND_HALLOWEEN_GIANT))
			vConditions.emplace_back("Giant");

		if (pEntity->InCond(TF_COND_HALLOWEEN_TINY))
			vConditions.emplace_back("Tiny");

		if (pEntity->InCond(TF_COND_HALLOWEEN_BOMB_HEAD))
			vConditions.emplace_back("Bomb");

		if (pEntity->InCond(TF_COND_BALLOON_HEAD))
			vConditions.emplace_back("Balloon");

		if (pEntity->InCond(TF_COND_HALLOWEEN_GHOST_MODE))
			vConditions.emplace_back("Ghost");

		if (pEntity->InCond(TF_COND_HALLOWEEN_KART))
			vConditions.emplace_back("Kart");

		if (pEntity->InCond(TF_COND_HALLOWEEN_KART_DASH))
			vConditions.emplace_back("Dash");

		if (pEntity->InCond(TF_COND_SWIMMING_CURSE) ||
			pEntity->InCond(TF_COND_SWIMMING_NO_EFFECTS))
			vConditions.emplace_back("Swim");

		if (pEntity->InCond(TF_COND_HALLOWEEN_KART_CAGE))
			vConditions.emplace_back("Cage");



	//	if (pEntity->InCond(TF_COND_HASRUNE))
	//		vConditions.emplace_back("Rune");

		if (pEntity->InCond(TF_COND_POWERUPMODE_DOMINANT))
			vConditions.emplace_back("Dominant");

		if (pEntity->InCond(TF_COND_RUNE_STRENGTH))
			vConditions.emplace_back("Strength");

		if (pEntity->InCond(TF_COND_RUNE_HASTE))
			vConditions.emplace_back("Haste");

		if (pEntity->InCond(TF_COND_RUNE_REGEN))
			vConditions.emplace_back("Regen");

		if (pEntity->InCond(TF_COND_RUNE_RESIST))
			vConditions.emplace_back("Resist");

		if (pEntity->InCond(TF_COND_RUNE_VAMPIRE))
			vConditions.emplace_back("Vampire");

		if (pEntity->InCond(TF_COND_RUNE_REFLECT))
			vConditions.emplace_back("Reflect");

		if (pEntity->InCond(TF_COND_RUNE_PRECISION))
			vConditions.emplace_back("Precision");

		if (pEntity->InCond(TF_COND_RUNE_AGILITY))
			vConditions.emplace_back("Agility");

		if (pEntity->InCond(TF_COND_RUNE_KNOCKOUT))
			vConditions.emplace_back("Knockout");

		if (pEntity->InCond(TF_COND_RUNE_IMBALANCE))
			vConditions.emplace_back("Imbalance");

		if (pEntity->InCond(TF_COND_RUNE_KING))
			vConditions.emplace_back("King");

		if (pEntity->InCond(TF_COND_RUNE_PLAGUE))
			vConditions.emplace_back("Plague");

		if (pEntity->InCond(TF_COND_RUNE_SUPERNOVA))
			vConditions.emplace_back("Supernova");

		if (pEntity->InCond(TF_COND_PLAGUE))
			vConditions.emplace_back("Plagued");

		if (pEntity->InCond(TF_COND_KING_BUFFED))
			vConditions.emplace_back("King buff");



		if (pEntity->InCond(TF_COND_HALLOWEEN_IN_HELL))
			vConditions.emplace_back("Hell");

		if (pEntity->InCond(TF_COND_PURGATORY))
			vConditions.emplace_back("Purgatory");

		if (pEntity->InCond(TF_COND_PASSTIME_INTERCEPTION))
			vConditions.emplace_back("Interception");

		if (pEntity->InCond(TF_COND_TEAM_GLOWS))
			vConditions.emplace_back("Team glows");



		if (pEntity->InCond(TF_COND_PREVENT_DEATH))
			vConditions.emplace_back("Prevent death");

		if (pEntity->InCond(TF_COND_GRAPPLINGHOOK))
			vConditions.emplace_back("Grapple");

		if (pEntity->InCond(TF_COND_GRAPPLINGHOOK_SAFEFALL))
			vConditions.emplace_back("Safefall");

		if (pEntity->InCond(TF_COND_GRAPPLINGHOOK_LATCHED))
			vConditions.emplace_back("Latched");

		if (pEntity->InCond(TF_COND_GRAPPLED_TO_PLAYER))
			vConditions.emplace_back("To player");

		if (pEntity->InCond(TF_COND_GRAPPLED_BY_PLAYER))
			vConditions.emplace_back("By player");

		if (pEntity->InCond(TF_COND_TELEPORTED))
			vConditions.emplace_back("Teleported");

		if (pEntity->InCond(TF_COND_SELECTED_TO_TELEPORT))
			vConditions.emplace_back("Teleporting");

		if (pEntity->InCond(TF_COND_MEDIGUN_DEBUFF))
			vConditions.emplace_back("Medigun debuff");

		if (pEntity->InCond(TF_COND_SAPPED))
			vConditions.emplace_back("Sapped");

		if (pEntity->InCond(TF_COND_DISGUISED_AS_DISPENSER))
			vConditions.emplace_back("Dispenser");

		if (pEntity->InCond(TF_COND_TAUNTING))
			vConditions.emplace_back("Taunt");

		if (pEntity->InCond(TF_COND_HALLOWEEN_THRILLER))
			vConditions.emplace_back("Thriller");

		if (pEntity->InCond(TF_COND_FREEZE_INPUT))
			vConditions.emplace_back("Freeze input");

		if (pEntity->InCond(TF_COND_REPROGRAMMED))
			vConditions.emplace_back("Reprogrammed");

		//if (pEntity->InCond(TF_COND_COMPETITIVE_WINNER))
		//	vConditions.emplace_back("Win");

		//if (pEntity->InCond(TF_COND_COMPETITIVE_LOSER))
		//	vConditions.emplace_back("Loss");
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