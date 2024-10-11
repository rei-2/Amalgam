#pragma once
#include "../../SDK/SDK.h"
#include <mutex>

#define DEFAULT_TAG 0
#define IGNORED_TAG 1
#define CHEATER_TAG 2
#define FRIEND_TAG 3

struct ListPlayer
{
	const char* Name{};
	uint32_t FriendsID{};
	int UserID{};
	int Team{};
	int Class{};
	bool Alive{};
	bool Local{};
	bool Friend{};
	bool Fake{};
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
	PriorityLabel_t* GetTag(int iID);
	int GetTag(std::string sTag);

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
		{ "Friend", { 100, 255, 100, 255 }, 0, true, false, true }
	};

	std::vector<ListPlayer> m_vPlayerCache = {};
	std::unordered_map<uint32_t, ListPlayer> m_mPriorityCache = {};

	bool m_bLoadPlayers = true;
	bool m_bSavePlayers = false;
	bool m_bLoadTags = true;
	bool m_bSaveTags = false;

	const std::vector<const char*> m_vListPitch = { "None", "Up", "Down", "Zero", "Auto" };
	const std::vector<const char*> m_vListYaw = { "None", "Forward", "Backward", "Left", "Right", "Invert", "Edge", "Auto" };
};

ADD_FEATURE(CPlayerlistUtils, PlayerUtils)