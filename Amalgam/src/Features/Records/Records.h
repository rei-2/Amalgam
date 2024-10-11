#pragma once
#include "../../SDK/SDK.h"

class CRecords
{
	void OutputInfo(int flags, std::string sName, std::string sOutput, std::string sChat);

	void TagsOnJoin(std::string sName, uint32_t friendsID);
	void AliasOnJoin(std::string sName, uint32_t friendsID);
	bool m_bInfoOnJoin = false;

public:
	void Event(IGameEvent* pEvent, uint32_t uHash, CTFPlayer* pLocal);
	void UserMessage(bf_read& msgData);
	void CheatDetection(std::string name, std::string action, std::string reason);

	void TagsChanged(std::string sName, std::string sAction, std::string sColor, std::string sTag);
	void AliasChanged(std::string sName, std::string sAction, std::string sAlias);
};

ADD_FEATURE(CRecords, Records)