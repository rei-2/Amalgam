#include "../SDK/SDK.h"
#include "../Features/Misc/SpectateAll/SpectateAll.h"

// Backup of original receive proxy
static RecvVarProxyFn oPlayerHealthProxy;

// Custom receive proxy for player health to support SpectateAll health override
void PlayerHealthProxy(const CRecvProxyData* pData, void* pStruct, void* pOut)
{
	// Call original proxy first to get the actual value
	oPlayerHealthProxy(pData, pStruct, pOut);

	// Check if we should override health display during SpectateAll
	if (Vars::Competitive::Features::SpectateAll.Value && 
		F::SpectateAll.ShouldSpectate())
	{
		auto pLocal = H::Entities.GetLocal();
		if (pLocal && !pLocal->IsAlive())
		{
			// Get the spectated friendly player
			auto pSpectatedFriendly = pLocal->m_hObserverTarget().Get();
			if (pSpectatedFriendly)
			{
				// Check if this health update is for the spectated friendly player
				int* pHealthOut = reinterpret_cast<int*>(pOut);
				
				// Calculate player index from struct offset (CPlayerResource health array)
				int playerIndex = (reinterpret_cast<uintptr_t>(pOut) - reinterpret_cast<uintptr_t>(pStruct)) / sizeof(int);
				
				// If this is the spectated friendly player's health data
				if (playerIndex == pSpectatedFriendly->entindex())
				{
					// Get the enemy player we're actually interested in
					auto pEnemyPlayer = F::SpectateAll.GetCurrentSpectatedPlayer();
					if (pEnemyPlayer && pEnemyPlayer->IsAlive())
					{
						// Override the health value with the enemy's health
						*pHealthOut = pEnemyPlayer->m_iHealth();
					}
				}
			}
		}
	}
}

namespace PlayerHealthProxyHook
{
	void Initialize()
	{
		// Hook the CPlayerResource m_iHealth proxy
		if (auto pProp = U::NetVars.GetNetProp("CPlayerResource", "m_iHealth"))
		{
			oPlayerHealthProxy = pProp->GetProxyFn();
			pProp->SetProxyFn(PlayerHealthProxy);
		}
	}

	void Shutdown()
	{
		// Restore original proxy
		if (auto pProp = U::NetVars.GetNetProp("CPlayerResource", "m_iHealth"))
		{
			if (oPlayerHealthProxy)
				pProp->SetProxyFn(oPlayerHealthProxy);
		}
	}
}