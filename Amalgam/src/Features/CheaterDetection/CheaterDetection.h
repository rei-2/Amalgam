#pragma once
#include "../../SDK/SDK.h"
#include "../Backtrack/Backtrack.h"

struct PlayerInfo
{
	bool bShouldScan = false;
	uint32_t friendsID = 0;
	std::string sName = "";

	int iDetections = 0;

	std::deque<int> vChokes = {};					// store last 3 choke counts
	bool bChoke = false;							// infract the user for choking?
	std::deque<std::pair<Vec3, bool>> vAngles = {};	// store last 3 angles & if damage was dealt
	bool bDamage = false;
	int iDuckSpeed = 0;								// how many times in a row a user has been detected for duck speed
};

class CCheaterDetection
{
	bool ShouldScan();

	bool InvalidPitch(CTFPlayer* pEntity);
	bool IsChoking(CTFPlayer* pEntity);
	bool IsFlicking(CTFPlayer* pEntity);
	bool IsDuckSpeed(CTFPlayer* pEntity);

	void Infract(CTFPlayer* pEntity, std::string sReason);

	std::unordered_map<CTFPlayer*, PlayerInfo> mData;

public:
	void Run();
	void Fill();

	void ReportChoke(CTFPlayer* pEntity, int iChoke);
	void ReportDamage(IGameEvent* pEvent);
	void Reset();
};

ADD_FEATURE(CCheaterDetection, CheaterDetection)