#include "../SDK/SDK.h"

MAKE_SIGNATURE(CBaseAnimating_SetupBones, "client.dll", "48 8B C4 44 89 40 ? 48 89 50 ? 55 53", 0x0);

MAKE_HOOK(CBaseAnimating_SetupBones, S::CBaseAnimating_SetupBones(), bool,
	void* rcx, matrix3x4* pBoneToWorldOut, int nMaxBones, int boneMask, float currentTime)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::CBaseAnimating_SetupBones.Map[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx, pBoneToWorldOut, nMaxBones, boneMask, currentTime);
#endif

	if (Vars::Misc::Game::SetupBonesOptimization.Value && !H::Entities.IsSettingUpBones())
	{
		auto pBaseEntity = reinterpret_cast<CBaseEntity*>(uintptr_t(rcx) - 8);
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
			if (pEntity->IsPlayer() && pEntity != H::Entities.GetLocal())
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

	return CALL_ORIGINAL(rcx, pBoneToWorldOut, nMaxBones, boneMask, currentTime);
}