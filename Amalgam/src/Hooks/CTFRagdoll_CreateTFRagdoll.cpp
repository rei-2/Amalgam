#include "../SDK/SDK.h"

MAKE_SIGNATURE(CTFRagdoll_CreateTFRagdoll, "client.dll", "48 89 4C 24 ? 55 53 56 57 48 8D AC 24 ? ? ? ? B8 ? ? ? ? E8 ? ? ? ? 48 2B E0 8B 91", 0x0);

#define Offset(type, ent, offset) *reinterpret_cast<type>(ent + offset)

void ClearEffects(CBaseEntity* pEntity)
{
	Offset(bool*, pEntity, 4273) = false; // Gib
	Offset(bool*, pEntity, 4274) = false; // Burning
	Offset(bool*, pEntity, 4275) = false; // Electrocuted
	Offset(bool*, pEntity, 4277) = false; // Dissolve
	Offset(bool*, pEntity, 4281) = false; // Ash
	Offset(bool*, pEntity, 4288) = false; // Gold
	Offset(bool*, pEntity, 4289) = false; // Ice
}

MAKE_HOOK(CTFRagdoll_CreateTFRagdoll, S::CTFRagdoll_CreateTFRagdoll(), void, __fastcall,
	void* ecx)
{
	if (Vars::Visuals::Ragdolls::NoRagdolls.Value)
		return;

	if (!Vars::Visuals::Ragdolls::Enabled.Value)
		return CALL_ORIGINAL(ecx);

	if (auto pEntity = static_cast<CBaseAnimating*>(ecx))
	{
		if (Vars::Visuals::Ragdolls::EnemyOnly.Value)
		{
			auto pLocal = H::Entities.GetLocal();
			if (pLocal && Offset(int*, pEntity, 4208) == pLocal->m_iTeamNum()) // Team offset
				return CALL_ORIGINAL(ecx);
		}

		ClearEffects(pEntity);

		Offset(bool*, pEntity, 4274) = Vars::Visuals::Ragdolls::Effects.Value & (1 << 0);
		Offset(bool*, pEntity, 4275) = Vars::Visuals::Ragdolls::Effects.Value & (1 << 1);
		Offset(bool*, pEntity, 4281) = Vars::Visuals::Ragdolls::Effects.Value & (1 << 2);
		Offset(bool*, pEntity, 4277) = Vars::Visuals::Ragdolls::Effects.Value & (1 << 3);
		Offset(bool*, pEntity, 4288) = Vars::Visuals::Ragdolls::Type.Value == 1;
		Offset(bool*, pEntity, 4289) = Vars::Visuals::Ragdolls::Type.Value == 2;

		pEntity->m_vecForce() *= Vars::Visuals::Ragdolls::Force.Value;
		pEntity->m_vecForce().x *= Vars::Visuals::Ragdolls::ForceHorizontal.Value;
		pEntity->m_vecForce().y *= Vars::Visuals::Ragdolls::ForceHorizontal.Value;
		pEntity->m_vecForce().z *= Vars::Visuals::Ragdolls::ForceVertical.Value;
	}

	CALL_ORIGINAL(ecx);
}