#include "CritHack.h"

#include "../Ticks/Ticks.h"

#define WEAPON_RANDOM_RANGE				10000
#define TF_DAMAGE_CRIT_MULTIPLIER		3.0f
#define TF_DAMAGE_CRIT_CHANCE			0.02f
#define TF_DAMAGE_CRIT_CHANCE_RAPID		0.02f
#define TF_DAMAGE_CRIT_CHANCE_MELEE		0.15f
#define TF_DAMAGE_CRIT_DURATION_RAPID	2.0f

#define STATS_SEND_FREQUENCY 1.f

#define SEED_ATTEMPTS 4096
#define BUCKET_ATTEMPTS 1000

//#define SERVER_CRIT_DATA

int CCritHack::GetCritCommand(CTFWeaponBase* pWeapon, int iCommandNumber, bool bCrit, bool bSafe)
{
	for (int i = iCommandNumber; i < iCommandNumber + SEED_ATTEMPTS; i++)
	{
		if (IsCritCommand(i, pWeapon, bCrit, bSafe))
			return i;
	}
	return 0;
}

bool CCritHack::IsCritCommand(int iCommandNumber, CTFWeaponBase* pWeapon, bool bCrit, bool bSafe)
{
	int iSeed = CommandToSeed(iCommandNumber);
	return IsCritSeed(iSeed, pWeapon, bCrit, bSafe);
}

bool CCritHack::IsCritSeed(int iSeed, CTFWeaponBase* pWeapon, bool bCrit, bool bSafe)
{
	if (iSeed == pWeapon->m_iCurrentSeed())
		return false;

	SDK::RandomSeed(iSeed);
	int iRandom = SDK::RandomInt(0, WEAPON_RANDOM_RANGE - 1);

	if (bSafe)
	{
		int iLower, iUpper;
		if (m_bMelee)
			iLower = 1500, iUpper = 6000;
		else
			iLower = 100, iUpper = 800;
		iLower *= m_flMultCritChance, iUpper *= m_flMultCritChance;

		if (bCrit ? iLower >= 0 : iUpper < WEAPON_RANDOM_RANGE)
			return bCrit ? iRandom < iLower : !(iRandom < iUpper);
	}

	int iRange = m_flCritChance * WEAPON_RANDOM_RANGE;
	return bCrit ? iRandom < iRange : !(iRandom < iRange);
}

int CCritHack::CommandToSeed(int iCommandNumber)
{
	int iSeed = MD5_PseudoRandom(iCommandNumber) & std::numeric_limits<int>::max();
	int iMask = m_bMelee
		? m_iEntIndex << 16 | I::EngineClient->GetLocalPlayer() << 8
		: m_iEntIndex << 8 | I::EngineClient->GetLocalPlayer();
	return iSeed ^ iMask;
}



