#include "PlayerUtils.h"

#include "../../SDK/Definitions/Types.h"
#include "../Output/Output.h"

uint32_t CPlayerlistUtils::GetFriendsID(int iIndex)
{
	PlayerInfo_t pi{};
	if (I::EngineClient->GetPlayerInfo(iIndex, &pi) && !pi.fakeplayer)
		return pi.friendsID;
	return 0;
}

PriorityLabel_t* CPlayerlistUtils::GetTag(int iID)
{
	if (iID > -1 && iID < m_vTags.size())
		return &m_vTags[iID];

	return nullptr;
}

int CPlayerlistUtils::GetTag(std::string sTag)
{
	auto uHash = FNV1A::Hash32(sTag.c_str());

	int iID = -1;
	for (auto& tTag : m_vTags)
	{
		iID++;
		if (uHash == FNV1A::Hash32(tTag.Name.c_str()))
			return iID;
	}

	return -1;
}



void CPlayerlistUtils::AddTag(uint32_t friendsID, int iID, bool bSave, std::string sName, std::unordered_map<uint32_t, std::vector<int>>& mPlayerTags)
{
	if (!friendsID)
		return;

	if (!HasTag(friendsID, iID))
	{
		mPlayerTags[friendsID].push_back(iID);
		m_bSave = bSave;
		if (sName.length())
		{
			if (PriorityLabel_t* pTag = GetTag(iID))
				F::Output.TagsChanged(sName, "Added", pTag->Color.ToHexA(), pTag->Name);
		}
	}
}
void CPlayerlistUtils::AddTag(int iIndex, int iID, bool bSave, std::string sName, std::unordered_map<uint32_t, std::vector<int>>& mPlayerTags)
{
	if (const uint32_t friendsID = GetFriendsID(iIndex))
		AddTag(friendsID, iID, bSave, sName, mPlayerTags);
}
void CPlayerlistUtils::AddTag(uint32_t friendsID, int iID, bool bSave, std::string sName)
{
	AddTag(friendsID, iID, bSave, sName, m_mPlayerTags);
}
void CPlayerlistUtils::AddTag(int iIndex, int iID, bool bSave, std::string sName)
{
	AddTag(iIndex, iID, bSave, sName, m_mPlayerTags);
}

void CPlayerlistUtils::RemoveTag(uint32_t friendsID, int iID, bool bSave, std::string sName, std::unordered_map<uint32_t, std::vector<int>>& mPlayerTags)
{
	if (!friendsID)
		return;

	auto& _vTags = mPlayerTags[friendsID];
	for (auto it = _vTags.begin(); it != _vTags.end(); it++)
	{
		if (iID == *it)
		{
			_vTags.erase(it);
			m_bSave = bSave;
			if (sName.length())
			{
				if (auto pTag = GetTag(iID))
					F::Output.TagsChanged(sName, "Removed", pTag->Color.ToHexA(), pTag->Name);
			}
			break;
		}
	}
}
void CPlayerlistUtils::RemoveTag(int iIndex, int iID, bool bSave, std::string sName, std::unordered_map<uint32_t, std::vector<int>>& mPlayerTags)
{
	if (const uint32_t friendsID = GetFriendsID(iIndex))
		RemoveTag(friendsID, iID, bSave, sName, mPlayerTags);
}
void CPlayerlistUtils::RemoveTag(uint32_t friendsID, int iID, bool bSave, std::string sName)
{
	RemoveTag(friendsID, iID, bSave, sName, m_mPlayerTags);
}
void CPlayerlistUtils::RemoveTag(int iIndex, int iID, bool bSave, std::string sName)
{
	RemoveTag(iIndex, iID, bSave, sName, m_mPlayerTags);
}

bool CPlayerlistUtils::HasTags(uint32_t friendsID, std::unordered_map<uint32_t, std::vector<int>>& mPlayerTags)
{
	if (!friendsID)
		return false;

	return mPlayerTags[friendsID].size();
}
bool CPlayerlistUtils::HasTags(int iIndex, std::unordered_map<uint32_t, std::vector<int>>& mPlayerTags)
{
	if (const uint32_t friendsID = GetFriendsID(iIndex))
		return HasTags(friendsID, mPlayerTags);
	return false;
}
bool CPlayerlistUtils::HasTags(uint32_t friendsID)
{
	return HasTags(friendsID, m_mPlayerTags);
}
bool CPlayerlistUtils::HasTags(int iIndex)
{
	return HasTags(iIndex, m_mPlayerTags);
}

