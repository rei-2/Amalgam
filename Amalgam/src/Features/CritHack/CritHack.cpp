#include "CritHack.h"

#define WEAPON_RANDOM_RANGE				10000
#define TF_DAMAGE_CRIT_MULTIPLIER		3.0f
#define TF_DAMAGE_CRIT_CHANCE			0.02f
#define TF_DAMAGE_CRIT_CHANCE_RAPID		0.02f
#define TF_DAMAGE_CRIT_CHANCE_MELEE		0.15f
#define TF_DAMAGE_CRIT_DURATION_RAPID	2.0f

void CCritHack::Fill(const CUserCmd* pCmd, int n)
{
	static int iStart = pCmd->command_number;

	for (auto& [iSlot, tStorage] : m_mStorage)
	{
		for (auto it = tStorage.m_vCritCommands.begin(); it != tStorage.m_vCritCommands.end();)
		{
			if (*it <= pCmd->command_number)
				it = tStorage.m_vCritCommands.erase(it);
			else
				++it;
		}
		for (auto it = tStorage.m_vSkipCommands.begin(); it != tStorage.m_vSkipCommands.end();)
		{
			if (*it <= pCmd->command_number)
				it = tStorage.m_vSkipCommands.erase(it);
			else
				++it;
		}

		for (int i = 0; i < n; i++)
		{
			if (tStorage.m_vCritCommands.size() >= unsigned(n))
				break;

			const int iCmdNum = iStart + i;
			if (IsCritCommand(iSlot, tStorage.m_iEntIndex, iCmdNum))
				tStorage.m_vCritCommands.push_back(iCmdNum);
		}
		for (int i = 0; i < n; i++)
		{
			if (tStorage.m_vSkipCommands.size() >= unsigned(n))
				break;

			const int iCmdNum = iStart + i;
			if (IsCritCommand(iSlot, tStorage.m_iEntIndex, iCmdNum, false))
				tStorage.m_vSkipCommands.push_back(iCmdNum);
		}
	}

	iStart += n;
}



bool CCritHack::IsCritCommand(int iSlot, int iIndex, const i32 command_number, const bool bCrit, const bool bSafe)
{
	const auto uSeed = MD5_PseudoRandom(command_number) & 0x7FFFFFFF;
	SDK::RandomSeed(DecryptOrEncryptSeed(iSlot, iIndex, uSeed));
	const int iRandom = SDK::RandomInt(0, WEAPON_RANDOM_RANGE - 1);

	if (bSafe)
		return bCrit ? iRandom < 100 : !(iRandom < 6000);
	else
	{
		const int iRange = (m_flCritChance - 0.1f) * WEAPON_RANDOM_RANGE;
		return bCrit ? iRandom < iRange : !(iRandom < iRange);
	}
}

u32 CCritHack::DecryptOrEncryptSeed(int iSlot, int iIndex, u32 uSeed)
{
	int iLeft = iSlot == SLOT_MELEE ? 8 : 0;
	unsigned int iMask = iIndex << (iLeft + 8) | I::EngineClient->GetLocalPlayer() << iLeft;
	return iMask ^ uSeed;
}



