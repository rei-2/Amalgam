#pragma once
#include "../Definitions.h"
#include "../Misc/BaseTypes.h"

#ifndef ARRAYSIZE
	#define ARRAYSIZE(A) (sizeof(A)/sizeof((A)[0]))
#endif

#define MAX_PLAYERS_ARRAY_SAFE (MAX_PLAYERS + 1)
#define INVALID_ITEM_DEF_INDEX 65535U

typedef uint64 itemid_t;
typedef int32 entityquality_t;
typedef uint8 style_index_t;
typedef uint16 item_definition_index_t;

enum TFStatType_t
{
	TFSTAT_UNDEFINED = 0,
	TFSTAT_SHOTS_HIT,
	TFSTAT_SHOTS_FIRED,
	TFSTAT_KILLS,
	TFSTAT_DEATHS,
	TFSTAT_DAMAGE,
	TFSTAT_CAPTURES,
	TFSTAT_DEFENSES,
	TFSTAT_DOMINATIONS,
	TFSTAT_REVENGE,
	TFSTAT_POINTSSCORED,
	TFSTAT_BUILDINGSDESTROYED,
	TFSTAT_HEADSHOTS,
	TFSTAT_PLAYTIME,
	TFSTAT_HEALING,
	TFSTAT_INVULNS,
	TFSTAT_KILLASSISTS,
	TFSTAT_BACKSTABS,
	TFSTAT_HEALTHLEACHED,
	TFSTAT_BUILDINGSBUILT,
	TFSTAT_MAXSENTRYKILLS,
	TFSTAT_TELEPORTS,
	TFSTAT_FIREDAMAGE,
	TFSTAT_BONUS_POINTS,
	TFSTAT_BLASTDAMAGE,
	TFSTAT_DAMAGETAKEN,
	TFSTAT_HEALTHKITS,
	TFSTAT_AMMOKITS,
	TFSTAT_CLASSCHANGES,
	TFSTAT_CRITS,
	TFSTAT_SUICIDES,
	TFSTAT_CURRENCY_COLLECTED,
	TFSTAT_DAMAGE_ASSIST,
	TFSTAT_HEALING_ASSIST,
	TFSTAT_DAMAGE_BOSS,
	TFSTAT_DAMAGE_BLOCKED,
	TFSTAT_DAMAGE_RANGED,
	TFSTAT_DAMAGE_RANGED_CRIT_RANDOM,
	TFSTAT_DAMAGE_RANGED_CRIT_BOOSTED,
	TFSTAT_REVIVED,
	TFSTAT_THROWABLEHIT,
	TFSTAT_THROWABLEKILL,
	TFSTAT_KILLSTREAK_MAX,
	TFSTAT_KILLS_RUNECARRIER,
	TFSTAT_FLAGRETURNS,
	TFSTAT_TOTAL
};
enum TFMapStatType_t
{
	TFMAPSTAT_UNDEFINED = 0,
	TFMAPSTAT_PLAYTIME,
	TFMAPSTAT_TOTAL
};
enum EEconItemQuality
{
	AE_UNDEFINED = -1,
	AE_NORMAL = 0,
	AE_RARITY1 = 1,  // Genuine
	AE_RARITY2 = 2,  // Customized (unused)
	AE_VINTAGE = 3,  // Vintage has to stay at 3 for backwards compatibility
	AE_RARITY3 = 4,  // Artisan
	AE_UNUSUAL = 5,  // Unusual
	AE_UNIQUE = 6,
	AE_COMMUNITY = 7,
	AE_DEVELOPER = 8,
	AE_SELFMADE = 9,
	AE_CUSTOMIZED = 10, // (unused)
	AE_STRANGE = 11,
	AE_COMPLETED = 12,
	AE_HAUNTED = 13,
	AE_COLLECTORS = 14,
	AE_PAINTKITWEAPON = 15,
	AE_RARITY_DEFAULT = 16,
	AE_RARITY_COMMON = 17,
	AE_RARITY_UNCOMMON = 18,
	AE_RARITY_RARE = 19,
	AE_RARITY_MYTHICAL = 20,
	AE_RARITY_LEGENDARY = 21,
	AE_RARITY_ANCIENT = 22,
	AE_MAX_TYPES,
	AE_DEPRECATED_UNIQUE = 3,
};
enum loadout_positions_t
{
	LOADOUT_POSITION_INVALID = -1,
	LOADOUT_POSITION_PRIMARY = 0,
	LOADOUT_POSITION_SECONDARY,
	LOADOUT_POSITION_MELEE,
	LOADOUT_POSITION_UTILITY,
	LOADOUT_POSITION_BUILDING,
	LOADOUT_POSITION_PDA,
	LOADOUT_POSITION_PDA2,
	LOADOUT_POSITION_HEAD,
	LOADOUT_POSITION_MISC,
	LOADOUT_POSITION_ACTION,
	LOADOUT_POSITION_MISC2,
	LOADOUT_POSITION_TAUNT,
	LOADOUT_POSITION_TAUNT2,
	LOADOUT_POSITION_TAUNT3,
	LOADOUT_POSITION_TAUNT4,
	LOADOUT_POSITION_TAUNT5,
	LOADOUT_POSITION_TAUNT6,
	LOADOUT_POSITION_TAUNT7,
	LOADOUT_POSITION_TAUNT8,
	CLASS_LOADOUT_POSITION_COUNT,
};

