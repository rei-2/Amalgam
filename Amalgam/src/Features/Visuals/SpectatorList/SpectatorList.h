#pragma once
#include "../../../SDK/SDK.h"
#include <unordered_map>

class CSpectatorList
{
private:
	struct Spectator_t
	{
		std::wstring Name;
		std::wstring Mode;
		int RespawnIn;
		bool RespawnTimeIncreased;
		bool IsFriend;
		int Team;
		int Index;
	};

	std::vector<Spectator_t> Spectators;
	std::unordered_map<int, float> RespawnCache;

public:
	int SpecListX = 30, SpecListY = 100;
	bool GetSpectators(CTFPlayer* pLocal);
	void Run(CTFPlayer* pLocal);
};

ADD_FEATURE(CSpectatorList, SpectatorList)