void CCritHack::UpdateWeaponInfo(CTFPlayer* pLocal, CTFWeaponBase* pWeapon)
{
	m_iEntIndex = pWeapon->entindex();
	m_bMelee = pWeapon->GetSlot() == SLOT_MELEE;
	if (m_bMelee)
		m_flCritChance = TF_DAMAGE_CRIT_CHANCE_MELEE * pLocal->GetCritMult();
	else if (pWeapon->IsRapidFire())
	{
		m_flCritChance = TF_DAMAGE_CRIT_CHANCE_RAPID * pLocal->GetCritMult();
		float flNonCritDuration = (TF_DAMAGE_CRIT_DURATION_RAPID / m_flCritChance) - TF_DAMAGE_CRIT_DURATION_RAPID;
		m_flCritChance = 1.f / flNonCritDuration;
	}
	else
		m_flCritChance = TF_DAMAGE_CRIT_CHANCE * pLocal->GetCritMult();
	m_flMultCritChance = SDK::AttribHookValue(1.f, "mult_crit_chance", pWeapon);
	m_flCritChance *= m_flMultCritChance;



	static CTFWeaponBase* pStaticWeapon = nullptr;
	const CTFWeaponBase* pOldWeapon = pStaticWeapon;
	pStaticWeapon = pWeapon;

	static float flStaticBucket = 0.f;
	const float flLastBucket = flStaticBucket;
	const float flBucket = flStaticBucket = pWeapon->m_flCritTokenBucket();

	static int iStaticCritChecks = 0.f;
	const int iLastCritChecks = iStaticCritChecks;
	const int iCritChecks = iStaticCritChecks = pWeapon->m_nCritChecks();

	static int iStaticCritSeedRequests = 0.f;
	const int iLastCritSeedRequests = iStaticCritSeedRequests;
	const int iCritSeedRequests = iStaticCritSeedRequests = pWeapon->m_nCritSeedRequests();

	if (pWeapon == pOldWeapon && flBucket == flLastBucket && iCritChecks == iLastCritChecks && iCritSeedRequests == iLastCritSeedRequests)
		return;

	static auto tf_weapon_criticals_bucket_cap = U::ConVars.FindVar("tf_weapon_criticals_bucket_cap");
	const float flBucketCap = tf_weapon_criticals_bucket_cap->GetFloat();
	bool bRapidFire = pWeapon->IsRapidFire();
	float flFireRate = pWeapon->GetFireRate();

	float flDamage = pWeapon->GetDamage();
	int nProjectilesPerShot = pWeapon->GetBulletsPerShot(false);
	if (!m_bMelee && nProjectilesPerShot > 0)
		nProjectilesPerShot = SDK::AttribHookValue(nProjectilesPerShot, "mult_bullets_per_shot", pWeapon);
	else
		nProjectilesPerShot = 1;
	float flBaseDamage = flDamage *= nProjectilesPerShot;
	if (bRapidFire)
	{
		flDamage *= TF_DAMAGE_CRIT_DURATION_RAPID / flFireRate;
		if (flDamage * TF_DAMAGE_CRIT_MULTIPLIER > flBucketCap)
			flDamage = flBucketCap / TF_DAMAGE_CRIT_MULTIPLIER;
	}

	float flMult = m_bMelee ? 0.5f : Math::RemapVal(float(iCritSeedRequests + 1) / (iCritChecks + 1), 0.1f, 1.f, 1.f, 3.f);
	float flCost = flDamage * TF_DAMAGE_CRIT_MULTIPLIER;

	int iPotentialCrits = (std::max(flBucketCap, flBucket) - flBaseDamage) / (TF_DAMAGE_CRIT_MULTIPLIER * flDamage / (m_bMelee ? 2 : 1) - flBaseDamage);
	int iAvailableCrits = 0;
	{
		int iTestShots = iCritChecks, iTestCrits = iCritSeedRequests;
		float flTestBucket = flBucket;
		for (int i = 0; i < BUCKET_ATTEMPTS; i++)
		{
			iTestShots++; iTestCrits++;

			float flTestMult = m_bMelee ? 0.5f : Math::RemapVal(float(iTestCrits) / iTestShots, 0.1f, 1.f, 1.f, 3.f);
			if (flTestBucket < flBucketCap)
				flTestBucket = std::min(flTestBucket + flBaseDamage, flBucketCap);
			flTestBucket -= flCost * flTestMult;
			if (flTestBucket < 0.f)
				break;

			iAvailableCrits++;
		}
	}

	int iNextCrit = 0;
	if (iAvailableCrits != iPotentialCrits)
	{
		int iTestShots = iCritChecks, iTestCrits = iCritSeedRequests;
		float flTestBucket = flBucket;
		float flTickBase = I::GlobalVars->curtime;
		float flLastRapidFireCritCheckTime = pWeapon->m_flLastRapidFireCritCheckTime();
		for (int i = 0; i < BUCKET_ATTEMPTS; i++)
		{
			int iCrits = 0;
			{
				int iTestShots2 = iTestShots, iTestCrits2 = iTestCrits;
				float flTestBucket2 = flTestBucket;
				for (int j = 0; j < BUCKET_ATTEMPTS; j++)
				{
					iTestShots2++; iTestCrits2++;

					float flTestMult = m_bMelee ? 0.5f : Math::RemapVal(float(iTestCrits2) / iTestShots2, 0.1f, 1.f, 1.f, 3.f);
					if (flTestBucket2 < flBucketCap)
						flTestBucket2 = std::min(flTestBucket2 + flBaseDamage, flBucketCap);
					flTestBucket2 -= flCost * flTestMult;
					if (flTestBucket2 < 0.f)
						break;

					iCrits++;
				}
			}
			if (iAvailableCrits < iCrits)
				break;

			if (!bRapidFire)
				iTestShots++;
			else 
			{
				flTickBase += std::ceilf(flFireRate / TICK_INTERVAL) * TICK_INTERVAL;
				if (flTickBase >= flLastRapidFireCritCheckTime + 1.f || !i && flTestBucket == flBucketCap)
				{
					iTestShots++;
					flLastRapidFireCritCheckTime = flTickBase;
				}
			}

			if (flTestBucket < flBucketCap)
				flTestBucket = std::min(flTestBucket + flBaseDamage, flBucketCap);

			iNextCrit++;
		}
	}

	m_flDamage = flBaseDamage;
	m_flCost = flCost * flMult;
	m_iPotentialCrits = iPotentialCrits;
	m_iAvailableCrits = iAvailableCrits;
	m_iNextCrit = iNextCrit;
}

