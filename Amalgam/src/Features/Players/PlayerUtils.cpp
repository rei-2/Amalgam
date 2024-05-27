#include "PlayerUtils.h"

#include "../../SDK/Definitions/Types.h"
#include "../Records/Records.h"

uint32_t GetFriendsID(int iIndex)
{
	PlayerInfo_t pi{};
	if (I::EngineClient->GetPlayerInfo(iIndex, &pi) && !pi.fakeplayer)
		return pi.friendsID;
	return 0;
}

bool CPlayerlistUtils::GetTag(std::string sTag, PriorityLabel_t* plTag)
{
	if (!sTag.length())
		return false;

	if (mTags.contains(sTag))
	{
		*plTag = mTags[sTag];
		return true;
	}

	return false;
}



void CPlayerlistUtils::AddTag(uint32_t friendsID, std::string sTag, bool bSave, std::string sName)
{
	if (!friendsID)
		return;

	if (!HasTag(friendsID, sTag))
	{
		mPlayerTags[friendsID].push_back(sTag);
		bSavePlayers = bSave;
		if (sName.length())
		{
			PriorityLabel_t plTag;
			if (GetTag(sTag, &plTag))
				F::Records.TagsChanged(sName, "Added", plTag.Color.ToHexA(), sTag);
		}
	}
}
void CPlayerlistUtils::AddTag(int iIndex, std::string sTag, bool bSave, std::string sName)
{
	if (const uint32_t friendsID = GetFriendsID(iIndex))
		AddTag(friendsID, sTag, bSave, sName);
}

void CPlayerlistUtils::RemoveTag(uint32_t friendsID, std::string sTag, bool bSave, std::string sName)
{
	if (!friendsID)
		return;

	auto uHash = FNV1A::Hash(sTag.c_str());
	auto& mTags = mPlayerTags[friendsID];
	for (auto it = mTags.begin(); it != mTags.end(); it++)
	{
		if (uHash = FNV1A::Hash(it->c_str()))
		{
			mTags.erase(it);
			bSavePlayers = bSave;
			if (sName.length())
			{
				PriorityLabel_t plTag;
				if (GetTag(sTag, &plTag))
					F::Records.TagsChanged(sName, "Removed", plTag.Color.ToHexA(), sTag);
			}
			break;
		}
	}
}
void CPlayerlistUtils::RemoveTag(int iIndex, std::string sTag, bool bSave, std::string sName)
{
	if (const uint32_t friendsID = GetFriendsID(iIndex))
		RemoveTag(friendsID, sTag, bSave, sName);
}

bool CPlayerlistUtils::HasTags(uint32_t friendsID)
{
	if (!friendsID)
		return false;

	return mPlayerTags[friendsID].size();
}
bool CPlayerlistUtils::HasTags(int iIndex)
{
	if (const uint32_t friendsID = GetFriendsID(iIndex))
		return HasTags(friendsID);
	return false;
}

bool CPlayerlistUtils::HasTag(uint32_t friendsID, std::string sTag)
{
	if (!friendsID)
		return false;

	auto uHash = FNV1A::Hash(sTag.c_str());
	auto it = std::ranges::find_if(mPlayerTags[friendsID], [uHash](const auto& _sTag) { return uHash == FNV1A::Hash(_sTag.c_str()); });
	return it != mPlayerTags[friendsID].end();
}
bool CPlayerlistUtils::HasTag(int iIndex, std::string sTag)
{
	if (const uint32_t friendsID = GetFriendsID(iIndex))
		return HasTag(friendsID, sTag);
	return false;
}



int CPlayerlistUtils::GetPriority(uint32_t friendsID, bool bCache)
{
	if (bCache)
		return H::Entities.GetPriority(friendsID);

	const int iDefault = mTags["Default"].Priority;
	if (!friendsID)
		return iDefault;

	if (HasTag(friendsID, "Ignored"))
		return mTags["Ignored"].Priority;

	std::vector<int> vPriorities;
	for (const auto& sTag : mPlayerTags[friendsID])
	{
		PriorityLabel_t plTag;
		if (F::PlayerUtils.GetTag(sTag, &plTag) && !plTag.Label)
			vPriorities.push_back(plTag.Priority);
	}
	if (H::Entities.IsFriend(friendsID))
	{
		auto& plTag = mTags["Friend"];
		if (!plTag.Label)
			vPriorities.push_back(plTag.Priority);
	}

	if (vPriorities.size())
	{
		std::sort(vPriorities.begin(), vPriorities.end(), std::greater<int>());
		return *vPriorities.begin();
	}
	return iDefault;
}
int CPlayerlistUtils::GetPriority(int iIndex, bool bCache)
{
	if (bCache)
		return H::Entities.GetPriority(iIndex);

	if (const uint32_t friendsID = GetFriendsID(iIndex))
		return GetPriority(friendsID);
	return mTags["Default"].Priority;
}

