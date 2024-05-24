#include "../SDK/SDK.h"

#include "../Features/Backtrack/Backtrack.h"

MAKE_SIGNATURE(CBaseAnimating_SetupBones, "client.dll", "48 8B C4 44 89 40 ? 48 89 50 ? 55 53", 0x0);

MAKE_HOOK(CBaseAnimating_SetupBones, S::CBaseAnimating_SetupBones(), bool, __fastcall,
	void* ecx, matrix3x4* pBoneToWorldOut, int nMaxBones, int boneMask, float currentTime)
{
	if (Vars::Misc::Game::SetupBonesOptimization.Value && !F::Backtrack.bSettingUpBones)
	{
		auto pBaseEntity = reinterpret_cast<CBaseEntity*>(std::uintptr_t(ecx) - 8);
		if (pBaseEntity)
		{
			auto GetRootMoveParent = [&]()
				{
					auto pEntity = pBaseEntity;
					auto pParent = pBaseEntity->GetMoveParent();

					int i = 0;
					while (pParent)
					{
						if (i > 32) //XD
							break;
						i++;

						pEntity = pParent;
						pParent = pEntity->GetMoveParent();
					}

					return pEntity;
				};

			auto pOwner = GetRootMoveParent();
			auto pEntity = pOwner ? pOwner : pBaseEntity;
			if (pEntity->GetClassID() == ETFClassID::CTFPlayer && pEntity != H::Entities.GetLocal())
			{
				if (pBoneToWorldOut)
				{
					auto bones = pEntity->As<CBaseAnimating>()->GetCachedBoneData();
					if (bones)
						std::memcpy(pBoneToWorldOut, bones->Base(), sizeof(matrix3x4) * std::min(nMaxBones, bones->Count()));
				}

				return true;
			}
		}
	}

	return CALL_ORIGINAL(ecx, pBoneToWorldOut, nMaxBones, boneMask, currentTime);
}