void CCritHack::UpdateInfo(CTFPlayer* pLocal, CTFWeaponBase* pWeapon)
{
	UpdateWeaponInfo(pLocal, pWeapon);

	m_bCritBanned = false;
	m_flDamageTilFlip = 0;
	if (!m_bMelee)
	{
		const float flNormalizedDamage = m_iCritDamage / TF_DAMAGE_CRIT_MULTIPLIER;
		float flCritChance = m_flCritChance + 0.1f;
		if (m_iRangedDamage && m_iCritDamage)
		{
			const float flObservedCritChance = flNormalizedDamage / (flNormalizedDamage + m_iRangedDamage - m_iCritDamage);
			m_bCritBanned = flObservedCritChance > flCritChance;
		}

		if (m_bCritBanned)
			m_flDamageTilFlip = flNormalizedDamage / flCritChance + flNormalizedDamage * 2 - m_iRangedDamage;
		else
			m_flDamageTilFlip = TF_DAMAGE_CRIT_MULTIPLIER * (flNormalizedDamage - flCritChance * (flNormalizedDamage + m_iRangedDamage - m_iCritDamage)) / (flCritChance - 1);
	}

	if (auto pResource = H::Entities.GetResource())
	{
		m_iResourceDamage = pResource->m_iDamage(I::EngineClient->GetLocalPlayer());
		/* // more of a proof of concept for resyncing crit damage
		if (m_flLastDamageTime < I::GlobalVars->curtime + STATS_SEND_FREQUENCY * 2)
		{	// attempt to resync damages
			m_iRangedDamage = m_iResourceDamage - m_iMeleeDamage;

			float flObservedCritChance = pWeapon->m_flObservedCritChance();
			m_iCritDamage = (TF_DAMAGE_CRIT_MULTIPLIER * flObservedCritChance * m_iResourceDamage) / (1 + 2 * flObservedCritChance);
			SDK::Output("Info", std::format("{}, {}", m_iRangedDamage, m_iCritDamage).c_str());
		}
		*/
		m_iDesyncDamage = m_iRangedDamage + m_iMeleeDamage - m_iResourceDamage;
	}
}

bool CCritHack::WeaponCanCrit(CTFWeaponBase* pWeapon, bool bWeaponOnly)
{
	if (!bWeaponOnly && !pWeapon->AreRandomCritsEnabled() || SDK::AttribHookValue(1.f, "mult_crit_chance", pWeapon) <= 0.f)
		return false;

	switch (pWeapon->GetWeaponID())
	{
	case TF_WEAPON_PDA:
	case TF_WEAPON_PDA_ENGINEER_BUILD:
	case TF_WEAPON_PDA_ENGINEER_DESTROY:
	case TF_WEAPON_PDA_SPY:
	case TF_WEAPON_PDA_SPY_BUILD:
	case TF_WEAPON_BUILDER:
	case TF_WEAPON_INVIS:
	case TF_WEAPON_JAR_MILK:
	case TF_WEAPON_LUNCHBOX:
	case TF_WEAPON_BUFF_ITEM:
	case TF_WEAPON_FLAME_BALL:
	case TF_WEAPON_ROCKETPACK:
	case TF_WEAPON_JAR_GAS:
	case TF_WEAPON_LASER_POINTER:
	case TF_WEAPON_MEDIGUN:
	case TF_WEAPON_SNIPERRIFLE:
	case TF_WEAPON_SNIPERRIFLE_DECAP:
	case TF_WEAPON_SNIPERRIFLE_CLASSIC:
	case TF_WEAPON_COMPOUND_BOW:
	case TF_WEAPON_JAR:
	case TF_WEAPON_KNIFE:
	case TF_WEAPON_PASSTIME_GUN:
		return false;
	}

	return true;
}

