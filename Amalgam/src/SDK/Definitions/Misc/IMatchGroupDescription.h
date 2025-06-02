#pragma once
#include "ConVar.h"
#include "../Interfaces/IVEngineClient.h"
#include "../Main/UtlVector.h"
#include "../Steam/SteamClientPublic.h"
#include "../Definitions.h"
#include "../../../Utils/Signatures/Signatures.h"
#include "../../../Utils/Memory/Memory.h"

MAKE_SIGNATURE(IProgressionDesc_GetLevelByNumber, "client.dll", "83 EA ? 4C 8B C1", 0x0);
MAKE_SIGNATURE(CTFRatingData_GetRatingData, "client.dll", "8B 41 ? 89 02 8B 41 ? 89 42 ? 8B 41 ? 89 42 ? 48 8B C2", 0x0);
MAKE_SIGNATURE(CTFRatingData_YieldingGetPlayerRatingDataBySteamID, "client.dll", "48 89 5C 24 ? 57 48 83 EC ? 8B FA 48 8B D9 E8 ? ? ? ? 48 8B C8", 0x0);

enum EMatchMode
{
	eMatchMode_Invalid,
	eMatchMode_MatchMaker_CompleteFromQueue,
	eMatchMode_MatchMaker_LateJoinDropIn,
	eMatchMode_MatchMaker_LateJoinMatchBased,
	eMatchMode_Manual_NewMatch,
	eMatchMode_Manual_ExistingMatchBased
};

enum EMMPenaltyPool
{
	eMMPenaltyPool_Invalid = -1,
	eMMPenaltyPool_Casual = 0,
	eMMPenaltyPool_Ranked,
	eMMPenaltyPool_Count
};

enum EMatchType_t
{
	MATCH_TYPE_NONE = 0,
	MATCH_TYPE_MVM,
	MATCH_TYPE_COMPETITIVE,
	MATCH_TYPE_CASUAL
};

enum EMMRating
{
	k_nMMRating_LowestValue = -1,
	k_nMMRating_Invalid = -1,
	k_nMMRating_First = 0,
	k_nMMRating_6v6_DRILLO = 0,
	k_nMMRating_6v6_DRILLO_PlayerAcknowledged = 1,
	k_nMMRating_6v6_GLICKO = 2,
	k_nMMRating_12v12_DRILLO = 3,
	k_nMMRating_Casual_12v12_GLICKO = 4,
	k_nMMRating_Casual_XP = 5,
	k_nMMRating_Casual_XP_PlayerAcknowledged = 6,
	k_nMMRating_6v6_Rank = 7,
	k_nMMRating_6v6_Rank_PlayerAcknowledged = 8,
	k_nMMRating_Casual_12v12_Rank = 9,
	k_nMMRating_Casual_12v12_Rank_PlayerAcknowledged = 10,
	k_nMMRating_6v6_GLICKO_PlayerAcknowledged = 11,
	k_nMMRating_Comp_12v12_Rank = 12,
	k_nMMRating_Comp_12v12_Rank_PlayerAcknowledged = 13,
	k_nMMRating_Comp_12v12_GLICKO = 14,
	k_nMMRating_Comp_12v12_GLICKO_PlayerAcknowledged = 15,
	k_nMMRating_Last = 15,
};

enum EMatchGroupLeaderboard
{
	k_eMatchGroupLeaderboard_Invalid = -1,
	k_eMatchGroupLeaderboard_Ladder6v6 = 0,
	k_eMatchGroupLeaderboard_Casual12v12,
	k_eMatchGroupLeaderboard_Count
};

struct LevelInfo_t
{
	unsigned m_nLevelNum;
	unsigned m_nDisplayLevel;
	unsigned m_nStartXP; // Inclusive
	unsigned m_nEndXP; // Non-inclusive
	CUtlString m_pszLevelTitle;
	const char* m_pszLevelUpSound;
	const char* m_pszLobbyBackgroundImage;
};

struct MMRatingData_t
{
	uint32_t unRatingPrimary;
	uint32_t unRatingSecondary;
	uint32_t unRatingTertiary;
};

class CTFRatingData
{
public:
	inline MMRatingData_t GetRatingData()
	{
		MMRatingData_t rvoData;
		return *S::CTFRatingData_GetRatingData.Call<MMRatingData_t*>(this, &rvoData);
	}
};

