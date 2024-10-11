#include "../SDK/SDK.h"

MAKE_SIGNATURE(CBaseClientState_ForceFullUpdate, "engine.dll", "40 53 48 83 EC ? 83 B9 ? ? ? ? ? 48 8B D9 74 ? E8", 0x0);
MAKE_SIGNATURE(Cmd_Exec_f, "engine.dll", "48 89 5C 24 ? 55 56 57 41 56 41 57 48 8D AC 24 ? ? ? ? B8", 0x0);
MAKE_SIGNATURE(ClientDLL_FrameStageNotify, "engine.dll", "4C 8B DC 56 48 83 EC", 0x0);

struct CriticalStorage
{
	float m_flCritTokenBucket;
	int m_nCritChecks;
	int m_nCritSeedRequests;
};
std::unordered_map<int, CriticalStorage> mCriticalStorage = {};
int iMode = 0; // 0 - idle, 1 - store, 2 - restore

MAKE_HOOK(CBaseClientState_ForceFullUpdate, S::CBaseClientState_ForceFullUpdate(), void, __fastcall,
	void* rcx)
{
	iMode = 1;

	CALL_ORIGINAL(rcx);
}

MAKE_HOOK(Cmd_Exec_f, S::Cmd_Exec_f(), void, __fastcall,
	const CCommand& args)
{
	if (iMode)
	{
		SDK::Output("ForceFullUpdate", "Failed to restore weapon crit data!", { 255, 100, 100, 255 }, Vars::Debug::Logging.Value);
		mCriticalStorage.clear();
		iMode = 0;
	}

	CALL_ORIGINAL(args);
}

MAKE_HOOK(ClientDLL_FrameStageNotify, S::ClientDLL_FrameStageNotify(), void, __fastcall,
	ClientFrameStage_t curStage)
{
	if (!iMode)
		return CALL_ORIGINAL(curStage);

	CTFPlayer* pLocal = I::ClientEntityList->GetClientEntity(I::EngineClient->GetLocalPlayer())->As<CTFPlayer>();
	CTFWeaponBase* pWeapon = pLocal ? pLocal->m_hActiveWeapon().Get()->As<CTFWeaponBase>() : nullptr;

	if (!pWeapon)
	{
		iMode = 2;
		return CALL_ORIGINAL(curStage);
	}

	switch (curStage)
	{
	case FRAME_START:
		if (iMode == 1)
		{
			std::unordered_map<int, bool> mWeapons = {};
			auto hWeapons = pLocal->GetMyWeapons();
			if (!hWeapons)
				break;

			for (size_t i = 0; hWeapons[i]; i++)
			{
				auto pWeapon = pLocal->GetWeaponFromSlot(int(i));
				if (!pWeapon)
					break;

				const int iSlot = pWeapon->GetSlot();
				mCriticalStorage[iSlot].m_flCritTokenBucket = pWeapon->m_flCritTokenBucket();
				mCriticalStorage[iSlot].m_nCritChecks = pWeapon->m_nCritChecks();
				mCriticalStorage[iSlot].m_nCritSeedRequests = pWeapon->m_nCritSeedRequests();
			}
		}
		break;
	case FRAME_NET_UPDATE_POSTDATAUPDATE_START:
		if (iMode == 2 && !mCriticalStorage.empty())
		{
			iMode = 0;

			for (auto& [iSlot, tStorage] : mCriticalStorage)
			{
				auto pWeapon = pLocal->GetWeaponFromSlot(iSlot);
				if (!pWeapon)
					break;

				pWeapon->m_flCritTokenBucket() = tStorage.m_flCritTokenBucket;
				pWeapon->m_nCritChecks() = tStorage.m_nCritChecks;
				pWeapon->m_nCritSeedRequests() = tStorage.m_nCritSeedRequests;

				if (Vars::Debug::Logging.Value) I::CVar->ConsolePrintf("\n");
				SDK::Output("ForceFullUpdate", std::format("{}: pWeapon->m_flCritTokenBucket() = {}", iSlot, tStorage.m_flCritTokenBucket).c_str(), { 100, 255, 150, 255 }, Vars::Debug::Logging.Value);
				SDK::Output("ForceFullUpdate", std::format("{}: pWeapon->m_nCritChecks() = {}", iSlot, tStorage.m_nCritChecks).c_str(), { 100, 255, 150, 255 }, Vars::Debug::Logging.Value);
				SDK::Output("ForceFullUpdate", std::format("{}: pWeapon->m_nCritSeedRequests() = {}", iSlot, tStorage.m_nCritSeedRequests).c_str(), { 100, 255, 150, 255 }, Vars::Debug::Logging.Value);
			}
		}
	}

	CALL_ORIGINAL(curStage);
}

/*
int iOld = -1; float flOld = -1.f; bool bOld = false;
MAKE_HOOK(ClientDLL_FrameStageNotify, S::ClientDLL_FrameStageNotify(), void, __fastcall,
	ClientFrameStage_t curStage)
{
	CTFPlayer* pLocal = I::ClientEntityList->GetClientEntity(I::EngineClient->GetLocalPlayer())->As<CTFPlayer>();
	CTFWeaponBase* pWeapon = pLocal ? pLocal->m_hActiveWeapon().Get()->As<CTFWeaponBase>() : nullptr;

	if (iMode || Vars::Debug::Info.Value)
	{
		float flNew = pWeapon ? pWeapon->m_flCritTokenBucket() : -1.f;
		int iNew = curStage; bool bNew = false;
		SDK::Output("FrameStageNotify", std::format("Pre ({}, {})", flNew, iNew).c_str(), { 255, 0, 255, 255 }, Vars::Debug::Logging.Value);

		if (flOld != flNew)
			SDK::Output("FrameStageNotify", std::format("{} -> {}, {} -> {}, {} -> {}", flOld, flNew, iOld, iNew, bOld, bNew).c_str());

		flOld = flNew;
		if (pWeapon)
			iOld = iNew, bOld = bNew;
	}

	CALL_ORIGINAL(curStage);

	if (iMode || Vars::Debug::Info.Value)
	{
		float flNew = pWeapon ? pWeapon->m_flCritTokenBucket() : -1.f;
		int iNew = curStage; bool bNew = true;
		SDK::Output("FrameStageNotify", std::format("Post ({}, {})", flNew, iNew).c_str(), { 255, 0, 255, 255 }, Vars::Debug::Logging.Value);

		if (flOld != flNew)
			SDK::Output("FrameStageNotify", std::format("{} -> {}, {} -> {}, {} -> {}", flOld, flNew, iOld, iNew, bOld, bNew).c_str());

		flOld = flNew;
		if (pWeapon)
			iOld = iNew, bOld = bNew;
	}

	if ((iMode || Vars::Debug::Info.Value) && Vars::Debug::Logging.Value)
		I::CVar->ConsolePrintf("\n");
}
*/