void CCritHack::Reset()
{
	m_iCritDamage = 0;
	m_iRangedDamage = 0;
	m_iMeleeDamage = 0;
	m_iResourceDamage = 0;
	m_iDesyncDamage = 0;

	m_bCritBanned = false;
	m_flDamageTilFlip = 0;

	m_mHealthHistory.clear();

	//m_flLastDamageTime = 0.f;
}



int CCritHack::GetCritRequest(CUserCmd* pCmd, CTFWeaponBase* pWeapon)
{
	bool bCanCrit = m_iAvailableCrits > 0 && !m_bCritBanned;
	bool bPressed = Vars::CritHack::ForceCrits.Value || Vars::CritHack::AlwaysMeleeCrit.Value && m_bMelee && (Vars::Aimbot::General::AutoShoot.Value ? pCmd->buttons & IN_ATTACK && !(G::OriginalCmd.buttons & IN_ATTACK) : Vars::Aimbot::General::AimType.Value);
	
	bool bSkip = Vars::CritHack::AvoidRandomCrits.Value;
	bool bDesync = CommandToSeed(pCmd->command_number) == pWeapon->m_iCurrentSeed();

	return bCanCrit && bPressed ? CritRequestEnum::Crit : bSkip || bDesync ? CritRequestEnum::Skip : CritRequestEnum::Any;
}

void CCritHack::Run(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd)
{
	if (!pWeapon || !pLocal->IsAlive() || pLocal->IsAGhost() || !I::EngineClient->IsInGame())
		return;

	UpdateInfo(pLocal, pWeapon);
	if (pLocal->IsCritBoosted() || pWeapon->m_flCritTime() > I::GlobalVars->curtime || !WeaponCanCrit(pWeapon))
		return;

	if (pWeapon->GetWeaponID() == TF_WEAPON_MINIGUN && pCmd->buttons & IN_ATTACK)
		pCmd->buttons &= ~IN_ATTACK2;
	
	bool bAttacking = G::Attacking /*== 1*/ || F::Ticks.m_bDoubletap || F::Ticks.m_bSpeedhack;
	if (m_bMelee)
	{
		bAttacking = G::CanPrimaryAttack && pCmd->buttons & IN_ATTACK;
		if (!bAttacking && pWeapon->GetWeaponID() == TF_WEAPON_FISTS)
			bAttacking = G::CanPrimaryAttack && pCmd->buttons & IN_ATTACK2;
	}
	else if (pWeapon->GetWeaponID() == TF_WEAPON_MINIGUN && !(G::LastUserCmd->buttons & IN_ATTACK))
		bAttacking = false;
	else if (!bAttacking)
	{
		switch (pWeapon->GetWeaponID())
		{
		case TF_WEAPON_ROCKETLAUNCHER:
		case TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT:
			if (pWeapon->IsInReload() && G::CanPrimaryAttack && SDK::AttribHookValue(0, "can_overload", pWeapon))
			{
				int iClip1 = pWeapon->m_iClip1();
				if (pWeapon->m_bRemoveable() && iClip1 > 0)
					bAttacking = true;
				else if (iClip1 >= pWeapon->GetMaxClip1() || iClip1 > 0 && pLocal->GetAmmoCount(pWeapon->m_iPrimaryAmmoType()) == 0)
					bAttacking = true;
			}
		}
	}
	if (!bAttacking || pWeapon->IsRapidFire() && I::GlobalVars->curtime < pWeapon->m_flLastRapidFireCritCheckTime() + 1.f)
		return;

	int iRequest = GetCritRequest(pCmd, pWeapon);
	if (iRequest == CritRequestEnum::Any)
		return;

	if (!Vars::Misc::Game::AntiCheatCompatibility.Value)
	{
		if (int iCommand = GetCritCommand(pWeapon, pCmd->command_number, iRequest == CritRequestEnum::Crit))
		{
			pCmd->command_number = iCommand;
			pCmd->random_seed = MD5_PseudoRandom(iCommand) & std::numeric_limits<int>::max();
		}
	}
	else if (Vars::Misc::Game::AntiCheatCritHack.Value)
	{
		if (!IsCritCommand(pCmd->command_number, pWeapon, iRequest == CritRequestEnum::Crit, false))
		{
			pCmd->buttons &= ~IN_ATTACK;
			pCmd->viewangles = G::OriginalCmd.viewangles;
			G::PSilentAngles = false;
		}
	}
}

