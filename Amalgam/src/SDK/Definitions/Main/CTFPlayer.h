#pragma once
#include "CBasePlayer.h"
#include "CMultiPlayerAnimState.h"
#include "../../../Utils/Signatures/Signatures.h"

MAKE_SIGNATURE(CTFPlayer_IsPlayerOnSteamFriendsList, "client.dll", "40 57 48 81 EC ? ? ? ? 48 8B FA E8", 0x0);
MAKE_SIGNATURE(TeamFortress_CalculateMaxSpeed, "client.dll", "88 54 24 ? 53 55", 0x0);
MAKE_SIGNATURE(CTFPlayer_UpdateClientSideAnimation, "client.dll", "48 89 5C 24 ? 57 48 83 EC ? 48 8B D9 E8 ? ? ? ? 48 8B F8 48 85 C0 74 ? 48 8B 00 48 8B CF FF 90 ? ? ? ? 84 C0 75 ? 33 FF 48 3B DF", 0x0);
MAKE_SIGNATURE(CTFPlayer_UpdateWearables, "client.dll", "40 53 48 83 EC ? 48 8B D9 E8 ? ? ? ? 48 8B 03 48 8B CB FF 90 ? ? ? ? 4C 8D 0D ? ? ? ? C7 44 24 ? ? ? ? ? 48 8B C8 4C 8D 05 ? ? ? ? 33 D2 E8 ? ? ? ? 48 85 C0", 0x0);