class IProgressionDesc
{
public:
	inline const LevelInfo_t& GetLevelForRating(unsigned nExperience) const
	{
		return U::Memory.CallVirtual<2, const LevelInfo_t&>(const_cast<IProgressionDesc*>(this), nExperience);
	}

	inline const LevelInfo_t& GetLevelByNumber(unsigned nNumber) const
	{
		return S::IProgressionDesc_GetLevelByNumber.Call<LevelInfo_t&>(this, nNumber);
	}

public:
	const CUtlString m_strBadgeName;
	const char* m_pszLevelToken;
	const char* m_pszProgressionResFile;
	CUtlVector<LevelInfo_t> m_vecLevels;
};

class IMatchGroupDescription
{
public:
	inline int GetLevelForIndex(int iIndex)
	{
		PlayerInfo_t pi{};
		if (!I::EngineClient->GetPlayerInfo(iIndex, &pi) || pi.fakeplayer)
			return -1;

		CSteamID cSteamID = { pi.friendsID, 1, k_EUniversePublic, k_EAccountTypeIndividual };
		if (!cSteamID.IsValid())
			return -1;

		auto pRating = S::CTFRatingData_YieldingGetPlayerRatingDataBySteamID.Call<CTFRatingData*>(&cSteamID, m_eMatchType == MATCH_TYPE_CASUAL ? m_eCurrentDisplayRating : m_eCurrentDisplayRank);
		if (!pRating)
			return -1;

		int nLevel = pRating->GetRatingData().unRatingPrimary;
		if (m_eMatchType == MATCH_TYPE_CASUAL)
			return m_pProgressionDesc->GetLevelForRating(nLevel).m_nLevelNum;
		else
			return m_pProgressionDesc->GetLevelByNumber(nLevel).m_nLevelNum;
	}

public:
	byte pad0[8];
	const ETFMatchGroup m_eMatchGroup;
	const IProgressionDesc* m_pProgressionDesc;
	bool m_bActive;
	EMatchMode m_eLateJoinMode;
	EMMPenaltyPool m_ePenaltyPool;
	bool m_bUsesSkillRatings;
	bool m_bSupportsLowPriorityQueue;
	const ConVar* m_pmm_required_score;
	bool m_bUseMatchHud;
	const char* m_pszExecFileName;
	const ConVar* m_pmm_match_group_size;
	const ConVar* m_pmm_match_group_size_minimum;
	bool m_bShowPreRoundDoors;
	bool m_bShowPostRoundDoors;
	const char* m_pszMatchEndKickWarning;
	const char* m_pszMatchStartSound;
	bool m_bAutoReady;
	bool m_bUseMatchSummaryStage;
	bool m_bDistributePerformanceMedals;
	bool m_bUseFirstBlood;
	bool m_bUseReducedBonusTime;
	bool m_bUseAutoBalance;
	bool m_bAllowTeamChange;
	bool m_bRandomWeaponCrits;
	bool m_bFixedWeaponSpread;
	bool m_bRequireCompleteMatch;
	bool m_bTrustedServersOnly;
	bool m_bForceClientSettings;
	bool m_bAllowDrawingAtMatchSummary;
	bool m_bAllowSpecModeChange;
	bool m_bAutomaticallyRequeueAfterMatchEnds;
	bool m_bUsesMapVoteOnRoundEnd;
	bool m_bScramblesTeamsOnRollingMatch;
	bool m_bUsesXP;
	bool m_bUsesDashboardOnRoundEnd;
	bool m_bUsesSurveys;
	bool m_bStrictMatchmakerScoring;
	const char* m_pszModeNameLocToken;
	const char* m_pszRichPresenceLocToken;
	bool m_bAllowPartyJoins;
	bool m_bAllowPartySpectate;
	bool m_bContractProgressAllowed;
	EMatchType_t m_eMatchType;
	int m_nNumWinsToExitPlacement;
	int m_nNumPlacementMatchesPerDay;
	EMMRating m_eCurrentDisplayRating;
	EMMRating m_eLastAckdDisplayRating;
	EMMRating m_eCurrentDisplayRank;
	EMMRating m_eLastAckdDisplayRank;
	bool m_bUsesStickyRanks;
	bool m_bUsesStrictAbandons;
	bool m_bRequiresCompetitiveAccess;
	EMatchGroupLeaderboard  m_eLeaderboard;
	bool m_bUsesStrictSpectatorRules;
};