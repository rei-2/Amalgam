#pragma once
#include "../../SDK/SDK.h"

class COutput
{
private:
	bool m_bInfoOnJoin = false;

public:
	void Event(IGameEvent* pEvent, uint32_t uHash, CTFPlayer* pLocal);
	void UserMessage(bf_read& msgData);
	void CheatDetection(const char* sName, const char* sAction, const char* sReason);

	void TagsChanged(const char* sName, const char* sAction, const char* sColor, const char* sTag);
	void AliasChanged(const char* sName, const char* sAction, const char* sAlias);

	void ReportResolver(int iIndex, const char* sAction, const char* sAxis, float flValue);
	void ReportResolver(int iIndex, const char* sAction, const char* sAxis, bool bValue);
	void ReportResolver(int iIndex, const char* sAction, const char* sAxis, const char* sValue);
	void ReportResolver(const char* sMessage);

	void TagsOnJoin(const char* sName, uint32_t uAccountID);
	void AliasOnJoin(const char* sName, uint32_t uAccountID);
};

ADD_FEATURE(COutput, Output);