int CCritHack::PredictCmdNum(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd)
{
	auto getCmdNum = [&](int iCommandNumber)
		{
			if (!pWeapon || !pLocal->IsAlive() || !I::EngineClient->IsInGame() || Vars::Misc::Game::AntiCheatCompatibility.Value
				|| pLocal->IsCritBoosted() || pWeapon->m_flCritTime() > I::GlobalVars->curtime || !WeaponCanCrit(pWeapon))
				return iCommandNumber;

			UpdateInfo(pLocal, pWeapon);
			if (pWeapon->IsRapidFire() && I::GlobalVars->curtime < pWeapon->m_flLastRapidFireCritCheckTime() + 1.f)
				return iCommandNumber;

			int iRequest = GetCritRequest(pCmd, pWeapon);
			if (iRequest == CritRequestEnum::Any)
				return iCommandNumber;

			if (int iCommand = GetCritCommand(pWeapon, iCommandNumber, iRequest == CritRequestEnum::Crit))
				return iCommand;
			return iCommandNumber;
		};

	static int iCommandNumber = 0; // cache, don't constantly test

	static int iStaticCommand = 0;
	if (pCmd->command_number != iStaticCommand)
	{
		iCommandNumber = getCmdNum(pCmd->command_number);
		iStaticCommand = pCmd->command_number;
	}

	return iCommandNumber;
}

void CCritHack::Event(IGameEvent* pEvent, uint32_t uHash, CTFPlayer* pLocal)
{
	switch (uHash)
	{
	case FNV1A::Hash32Const("player_hurt"):
	{
		if (!pLocal)
			break;

		int iVictim = I::EngineClient->GetPlayerForUserID(pEvent->GetInt("userid"));
		int iAttacker = I::EngineClient->GetPlayerForUserID(pEvent->GetInt("attacker"));
		bool bCrit = pEvent->GetBool("crit") || pEvent->GetBool("minicrit");
		int iDamage = pEvent->GetInt("damageamount");
		int iHealth = pEvent->GetInt("health");
		int iWeaponID = pEvent->GetInt("weaponid");

		if (m_mHealthHistory.contains(iVictim))
		{
			auto& tHistory = m_mHealthHistory[iVictim];
			auto pVictim = I::ClientEntityList->GetClientEntity(iVictim)->As<CTFPlayer>();

			if (!iHealth)
				iDamage = std::min(iDamage, tHistory.m_iNewHealth);
			else if (pVictim && (pVictim->m_bFeignDeathReady() || pVictim->InCond(TF_COND_FEIGN_DEATH))) // damage number is spoofed upon sending, correct it
			{
				int iOldHealth = (tHistory.m_mHistory.contains(iHealth) ? tHistory.m_mHistory[iHealth].m_iOldHealth : tHistory.m_iNewHealth) % 32768;
				if (iHealth > iOldHealth)
				{
					for (auto& [_, tOldHealth] : tHistory.m_mHistory)
					{
						int iOldHealth2 = tOldHealth.m_iOldHealth % 32768;
						if (iOldHealth2 > iHealth)
							iOldHealth = iHealth > iOldHealth ? iOldHealth2 : std::min(iOldHealth, iOldHealth2);
					}
				}
				iDamage = std::clamp(iOldHealth - iHealth, 0, iDamage);
			}
		}
		if (iHealth)
			StoreHealthHistory(iVictim, iHealth);

		if (iVictim == iAttacker || iAttacker != I::EngineClient->GetLocalPlayer())
			break;

		if (auto pGameRules = I::TFGameRules())
		{
			auto pMatchDesc = pGameRules->GetMatchGroupDescription();
			if (pMatchDesc && pGameRules->m_iRoundState() != GR_STATE_RND_RUNNING)
			{
				switch (pMatchDesc->m_eMatchType)
				{
				case MATCH_TYPE_COMPETITIVE:
				case MATCH_TYPE_CASUAL:
					return;
				}
			}
		}

		//m_flLastDamageTime = I::GlobalVars->curtime;

		CTFWeaponBase* pWeapon = nullptr;
		for (int i = 0; i < MAX_WEAPONS; i++)
		{
			auto pWeapon2 = pLocal->GetWeaponFromSlot(i);
			if (!pWeapon2 || pWeapon2->GetWeaponID() != iWeaponID)
				continue;

			pWeapon = pWeapon2;
			break;
		}

		if (!pWeapon || pWeapon->GetSlot() != SLOT_MELEE)
		{
			m_iRangedDamage += iDamage;
			if (bCrit && !pLocal->IsCritBoosted())
				m_iCritDamage += iDamage;
		}
		else
			m_iMeleeDamage += iDamage;

		break;
	}
	case FNV1A::Hash32Const("scorestats_accumulated_update"):
	case FNV1A::Hash32Const("mvm_reset_stats"):
		m_iRangedDamage = m_iCritDamage = m_iMeleeDamage = 0;
		break;
	case FNV1A::Hash32Const("client_beginconnect"):
	case FNV1A::Hash32Const("client_disconnect"):
	case FNV1A::Hash32Const("game_newmap"):
		Reset();
	}
}