class CTFPlayer : public CBasePlayer
{
public:
	NETVAR(m_bSaveMeParity, bool, "CTFPlayer", "m_bSaveMeParity");
	NETVAR(m_bIsMiniBoss, bool, "CTFPlayer", "m_bIsMiniBoss");
	NETVAR(m_bIsABot, bool, "CTFPlayer", "m_bIsABot");
	NETVAR(m_nBotSkill, int, "CTFPlayer", "m_nBotSkill");
	NETVAR(m_nWaterLevel, byte, "CTFPlayer", "m_nWaterLevel");
	NETVAR(m_hRagdoll, EHANDLE, "CTFPlayer", "m_hRagdoll");
	NETVAR(m_PlayerClass, void*, "CTFPlayer", "m_PlayerClass");
	NETVAR(m_iClass, int, "CTFPlayer", "m_iClass");
	NETVAR(m_iszClassIcon, const char*, "CTFPlayer", "m_iszClassIcon");
	NETVAR(m_iszCustomModel, const char*, "CTFPlayer", "m_iszCustomModel");
	NETVAR(m_vecCustomModelOffset, Vec3, "CTFPlayer", "m_vecCustomModelOffset");
	NETVAR(m_angCustomModelRotation, Vec3, "CTFPlayer", "m_angCustomModelRotation");
	NETVAR(m_bCustomModelRotates, bool, "CTFPlayer", "m_bCustomModelRotates");
	NETVAR(m_bCustomModelRotationSet, bool, "CTFPlayer", "m_bCustomModelRotationSet");
	NETVAR(m_bCustomModelVisibleToSelf, bool, "CTFPlayer", "m_bCustomModelVisibleToSelf");
	NETVAR(m_bUseClassAnimations, bool, "CTFPlayer", "m_bUseClassAnimations");
	NETVAR(m_iClassModelParity, int, "CTFPlayer", "m_iClassModelParity");
	//NETVAR(m_Shared, void*, "CTFPlayer", "m_Shared");
	NETVAR(m_nPlayerCond, int, "CTFPlayer", "m_nPlayerCond");
	NETVAR(m_bJumping, bool, "CTFPlayer", "m_bJumping");
	NETVAR(m_nNumHealers, int, "CTFPlayer", "m_nNumHealers");
	NETVAR(m_iCritMult, int, "CTFPlayer", "m_iCritMult");
	NETVAR(m_iAirDash, int, "CTFPlayer", "m_iAirDash");
	NETVAR(m_nAirDucked, int, "CTFPlayer", "m_nAirDucked");
	NETVAR(m_flDuckTimer, float, "CTFPlayer", "m_flDuckTimer");
	NETVAR(m_nPlayerState, int, "CTFPlayer", "m_nPlayerState");
	NETVAR(m_iDesiredPlayerClass, int, "CTFPlayer", "m_iDesiredPlayerClass");
	NETVAR(m_flMovementStunTime, float, "CTFPlayer", "m_flMovementStunTime");
	NETVAR(m_iMovementStunAmount, int, "CTFPlayer", "m_iMovementStunAmount");
	NETVAR(m_iMovementStunParity, int, "CTFPlayer", "m_iMovementStunParity");
	NETVAR(m_hStunner, EHANDLE, "CTFPlayer", "m_hStunner");
	NETVAR(m_iStunFlags, int, "CTFPlayer", "m_iStunFlags");
	NETVAR(m_nArenaNumChanges, int, "CTFPlayer", "m_nArenaNumChanges");
	NETVAR(m_bArenaFirstBloodBoost, bool, "CTFPlayer", "m_bArenaFirstBloodBoost");
	NETVAR(m_iWeaponKnockbackID, int, "CTFPlayer", "m_iWeaponKnockbackID");
	NETVAR(m_bLoadoutUnavailable, bool, "CTFPlayer", "m_bLoadoutUnavailable");
	NETVAR(m_iItemFindBonus, int, "CTFPlayer", "m_iItemFindBonus");
	NETVAR(m_bShieldEquipped, bool, "CTFPlayer", "m_bShieldEquipped");
	NETVAR(m_bParachuteEquipped, bool, "CTFPlayer", "m_bParachuteEquipped");
	NETVAR(m_iNextMeleeCrit, int, "CTFPlayer", "m_iNextMeleeCrit");
	NETVAR(m_iDecapitations, int, "CTFPlayer", "m_iDecapitations");
	NETVAR(m_iRevengeCrits, int, "CTFPlayer", "m_iRevengeCrits");
	NETVAR(m_iDisguiseBody, int, "CTFPlayer", "m_iDisguiseBody");
	NETVAR(m_hCarriedObject, EHANDLE, "CTFPlayer", "m_hCarriedObject");
	NETVAR(m_bCarryingObject, bool, "CTFPlayer", "m_bCarryingObject");
	NETVAR(m_flNextNoiseMakerTime, float, "CTFPlayer", "m_flNextNoiseMakerTime");
	NETVAR(m_iSpawnRoomTouchCount, int, "CTFPlayer", "m_iSpawnRoomTouchCount");
	NETVAR(m_iKillCountSinceLastDeploy, int, "CTFPlayer", "m_iKillCountSinceLastDeploy");
	NETVAR(m_flFirstPrimaryAttack, float, "CTFPlayer", "m_flFirstPrimaryAttack");
	NETVAR(m_flEnergyDrinkMeter, float, "CTFPlayer", "m_flEnergyDrinkMeter");
	NETVAR(m_flHypeMeter, float, "CTFPlayer", "m_flHypeMeter");
	NETVAR(m_flChargeMeter, float, "CTFPlayer", "m_flChargeMeter");
	NETVAR(m_flInvisChangeCompleteTime, float, "CTFPlayer", "m_flInvisChangeCompleteTime");
	NETVAR(m_nDisguiseTeam, int, "CTFPlayer", "m_nDisguiseTeam");
	NETVAR(m_nDisguiseClass, int, "CTFPlayer", "m_nDisguiseClass");
	NETVAR(m_nDisguiseSkinOverride, int, "CTFPlayer", "m_nDisguiseSkinOverride");
	NETVAR(m_nMaskClass, int, "CTFPlayer", "m_nMaskClass");
	NETVAR(m_iDisguiseTargetIndex, int, "CTFPlayer", "m_iDisguiseTargetIndex");
	NETVAR(m_iDisguiseHealth, int, "CTFPlayer", "m_iDisguiseHealth");
	NETVAR(m_bFeignDeathReady, bool, "CTFPlayer", "m_bFeignDeathReady");
	NETVAR(m_hDisguiseWeapon, EHANDLE, "CTFPlayer", "m_hDisguiseWeapon");
	NETVAR(m_nTeamTeleporterUsed, int, "CTFPlayer", "m_nTeamTeleporterUsed");
	NETVAR(m_flCloakMeter, float, "CTFPlayer", "m_flCloakMeter");
	NETVAR(m_flSpyTranqBuffDuration, float, "CTFPlayer", "m_flSpyTranqBuffDuration");
	NETVAR(m_nDesiredDisguiseTeam, int, "CTFPlayer", "m_nDesiredDisguiseTeam");
	NETVAR(m_nDesiredDisguiseClass, int, "CTFPlayer", "m_nDesiredDisguiseClass");
	NETVAR(m_flStealthNoAttackExpire, float, "CTFPlayer", "m_flStealthNoAttackExpire");
	NETVAR(m_flStealthNextChangeTime, float, "CTFPlayer", "m_flStealthNextChangeTime");
	NETVAR(m_bLastDisguisedAsOwnTeam, bool, "CTFPlayer", "m_bLastDisguisedAsOwnTeam");
	NETVAR(m_flRageMeter, float, "CTFPlayer", "m_flRageMeter");
	NETVAR(m_bRageDraining, bool, "CTFPlayer", "m_bRageDraining");
	NETVAR(m_flNextRageEarnTime, float, "CTFPlayer", "m_flNextRageEarnTime");
	NETVAR(m_bInUpgradeZone, bool, "CTFPlayer", "m_bInUpgradeZone");
	NETVAR(m_flItemChargeMeter, void*, "CTFPlayer", "m_flItemChargeMeter");
	NETVAR(m_bPlayerDominated, void*, "CTFPlayer", "m_bPlayerDominated");
	NETVAR(m_bPlayerDominatingMe, void*, "CTFPlayer", "m_bPlayerDominatingMe");
	NETVAR(m_ScoreData, void*, "CTFPlayer", "m_ScoreData");
	NETVAR(m_iCaptures, int, "CTFPlayer", "m_iCaptures");
	NETVAR(m_iDefenses, int, "CTFPlayer", "m_iDefenses");
	NETVAR(m_iKills, int, "CTFPlayer", "m_iKills");
	NETVAR(m_iDeaths, int, "CTFPlayer", "m_iDeaths");
	NETVAR(m_iSuicides, int, "CTFPlayer", "m_iSuicides");
	NETVAR(m_iDominations, int, "CTFPlayer", "m_iDominations");
	NETVAR(m_iRevenge, int, "CTFPlayer", "m_iRevenge");
	NETVAR(m_iBuildingsBuilt, int, "CTFPlayer", "m_iBuildingsBuilt");
	NETVAR(m_iBuildingsDestroyed, int, "CTFPlayer", "m_iBuildingsDestroyed");
	NETVAR(m_iHeadshots, int, "CTFPlayer", "m_iHeadshots");
	NETVAR(m_iBackstabs, int, "CTFPlayer", "m_iBackstabs");
	NETVAR(m_iHealPoints, int, "CTFPlayer", "m_iHealPoints");
	NETVAR(m_iInvulns, int, "CTFPlayer", "m_iInvulns");
	NETVAR(m_iTeleports, int, "CTFPlayer", "m_iTeleports");
	NETVAR(m_iResupplyPoints, int, "CTFPlayer", "m_iResupplyPoints");
	NETVAR(m_iKillAssists, int, "CTFPlayer", "m_iKillAssists");
	NETVAR(m_iPoints, int, "CTFPlayer", "m_iPoints");
	NETVAR(m_iBonusPoints, int, "CTFPlayer", "m_iBonusPoints");
	NETVAR(m_iDamageDone, int, "CTFPlayer", "m_iDamageDone");
	NETVAR(m_iCrits, int, "CTFPlayer", "m_iCrits");
	NETVAR(m_RoundScoreData, void*, "CTFPlayer", "m_RoundScoreData");
	NETVAR(m_ConditionList, void*, "CTFPlayer", "m_ConditionList");
	NETVAR(_condition_bits, int, "CTFPlayer", "_condition_bits");
	NETVAR(m_iTauntIndex, int, "CTFPlayer", "m_iTauntIndex");
	NETVAR(m_iTauntConcept, int, "CTFPlayer", "m_iTauntConcept");
	NETVAR(m_nPlayerCondEx, int, "CTFPlayer", "m_nPlayerCondEx");
	NETVAR(m_iStunIndex, int, "CTFPlayer", "m_iStunIndex");
	NETVAR(m_nHalloweenBombHeadStage, int, "CTFPlayer", "m_nHalloweenBombHeadStage");
	NETVAR(m_nPlayerCondEx2, int, "CTFPlayer", "m_nPlayerCondEx2");
	NETVAR(m_nPlayerCondEx3, int, "CTFPlayer", "m_nPlayerCondEx3");
	NETVAR(m_nStreaks, void*, "CTFPlayer", "m_nStreaks");
	NETVAR(m_unTauntSourceItemID_Low, int, "CTFPlayer", "m_unTauntSourceItemID_Low");
	NETVAR(m_unTauntSourceItemID_High, int, "CTFPlayer", "m_unTauntSourceItemID_High");
	NETVAR(m_flRuneCharge, float, "CTFPlayer", "m_flRuneCharge");
	NETVAR(m_bHasPasstimeBall, bool, "CTFPlayer", "m_bHasPasstimeBall");
	NETVAR(m_bIsTargetedForPasstimePass, bool, "CTFPlayer", "m_bIsTargetedForPasstimePass");
	NETVAR(m_hPasstimePassTarget, EHANDLE, "CTFPlayer", "m_hPasstimePassTarget");
	NETVAR(m_askForBallTime, float, "CTFPlayer", "m_askForBallTime");
	NETVAR(m_bKingRuneBuffActive, bool, "CTFPlayer", "m_bKingRuneBuffActive");
	NETVAR(m_nPlayerCondEx4, int, "CTFPlayer", "m_nPlayerCondEx4");
	NETVAR(m_flHolsterAnimTime, float, "CTFPlayer", "m_flHolsterAnimTime");
	NETVAR(m_hSwitchTo, EHANDLE, "CTFPlayer", "m_hSwitchTo");
	NETVAR(m_hItem, EHANDLE, "CTFPlayer", "m_hItem");
	NETVAR(m_bIsCoaching, bool, "CTFPlayer", "m_bIsCoaching");
	NETVAR(m_hCoach, EHANDLE, "CTFPlayer", "m_hCoach");
	NETVAR(m_hStudent, EHANDLE, "CTFPlayer", "m_hStudent");
	NETVAR(m_nCurrency, int, "CTFPlayer", "m_nCurrency");
	NETVAR(m_nExperienceLevel, int, "CTFPlayer", "m_nExperienceLevel");
	NETVAR(m_nExperienceLevelProgress, int, "CTFPlayer", "m_nExperienceLevelProgress");
	NETVAR(m_bMatchSafeToLeave, bool, "CTFPlayer", "m_bMatchSafeToLeave");
	NETVAR(m_vecOrigin, Vec3, "CTFPlayer", "m_vecOrigin");
	NETVAR(m_angEyeAnglesX, float, "CTFPlayer", "m_angEyeAngles[0]");
	NETVAR(m_angEyeAnglesY, float, "CTFPlayer", "m_angEyeAngles[1]");
	NETVAR(m_bAllowMoveDuringTaunt, bool, "CTFPlayer", "m_bAllowMoveDuringTaunt");
	NETVAR(m_bIsReadyToHighFive, bool, "CTFPlayer", "m_bIsReadyToHighFive");
	NETVAR(m_hHighFivePartner, EHANDLE, "CTFPlayer", "m_hHighFivePartner");
	NETVAR(m_nForceTauntCam, int, "CTFPlayer", "m_nForceTauntCam");
	NETVAR(m_flTauntYaw, float, "CTFPlayer", "m_flTauntYaw");
	NETVAR(m_nActiveTauntSlot, int, "CTFPlayer", "m_nActiveTauntSlot");
	NETVAR(m_iTauntItemDefIndex, int, "CTFPlayer", "m_iTauntItemDefIndex");
	NETVAR(m_flCurrentTauntMoveSpeed, float, "CTFPlayer", "m_flCurrentTauntMoveSpeed");
	NETVAR(m_flVehicleReverseTime, float, "CTFPlayer", "m_flVehicleReverseTime");
	NETVAR(m_flMvMLastDamageTime, float, "CTFPlayer", "m_flMvMLastDamageTime");
	NETVAR(m_flLastDamageTime, float, "CTFPlayer", "m_flLastDamageTime");
	NETVAR(m_bInPowerPlay, bool, "CTFPlayer", "m_bInPowerPlay");
	NETVAR(m_iSpawnCounter, int, "CTFPlayer", "m_iSpawnCounter");
	NETVAR(m_bArenaSpectator, bool, "CTFPlayer", "m_bArenaSpectator");
	NETVAR(m_AttributeManager, void*, "CTFPlayer", "m_AttributeManager");
	NETVAR(m_hOuter, EHANDLE, "CTFPlayer", "m_hOuter");
	NETVAR(m_ProviderType, int, "CTFPlayer", "m_ProviderType");
	NETVAR(m_iReapplyProvisionParity, int, "CTFPlayer", "m_iReapplyProvisionParity");
	NETVAR(m_flHeadScale, float, "CTFPlayer", "m_flHeadScale");
	NETVAR(m_flTorsoScale, float, "CTFPlayer", "m_flTorsoScale");
	NETVAR(m_flHandScale, float, "CTFPlayer", "m_flHandScale");
	NETVAR(m_bUseBossHealthBar, bool, "CTFPlayer", "m_bUseBossHealthBar");
	NETVAR(m_bUsingVRHeadset, bool, "CTFPlayer", "m_bUsingVRHeadset");
	NETVAR(m_bForcedSkin, bool, "CTFPlayer", "m_bForcedSkin");
	NETVAR(m_nForcedSkin, int, "CTFPlayer", "m_nForcedSkin");
	NETVAR(m_bGlowEnabled, bool, "CTFPlayer", "m_bGlowEnabled");
	NETVAR(m_nActiveWpnClip, int, "CTFPlayer", "m_nActiveWpnClip");
	NETVAR(m_flKartNextAvailableBoost, float, "CTFPlayer", "m_flKartNextAvailableBoost");
	NETVAR(m_iKartHealth, int, "CTFPlayer", "m_iKartHealth");
	NETVAR(m_iKartState, int, "CTFPlayer", "m_iKartState");
	NETVAR(m_hGrapplingHookTarget, EHANDLE, "CTFPlayer", "m_hGrapplingHookTarget");
	NETVAR(m_hSecondaryLastWeapon, EHANDLE, "CTFPlayer", "m_hSecondaryLastWeapon");
	NETVAR(m_bUsingActionSlot, bool, "CTFPlayer", "m_bUsingActionSlot");
	NETVAR(m_flInspectTime, float, "CTFPlayer", "m_flInspectTime");
	NETVAR(m_flHelpmeButtonPressTime, float, "CTFPlayer", "m_flHelpmeButtonPressTime");
	NETVAR(m_iCampaignMedals, int, "CTFPlayer", "m_iCampaignMedals");
	NETVAR(m_iPlayerSkinOverride, int, "CTFPlayer", "m_iPlayerSkinOverride");
	NETVAR(m_bViewingCYOAPDA, bool, "CTFPlayer", "m_bViewingCYOAPDA");
	inline void* m_Shared()
	{
		static int nOffset = U::NetVars.GetNetVar("CTFPlayer", "m_Shared");
		return reinterpret_cast<void*>(uintptr_t(this) + nOffset);
	};

