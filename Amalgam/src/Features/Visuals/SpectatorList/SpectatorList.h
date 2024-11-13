#pragma once
#include "../../../SDK/SDK.h"
#include <unordered_map>

class CSpectatorList
{
private:
	struct Spectator_t
	{
		std::string m_sName;
		std::string m_sMode;
		int m_iRespawnIn;
		bool m_bRespawnTimeIncreased;
		bool m_bIsFriend;
		int m_iIndex;
	};

	std::vector<Spectator_t> m_vSpectators;
	std::unordered_map<int, float> m_mRespawnCache;

public:
	bool GetSpectators(CTFPlayer* pLocal);
	void Draw(CTFPlayer* pLocal);
};

ADD_FEATURE(CSpectatorList, SpectatorList)