#include "../SDK/SDK.h"

#include "../Features/Players/PlayerUtils.h"
#include "../Features/Misc/SpectateAll/SpectateAll.h"

MAKE_SIGNATURE(CPlayerResource_GetPlayerName, "client.dll", "48 89 5C 24 ? 56 48 83 EC ? 48 63 F2", 0x0);

MAKE_HOOK(CPlayerResource_GetPlayerName, S::CPlayerResource_GetPlayerName(), const char*,
	void* rcx, int iIndex)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::CPlayerResource_GetPlayerName[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx, iIndex);
#endif

	// Get the original name
	const char* originalName = CALL_ORIGINAL(rcx, iIndex);

	// Check if we should override name display during SpectateAll
	if (Vars::Competitive::Features::SpectateAll.Value && 
		F::SpectateAll.ShouldSpectate())
	{
		auto pLocal = H::Entities.GetLocal();
		if (pLocal && !pLocal->IsAlive())
		{
			// Get the spectated friendly player
			auto pSpectatedFriendly = pLocal->m_hObserverTarget().Get();
			if (pSpectatedFriendly && iIndex == pSpectatedFriendly->entindex())
			{
				// Get the enemy player we're actually interested in
				auto pEnemyPlayer = F::SpectateAll.GetCurrentSpectatedPlayer();
				if (pEnemyPlayer)
				{
					// Get enemy's name through PlayerInfo
					PlayerInfo_t pi{};
					if (I::EngineClient->GetPlayerInfo(pEnemyPlayer->entindex(), &pi))
					{
						// Return the enemy's name instead of the friendly's name
						return F::PlayerUtils.GetPlayerName(pEnemyPlayer->entindex(), pi.name);
					}
				}
			}
		}
	}

	// Default behavior - use PlayerUtils for normal name processing
	return F::PlayerUtils.GetPlayerName(iIndex, originalName);
}