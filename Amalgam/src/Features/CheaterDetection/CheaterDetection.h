#pragma once
#include "../../SDK/SDK.h"

struct AngleHistory_t
{
	Vec3 m_vAngle;
	bool m_bAttacking;
};

struct PlayerInfo
{
	uint32_t m_uFriendsID = 0;
	std::string m_sName = "";

	int m_iDetections = 0;

	struct PacketChoking_t
	{
		std::deque<int> m_vChokes = {}; // store last 3 choke counts
		bool m_bInfract = false; // infract the user for choking?
	} m_PacketChoking;

	struct AimFlicking_t
	{
		std::deque<AngleHistory_t> m_vAngles = {}; // store last 3 angles & if damage was dealt
	} m_AimFlicking;
					
	struct DuckSpeed_t
	{
		int m_iStartTick = 0;
	} m_DuckSpeed;
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

	void ReportChoke(CTFPlayer* pEntity, int iChoke);
	void ReportDamage(IGameEvent* pEvent);
	void Reset();
};

ADD_FEATURE(CCheaterDetection, CheaterDetection)