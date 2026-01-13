#include "PlayerUtils.h"

#include "../ImGui/Menu/Menu.h"
#include "../Output/Output.h"
#include "../../SDK/Definitions/Types.h"

uint32_t CPlayerlistUtils::GetAccountID(int iIndex)
{
	auto pResource = H::Entities.GetResource();
	if (pResource && pResource->m_bValid(iIndex) && !pResource->IsFakePlayer(iIndex))
		return pResource->m_iAccountID(iIndex);
	return 0;
}

int CPlayerlistUtils::GetIndex(uint32_t uAccountID)
{
	auto pResource = H::Entities.GetResource();
	if (!pResource)
		return 0;
	for (int n = 1; n <= I::EngineClient->GetMaxClients(); n++)
	{
		if (pResource->m_bValid(n) && !pResource->IsFakePlayer(n) && pResource->m_iAccountID(n) == uAccountID)
			return n;
	}
	return 0;
}

PriorityLabel_t* CPlayerlistUtils::GetTag(int iID)
{
	if (iID > -1 && iID < m_vTags.size())
		return &m_vTags[iID];

	return nullptr;
}

int CPlayerlistUtils::GetTag(const std::string& sTag)
{
	auto uHash = FNV1A::Hash32(sTag.c_str());

	int iID = -1;
	for (auto& tTag : m_vTags)
	{
		iID++;
		if (uHash == FNV1A::Hash32(tTag.m_sName.c_str()))
			return iID;
	}

	return -1;
}



void CPlayerlistUtils::AddTag(uint32_t uAccountID, int iID, bool bSave, const char* sName, std::unordered_map<uint32_t, std::vector<int>>& mPlayerTags)
{
	if (!uAccountID)
		return;

	if (!HasTag(uAccountID, iID))
	{
		mPlayerTags[uAccountID].push_back(iID);
		m_bSave = bSave;
		if (auto pTag = GetTag(iID); pTag && sName)
			F::Output.TagsChanged(sName, "Added", pTag->m_tColor.ToHexA().c_str(), pTag->m_sName.c_str());
	}
}
void CPlayerlistUtils::AddTag(int iIndex, int iID, bool bSave, const char* sName, std::unordered_map<uint32_t, std::vector<int>>& mPlayerTags)
{
	AddTag(GetAccountID(iIndex), iID, bSave, sName, mPlayerTags);
}
void CPlayerlistUtils::AddTag(uint32_t uAccountID, int iID, bool bSave, const char* sName)
{
	AddTag(uAccountID, iID, bSave, sName, m_mPlayerTags);
}
void CPlayerlistUtils::AddTag(int iIndex, int iID, bool bSave, const char* sName)
{
	AddTag(iIndex, iID, bSave, sName, m_mPlayerTags);
}

void CPlayerlistUtils::RemoveTag(uint32_t uAccountID, int iID, bool bSave, const char* sName, std::unordered_map<uint32_t, std::vector<int>>& mPlayerTags)
{
	if (!uAccountID)
		return;

	auto& vTags = mPlayerTags[uAccountID];
	for (auto it = vTags.begin(); it != vTags.end(); it++)
	{
		if (iID == *it)
		{
			vTags.erase(it);
			m_bSave = bSave;
			if (auto pTag = GetTag(iID); pTag && sName)
				F::Output.TagsChanged(sName, "Removed", pTag->m_tColor.ToHexA().c_str(), pTag->m_sName.c_str());
			break;
		}
	}
	if (vTags.empty())
		mPlayerTags.erase(uAccountID);
}
void CPlayerlistUtils::RemoveTag(int iIndex, int iID, bool bSave, const char* sName, std::unordered_map<uint32_t, std::vector<int>>& mPlayerTags)
{
	RemoveTag(GetAccountID(iIndex), iID, bSave, sName, mPlayerTags);
}
void CPlayerlistUtils::RemoveTag(uint32_t uAccountID, int iID, bool bSave, const char* sName)
{
	RemoveTag(uAccountID, iID, bSave, sName, m_mPlayerTags);
}
void CPlayerlistUtils::RemoveTag(int iIndex, int iID, bool bSave, const char* sName)
{
	RemoveTag(iIndex, iID, bSave, sName, m_mPlayerTags);
}

