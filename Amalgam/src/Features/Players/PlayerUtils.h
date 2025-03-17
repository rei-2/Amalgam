#pragma once
#include "../../SDK/SDK.h"
#include <mutex>

#define DEFAULT_TAG 0
#define IGNORED_TAG (DEFAULT_TAG-1)
#define CHEATER_TAG (IGNORED_TAG-1)
#define FRIEND_TAG (CHEATER_TAG-1)
#define PARTY_TAG (FRIEND_TAG-1)
#define F2P_TAG (PARTY_TAG-1)
#define TAG_COUNT (-F2P_TAG)

struct ListPlayer
{
	std::string m_sName;
	uint32_t m_uFriendsID;
	int m_iUserID;
	int m_iTeam;
	bool m_bAlive;
	bool m_bLocal;
	bool m_bFake;
	bool m_bFriend;
	bool m_bParty;
	bool m_bF2P;
	int m_iLevel;
};

struct PriorityLabel_t
{
	std::string Name = "";
	Color_t Color = {};
	int Priority = 0;

	bool Label = false;
	bool Assignable = true;
	bool Locked = false; // don't allow it to be removed
};

class CPlayerlistUtils
{
public:
	uint32_t GetFriendsID(int iIndex);
	PriorityLabel_t* GetTag(int iID);
	int GetTag(std::string sTag);
	inline int TagToIndex(int iTag)
	{
		if (iTag <= 0)
			iTag = -iTag;
		else
			iTag += TAG_COUNT;
		return iTag;
	}
	inline int IndexToTag(int iID)
	{
		if (iID <= TAG_COUNT)
			iID = -iID;
		else
			iID -= TAG_COUNT;
		return iID;
	}

	void AddTag(uint32_t friendsID, int iID, bool bSave = true, std::string sName = "");
	void AddTag(int iIndex, int iID, bool bSave = true, std::string sName = "");
	void RemoveTag(uint32_t friendsID, int iID, bool bSave = true, std::string sName = "");
	void RemoveTag(int iIndex, int iID, bool bSave = true, std::string sName = "");
	bool HasTags(uint32_t friendsID);
	bool HasTags(int iIndex);
	bool HasTag(uint32_t friendsID, int iID);
	bool HasTag(int iIndex, int iID);

	int GetPriority(uint32_t friendsID, bool bCache = true);
	int GetPriority(int iIndex, bool bCache = true);
	PriorityLabel_t* GetSignificantTag(uint32_t friendsID, int iMode = 1); // iMode: 0 - Priorities & Labels, 1 - Priorities, 2 - Labels
	PriorityLabel_t* GetSignificantTag(int iIndex, int iMode = 1); // iMode: 0 - Priorities & Labels, 1 - Priorities, 2 - Labels
	bool IsIgnored(uint32_t friendsID);
	bool IsIgnored(int iIndex);
	bool IsPrioritized(uint32_t friendsID);
	bool IsPrioritized(int iIndex);

	const char* GetPlayerName(int iIndex, const char* sDefault, int* pType = nullptr);

	void UpdatePlayers();
	std::mutex m_mutex;

public:
	std::unordered_map<uint32_t, std::vector<int>> m_mPlayerTags = {};
	std::unordered_map<uint32_t, std::string> m_mPlayerAliases = {};

	std::vector<PriorityLabel_t> m_vTags = {
		{ "Default", { 200, 200, 200, 255 }, 0, false, false, true },
		{ "Ignored", { 200, 200, 200, 255 }, -1, false, true, true },
		{ "Cheater", { 255, 100, 100, 255 }, 1, false, true, true },
		{ "Friend", { 100, 255, 100, 255 }, 0, true, false, true },
		{ "Party", { 100, 100, 255, 255 }, 0, true, false, true },
		{ "F2P", { 255, 255, 255, 255 }, 0, true, false, true }
	};

	std::vector<ListPlayer> m_vPlayerCache = {};
	std::unordered_map<uint32_t, ListPlayer> m_mPriorityCache = {};

	bool m_bLoad = true;
	bool m_bSave = false;
};

ADD_FEATURE(CPlayerlistUtils, PlayerUtils)