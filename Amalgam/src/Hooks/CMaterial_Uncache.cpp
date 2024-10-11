#include "../SDK/SDK.h"

#include "../Features/Visuals/Materials/Materials.h"

MAKE_SIGNATURE(CMaterial_Uncache, "materialsystem.dll", "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 81 EC ? ? ? ? 48 8B F9", 0x0);

MAKE_HOOK(CMaterial_Uncache, S::CMaterial_Uncache(), void, __fastcall,
	IMaterial* rcx, bool bPreserveVars)
{
	if (rcx && F::Materials.mMatList.contains(rcx))
		return;

	CALL_ORIGINAL(rcx, bPreserveVars);
}