void CCritHack::GetTotalCrits(CTFPlayer* pLocal, CTFWeaponBase* pWeapon)
{
	int iSlot = pWeapon->GetSlot();
	auto& tStorage = m_mStorage[iSlot];

	static float flOldBucket = 0.f; static int iOldID = 0, iOldCritChecks = 0, iOldCritSeedRequests = 0;
	const float flBucket = pWeapon->m_flCritTokenBucket(); const int iID = pWeapon->GetWeaponID(), iCritChecks = pWeapon->m_nCritChecks(), iCritSeedRequests = pWeapon->m_nCritSeedRequests();
	const bool bMatch = tStorage.m_flDamage > 0 && flOldBucket == flBucket && iOldID == iID && iOldCritChecks == iCritChecks && iOldCritSeedRequests == iCritSeedRequests;

	auto pWeaponInfo = pWeapon->GetWeaponInfo();
	if (bMatch || !pWeaponInfo)
		return;
	flOldBucket = flBucket; iOldID = iID, iOldCritChecks = iCritChecks, iOldCritSeedRequests = iCritSeedRequests;

	static auto tf_weapon_criticals_bucket_cap = U::ConVars.FindVar("tf_weapon_criticals_bucket_cap");
	const float flBucketCap = tf_weapon_criticals_bucket_cap ? tf_weapon_criticals_bucket_cap->GetFloat() : 1000.f;

	auto& tWeaponData = pWeaponInfo->GetWeaponData(0);

	float flDamage = tWeaponData.m_nDamage;
	flDamage = SDK::AttribHookValue(flDamage, "mult_dmg", pWeapon);
	int nProjectilesPerShot = tWeaponData.m_nBulletsPerShot;
	if (nProjectilesPerShot >= 1)
		nProjectilesPerShot = SDK::AttribHookValue(nProjectilesPerShot, "mult_bullets_per_shot", pWeapon);
	else
		nProjectilesPerShot = 1;
	tStorage.m_flDamage = flDamage *= nProjectilesPerShot;

	if (tWeaponData.m_bUseRapidFireCrits)
	{
		flDamage *= TF_DAMAGE_CRIT_DURATION_RAPID / tWeaponData.m_flTimeFireDelay;
		if (flDamage * TF_DAMAGE_CRIT_MULTIPLIER > flBucketCap)
			flDamage = flBucketCap / TF_DAMAGE_CRIT_MULTIPLIER;
	}

	float flMult = iSlot == SLOT_MELEE ? 0.5f : Math::RemapValClamped(float(iCritSeedRequests + 1) / (iCritChecks + 1), 0.1f, 1.f, 1.f, 3.f);
	tStorage.m_flCost = flDamage * TF_DAMAGE_CRIT_MULTIPLIER * flMult;

	if (flBucketCap)
		tStorage.m_iPotentialCrits = (flBucketCap - tStorage.m_flDamage) / (3 * flDamage / (iSlot == SLOT_MELEE ? 2 : 1) - tStorage.m_flDamage);

	int iCrits = 0;
	{
		int shots = iCritChecks, crits = iCritSeedRequests;
		float bucket = flBucket, flCost = flDamage * TF_DAMAGE_CRIT_MULTIPLIER;
		const int iAttempts = std::min(tStorage.m_iPotentialCrits + 1, 100);
		for (int i = 0; i < iAttempts; i++)
		{
			shots++; crits++;

			flMult = iSlot == SLOT_MELEE ? 0.5f : Math::RemapValClamped(float(crits) / shots, 0.1f, 1.f, 1.f, 3.f);
			bucket = std::min(bucket + tStorage.m_flDamage, flBucketCap) - flCost * flMult;
			if (bucket < 0.f)
				break;

			iCrits++;
		}
	}

	if (!iCrits)
	{
		int shots = iCritChecks + 1, crits = iCritSeedRequests + 1;
		float bucket = std::min(flBucket + tStorage.m_flDamage, flBucketCap), flCost = flDamage * TF_DAMAGE_CRIT_MULTIPLIER;
		for (int i = 0; i < 100; i++)
		{
			iCrits--;
			if (!tWeaponData.m_bUseRapidFireCrits || !(i % int(tWeaponData.m_flTimeFireDelay / TICK_INTERVAL)))
				shots++;

			flMult = iSlot == SLOT_MELEE ? 0.5f : Math::RemapValClamped(float(crits) / shots, 0.1f, 1.f, 1.f, 3.f);
			bucket = std::min(bucket + tStorage.m_flDamage, flBucketCap);
			if (bucket >= flCost * flMult)
				break;
		}
	}

	tStorage.m_iAvailableCrits = iCrits;
}