	NETVAR_OFF(m_flGravity, float, "CTFPlayer", "m_nWaterLevel", -24);
	NETVAR_OFF(m_MoveType, byte, "CTFPlayer", "m_nWaterLevel", -4);
	NETVAR_OFF(m_MoveCollide, byte, "CTFPlayer", "m_nWaterLevel", -3);
	NETVAR_OFF(m_nWaterType, byte, "CTFPlayer", "m_nWaterLevel", 1);
	NETVAR_OFF(m_flInvisibility, float, "CTFPlayer", "m_flInvisChangeCompleteTime", -8);
	NETVAR_OFF(m_flTankPressure, float, "CTFPlayer", "m_Shared", 636);
	NETVAR_OFF(GetAnimState, CTFPlayerAnimState*, "CTFPlayer", "m_hItem", -88);
	NETVAR_OFF(m_flPrevTauntYaw, float, "CTFPlayer", "m_flTauntYaw", 4);
	NETVAR_OFF(m_flLastMovementStunChange, float, "CTFPlayer", "m_hItem", -180);
	NETVAR_OFF(m_flStunLerpTarget, float, "CTFPlayer", "m_hItem", -184);
	NETVAR_OFF(m_bStunNeedsFadeOut, bool, "CTFPlayer", "m_hItem", -188);
	NETVAR_OFF(m_bTauntForceMoveForward, bool, "CTFPlayer", "m_bAllowMoveDuringTaunt", 1);

