#include "CritHack.h"

#include "../Aimbot/AutoRocketJump/AutoRocketJump.h"

#define WEAPON_RANDOM_RANGE				10000
#define TF_DAMAGE_CRIT_MULTIPLIER		3.0f
#define TF_DAMAGE_CRIT_CHANCE			0.02f
#define TF_DAMAGE_CRIT_CHANCE_RAPID		0.02f
#define TF_DAMAGE_CRIT_CHANCE_MELEE		0.15f
#define TF_DAMAGE_CRIT_DURATION_RAPID	2.0f

void CCritHack::Fill(const CUserCmd* pCmd, int n)
{
	static int iStart = pCmd->command_number;

	for (auto& [iSlot, tStorage] : Storage)
	{
		for (auto it = tStorage.CritCommands.begin(); it != tStorage.CritCommands.end();)
		{
			if (*it <= pCmd->command_number)
				it = tStorage.CritCommands.erase(it);
			else
				++it;
		}
		for (auto it = tStorage.SkipCommands.begin(); it != tStorage.SkipCommands.end();)
		{
			if (*it <= pCmd->command_number)
				it = tStorage.SkipCommands.erase(it);
			else
				++it;
		}

		for (int i = 0; i < n; i++)
		{
			if (tStorage.CritCommands.size() >= unsigned(n))
				break;

			const int iCmdNum = iStart + i;
			if (IsCritCommand(iSlot, tStorage.EntIndex, iCmdNum))
				tStorage.CritCommands.push_back(iCmdNum);
		}
		for (int i = 0; i < n; i++)
		{
			if (tStorage.SkipCommands.size() >= unsigned(n))
				break;

			const int iCmdNum = iStart + i;
			if (IsCritCommand(iSlot, tStorage.EntIndex, iCmdNum, false))
				tStorage.SkipCommands.push_back(iCmdNum);
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
		const int iRange = (CritChance - 0.1f) * WEAPON_RANDOM_RANGE;
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
	const int iSlot = pWeapon->m_iSlot();

	static float flOldBucket = 0.f; static int iOldID = 0, iOldCritChecks = 0, iOldCritSeedRequests = 0;
	const float flBucket = pWeapon->m_flCritTokenBucket(); const int iID = pWeapon->m_iWeaponID(), iCritChecks = pWeapon->m_nCritChecks(), iCritSeedRequests = pWeapon->m_nCritSeedRequests();
	const bool bMatch = Storage[iSlot].Damage > 0 && flOldBucket == flBucket && iOldID == iID && iOldCritChecks == iCritChecks && iOldCritSeedRequests == iCritSeedRequests;

	auto tfWeaponInfo = pWeapon->GetWeaponInfo();
	if (bMatch || !tfWeaponInfo)
		return;
	flOldBucket = flBucket; iOldID = iID, iOldCritChecks = iCritChecks, iOldCritSeedRequests = iCritSeedRequests;

	static auto bucketCap = U::ConVars.FindVar("tf_weapon_criticals_bucket_cap");
	const float flBucketCap = bucketCap ? bucketCap->GetFloat() : 1000.f;

	auto& tWeaponData = tfWeaponInfo->GetWeaponData(0);

	float flDamage = tWeaponData.m_nDamage;
	flDamage = SDK::AttribHookValue(flDamage, "mult_dmg", pWeapon);
	int nProjectilesPerShot = tWeaponData.m_nBulletsPerShot;
	if (nProjectilesPerShot >= 1)
		nProjectilesPerShot = SDK::AttribHookValue(nProjectilesPerShot, "mult_bullets_per_shot", pWeapon);
	else
		nProjectilesPerShot = 1;
	Storage[iSlot].Damage = flDamage *= nProjectilesPerShot;

	if (pWeapon->IsStreamingWeapon())
	{
		flDamage *= TF_DAMAGE_CRIT_DURATION_RAPID / tWeaponData.m_flTimeFireDelay;
		if (flDamage * TF_DAMAGE_CRIT_MULTIPLIER > flBucketCap)
			flDamage = flBucketCap / TF_DAMAGE_CRIT_MULTIPLIER;
	}

	float flMult = iSlot == SLOT_MELEE ? 0.5f : Math::RemapValClamped(float(iCritSeedRequests + 1) / (iCritChecks + 1), 0.1f, 1.f, 1.f, 3.f);
	Storage[iSlot].Cost = flDamage * TF_DAMAGE_CRIT_MULTIPLIER * flMult;

	if (flBucketCap)
		Storage[iSlot].PotentialCrits = (flBucketCap - Storage[iSlot].Damage) / (3 * flDamage / (iSlot == SLOT_MELEE ? 2 : 1) - Storage[iSlot].Damage);

	int iCrits = 0;
	{
		int shots = iCritChecks, crits = iCritSeedRequests;
		float bucket = flBucket, flCost = flDamage * TF_DAMAGE_CRIT_MULTIPLIER;
		const int iAttempts = std::min(Storage[iSlot].PotentialCrits + 1, 100);
		for (int i = 0; i < iAttempts; i++)
		{
			shots++; crits++;

			flMult = iSlot == SLOT_MELEE ? 0.5f : Math::RemapValClamped(float(crits) / shots, 0.1f, 1.f, 1.f, 3.f);
			bucket = std::min(bucket + Storage[iSlot].Damage, flBucketCap) - flCost * flMult;
			if (bucket < 0.f)
				break;

			iCrits++;
		}
	}

	if (!iCrits)
	{
		int shots = iCritChecks + 1, crits = iCritSeedRequests + 1;
		float bucket = std::min(flBucket + Storage[iSlot].Damage, flBucketCap), flCost = flDamage * TF_DAMAGE_CRIT_MULTIPLIER;
		for (int i = 0; i < 100; i++)
		{
			iCrits--;
			if (!pWeapon->IsStreamingWeapon() || !(i % int(tWeaponData.m_flTimeFireDelay / TICK_INTERVAL)))
				shots++;

			flMult = iSlot == SLOT_MELEE ? 0.5f : Math::RemapValClamped(float(crits) / shots, 0.1f, 1.f, 1.f, 3.f);
			bucket = std::min(bucket + Storage[iSlot].Damage, flBucketCap);
			if (bucket >= flCost * flMult)
				break;
		}
	}

	Storage[iSlot].AvailableCrits = iCrits;
}

void CCritHack::CanFireCritical(CTFPlayer* pLocal, CTFWeaponBase* pWeapon)
{
	CritBanned = false;
	DamageTilUnban = 0;

	if (pWeapon->m_iSlot() == SLOT_MELEE)
		CritChance = TF_DAMAGE_CRIT_CHANCE_MELEE * pLocal->GetCritMult();
	else if (pWeapon->IsStreamingWeapon())
	{
		CritChance = TF_DAMAGE_CRIT_CHANCE_RAPID * pLocal->GetCritMult();
		float flNonCritDuration = (TF_DAMAGE_CRIT_DURATION_RAPID / CritChance) - TF_DAMAGE_CRIT_DURATION_RAPID;
		CritChance = 1.f / flNonCritDuration;
	}
	else
		CritChance = TF_DAMAGE_CRIT_CHANCE * pLocal->GetCritMult();
	CritChance = SDK::AttribHookValue(CritChance, "mult_crit_chance", pWeapon) + 0.1f;

	if (!AllDamage || !CritDamage || pWeapon->m_iSlot() == SLOT_MELEE)
		return;

	const float flNormalizedDamage = (float)CritDamage / TF_DAMAGE_CRIT_MULTIPLIER;
	const float flObservedCritChance = flNormalizedDamage / (flNormalizedDamage + AllDamage - CritDamage);
	if (CritBanned = flObservedCritChance > CritChance)
		DamageTilUnban = flNormalizedDamage / CritChance + CritDamage - flNormalizedDamage - AllDamage;
}

bool CCritHack::WeaponCanCrit(CTFWeaponBase* pWeapon)
{
	static auto tf_weapon_criticals = U::ConVars.FindVar("tf_weapon_criticals");
	if (!tf_weapon_criticals || !tf_weapon_criticals->GetBool())
		return false;

	if (SDK::AttribHookValue(1.f, "mult_crit_chance", pWeapon) <= 0.f)
		return false;

	switch (pWeapon->m_iWeaponID())
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
		if (SDK::HandleToIDX(unsigned(hWeapons[i])) < 0 || SDK::HandleToIDX(unsigned(hWeapons[i])) >= 2048)
			continue;
		
		if (const auto pWeapon = I::ClientEntityList->GetClientEntityFromHandle(unsigned(hWeapons[i]))->As<CTFWeaponBase>())
		{
			const int iSlot = pWeapon->m_iSlot();
			const int iDefIndex = pWeapon->m_iItemDefinitionIndex();
			mWeapons[iSlot] = true;

			if (Storage[iSlot].DefIndex == iDefIndex)
				continue;

			Storage[iSlot] = {};
			Storage[iSlot].EntIndex = pWeapon->entindex();
			Storage[iSlot].DefIndex = iDefIndex;

			SDK::Output("Crithack", std::format("Resetting weapon {}", iDefIndex).c_str(), { 0, 255, 255, 255 }, Vars::Debug::Logging.Value);
		}
	}

	for (auto& [iSlot, _] : Storage)
	{
		if (!mWeapons[iSlot])
			Storage[iSlot] = {};
	}
}

void CCritHack::Reset()
{
	Storage = {};

	CritDamage = 0;
	AllDamage = 0;

	CritBanned = false;
	DamageTilUnban = 0;
	CritChance = 0.f;

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

	if (pWeapon->m_iWeaponID() == TF_WEAPON_MINIGUN && pCmd->buttons & IN_ATTACK)
		pCmd->buttons &= ~IN_ATTACK2;

	bool bAttacking = G::IsAttacking;
	if (G::WeaponType == EWeaponType::MELEE)
	{
		bAttacking = G::CanPrimaryAttack && pCmd->buttons & IN_ATTACK;
		if (!bAttacking && pWeapon->m_iWeaponID() == TF_WEAPON_FISTS)
			bAttacking = G::CanPrimaryAttack && pCmd->buttons & IN_ATTACK2;
	}
	if (pWeapon->m_iWeaponID() == TF_WEAPON_PIPEBOMBLAUNCHER || pWeapon->m_iWeaponID() == TF_WEAPON_STICKY_BALL_LAUNCHER || pWeapon->m_iWeaponID() == TF_WEAPON_GRENADE_STICKY_BALL
		|| pWeapon->m_iWeaponID() == TF_WEAPON_CANNON)
	{
		static float flBegin = -1.f;
		if (pCmd->buttons& IN_ATTACK&& flBegin < 0.f && G::CanPrimaryAttack)
			flBegin = I::GlobalVars->curtime;

		const float flCharge = flBegin > 0.f ? I::GlobalVars->curtime - flBegin : 0.f;
		const float flAmount = pWeapon->m_iWeaponID() != TF_WEAPON_CANNON
			? Math::RemapValClamped(flCharge, 0.f, SDK::AttribHookValue(4.f, "stickybomb_charge_rate", pWeapon), 0.f, 1.f)
			: Math::RemapValClamped(flCharge, 0.f, SDK::AttribHookValue(0.f, "grenade_launcher_mortar_mode", pWeapon), 0.f, 1.f);

		const bool bUnheld = !(pCmd->buttons & IN_ATTACK) && flAmount > 0.f;
		const bool bSwapping = pCmd->weaponselect;
		const bool bFull = flAmount == 1.f; // possibly add exception to skip when full with cannon

		bAttacking = (bUnheld || bFull) && !bSwapping;
		if (bAttacking || !G::CanPrimaryAttack || bSwapping)
			flBegin = -1.f;
	}
	if ((pWeapon->m_iWeaponID() == TF_WEAPON_MINIGUN || pWeapon->m_iWeaponID() == TF_WEAPON_FLAMETHROWER) && !(G::LastUserCmd->buttons & IN_ATTACK)) // silly
		bAttacking = false;

	const int iSlot = pWeapon->m_iSlot();
	if (Storage[iSlot].StreamWait <= I::GlobalVars->tickcount - 1)
		Storage[pWeapon->m_iSlot()].StreamWait = -1;

	const bool bRapidFire = pWeapon->IsStreamingWeapon();
	const bool bStreamWait = Storage[iSlot].StreamWait > 0;
	//const bool bStreamWait = pWeapon->IsStreamingWeapon() && pWeapon->LastRapidfireCritCheckTime() + 1.f > TICK_INTERVAL * pLocal->m_nTickBase();

	const int closestCrit = !Storage[iSlot].CritCommands.empty() ? Storage[iSlot].CritCommands.front() : 0;
	const int closestSkip = !Storage[iSlot].SkipCommands.empty() ? Storage[iSlot].SkipCommands.front() : 0;

	static bool bFirstTimePredicted = false;
	if (!I::ClientState->chokedcommands)
		bFirstTimePredicted = false;
	if (bAttacking && !pWeapon->IsInReload() && !bFirstTimePredicted)
	{
		bFirstTimePredicted = true;

		const bool bCanCrit = Storage[pWeapon->m_iSlot()].AvailableCrits > 0 && (!CritBanned || pWeapon->m_iSlot() == SLOT_MELEE) && !bStreamWait;
		const bool bPressed = Vars::CritHack::ForceCrits.Value || pWeapon->m_iSlot() == SLOT_MELEE && Vars::CritHack::AlwaysMelee.Value;
		if (bCanCrit && bPressed && closestCrit)
			pCmd->command_number = closestCrit;
		else if (Vars::CritHack::AvoidRandom.Value && closestSkip)
			pCmd->command_number = closestSkip;

		if (bRapidFire && !bStreamWait)
			Storage[pWeapon->m_iSlot()].StreamWait = I::GlobalVars->tickcount + 1 / TICK_INTERVAL;
	}
	//else if (Vars::CritHack::AvoidRandom.Value && closestSkip)
	//	pCmd->command_number = closestSkip;

	//if (pCmd->command_number == closestCrit || pCmd->command_number == closestSkip)
	WishRandomSeed = MD5_PseudoRandom(pCmd->command_number) & 0x7FFFFFFF;

	if (pCmd->command_number == closestCrit)
		Storage[iSlot].CritCommands.pop_front();
	else if (pCmd->command_number == closestSkip)
		Storage[iSlot].SkipCommands.pop_front();
}

bool CCritHack::CalcIsAttackCriticalHandler(CTFPlayer* pLocal, CTFWeaponBase* pWeapon)
{
	if (!I::Prediction->m_bFirstTimePredicted || !pLocal || !pWeapon)
		return false;

	if (pWeapon->m_iWeaponID() == TF_WEAPON_MINIGUN || pWeapon->m_iWeaponID() == TF_WEAPON_FLAMETHROWER)
	{
		static int iOldAmmo = pLocal->GetAmmoCount(pWeapon->m_iPrimaryAmmoType());
		const int iNewAmmo = pLocal->GetAmmoCount(pWeapon->m_iPrimaryAmmoType());

		const bool bFired = iOldAmmo != iNewAmmo;

		iOldAmmo = iNewAmmo;

		if (!bFired)
			return false;
	}

	if (WishRandomSeed)
	{
		*G::RandomSeed() = WishRandomSeed;
		WishRandomSeed = 0;
	}

	return true;
}

void CCritHack::Event(IGameEvent* pEvent, FNV1A_t uHash, CTFPlayer* pLocal)
{
	switch (uHash)
	{
	case FNV1A::HashConst("player_hurt"):
	{
		auto pWeapon = H::Entities.GetWeapon();
		if (!pLocal || !pWeapon)
			return;

		const int iVictim = I::EngineClient->GetPlayerForUserID(pEvent->GetInt("userid"));
		const int iAttacker = I::EngineClient->GetPlayerForUserID(pEvent->GetInt("attacker"));

		const bool bCrit = pEvent->GetBool("crit") || pEvent->GetBool("minicrit");
		int iDamage = pEvent->GetInt("damageamount");
		if (mHealthStorage.contains(iVictim))
			iDamage = std::min(iDamage, mHealthStorage[iVictim]);
		const auto iWeaponID = pEvent->GetInt("weaponid");

		if (iVictim == iAttacker || iAttacker != pLocal->entindex() || iWeaponID != pWeapon->m_iWeaponID() || pWeapon->m_iSlot() == SLOT_MELEE) // weapon id stuff is dumb simplification
			return;

		AllDamage += iDamage;
		if (bCrit && !pLocal->IsCritBoosted())
			CritDamage += iDamage;

		return;
	}
	case FNV1A::HashConst("teamplay_round_start"):
		AllDamage = CritDamage = 0;
		return;
	case FNV1A::HashConst("client_beginconnect"):
	case FNV1A::HashConst("client_disconnect"):
	case FNV1A::HashConst("game_newmap"):
		Reset();
	}
}

void CCritHack::Store()
{
	auto pResource = H::Entities.GetPR();
	if (!pResource)
		return;

	for (auto it = mHealthStorage.begin(); it != mHealthStorage.end();)
	{
		if (I::ClientEntityList->GetClientEntity(it->first))
			++it;
		else
			it = mHealthStorage.erase(it);
	}
	for (auto& pEntity : H::Entities.GetGroup(EGroupType::PLAYERS_ALL))
	{
		auto pPlayer = pEntity->As<CTFPlayer>();
		if (pPlayer->IsAlive() && !pPlayer->IsAGhost())
			mHealthStorage[pPlayer->entindex()] = pPlayer->IsDormant() ? pResource->GetHealth(pPlayer->entindex()) : pPlayer->m_iHealth();
	}
}

void CCritHack::Draw(CTFPlayer* pLocal)
{
	static auto tf_weapon_criticals = U::ConVars.FindVar("tf_weapon_criticals");
	const bool bWeaponCriticals = tf_weapon_criticals ? tf_weapon_criticals->GetBool() : true;
	if (!(Vars::Menu::Indicators.Value & (1 << 1)) || !I::EngineClient->IsInGame() || !bWeaponCriticals || !G::CurrentUserCmd)
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
		const auto iSlot = pWeapon->m_iSlot();
		const auto bRapidFire = pWeapon->IsStreamingWeapon();

		if (Storage[iSlot].Damage > 0)
		{
			if (pLocal->IsCritBoosted())
				H::Draw.String(fFont, x, y, { 100, 255, 255, 255 }, align, "Crit Boosted");
			else if (pWeapon->m_flCritTime() > TICK_INTERVAL * pLocal->m_nTickBase())
			{
				const float flTime = pWeapon->m_flCritTime() - TICK_INTERVAL * pLocal->m_nTickBase();
				H::Draw.String(fFont, x, y, { 100, 255, 255, 255 }, align, std::format("Streaming crits {:.1f}s", flTime).c_str());
			}
			else if (!CritBanned)
			{
				if (Storage[iSlot].AvailableCrits > 0)
				{
					if (bRapidFire && Storage[iSlot].StreamWait > 0)
					{
						//const float flTime = std::max((TICKS_TO_TIME(Storage[iSlot].StreamWait - pLocal->m_nTickBase())), 0.f);
						const float flTime = pWeapon->m_flLastRapidFireCritCheckTime() + 1.f - TICK_INTERVAL * pLocal->m_nTickBase();
						H::Draw.String(fFont, x, y, Vars::Menu::Theme::Active.Value, align, std::format("Wait {:.1f}s", flTime).c_str());
					}
					else
						H::Draw.String(fFont, x, y, { 150, 255, 150, 255 }, align, "Crit Ready");
				}
				else
				{
					const int shots = -Storage[iSlot].AvailableCrits;
					H::Draw.String(fFont, x, y, { 255, 150, 150, 255 }, align, shots == 1 ? std::format("Crit in {} shot", shots).c_str() : std::format("Crit in {}{} shots", shots, shots == 100 ? "+" : "").c_str());
				}
			}
			else
				H::Draw.String(fFont, x, y, { 255, 150, 150, 255 }, align, std::format("Deal {} damage", DamageTilUnban).c_str());

			H::Draw.String(fFont, x, y + fFont.m_nTall + 1, Vars::Menu::Theme::Active.Value, align, std::format("{} / {} potential crits", std::max(Storage[iSlot].AvailableCrits, 0), Storage[iSlot].PotentialCrits).c_str());
		}
		else
			H::Draw.String(fFont, x, y, Vars::Menu::Theme::Active.Value, align, "Calculating");

		if (Vars::Debug::Info.Value)
		{
			H::Draw.String(fFont, x, y + fFont.m_nTall * 3, { 255, 255, 255, 255 }, align, std::format("AllDamage: {}, CritDamage: {}", AllDamage, CritDamage).c_str());
			H::Draw.String(fFont, x, y + fFont.m_nTall * 4, { 255, 255, 255, 255 }, align, std::format("Bucket: {}", pWeapon->m_flCritTokenBucket()).c_str());
			H::Draw.String(fFont, x, y + fFont.m_nTall * 5, { 255, 255, 255, 255 }, align, std::format("Damage: {}, Cost: {}", Storage[iSlot].Damage, Storage[iSlot].Cost).c_str());
			H::Draw.String(fFont, x, y + fFont.m_nTall * 6, { 255, 255, 255, 255 }, align, std::format("Shots: {}, Crits: {}", pWeapon->m_nCritChecks(), pWeapon->m_nCritSeedRequests()).c_str());
			H::Draw.String(fFont, x, y + fFont.m_nTall * 7, { 255, 255, 255, 255 }, align, std::format("CritBanned: {}, DamageTilUnban: {}", CritBanned, DamageTilUnban).c_str());
			H::Draw.String(fFont, x, y + fFont.m_nTall * 8, { 255, 255, 255, 255 }, align, std::format("CritChance: {:.2f} ({:.2f})", CritChance, CritChance - 0.1f).c_str());
			H::Draw.String(fFont, x, y + fFont.m_nTall * 9, { 255, 255, 255, 255 }, align, std::format("Force: {}, Skip: {}", Storage[iSlot].CritCommands.size(), Storage[iSlot].SkipCommands.size()).c_str());
		}
	}
}