void CCritHack::CanFireCritical(CTFPlayer* pLocal, CTFWeaponBase* pWeapon)
{
	m_bCritBanned = false;
	m_iDamageTilUnban = 0;

	if (pWeapon->GetSlot() == SLOT_MELEE)
		m_flCritChance = TF_DAMAGE_CRIT_CHANCE_MELEE * pLocal->GetCritMult();
	else if (pWeapon->IsRapidFire())
	{
		m_flCritChance = TF_DAMAGE_CRIT_CHANCE_RAPID * pLocal->GetCritMult();
		float flNonCritDuration = (TF_DAMAGE_CRIT_DURATION_RAPID / m_flCritChance) - TF_DAMAGE_CRIT_DURATION_RAPID;
		m_flCritChance = 1.f / flNonCritDuration;
	}
	else
		m_flCritChance = TF_DAMAGE_CRIT_CHANCE * pLocal->GetCritMult();
	m_flCritChance = SDK::AttribHookValue(m_flCritChance, "mult_crit_chance", pWeapon) + 0.1f;

	if (!m_iAllDamage || !m_iCritDamage || pWeapon->GetSlot() == SLOT_MELEE)
		return;

	const float flNormalizedDamage = m_iCritDamage / TF_DAMAGE_CRIT_MULTIPLIER;
	const float flObservedCritChance = flNormalizedDamage / (flNormalizedDamage + m_iAllDamage - m_iCritDamage);
	if (m_bCritBanned = flObservedCritChance > m_flCritChance)
		m_iDamageTilUnban = flNormalizedDamage / m_flCritChance + m_iCritDamage - flNormalizedDamage - m_iAllDamage;
}

bool CCritHack::WeaponCanCrit(CTFWeaponBase* pWeapon)
{
	if (!pWeapon->AreRandomCritsEnabled() || SDK::AttribHookValue(1.f, "mult_crit_chance", pWeapon) <= 0.f)
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
	case TF_WEAPON_GRAPPLINGHOOK:
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
		return false;
	}

	return true;
}



void CCritHack::ResetWeapons(CTFPlayer* pLocal)
{
	std::unordered_map<int, bool> mWeapons = {};
	auto hWeapons = pLocal->GetMyWeapons();
	if (!hWeapons)
		return;

	for (size_t i = 0; hWeapons[i]; i++)
	{
		auto pWeapon = pLocal->GetWeaponFromSlot(int(i));
		if (!pWeapon)
			break;

		int iSlot = pWeapon->GetSlot();
		auto& tStorage = m_mStorage[iSlot];
		mWeapons[iSlot] = true;
		int iEntIndex = pWeapon->entindex();
		int iDefIndex = pWeapon->m_iItemDefinitionIndex();

		if (tStorage.m_iEntIndex != iEntIndex || tStorage.m_iDefIndex != iDefIndex)
		{
			tStorage = { iEntIndex, iDefIndex };

			SDK::Output("Crithack", std::format("Resetting weapon {}", iDefIndex).c_str(), { 0, 255, 255, 255 }, Vars::Debug::Logging.Value);
		}
	}

	for (auto& [iSlot, _] : m_mStorage)
	{
		if (!mWeapons[iSlot])
			m_mStorage[iSlot] = {};
	}
}

void CCritHack::Reset()
{
	m_mStorage = {};

	m_iCritDamage = 0;
	m_iAllDamage = 0;

	m_bCritBanned = false;
	m_iDamageTilUnban = 0;
	m_flCritChance = 0.f;

	SDK::Output("Crithack", "Resetting all", { 0, 255, 255, 255 }, Vars::Debug::Logging.Value);
}