	VIRTUAL(GetMaxHealth, int, void*, this, 107);

	inline Vec3 GetEyeAngles()
	{
		return { m_angEyeAnglesX(), m_angEyeAnglesY(), 0.f };
	}

	inline Vec3 GetViewOffset() // use on nonlocal players
	{
		if (!IsPlayer())
			return (m_vecMins() + m_vecMaxs()) / 2;

		auto getMainOffset = [this]() -> Vec3
			{
				if (IsDucking())
					return { 0.f, 0.f, 45.f };

				switch (m_iClass())
				{
				case TF_CLASS_SCOUT: return { 0.f, 0.f, 65.f };
				case TF_CLASS_SOLDIER: return { 0.f, 0.f, 68.f };
				case TF_CLASS_PYRO: return { 0.f, 0.f, 68.f };
				case TF_CLASS_DEMOMAN: return { 0.f, 0.f, 68.f };
				case TF_CLASS_HEAVY: return { 0.f, 0.f, 75.f };
				case TF_CLASS_ENGINEER: return { 0.f, 0.f, 68.f };
				case TF_CLASS_MEDIC: return { 0.f, 0.f, 75.f };
				case TF_CLASS_SNIPER: return { 0.f, 0.f, 75.f };
				case TF_CLASS_SPY: return { 0.f, 0.f, 75.f };
				}

				const Vec3 vOffset = m_vecViewOffset();
				if (vOffset.z)
					return vOffset;

				return { 0.f, 0.f, 68.f };
			};

		return getMainOffset() * m_flModelScale();
	}