bool CPlayerlistUtils::GetSignificantTag(uint32_t friendsID, std::string* sTag, PriorityLabel_t* plTag, int iMode)
{
	if (!friendsID)
		return false;

	std::vector<std::pair<std::string, PriorityLabel_t>> vLabels;
	if (!iMode || iMode == 1)
	{
		if (HasTag(friendsID, "Ignored"))
		{
			*sTag = "Ignored";
			*plTag = mTags["Ignored"];
			return true;
		}

		for (const auto& _sTag : mPlayerTags[friendsID])
		{
			PriorityLabel_t _plTag;
			if (F::PlayerUtils.GetTag(_sTag, &_plTag) && !_plTag.Label)
				vLabels.push_back({ _sTag, _plTag });
		}
		if (H::Entities.IsFriend(friendsID))
		{
			auto& _plTag = mTags["Friend"];
			if (!_plTag.Label)
				vLabels.push_back({ "Friend", _plTag });
		}
	}
	if ((!iMode || iMode == 2) && !vLabels.size())
	{
		for (const auto& _sTag : mPlayerTags[friendsID])
		{
			PriorityLabel_t _plTag;
			if (F::PlayerUtils.GetTag(_sTag, &_plTag) && _plTag.Label)
				vLabels.push_back({ _sTag, _plTag });
		}
		if (H::Entities.IsFriend(friendsID))
		{
			auto& _plTag = mTags["Friend"];
			if (_plTag.Label)
				vLabels.push_back({ "Friend", _plTag });
		}
	}
	if (!vLabels.size())
		return false;

	std::sort(vLabels.begin(), vLabels.end(), [&](const auto& a, const auto& b) -> bool
		{
			// sort by priority if unequal
			if (a.second.Priority != b.second.Priority)
				return a.second.Priority > b.second.Priority;

			return a.first < b.first;
		});
	*sTag = vLabels.begin()->first;
	*plTag = vLabels.begin()->second;
	return true;
}
bool CPlayerlistUtils::GetSignificantTag(int iIndex, std::string* sTag, PriorityLabel_t* plTag, int iMode)
{
	if (const uint32_t friendsID = GetFriendsID(iIndex))
		return GetSignificantTag(friendsID, sTag, plTag, iMode);
	return false;
}

bool CPlayerlistUtils::IsIgnored(uint32_t friendsID)
{
	if (!friendsID)
		return false;

	const int iPriority = GetPriority(friendsID);
	const int iIgnored = mTags["Ignored"].Priority;
	return iPriority <= iIgnored;
}
bool CPlayerlistUtils::IsIgnored(int iIndex)
{
	if (const uint32_t friendsID = GetFriendsID(iIndex))
		return IsIgnored(friendsID);
	return false;
}



void CPlayerlistUtils::UpdatePlayers()
{
	static Timer updateTimer{};
	if (updateTimer.Run(1000))
	{
		std::lock_guard lock(mutex);
		vPlayerCache.clear();

		auto pResource = H::Entities.GetPR();
		if (!pResource)
			return;

		for (int n = 1; n <= I::EngineClient->GetMaxClients(); n++)
		{
			if (!pResource->GetValid(n) || !pResource->GetConnected(n))
				continue;

			bool bFake = true, bFriend = false;
			PlayerInfo_t pi{};
			if (I::EngineClient->GetPlayerInfo(n, &pi))
			{
				bFake = pi.fakeplayer;
				bFriend = H::Entities.IsFriend(n);
			}

			vPlayerCache.push_back({
				pResource->GetPlayerName(n),
				pResource->GetAccountID(n),
				pResource->GetUserID(n),
				pResource->GetTeam(n),
				pResource->GetClass(n),
				pResource->IsAlive(n),
				n == I::EngineClient->GetLocalPlayer(),
				bFriend,
				bFake
			});
		}
	}
}