void CCritHack::Store()
{
	for (int n = 1; n <= I::EngineClient->GetMaxClients(); n++)
	{
		auto pPlayer = I::ClientEntityList->GetClientEntity(n)->As<CTFPlayer>();
		if (pPlayer && pPlayer->IsAlive() && !pPlayer->IsAGhost())
			StoreHealthHistory(pPlayer->entindex(), pPlayer->m_iHealth());
	}
}

void CCritHack::StoreHealthHistory(int iIndex, int iHealth, bool bDamage)
{
	bool bContains = m_mHealthHistory.contains(iIndex);
	auto& tHistory = m_mHealthHistory[iIndex];

	if (!bContains)
		tHistory = { iHealth, iHealth };
	else if (iHealth != tHistory.m_iNewHealth)
	{
		tHistory.m_iOldHealth = std::max(bDamage && tHistory.m_mHistory.contains(iHealth % 32768) ? tHistory.m_mHistory[iHealth % 32768].m_iOldHealth : tHistory.m_iNewHealth, iHealth);
		tHistory.m_iNewHealth = iHealth;
	}

	tHistory.m_mHistory[iHealth % 32768] = { tHistory.m_iOldHealth, float(SDK::PlatFloatTime()) };
	while (tHistory.m_mHistory.size() > 3)
	{
		int iIndex2; float flMin = std::numeric_limits<float>::max();
		for (auto& [i, tStorage] : tHistory.m_mHistory)
		{
			if (tStorage.m_flTime < flMin)
				flMin = tStorage.m_flTime, iIndex2 = i;
		}
		tHistory.m_mHistory.erase(iIndex2);
	}
}

#ifdef SERVER_CRIT_DATA
MAKE_SIGNATURE(CTFGameStats_FindPlayerStats, "server.dll", "4C 8B C1 48 85 D2 75", 0x0);
MAKE_SIGNATURE(UTIL_PlayerByIndex, "server.dll", "48 83 EC ? 8B D1 85 C9 7E ? 48 8B 05", 0x0);

static void* pCTFGameStats = nullptr;
MAKE_HOOK(CTFGameStats_FindPlayerStats, S::CTFGameStats_FindPlayerStats(), void*,
	void* rcx, CBasePlayer* pPlayer)
{
	pCTFGameStats = rcx;
	return CALL_ORIGINAL(rcx, pPlayer);
}
#endif