	inline bool IsFriend()
	{
		return S::CTFPlayer_IsPlayerOnSteamFriendsList.Call<bool>(this, this);
	}

	inline bool InCond(const ETFCond cond)
	{
		const int iCond = static_cast<int>(cond);
		switch (iCond / 32)
		{
		case 0:
		{
			const int bit = (1 << iCond);
			if ((m_nPlayerCond() & bit) == bit || (_condition_bits() & bit) == bit)
				return true;
			break;
		}
		case 1:
		{
			const int bit = 1 << (iCond - 32);
			if ((m_nPlayerCondEx() & bit) == bit)
				return true;
			break;
		}
		case 2:
		{
			const int bit = 1 << (iCond - 64);
			if ((m_nPlayerCondEx2() & bit) == bit)
				return true;
			break;
		}
		case 3:
		{
			const int bit = 1 << (iCond - 96);
			if ((m_nPlayerCondEx3() & bit) == bit)
				return true;
			break;
		}
		case 4:
		{
			const int bit = 1 << (iCond - 128);
			if ((m_nPlayerCondEx4() & bit) == bit)
				return true;
			break;
		}
		}

		return false;
	}

	inline bool IsAGhost()
	{
		return InCond(TF_COND_HALLOWEEN_GHOST_MODE);
	};
	inline bool IsTaunting()
	{
		return InCond(TF_COND_TAUNTING);
	};

