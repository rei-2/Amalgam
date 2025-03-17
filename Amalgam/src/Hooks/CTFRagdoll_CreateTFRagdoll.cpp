#include "../SDK/SDK.h"

MAKE_SIGNATURE(CTFRagdoll_CreateTFRagdoll, "client.dll", "48 89 4C 24 ? 55 53 56 57 48 8D AC 24 ? ? ? ? B8 ? ? ? ? E8 ? ? ? ? 48 2B E0 8B 91", 0x0);

MAKE_HOOK(CTFRagdoll_CreateTFRagdoll, S::CTFRagdoll_CreateTFRagdoll(), void,
	void* rcx)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::CTFRagdoll_CreateTFRagdoll.Map[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx);
#endif

	if (Vars::Visuals::Ragdolls::NoRagdolls.Value)
		return;

	if (!Vars::Visuals::Ragdolls::Enabled.Value)
		return CALL_ORIGINAL(rcx);

	if (auto pRagdoll = reinterpret_cast<CTFRagdoll*>(rcx))
	{
		if (Vars::Visuals::Ragdolls::EnemyOnly.Value)
		{
			auto pLocal = H::Entities.GetLocal();
			if (pLocal && pRagdoll->m_iTeam() == pLocal->m_iTeamNum())
				return CALL_ORIGINAL(rcx);
		}

		pRagdoll->m_bGib() = false;
		pRagdoll->m_bBurning() = Vars::Visuals::Ragdolls::Effects.Value & Vars::Visuals::Ragdolls::EffectsEnum::Burning;
		pRagdoll->m_bElectrocuted() = Vars::Visuals::Ragdolls::Effects.Value & Vars::Visuals::Ragdolls::EffectsEnum::Electrocuted;
		pRagdoll->m_bBecomeAsh() = Vars::Visuals::Ragdolls::Effects.Value & Vars::Visuals::Ragdolls::EffectsEnum::Ash;
		pRagdoll->m_bDissolving() = Vars::Visuals::Ragdolls::Effects.Value & Vars::Visuals::Ragdolls::EffectsEnum::Dissolve;
		pRagdoll->m_bGoldRagdoll() = Vars::Visuals::Ragdolls::Type.Value == Vars::Visuals::Ragdolls::TypeEnum::Gold;
		pRagdoll->m_bIceRagdoll() = Vars::Visuals::Ragdolls::Type.Value == Vars::Visuals::Ragdolls::TypeEnum::Ice;

		pRagdoll->m_vecForce() *= Vars::Visuals::Ragdolls::Force.Value;
		pRagdoll->m_vecForce().x *= Vars::Visuals::Ragdolls::ForceHorizontal.Value;
		pRagdoll->m_vecForce().y *= Vars::Visuals::Ragdolls::ForceHorizontal.Value;
		pRagdoll->m_vecForce().z *= Vars::Visuals::Ragdolls::ForceVertical.Value;
	}

	CALL_ORIGINAL(rcx);
}