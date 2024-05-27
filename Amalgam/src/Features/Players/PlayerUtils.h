#pragma once
#include "../../SDK/SDK.h"
#include <mutex>

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
	Color_t Color = { 255, 255, 255, 255 };
	int Priority = 0;

	bool Label = false;
	bool Assignable = true;
	bool Locked = false; // don't allow it to be removed
};

class CPlayerlistUtils
{
public:
	bool GetTag(std::string sTag, PriorityLabel_t* plTag);

	void AddTag(uint32_t friendsID, std::string sTag, bool bSave = true, std::string sName = "");
	void AddTag(int iIndex, std::string sTag, bool bSave = true, std::string sName = "");
	void RemoveTag(uint32_t friendsID, std::string sTag, bool bSave = true, std::string sName = "");
	void RemoveTag(int iIndex, std::string sTag, bool bSave = true, std::string sName = "");
	bool HasTags(uint32_t friendsID);
	bool HasTags(int iIndex);
	bool HasTag(uint32_t friendsID, std::string sTag);
	bool HasTag(int iIndex, std::string sTag);

	int GetPriority(uint32_t friendsID, bool bCache = true);
	int GetPriority(int iIndex, bool bCache = true);
	bool GetSignificantTag(uint32_t friendsID, std::string* sTag, PriorityLabel_t* plTag, int iMode = 1); // iMode: 0 - Priorities & Labels, 1 - Priorities, 2 - Labels
	bool GetSignificantTag(int iIndex, std::string* sTag, PriorityLabel_t* plTag, int iMode = 1); // iMode: 0 - Priorities & Labels, 1 - Priorities, 2 - Labels
	bool IsIgnored(uint32_t friendsID);
	bool IsIgnored(int iIndex);

	void UpdatePlayers();
	std::mutex mutex;

public:
	std::unordered_map<uint32_t, std::vector<std::string>> mPlayerTags = {};

	std::unordered_map<std::string, PriorityLabel_t> mTags = {
		{ "Default", { { 200, 200, 200, 255 }, 0, false, false, true } },
		{ "Ignored", { { 200, 200, 200, 255 }, -1, false, true, true } },
		{ "Cheater", { { 255, 100, 100, 255 }, 1, false, true, true } },
		{ "Friend", { { 100, 255, 100, 255 }, 0, true, false, true } }
	};

	std::vector<ListPlayer> vPlayerCache = {};
	std::unordered_map<uint32_t, ListPlayer> mPriorityCache = {};

	bool bLoadPlayers = true;
	bool bSavePlayers = false;
	bool bLoadTags = true;
	bool bSaveTags = false;

	const std::vector<const char*> vListPitch = { "None", "Up", "Down", "Zero", "Auto" };
	const std::vector<const char*> vListYaw = { "None", "Forward", "Backward", "Left", "Right", "Invert", "Edge", "Auto" };
};

ADD_FEATURE(CPlayerlistUtils, PlayerUtils)