	inline bool IsInvisible()
	{
		if (InCond(TF_COND_BURNING)
			|| InCond(TF_COND_BURNING_PYRO)
			|| InCond(TF_COND_MAD_MILK)
			|| InCond(TF_COND_URINE))
			return false;

		return m_flInvisibility() >= 1.f;
	}

	inline float GetInvisPercentage()
	{
		static auto tf_spy_invis_time = I::CVar->FindVar("tf_spy_invis_time");
		const float flInvisTime = tf_spy_invis_time ? tf_spy_invis_time->GetFloat() : 1.f;
		const float GetInvisPercent = Math::RemapValClamped(m_flInvisChangeCompleteTime() - I::GlobalVars->curtime, flInvisTime, 0.f, 0.f, 100.f);

		return GetInvisPercent;
	}

	inline bool IsInvulnerable()
	{
		return InCond(TF_COND_INVULNERABLE)
			|| InCond(TF_COND_INVULNERABLE_CARD_EFFECT)
			|| InCond(TF_COND_INVULNERABLE_HIDE_UNLESS_DAMAGED)
			|| InCond(TF_COND_INVULNERABLE_USER_BUFF)
			|| InCond(TF_COND_PHASE);
	}

	inline bool IsUbered()
	{
		return InCond(TF_COND_INVULNERABLE)
			|| InCond(TF_COND_INVULNERABLE_CARD_EFFECT)
			|| InCond(TF_COND_INVULNERABLE_HIDE_UNLESS_DAMAGED)
			|| InCond(TF_COND_INVULNERABLE_USER_BUFF);
	}