void CCritHack::Draw(CTFPlayer* pLocal)
{
	if (!(Vars::Menu::Indicators.Value & Vars::Menu::IndicatorsEnum::CritHack) || !I::EngineClient->IsInGame())
		return;

	auto pWeapon = H::Entities.GetWeapon();
	if (!pWeapon || !pLocal->IsAlive() || pLocal->IsAGhost()
		|| !WeaponCanCrit(pWeapon, true))
		return;



	int x = Vars::Menu::CritsDisplay.Value.x;
	int y = Vars::Menu::CritsDisplay.Value.y + 8;
	const auto& fFont = H::Fonts.GetFont(FONT_INDICATORS);
	const int nTall = fFont.m_nTall + H::Draw.Scale(1);
	y -= nTall;

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

	if (!pWeapon->AreRandomCritsEnabled())
	{
		H::Draw.StringOutlined(fFont, x, y += nTall, Vars::Colors::IndicatorTextBad.Value, Vars::Menu::Theme::Background.Value, align, "Random crits disabled");
		return;
	}



	float flTickBase = TICKS_TO_TIME(pLocal->m_nTickBase());

	if (Vars::Misc::Game::AntiCheatCompatibility.Value)
		H::Draw.StringOutlined(fFont, x, y += nTall, Vars::Colors::IndicatorTextBad.Value, Vars::Menu::Theme::Background.Value, align, "Anticheat compatibility");

	if (pLocal->IsCritBoosted())
		H::Draw.StringOutlined(fFont, x, y += nTall, Vars::Colors::IndicatorTextMisc.Value, Vars::Menu::Theme::Background.Value, align, "Crit Boosted");
	else if (pWeapon->m_flCritTime() > flTickBase)
	{
		float flTime = pWeapon->m_flCritTime() - flTickBase;
		H::Draw.StringOutlined(fFont, x, y += nTall, Vars::Colors::IndicatorTextMisc.Value, Vars::Menu::Theme::Background.Value, align, std::format("Streaming crits {:.1f}s", flTime).c_str());
	}
	else if (!m_bCritBanned)
	{
		if (m_iPotentialCrits > 0)
		{
			if (m_iAvailableCrits > 0)
			{
				if (!pWeapon->IsRapidFire() || flTickBase >= pWeapon->m_flLastRapidFireCritCheckTime() + 1.f)
					H::Draw.StringOutlined(fFont, x, y += nTall, Vars::Colors::IndicatorTextGood.Value, Vars::Menu::Theme::Background.Value, align, "Crit Ready");
				else
				{
					float flTime = pWeapon->m_flLastRapidFireCritCheckTime() + 1.f - flTickBase;
					H::Draw.StringOutlined(fFont, x, y += nTall, Vars::Menu::Theme::Active.Value, Vars::Menu::Theme::Background.Value, align, std::format("Wait {:.1f}s", flTime).c_str());
				}
			}
			else
			{
				int iShots = m_iNextCrit;
				H::Draw.StringOutlined(fFont, x, y += nTall, Vars::Colors::IndicatorTextBad.Value, Vars::Menu::Theme::Background.Value, align, std::format("Crit in {}{} shot{}", iShots, iShots == BUCKET_ATTEMPTS ? "+" : "", iShots == 1 ? "" : "s").c_str());
			}
		}
	}
	else
		H::Draw.StringOutlined(fFont, x, y += nTall, Vars::Colors::IndicatorTextBad.Value, Vars::Menu::Theme::Background.Value, align, std::format("Deal {} damage", ceilf(m_flDamageTilFlip)).c_str());
	
	if (m_iPotentialCrits > 0)
	{
		int iCrits = m_iAvailableCrits;
		H::Draw.StringOutlined(fFont, x, y += nTall, Vars::Menu::Theme::Active.Value, Vars::Menu::Theme::Background.Value, align, std::format("{}{} / {} crits", iCrits, iCrits == BUCKET_ATTEMPTS ? "+" : "", m_iPotentialCrits).c_str());
		
		if (m_iNextCrit && iCrits)
		{
			int iShots = m_iNextCrit;
			H::Draw.StringOutlined(fFont, x, y += nTall, Vars::Menu::Theme::Active.Value, Vars::Menu::Theme::Background.Value, align, std::format("Next in {}{} shot{}", iShots, iShots == BUCKET_ATTEMPTS ? "+" : "", iShots == 1 ? "" : "s").c_str());
		}
	}

	if (m_flDamageTilFlip && !m_bCritBanned)
		H::Draw.StringOutlined(fFont, x, y += nTall, Vars::Colors::IndicatorTextGood.Value, Vars::Menu::Theme::Background.Value, align, std::format("{} damage", floorf(m_flDamageTilFlip)).c_str());

	if (m_iDesyncDamage)
	{
		auto tColor = m_iDesyncDamage < 0
			? Vars::Menu::Theme::Active.Value.Lerp(Vars::Colors::IndicatorTextMid.Value, std::min(fabsf(m_iDesyncDamage) / 100, 1.f))
			: Vars::Colors::IndicatorTextBad.Value;
		H::Draw.StringOutlined(fFont, x, y += nTall, tColor, Vars::Menu::Theme::Background.Value, align, std::format("{}{} desync", m_iDesyncDamage > 0 ? "+" : "", m_iDesyncDamage).c_str());
	}



	if (Vars::Debug::Info.Value)
	{
		H::Draw.StringOutlined(fFont, x, y += nTall * 2, Vars::Menu::Theme::Active.Value, Vars::Menu::Theme::Background.Value, align, std::format("RangedDamage: {}, CritDamage: {}", m_iRangedDamage, m_iCritDamage).c_str());

#ifdef SERVER_CRIT_DATA
		H::Draw.StringOutlined(fFont, x, y += nTall, Vars::Menu::Theme::Active.Value, Vars::Menu::Theme::Background.Value, align, std::format("AllDamage: {} ({})", m_iRangedDamage + m_iMeleeDamage, m_iMeleeDamage).c_str());

		if (pCTFGameStats)
		{
			if (auto pPlayer2 = S::UTIL_PlayerByIndex.Call<void*>(I::EngineClient->GetLocalPlayer()))
			{
				if (auto pPlayerStats = S::CTFGameStats_FindPlayerStats.Call<PlayerStats_t*>(pCTFGameStats, pPlayer2))
				{
					int iRangedDamage = pPlayerStats->statsCurrentRound.m_iStat[TFSTAT_DAMAGE_RANGED] /*= m_iRangedDamage*/;
					int iCritDamage = pPlayerStats->statsCurrentRound.m_iStat[TFSTAT_DAMAGE_RANGED_CRIT_RANDOM] /*= m_iCritDamage = 0*/;
					int iDamage = pPlayerStats->statsCurrentRound.m_iStat[TFSTAT_DAMAGE] /*= m_iRangedDamage + m_iMeleeDamage*/;

					H::Draw.StringOutlined(fFont, x, y += nTall, Vars::Menu::Theme::Active.Value, Vars::Menu::Theme::Background.Value, align, std::format(
						"RangedDamage: {}, CritDamage: {}", iRangedDamage, iCritDamage
					).c_str());
					H::Draw.StringOutlined(fFont, x, y += nTall, Vars::Menu::Theme::Active.Value, Vars::Menu::Theme::Background.Value, align, std::format(
						"AllDamage: {} ({})", iDamage, iDamage - iRangedDamage
					).c_str());
				}
			}
		}

		H::Draw.StringOutlined(fFont, x, y += nTall, Vars::Menu::Theme::Active.Value, Vars::Menu::Theme::Background.Value, align, std::format("ResourceDamage: {} ({})", m_iResourceDamage, m_iMeleeDamage).c_str());
#endif

		H::Draw.StringOutlined(fFont, x, y += nTall * 2, Vars::Menu::Theme::Active.Value, Vars::Menu::Theme::Background.Value, align, std::format("Bucket: {}, Shots: {}, Crits: {}", pWeapon->m_flCritTokenBucket(), pWeapon->m_nCritChecks(), pWeapon->m_nCritSeedRequests()).c_str());
		H::Draw.StringOutlined(fFont, x, y += nTall, Vars::Menu::Theme::Active.Value, Vars::Menu::Theme::Background.Value, align, std::format("Damage: {}, Cost: {}", m_flDamage, m_flCost).c_str());
		H::Draw.StringOutlined(fFont, x, y += nTall, Vars::Menu::Theme::Active.Value, Vars::Menu::Theme::Background.Value, align, std::format("CritChance: {:.2f} ({:.2f})", m_flCritChance, m_flCritChance + 0.1f).c_str());
	}
}