bool CPlayerlistUtils::HasTag(uint32_t friendsID, int iID, std::unordered_map<uint32_t, std::vector<int>>& mPlayerTags)
{
	if (!friendsID)
		return false;

	auto it = std::ranges::find_if(mPlayerTags[friendsID], [iID](const auto& _iID) { return iID == _iID; });
	return it != mPlayerTags[friendsID].end();
}
bool CPlayerlistUtils::HasTag(int iIndex, int iID, std::unordered_map<uint32_t, std::vector<int>>& mPlayerTags)
{
	if (const uint32_t friendsID = GetFriendsID(iIndex))
		return HasTag(friendsID, iID, mPlayerTags);
	return false;
}
bool CPlayerlistUtils::HasTag(uint32_t friendsID, int iID)
{
	return HasTag(friendsID, iID, m_mPlayerTags);
}
bool CPlayerlistUtils::HasTag(int iIndex, int iID)
{
	return HasTag(iIndex, iID, m_mPlayerTags);
}



int CPlayerlistUtils::GetPriority(uint32_t friendsID, bool bCache)
{
	if (bCache)
		return H::Entities.GetPriority(friendsID);

	const int iDefault = m_vTags[TagToIndex(DEFAULT_TAG)].Priority;
	if (!friendsID)
		return iDefault;

	if (HasTag(friendsID, TagToIndex(IGNORED_TAG)))
		return m_vTags[TagToIndex(IGNORED_TAG)].Priority;

	std::vector<int> vPriorities;
	for (auto& iID : m_mPlayerTags[friendsID])
	{
		auto pTag = GetTag(iID);
		if (pTag && !pTag->Label)
			vPriorities.push_back(pTag->Priority);
	}
	if (H::Entities.IsFriend(friendsID))
	{
		auto& tTag = m_vTags[TagToIndex(FRIEND_TAG)];
		if (!tTag.Label)
			vPriorities.push_back(tTag.Priority);
	}
	if (H::Entities.InParty(friendsID))
	{
		auto& tTag = m_vTags[TagToIndex(PARTY_TAG)];
		if (!tTag.Label)
			vPriorities.push_back(tTag.Priority);
	}
	if (H::Entities.IsF2P(friendsID))
	{
		auto& tTag = m_vTags[TagToIndex(F2P_TAG)];
		if (!tTag.Label)
			vPriorities.push_back(tTag.Priority);
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
	return m_vTags[TagToIndex(DEFAULT_TAG)].Priority;
}

PriorityLabel_t* CPlayerlistUtils::GetSignificantTag(uint32_t friendsID, int iMode)
{
	if (!friendsID)
		return nullptr;

	std::vector<PriorityLabel_t*> vTags;
	if (!iMode || iMode == 1)
	{
		if (HasTag(friendsID, TagToIndex(IGNORED_TAG)))
			return &m_vTags[TagToIndex(IGNORED_TAG)];

		for (auto& iID : m_mPlayerTags[friendsID])
		{
			PriorityLabel_t* _pTag = GetTag(iID);
			if (_pTag && !_pTag->Label)
				vTags.push_back(_pTag);
		}
		if (H::Entities.IsFriend(friendsID))
		{
			auto _pTag = &m_vTags[TagToIndex(FRIEND_TAG)];
			if (!_pTag->Label)
				vTags.push_back(_pTag);
		}
		if (H::Entities.InParty(friendsID))
		{
			auto _pTag = &m_vTags[TagToIndex(PARTY_TAG)];
			if (!_pTag->Label)
				vTags.push_back(_pTag);
		}
		if (H::Entities.IsF2P(friendsID))
		{
			auto _pTag = &m_vTags[TagToIndex(F2P_TAG)];
			if (!_pTag->Label)
				vTags.push_back(_pTag);
		}
	}
	if ((!iMode || iMode == 2) && !vTags.size())
	{
		for (auto& iID : m_mPlayerTags[friendsID])
		{
			PriorityLabel_t* _pTag = GetTag(iID);
			if (_pTag && _pTag->Label)
				vTags.push_back(_pTag);
		}
		if (H::Entities.IsFriend(friendsID))
		{
			auto _pTag = &m_vTags[TagToIndex(FRIEND_TAG)];
			if (_pTag->Label)
				vTags.push_back(_pTag);
		}
		if (H::Entities.InParty(friendsID))
		{
			auto _pTag = &m_vTags[TagToIndex(PARTY_TAG)];
			if (_pTag->Label)
				vTags.push_back(_pTag);
		}
		if (H::Entities.IsF2P(friendsID))
		{
			auto _pTag = &m_vTags[TagToIndex(F2P_TAG)];
			if (_pTag->Label)
				vTags.push_back(_pTag);
		}
	}
	if (!vTags.size())
		return nullptr;

	std::sort(vTags.begin(), vTags.end(), [&](const auto a, const auto b) -> bool
		{
			// sort by priority if unequal
			if (a->Priority != b->Priority)
				return a->Priority > b->Priority;

			return a->Name < b->Name;
		});
	return vTags.front();
}
PriorityLabel_t* CPlayerlistUtils::GetSignificantTag(int iIndex, int iMode)
{
	if (const uint32_t friendsID = GetFriendsID(iIndex))
		return GetSignificantTag(friendsID, iMode);
	return nullptr;
}

bool CPlayerlistUtils::IsIgnored(uint32_t friendsID)
{
	if (!friendsID)
		return false;

	const int iPriority = GetPriority(friendsID);
	const int iIgnored = m_vTags[TagToIndex(IGNORED_TAG)].Priority;
	return iPriority <= iIgnored;
}
bool CPlayerlistUtils::IsIgnored(int iIndex)
{
	if (const uint32_t friendsID = GetFriendsID(iIndex))
		return IsIgnored(friendsID);
	return false;
}

bool CPlayerlistUtils::IsPrioritized(uint32_t friendsID)
{
	if (!friendsID)
		return false;

	const int iPriority = GetPriority(friendsID);
	const int iDefault = m_vTags[TagToIndex(DEFAULT_TAG)].Priority;
	return iPriority > iDefault;
}
bool CPlayerlistUtils::IsPrioritized(int iIndex)
{
	if (const uint32_t friendsID = GetFriendsID(iIndex))
		return IsPrioritized(friendsID);
	return false;
}

const char* CPlayerlistUtils::GetPlayerName(int iIndex, const char* sDefault, int* pType)
{
	if (Vars::Visuals::UI::StreamerMode.Value)
	{
		if (iIndex == I::EngineClient->GetLocalPlayer())
		{
			if (Vars::Visuals::UI::StreamerMode.Value >= Vars::Visuals::UI::StreamerModeEnum::Local)
			{
				if (pType) *pType = 1;
				return "Local";
			}
		}
		else if (H::Entities.IsFriend(iIndex))
		{
			if (Vars::Visuals::UI::StreamerMode.Value >= Vars::Visuals::UI::StreamerModeEnum::Friends)
			{
				if (pType) *pType = 1;
				return "Friend";
			}
		}
		else if (H::Entities.InParty(iIndex))
		{
			if (Vars::Visuals::UI::StreamerMode.Value >= Vars::Visuals::UI::StreamerModeEnum::Party)
			{
				if (pType) *pType = 1;
				return "Party";
			}
		}
		else if (Vars::Visuals::UI::StreamerMode.Value >= Vars::Visuals::UI::StreamerModeEnum::All)
		{
			if (auto pTag = GetSignificantTag(iIndex, 0))
			{
				if (pType) *pType = 1;
				return pTag->Name.c_str();
			}
			else
			{
				if (pType) *pType = 1;
				auto pResource = H::Entities.GetPR();
				return !pResource || pResource->GetTeam(I::EngineClient->GetLocalPlayer()) != pResource->GetTeam(iIndex) ? "Enemy" : "Teammate";
			}
		}
	}
	if (const uint32_t friendsID = GetFriendsID(iIndex))
	{
		if (m_mPlayerAliases.contains(friendsID))
		{
			if (pType) *pType = 2;
			return m_mPlayerAliases[friendsID].c_str();
		}
	}
	return sDefault;
}



void CPlayerlistUtils::UpdatePlayers()
{
	static Timer tTimer = {};
	if (!tTimer.Run(1.f))
		return;

	std::lock_guard lock(m_mutex);
	m_vPlayerCache.clear();

	auto pResource = H::Entities.GetPR();
	if (!pResource)
		return;

	for (int n = 1; n <= I::EngineClient->GetMaxClients(); n++)
	{
		if (!pResource->GetValid(n) || !pResource->GetConnected(n))
			continue;

		PlayerInfo_t pi{};
		auto friendsID = pResource->GetAccountID(n);
		auto sName = pResource->GetPlayerName(n);
		m_vPlayerCache.emplace_back(
			sName ? sName : "",
			friendsID,
			pResource->GetUserID(n),
			pResource->GetTeam(n),
			pResource->IsAlive(n),
			n == I::EngineClient->GetLocalPlayer(),
			!I::EngineClient->GetPlayerInfo(n, &pi) || pi.fakeplayer,
			H::Entities.IsFriend(friendsID),
			H::Entities.InParty(friendsID),
			H::Entities.IsF2P(friendsID),
			H::Entities.GetLevel(friendsID)
		);
	}
}