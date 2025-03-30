#include "../SDK/SDK.h"

enum EHoliday
{
	kHoliday_None = 0,
	kHoliday_TFBirthday,
	kHoliday_Halloween,
	kHoliday_Christmas,
	kHoliday_CommunityUpdate,
	kHoliday_EOTL,
	kHoliday_Valentines,
	kHoliday_MeetThePyro,
	kHoliday_FullMoon,
	kHoliday_HalloweenOrFullMoon,
	kHoliday_HalloweenOrFullMoonOrValentines,
	kHoliday_AprilFools,
	kHolidayCount,
};

MAKE_SIGNATURE(TF_IsHolidayActive, "client.dll", "48 83 EC ? 48 8B 05 ? ? ? ? 44 8B C9", 0x0);
MAKE_SIGNATURE(CTFPlayer_FireEvent_IsHolidayActive_Call, "client.dll", "84 C0 74 ? 45 33 C9 C6 44 24 ? ? 4C 8B C6", 0x0);

MAKE_HOOK(TF_IsHolidayActive, S::TF_IsHolidayActive(), bool,
	int eHoliday)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::TF_IsHolidayActive[DEFAULT_BIND])
		return CALL_ORIGINAL(eHoliday);
#endif

	static const auto dwDesired = S::CTFPlayer_FireEvent_IsHolidayActive_Call();
	const auto dwRetAddr = uintptr_t(_ReturnAddress());
	
	return dwRetAddr == dwDesired ? true : CALL_ORIGINAL(eHoliday);
}