void CCritHack::Run(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd)
{
	if (!pLocal || !pWeapon || !pLocal->IsAlive() || !I::EngineClient->IsInGame())
		return;

	ResetWeapons(pLocal);
	Fill(pCmd, 15);
	GetTotalCrits(pLocal, pWeapon);
	CanFireCritical(pLocal, pWeapon);
	if (pLocal->IsCritBoosted() || !WeaponCanCrit(pWeapon))
		return;

	if (pWeapon->GetWeaponID() == TF_WEAPON_MINIGUN && pCmd->buttons & IN_ATTACK)
		pCmd->buttons &= ~IN_ATTACK2;

	bool bAttacking = G::IsAttacking = SDK::IsAttacking(pLocal, pWeapon, pCmd, false);
	if (G::PrimaryWeaponType == EWeaponType::MELEE)
	{
		bAttacking = G::CanPrimaryAttack && pCmd->buttons & IN_ATTACK;
		if (!bAttacking && pWeapon->GetWeaponID() == TF_WEAPON_FISTS)
			bAttacking = G::CanPrimaryAttack && pCmd->buttons & IN_ATTACK2;
	}
	else if (pWeapon->GetWeaponID() == TF_WEAPON_MINIGUN && !(G::LastUserCmd->buttons & IN_ATTACK))
		bAttacking = false;

	int iSlot = pWeapon->GetSlot();
	auto& tStorage = m_mStorage[iSlot];
	bool bRapidFire = pWeapon->IsRapidFire();
	bool bStreamWait = pWeapon->IsRapidFire() && TICKS_TO_TIME(pLocal->m_nTickBase()) < pWeapon->m_flLastRapidFireCritCheckTime() + 1.f;

	int closestCrit = !tStorage.m_vCritCommands.empty() ? tStorage.m_vCritCommands.front() : 0;
	int closestSkip = !tStorage.m_vSkipCommands.empty() ? tStorage.m_vSkipCommands.front() : 0;

	static bool bFirstTimePredicted = true;
	if (!I::ClientState->chokedcommands)
		bFirstTimePredicted = true;
	if (bAttacking && !pWeapon->IsInReload() && bFirstTimePredicted)
	{
		bFirstTimePredicted = false;

		const bool bCanCrit = tStorage.m_iAvailableCrits > 0 && (!m_bCritBanned || pWeapon->GetSlot() == SLOT_MELEE) && !bStreamWait;
		const bool bPressed = Vars::CritHack::ForceCrits.Value || pWeapon->GetSlot() == SLOT_MELEE && Vars::CritHack::AlwaysMeleeCrit.Value && (Vars::Aimbot::General::AutoShoot.Value ? pCmd->buttons & IN_ATTACK && !(G::Buttons & IN_ATTACK) : Vars::Aimbot::General::AimType.Value);
		if (bCanCrit && bPressed && closestCrit)
			pCmd->command_number = closestCrit;
		else if (Vars::CritHack::AvoidRandom.Value && closestSkip)
			pCmd->command_number = closestSkip;
	}
	//else if (Vars::CritHack::AvoidRandom.Value && closestSkip)
	//	pCmd->command_number = closestSkip;

	//if (pCmd->command_number == closestCrit || pCmd->command_number == closestSkip)
	m_iWishRandomSeed = MD5_PseudoRandom(pCmd->command_number) & std::numeric_limits<int>::max();

	if (pCmd->command_number == closestCrit)
		tStorage.m_vCritCommands.pop_front();
	else if (pCmd->command_number == closestSkip)
		tStorage.m_vSkipCommands.pop_front();
}

bool CCritHack::CalcIsAttackCriticalHandler(CTFPlayer* pLocal, CTFWeaponBase* pWeapon)
{
	if (!I::Prediction->m_bFirstTimePredicted || !pLocal || !pWeapon)
		return false;

	if (pWeapon->GetWeaponID() == TF_WEAPON_MINIGUN || pWeapon->GetWeaponID() == TF_WEAPON_FLAMETHROWER)
	{
		static int iStaticAmmo = pLocal->GetAmmoCount(pWeapon->m_iPrimaryAmmoType());
		int iOldAmmo = iStaticAmmo;
		int iNewAmmo = iStaticAmmo = pLocal->GetAmmoCount(pWeapon->m_iPrimaryAmmoType());
		if (iOldAmmo == iNewAmmo)
			return false;
	}

	if (m_iWishRandomSeed)
	{
		*G::RandomSeed() = m_iWishRandomSeed;
		m_iWishRandomSeed = 0;
	}

	return true;
}

