#include "../SDK/SDK.h"

#include <cmath>

MAKE_SIGNATURE(CMatchInviteNotification_OnTick, "client.dll", "40 53 48 83 EC ? 48 8B D9 E8 ? ? ? ? F7 83", 0x0);

#define JOIN_TIME (10.0)
#define FULL_TIME (180.0 - JOIN_TIME)

static std::unordered_map<void*, double> s_mLastAutoJoinTime = {};

MAKE_HOOK(CMatchInviteNotification_OnTick, S::CMatchInviteNotification_OnTick(), void,
	void* rcx)
{
	DEBUG_RETURN(CMatchInviteNotification_OnTick, rcx);

	if (Vars::Misc::Queueing::ExtendQueue.Value)
	{
		auto dFloatTime = SDK::PlatFloatTime();
		auto& dAutoJoinTime = *reinterpret_cast<double*>(uintptr_t(rcx) + 616);

		// the offset above is tied to the exact layout of CMatchInviteNotification,
		// bail out if it no longer looks like a join timer so a game update cannot corrupt the panel
		if (!std::isfinite(dAutoJoinTime) || dAutoJoinTime <= 0.0 || dAutoJoinTime > dFloatTime + 3600.0)
			return CALL_ORIGINAL(rcx);

		auto& dLastAutoJoinTime = s_mLastAutoJoinTime[rcx];
		double dExtend = dAutoJoinTime != dLastAutoJoinTime ? FULL_TIME : 0.0;
		dLastAutoJoinTime = dAutoJoinTime = std::max(dAutoJoinTime + dExtend, dFloatTime + 1.0);

		// erase expired entries after iterating, erasing while iterating a map is undefined behavior
		std::vector<void*> vExpired = {};
		for (auto& [pMatchInviteNotification, dLastAutoJoinTime] : s_mLastAutoJoinTime)
		{
			if (dFloatTime > dLastAutoJoinTime)
				vExpired.push_back(pMatchInviteNotification);
		}
		for (auto pExpired : vExpired)
			s_mLastAutoJoinTime.erase(pExpired);
	}

	CALL_ORIGINAL(rcx);
}