#pragma once
#include "../../SDK/SDK.h"

class CRecords
{
	void OutputInfo(int flags, std::string name, std::string string, std::string cstring);

	void TagsOnJoin(std::string name, uint32_t friendsID);
	bool bTagsOnJoin = false;

public:
	void Event(IGameEvent* pEvent, FNV1A_t uHash, CTFPlayer* pLocal);
	void UserMessage(bf_read& msgData);
	void CheatDetection(std::string name, std::string action, std::string reason);
	void TagsChanged(std::string name, std::string action, std::string color, std::string tag);
};

ADD_FEATURE(CRecords, Records)