#pragma once
#include "../../SDK/SDK.h"

class CPlayerlistCore
{
	void SavePlayers();
	void LoadPlayers();
	void SaveTags();
	void LoadTags();

public:
	void Run();
};

ADD_FEATURE(CPlayerlistCore, PlayerCore)