	inline bool IsCritBoosted()
	{
		return InCond(TF_COND_CRITBOOSTED)
			|| InCond(TF_COND_CRITBOOSTED_BONUS_TIME)
			|| InCond(TF_COND_CRITBOOSTED_CARD_EFFECT)
			|| InCond(TF_COND_CRITBOOSTED_CTF_CAPTURE)
			|| InCond(TF_COND_CRITBOOSTED_FIRST_BLOOD)
			|| InCond(TF_COND_CRITBOOSTED_ON_KILL)
			|| InCond(TF_COND_CRITBOOSTED_PUMPKIN)
			|| InCond(TF_COND_CRITBOOSTED_RAGE_BUFF)
			|| InCond(TF_COND_CRITBOOSTED_RUNE_TEMP)
			|| InCond(TF_COND_CRITBOOSTED_USER_BUFF);
	}

	inline bool IsMiniCritBoosted()
	{
		return InCond(TF_COND_OFFENSEBUFF)
			|| InCond(TF_COND_ENERGY_BUFF)
			|| InCond(TF_COND_MINICRITBOOSTED_ON_KILL)
			|| InCond(TF_COND_NOHEALINGDAMAGEBUFF)
			/*|| InCond(TF_COND_CRITBOOSTED_DEMO_CHARGE)*/;
	}

	inline bool IsMarked()
	{
		return InCond(TF_COND_URINE)
			|| InCond(TF_COND_MARKEDFORDEATH)
			|| InCond(TF_COND_MARKEDFORDEATH_SILENT);
	}