bool CPlayerlistUtils::HasTags(uint32_t uAccountID, std::unordered_map<uint32_t, std::vector<int>>& mPlayerTags)
{
	if (!uAccountID)
		return false;

	return !mPlayerTags[uAccountID].empty();
}
bool CPlayerlistUtils::HasTags(int iIndex, std::unordered_map<uint32_t, std::vector<int>>& mPlayerTags)
{
	return HasTags(GetAccountID(iIndex), mPlayerTags);
}
bool CPlayerlistUtils::HasTags(uint32_t uAccountID)
{
	return HasTags(uAccountID, m_mPlayerTags);
}
bool CPlayerlistUtils::HasTags(int iIndex)
{
	return HasTags(iIndex, m_mPlayerTags);
}

bool CPlayerlistUtils::HasTag(uint32_t uAccountID, int iID, std::unordered_map<uint32_t, std::vector<int>>& mPlayerTags)
{
	if (!uAccountID)
		return false;

	auto it = std::ranges::find_if(mPlayerTags[uAccountID], [iID](const auto& _iID) { return iID == _iID; });
	return it != mPlayerTags[uAccountID].end();
}
bool CPlayerlistUtils::HasTag(int iIndex, int iID, std::unordered_map<uint32_t, std::vector<int>>& mPlayerTags)
{
	return HasTag(GetAccountID(iIndex), iID, mPlayerTags);
}
bool CPlayerlistUtils::HasTag(uint32_t uAccountID, int iID)
{
	return HasTag(uAccountID, iID, m_mPlayerTags);
}
bool CPlayerlistUtils::HasTag(int iIndex, int iID)
{
	return HasTag(iIndex, iID, m_mPlayerTags);
}



int CPlayerlistUtils::GetPriority(uint32_t uAccountID, bool bCache)
{
	if (bCache)
		return H::Entities.GetPriority(uAccountID);

	const int iDefault = m_vTags[TagToIndex(DEFAULT_TAG)].m_iPriority;
	if (!uAccountID)
		return iDefault;

	if (HasTag(uAccountID, TagToIndex(IGNORED_TAG)))
		return m_vTags[TagToIndex(IGNORED_TAG)].m_iPriority;

	std::vector<int> vPriorities;
	if (m_mPlayerTags.contains(uAccountID))
	{
		for (auto& iID : m_mPlayerTags[uAccountID])
		{
			auto pTag = GetTag(iID);
			if (pTag && !pTag->m_bLabel)
				vPriorities.push_back(pTag->m_iPriority);
		}
	}
	if (H::Entities.IsFriend(uAccountID))
	{
		auto& tTag = m_vTags[TagToIndex(FRIEND_TAG)];
		if (!tTag.m_bLabel)
			vPriorities.push_back(tTag.m_iPriority);
	}
	if (H::Entities.InParty(uAccountID))
	{
		auto& tTag = m_vTags[TagToIndex(PARTY_TAG)];
		if (!tTag.m_bLabel)
			vPriorities.push_back(tTag.m_iPriority);
	}
	if (H::Entities.IsF2P(uAccountID))
	{
		auto& tTag = m_vTags[TagToIndex(F2P_TAG)];
		if (!tTag.m_bLabel)
			vPriorities.push_back(tTag.m_iPriority);
	}
	if (vPriorities.empty())
		return iDefault;

	std::sort(vPriorities.begin(), vPriorities.end(), std::greater<int>());
	return vPriorities.front();
}
int CPlayerlistUtils::GetPriority(int iIndex, bool bCache)
{
	if (bCache)
		return H::Entities.GetPriority(iIndex);

	return GetPriority(GetAccountID(iIndex));
}