struct RoundStats_t
{
	int m_iStat[TFSTAT_TOTAL];

	RoundStats_t() { Reset(); };

	inline int Get(int i) const
	{
		return m_iStat[i];
	}
	inline void Set(int i, int nValue)
	{
		m_iStat[i] = nValue;
	}
	void Reset()
	{
		for (int i = 0; i < ARRAYSIZE(m_iStat); i++)
			m_iStat[i] = 0;
	}
	void AccumulateRound(const RoundStats_t& other)
	{
		for (int i = 0; i < ARRAYSIZE(m_iStat); i++)
			m_iStat[i] += other.m_iStat[i];
	}
};
struct RoundMapStats_t
{
	int m_iStat[TFMAPSTAT_TOTAL];

	RoundMapStats_t() { Reset(); }

	inline int Get(int i) const
	{
		return m_iStat[i];
	}
	inline void Set(int i, int nValue)
	{
		m_iStat[i] = nValue;
	}
	void Reset()
	{
		for (int i = 0; i < ARRAYSIZE(m_iStat); i++)
			m_iStat[i] = 0;
	}
	void AccumulateRound(const RoundMapStats_t& other)
	{
		for (int i = 0; i < ARRAYSIZE(m_iStat); i++)
			m_iStat[i] += other.m_iStat[i];
	}
};
struct KillStats_t
{
	KillStats_t() { Reset(); }

	void Reset()
	{
		memset(iNumKilled, 0, sizeof(iNumKilled));
		memset(iNumKilledBy, 0, sizeof(iNumKilledBy));
		memset(iNumKilledByUnanswered, 0, sizeof(iNumKilledByUnanswered));
	}

	int iNumKilled[MAX_PLAYERS_ARRAY_SAFE];					// how many times this player has killed every other player
	int iNumKilledBy[MAX_PLAYERS_ARRAY_SAFE];				// how many times this player has been killed by every other player
	int iNumKilledByUnanswered[MAX_PLAYERS_ARRAY_SAFE];		// how many unanswered kills this player has been dealt by every other player
};
struct LoadoutStats_t
{
	LoadoutStats_t() { Reset(); }

	void Reset()
	{
		memset(iLoadoutItemDefIndices, INVALID_ITEM_DEF_INDEX, sizeof(iLoadoutItemDefIndices));
		memset(iLoadoutItemQualities, AE_UNDEFINED, sizeof(iLoadoutItemQualities));
		memset(iLoadoutItemStyles, 0, sizeof(iLoadoutItemStyles));

		flStartTime = 0;
		iClass = TF_CLASS_UNDEFINED;
	}

	void Set(int iPlayerClass, float flStartTime)
	{
		iClass = iPlayerClass;
		flStartTime = flStartTime;
	}

	void SetItemDef(int iSlot, itemid_t iItemDef, entityquality_t iItemQuality, style_index_t iStyle)
	{
		iLoadoutItemDefIndices[iSlot] = iItemDef;
		iLoadoutItemQualities[iSlot] = iItemQuality;
		iLoadoutItemStyles[iSlot] = iStyle;
	}

	item_definition_index_t iLoadoutItemDefIndices[CLASS_LOADOUT_POSITION_COUNT];
	entityquality_t iLoadoutItemQualities[CLASS_LOADOUT_POSITION_COUNT];
	style_index_t iLoadoutItemStyles[CLASS_LOADOUT_POSITION_COUNT];
	float flStartTime;
	int iClass;
};

struct PlayerStats_t
{
	PlayerStats_t()
	{
		Reset();
	}

	void Reset()
	{
		statsCurrentLife.Reset();
		statsCurrentRound.Reset();
		statsAccumulated.Reset();
		mapStatsCurrentLife.Reset();
		mapStatsCurrentRound.Reset();
		mapStatsAccumulated.Reset();
		statsKills.Reset();
		loadoutStats.Reset();
		iConnectTime = 0;
		iDisconnectTime = 0;
	}

	PlayerStats_t(const PlayerStats_t& other)
	{
		statsCurrentLife = other.statsCurrentLife;
		statsCurrentRound = other.statsCurrentRound;
		statsAccumulated = other.statsAccumulated;
		mapStatsCurrentLife = other.mapStatsCurrentLife;
		mapStatsCurrentRound = other.mapStatsCurrentRound;
		mapStatsAccumulated = other.mapStatsAccumulated;
		loadoutStats = other.loadoutStats;
		iConnectTime = other.iConnectTime;
		iDisconnectTime = other.iDisconnectTime;
	}

	RoundStats_t	statsCurrentLife;
	RoundStats_t	statsCurrentRound;
	RoundStats_t	statsAccumulated;
	RoundMapStats_t	mapStatsCurrentLife;
	RoundMapStats_t	mapStatsCurrentRound;
	RoundMapStats_t	mapStatsAccumulated;
	KillStats_t		statsKills;
	LoadoutStats_t	loadoutStats;
	int				iConnectTime;
	int				iDisconnectTime;
};