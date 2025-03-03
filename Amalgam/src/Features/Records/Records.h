#pragma once
#include "../../SDK/SDK.h"

class CRecords
{
	void OutputInfo(int flags, std::string sName, std::string sOutput, std::string sChat);

	bool m_bInfoOnJoin = false;

public:
	void Event(IGameEvent* pEvent, uint32_t uHash, CTFPlayer* pLocal);
	void UserMessage(bf_read& msgData);
	void CheatDetection(std::string sName, std::string sAction, std::string sReason);

	void TagsChanged(std::string sName, std::string sAction, std::string sColor, std::string sTag);
	void AliasChanged(std::string sName, std::string sAction, std::string sAlias);

	void ReportResolver(int iIndex, std::string sAction, std::string sAxis, float flValue);
	void ReportResolver(int iIndex, std::string sAction, std::string sAxis, bool bValue);
	void ReportResolver(int iIndex, std::string sAction, std::string sAxis, std::string sValue);
	void ReportResolver(std::string sMessage);

	void TagsOnJoin(std::string sName, uint32_t friendsID);
	void AliasOnJoin(std::string sName, uint32_t friendsID);
};

ADD_FEATURE(CRecords, Records)