PriorityLabel_t* CPlayerlistUtils::GetSignificantTag(uint32_t uAccountID, int iMode)
{
	if (!uAccountID)
		return nullptr;

	std::vector<PriorityLabel_t*> vTags;
	if (!iMode || iMode == 1)
	{
		if (HasTag(uAccountID, TagToIndex(IGNORED_TAG)))
			return &m_vTags[TagToIndex(IGNORED_TAG)];

		if (m_mPlayerTags.contains(uAccountID))
		{
			for (auto& iID : m_mPlayerTags[uAccountID])
			{
				PriorityLabel_t* _pTag = GetTag(iID);
				if (_pTag && !_pTag->m_bLabel)
					vTags.push_back(_pTag);
			}
		}
		if (H::Entities.IsFriend(uAccountID))
		{
			auto _pTag = &m_vTags[TagToIndex(FRIEND_TAG)];
			if (!_pTag->m_bLabel)
				vTags.push_back(_pTag);
		}
		if (H::Entities.InParty(uAccountID))
		{
			auto _pTag = &m_vTags[TagToIndex(PARTY_TAG)];
			if (!_pTag->m_bLabel)
				vTags.push_back(_pTag);
		}
		if (H::Entities.IsF2P(uAccountID))
		{
			auto _pTag = &m_vTags[TagToIndex(F2P_TAG)];
			if (!_pTag->m_bLabel)
				vTags.push_back(_pTag);
		}
	}
	if ((!iMode || iMode == 2) && !vTags.size())
	{
		if (m_mPlayerTags.contains(uAccountID))
		{
			for (auto& iID : m_mPlayerTags[uAccountID])
			{
				PriorityLabel_t* _pTag = GetTag(iID);
				if (_pTag && _pTag->m_bLabel)
					vTags.push_back(_pTag);
			}
		}
		if (H::Entities.IsFriend(uAccountID))
		{
			auto _pTag = &m_vTags[TagToIndex(FRIEND_TAG)];
			if (_pTag->m_bLabel)
				vTags.push_back(_pTag);
		}
		if (H::Entities.InParty(uAccountID))
		{
			auto _pTag = &m_vTags[TagToIndex(PARTY_TAG)];
			if (_pTag->m_bLabel)
				vTags.push_back(_pTag);
		}
		if (H::Entities.IsF2P(uAccountID))
		{
			auto _pTag = &m_vTags[TagToIndex(F2P_TAG)];
			if (_pTag->m_bLabel)
				vTags.push_back(_pTag);
		}
	}
	if (vTags.empty())
		return nullptr;

	std::sort(vTags.begin(), vTags.end(), [&](const PriorityLabel_t* a, const PriorityLabel_t* b) -> bool
		{
			// sort by priority if unequal
			if (a->m_iPriority != b->m_iPriority)
				return a->m_iPriority > b->m_iPriority;

			return a->m_sName < b->m_sName;
		});
	return vTags.front();
}
PriorityLabel_t* CPlayerlistUtils::GetSignificantTag(int iIndex, int iMode)
{
	return GetSignificantTag(GetAccountID(iIndex), iMode);
}

bool CPlayerlistUtils::IsIgnored(uint32_t uAccountID)
{
	const int iPriority = GetPriority(uAccountID);
	const int iIgnored = m_vTags[TagToIndex(IGNORED_TAG)].m_iPriority;
	return iPriority <= iIgnored;
}
bool CPlayerlistUtils::IsIgnored(int iIndex)
{
	return IsIgnored(GetAccountID(iIndex));
}

bool CPlayerlistUtils::IsPrioritized(uint32_t uAccountID)
{
	if (!uAccountID)
		return false;

	const int iPriority = GetPriority(uAccountID);
	const int iDefault = m_vTags[TagToIndex(DEFAULT_TAG)].m_iPriority;
	return iPriority > iDefault;
}
bool CPlayerlistUtils::IsPrioritized(int iIndex)
{
	return IsPrioritized(GetAccountID(iIndex));
}



int CPlayerlistUtils::GetNameType(int iIndex)
{
	if (Vars::Visuals::UI::StreamerMode.Value)
	{
		if (iIndex == I::EngineClient->GetLocalPlayer())
		{
			if (Vars::Visuals::UI::StreamerMode.Value >= Vars::Visuals::UI::StreamerModeEnum::Local)
				return NameTypeEnum::Local;
		}
		else if (H::Entities.IsFriend(iIndex))
		{
			if (Vars::Visuals::UI::StreamerMode.Value >= Vars::Visuals::UI::StreamerModeEnum::Friends)
				return NameTypeEnum::Friend;
		}
		else if (H::Entities.InParty(iIndex))
		{
			if (Vars::Visuals::UI::StreamerMode.Value >= Vars::Visuals::UI::StreamerModeEnum::Party)
				return NameTypeEnum::Party;
		}
		else if (Vars::Visuals::UI::StreamerMode.Value >= Vars::Visuals::UI::StreamerModeEnum::All)
			return NameTypeEnum::Player;
	}
	if (const uint32_t uAccountID = GetAccountID(iIndex); uAccountID && GetPlayerAlias(uAccountID))
		return NameTypeEnum::Custom;
	return NameTypeEnum::None;
}