void CCritHack::Event(IGameEvent* pEvent, uint32_t uHash, CTFPlayer* pLocal)
{
	switch (uHash)
	{
	case FNV1A::Hash32Const("player_hurt"):
	{
		auto pWeapon = H::Entities.GetWeapon();
		if (!pLocal || !pWeapon)
			return;

		const int iVictim = I::EngineClient->GetPlayerForUserID(pEvent->GetInt("userid"));
		const int iAttacker = I::EngineClient->GetPlayerForUserID(pEvent->GetInt("attacker"));
		const bool bCrit = pEvent->GetBool("crit") || pEvent->GetBool("minicrit");
		const int iDamage = m_mHealthStorage.contains(iVictim) ? std::min(pEvent->GetInt("damageamount"), m_mHealthStorage[iVictim]) : pEvent->GetInt("damageamount");
		const auto iWeaponID = pEvent->GetInt("weaponid");

		if (iVictim == iAttacker || iAttacker != pLocal->entindex() || iWeaponID != pWeapon->GetWeaponID() || pWeapon->GetSlot() == SLOT_MELEE) // weapon id stuff is dumb simplification
			return;

		m_iAllDamage += iDamage;
		if (bCrit && !pLocal->IsCritBoosted())
			m_iCritDamage += iDamage;

		return;
	}
	case FNV1A::Hash32Const("teamplay_round_start"):
		m_iAllDamage = m_iCritDamage = 0;
		return;
	case FNV1A::Hash32Const("client_beginconnect"):
	case FNV1A::Hash32Const("client_disconnect"):
	case FNV1A::Hash32Const("game_newmap"):
		Reset();
	}
}

void CCritHack::Store()
{
	auto pResource = H::Entities.GetPR();
	if (!pResource)
		return;

	for (auto it = m_mHealthStorage.begin(); it != m_mHealthStorage.end();)
	{
		if (I::ClientEntityList->GetClientEntity(it->first))
			++it;
		else
			it = m_mHealthStorage.erase(it);
	}
	for (auto& pEntity : H::Entities.GetGroup(EGroupType::PLAYERS_ALL))
	{
		auto pPlayer = pEntity->As<CTFPlayer>();
		if (pPlayer->IsAlive() && !pPlayer->IsAGhost())
			m_mHealthStorage[pPlayer->entindex()] = pPlayer->IsDormant() ? pResource->GetHealth(pPlayer->entindex()) : pPlayer->m_iHealth();
	}
}

