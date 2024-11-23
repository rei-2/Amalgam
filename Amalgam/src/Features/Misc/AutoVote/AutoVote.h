#pragma once
#include "../../../SDK/SDK.h"

class CAutoVote
{
public:
	void UserMessage(bf_read& msgData);
};

ADD_FEATURE(CAutoVote, AutoVote);