int CPlayerlistUtils::GetNameType(uint32_t uAccountID)
{
	if (Vars::Visuals::UI::StreamerMode.Value)
	{
		if (uAccountID == I::SteamUser->GetSteamID().GetAccountID())
		{
			if (Vars::Visuals::UI::StreamerMode.Value >= Vars::Visuals::UI::StreamerModeEnum::Local)
				return NameTypeEnum::Local;
		}
		else if (H::Entities.IsFriend(uAccountID))
		{
			if (Vars::Visuals::UI::StreamerMode.Value >= Vars::Visuals::UI::StreamerModeEnum::Friends)
				return NameTypeEnum::Friend;
		}
		else if (H::Entities.InParty(uAccountID))
		{
			if (Vars::Visuals::UI::StreamerMode.Value >= Vars::Visuals::UI::StreamerModeEnum::Party)
				return NameTypeEnum::Party;
		}
		else if (Vars::Visuals::UI::StreamerMode.Value >= Vars::Visuals::UI::StreamerModeEnum::All)
			return NameTypeEnum::Player;
	}
	if (GetPlayerAlias(uAccountID))
		return NameTypeEnum::Custom;
	return NameTypeEnum::None;
}

const char* CPlayerlistUtils::GetPlayerName(int iIndex, const char* sDefault, int* pType)
{
	int iType = GetNameType(iIndex);
	if (pType) *pType = iType;

	switch (iType)
	{
	case NameTypeEnum::Local:
		return "Local";
	case NameTypeEnum::Friend:
		return "Friend";
	case NameTypeEnum::Party:
		return "Party";
	case NameTypeEnum::Player:
		if (auto pTag = GetSignificantTag(iIndex, 0))
			return pTag->m_sName.c_str();
		else if (auto pResource = H::Entities.GetResource(); pResource && pResource->m_bValid(iIndex))
			return pResource->m_iTeam(I::EngineClient->GetLocalPlayer()) != pResource->m_iTeam(iIndex) ? "Enemy" : "Teammate";
		return "Player";
	case NameTypeEnum::Custom:
		if (auto sAlias = GetPlayerAlias(GetAccountID(iIndex)))
			return sAlias->c_str();
	}
	return sDefault;
}

const char* CPlayerlistUtils::GetPlayerName(uint32_t uAccountID, const char* sDefault, int* pType)
{
	int iType = GetNameType(uAccountID);
	if (pType) *pType = iType;

	switch (iType)
	{
	case NameTypeEnum::Local:
		return "Local";
	case NameTypeEnum::Friend:
		return "Friend";
	case NameTypeEnum::Party:
		return "Party";
	case NameTypeEnum::Player:
		if (auto pTag = GetSignificantTag(uAccountID, 0))
			return pTag->m_sName.c_str();
		else if (auto pResource = H::Entities.GetResource(); (iType = GetIndex(uAccountID)) && pResource && pResource->m_bValid(iType))
			return pResource->m_iTeam(I::EngineClient->GetLocalPlayer()) != pResource->m_iTeam(iType) ? "Enemy" : "Teammate";
		return "Player";
	case NameTypeEnum::Custom:
		if (auto sAlias = GetPlayerAlias(uAccountID))
			return sAlias->c_str();
	}
	return sDefault;
}

const char* CPlayerlistUtils::GetPlayerName(int iIndex)
{
	auto pResource = H::Entities.GetResource();
	return pResource && pResource->IsValid(iIndex) ? pResource->GetName(iIndex) : PLAYER_ERROR_NAME;
}

const char* CPlayerlistUtils::GetPlayerName(uint32_t uAccountID)
{
	auto pResource = H::Entities.GetResource();
	int iIndex = GetIndex(uAccountID);
	return pResource && pResource->IsValid(iIndex) ? pResource->GetName(iIndex) : PLAYER_ERROR_NAME;
}



void CPlayerlistUtils::Store()
{
	static Timer tTimer = {};
	if (!tTimer.Run(1.f))
		return;

	std::lock_guard tLock(F::Menu.m_tMutex);
	m_vPlayerCache.clear();

	auto pResource = H::Entities.GetResource();
	if (!pResource)
		return;

	for (int n = 1; n <= I::EngineClient->GetMaxClients(); n++)
	{
		if (!pResource->m_bValid(n) || !pResource->m_bConnected(n))
			continue;

		uint32_t uAccountID = pResource->m_iAccountID(n);
		m_vPlayerCache.emplace_back(
			pResource->GetName(n),
			uAccountID,
			pResource->m_iUserID(n),
			pResource->m_iTeam(n),
			pResource->m_bAlive(n),
			n == I::EngineClient->GetLocalPlayer(),
			pResource->IsFakePlayer(n),
			H::Entities.IsFriend(uAccountID),
			H::Entities.InParty(uAccountID),
			H::Entities.IsF2P(uAccountID),
			H::Entities.GetLevel(uAccountID),
			H::Entities.GetParty(uAccountID)
		);
	}
}