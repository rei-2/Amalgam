#include "../SDK/SDK.h"

#include "../Features/Resolver/Resolver.h"

MAKE_HOOK(CTFPlayer_UpdateClientSideAnimation, S::CTFPlayer_UpdateClientSideAnimation(), void,
	void* rcx)
{
	DEBUG_RETURN(CTFPlayer_UpdateClientSideAnimation, rcx);

	auto pPlayer = reinterpret_cast<CTFPlayer*>(rcx);
	if ((!Vars::Visuals::Removals::Interpolation.Value && !F::Resolver.GetAngles(pPlayer) || G::UpdatingAnims)
		&& (pPlayer->entindex() != I::EngineClient->GetLocalPlayer() || I::EngineClient->IsPlayingDemo()))
		CALL_ORIGINAL(rcx);
}