void CCritHack::Draw(CTFPlayer* pLocal)
{
	static auto tf_weapon_criticals = U::ConVars.FindVar("tf_weapon_criticals");
	const bool bWeaponCriticals = tf_weapon_criticals ? tf_weapon_criticals->GetBool() : true;
	if (!(Vars::Menu::Indicators.Value & Vars::Menu::IndicatorsEnum::CritHack) || !I::EngineClient->IsInGame() || !bWeaponCriticals)
		return;

	auto pWeapon = H::Entities.GetWeapon();
	if (!pWeapon || !pLocal->IsAlive() || pLocal->IsAGhost())
		return;


	int x = Vars::Menu::CritsDisplay.Value.x;
	int y = Vars::Menu::CritsDisplay.Value.y + 8;

	const auto& fFont = H::Fonts.GetFont(FONT_INDICATORS);

	EAlign align = ALIGN_TOP;
	if (x <= (100 + 50 * Vars::Menu::DPI.Value))
	{
		x -= 42 * Vars::Menu::DPI.Value;
		align = ALIGN_TOPLEFT;
	}
	else if (x >= H::Draw.m_nScreenW - (100 + 50 * Vars::Menu::DPI.Value))
	{
		x += 42 * Vars::Menu::DPI.Value;
		align = ALIGN_TOPRIGHT;
	}

	if (WeaponCanCrit(pWeapon))
	{
		const auto iSlot = pWeapon->GetSlot();
		const auto bRapidFire = pWeapon->IsRapidFire();
		const float flTickBase = TICKS_TO_TIME(pLocal->m_nTickBase());

		if (m_mStorage[iSlot].m_flDamage > 0)
		{
			if (pLocal->IsCritBoosted())
				H::Draw.String(fFont, x, y, { 100, 255, 255, 255 }, align, "Crit Boosted");
			else if (pWeapon->m_flCritTime() > flTickBase)
			{
				const float flTime = pWeapon->m_flCritTime() - flTickBase;
				H::Draw.String(fFont, x, y, { 100, 255, 255, 255 }, align, std::format("Streaming crits {:.1f}s", flTime).c_str());
			}
			else if (!m_bCritBanned)
			{
				if (m_mStorage[iSlot].m_iAvailableCrits > 0)
				{
					if (bRapidFire && flTickBase < pWeapon->m_flLastRapidFireCritCheckTime() + 1.f)
					{
						const float flTime = pWeapon->m_flLastRapidFireCritCheckTime() + 1.f - flTickBase;
						H::Draw.String(fFont, x, y, Vars::Menu::Theme::Active.Value, align, std::format("Wait {:.1f}s", flTime).c_str());
					}
					else
						H::Draw.String(fFont, x, y, { 150, 255, 150, 255 }, align, "Crit Ready");
				}
				else
				{
					const int shots = -m_mStorage[iSlot].m_iAvailableCrits;
					H::Draw.String(fFont, x, y, { 255, 150, 150, 255 }, align, shots == 1 ? std::format("Crit in {} shot", shots).c_str() : std::format("Crit in {}{} shots", shots, shots == 100 ? "+" : "").c_str());
				}
			}
			else
				H::Draw.String(fFont, x, y, { 255, 150, 150, 255 }, align, std::format("Deal {} damage", m_iDamageTilUnban).c_str());

			H::Draw.String(fFont, x, y + fFont.m_nTall + 1, Vars::Menu::Theme::Active.Value, align, std::format("{} / {} potential crits", std::max(m_mStorage[iSlot].m_iAvailableCrits, 0), m_mStorage[iSlot].m_iPotentialCrits).c_str());
		}
		else
			H::Draw.String(fFont, x, y, Vars::Menu::Theme::Active.Value, align, "Calculating");

		if (Vars::Debug::Info.Value)
		{
			H::Draw.String(fFont, x, y + fFont.m_nTall * 3, {}, align, std::format("AllDamage: {}, CritDamage: {}", m_iAllDamage, m_iCritDamage).c_str());
			H::Draw.String(fFont, x, y + fFont.m_nTall * 4, {}, align, std::format("Bucket: {}", pWeapon->m_flCritTokenBucket()).c_str());
			H::Draw.String(fFont, x, y + fFont.m_nTall * 5, {}, align, std::format("Damage: {}, Cost: {}", m_mStorage[iSlot].m_flDamage, m_mStorage[iSlot].m_flCost).c_str());
			H::Draw.String(fFont, x, y + fFont.m_nTall * 6, {}, align, std::format("Shots: {}, Crits: {}", pWeapon->m_nCritChecks(), pWeapon->m_nCritSeedRequests()).c_str());
			H::Draw.String(fFont, x, y + fFont.m_nTall * 7, {}, align, std::format("CritBanned: {}, DamageTilUnban: {}", m_bCritBanned, m_iDamageTilUnban).c_str());
			H::Draw.String(fFont, x, y + fFont.m_nTall * 8, {}, align, std::format("CritChance: {:.2f} ({:.2f})", m_flCritChance, m_flCritChance - 0.1f).c_str());
			H::Draw.String(fFont, x, y + fFont.m_nTall * 9, {}, align, std::format("Force: {}, Skip: {}", m_mStorage[iSlot].m_vCritCommands.size(), m_mStorage[iSlot].m_vSkipCommands.size()).c_str());
		}
	}
}