	inline bool CanAttack()
	{
		if (!IsAlive() || IsAGhost() || IsTaunting() || InCond(TF_COND_PHASE) || InCond(TF_COND_HALLOWEEN_KART) || m_fFlags() & FL_FROZEN)
			return false;

		if (m_iClass() == TF_CLASS_SPY)
		{
			if (m_bFeignDeathReady() && !InCond(TF_COND_STEALTHED))
				return false;

			//Invis
			static float flTimer = 0.f;
			if (InCond(TF_COND_STEALTHED))
			{
				flTimer = 0.f;
				return false;
			}
			else
			{
				if (!flTimer)
					flTimer = I::GlobalVars->curtime;

				if (flTimer > I::GlobalVars->curtime)
					flTimer = 0.f;

				if ((I::GlobalVars->curtime - flTimer) < 2.f)
					return false;
			}
		}

		return true;
	}

	inline float TeamFortress_CalculateMaxSpeed(bool bIgnoreSpecialAbility = false)
	{
		return S::TeamFortress_CalculateMaxSpeed.Call<float>(this, bIgnoreSpecialAbility);
	}

	inline void UpdateClientSideAnimation()
	{
		S::CTFPlayer_UpdateClientSideAnimation.Call<void>(this);
	}

	inline float GetCritMult()
	{
		return Math::RemapValClamped(static_cast<float>(m_iCritMult()), 0.f, 255.f, 1.f, 4.f);
	}

	inline void UpdateWearables()
	{
		S::CTFPlayer_UpdateWearables.Call<void>(this);
	}

	inline void ThirdPersonSwitch(/*bool bThirdperson*/)
	{
		return reinterpret_cast<void(*)(void*/*, bool*/)>(U::Memory.GetVFunc(this, 256))(this/*, bThirdperson*/);
	};
};

class CTFRagdoll : public CBaseFlex
{
public:
	NETVAR(m_hPlayer, EHANDLE, "CTFRagdoll", "m_hPlayer")
	NETVAR(m_vecRagdollOrigin, Vec3, "CTFRagdoll", "m_vecRagdollOrigin");
	NETVAR(m_iPlayerIndex, int, "CTFRagdoll", "m_iPlayerIndex");
	NETVAR(m_vecForce, Vec3, "CTFRagdoll", "m_vecForce");
	NETVAR(m_vecRagdollVelocity, Vec3, "CTFRagdoll", "m_vecRagdollVelocity");
	NETVAR(m_nForceBone, int, "CTFRagdoll", "m_nForceBone");
	NETVAR(m_bGib, bool, "CTFRagdoll", "m_bGib");
	NETVAR(m_bBurning, bool, "CTFRagdoll", "m_bBurning");
	NETVAR(m_bElectrocuted, bool, "CTFRagdoll", "m_bElectrocuted");
	NETVAR(m_bFeignDeath, bool, "CTFRagdoll", "m_bFeignDeath");
	NETVAR(m_bWasDisguised, bool, "CTFRagdoll", "m_bWasDisguised");
	NETVAR(m_bOnGround, bool, "CTFRagdoll", "m_bOnGround");
	NETVAR(m_bCloaked, bool, "CTFRagdoll", "m_bCloaked");
	NETVAR(m_bBecomeAsh, bool, "CTFRagdoll", "m_bBecomeAsh");
	NETVAR(m_iDamageCustom, int, "CTFRagdoll", "m_iDamageCustom");
	NETVAR(m_iTeam, int, "CTFRagdoll", "m_iTeam");
	NETVAR(m_iClass, int, "CTFRagdoll", "m_iClass");
	NETVAR(m_bGoldRagdoll, bool, "CTFRagdoll", "m_bGoldRagdoll");
	NETVAR(m_bIceRagdoll, bool, "CTFRagdoll", "m_bIceRagdoll");
	NETVAR(m_bCritOnHardHit, bool, "CTFRagdoll", "m_bCritOnHardHit");
	NETVAR(m_flHeadScale, float, "CTFRagdoll", "m_flHeadScale");
	NETVAR(m_flTorsoScale, float, "CTFRagdoll", "m_flTorsoScale");
	NETVAR(m_flHandScale, float, "CTFRagdoll", "m_flHandScale");

	NETVAR_OFF(m_bDissolving, bool, "CTFRagdoll", "m_bFeignDeath", -1);
};