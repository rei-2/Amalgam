#pragma once
#include "../../../Utils/Interfaces/Interfaces.h"
#include "../../../Utils/Signatures/Signatures.h"

MAKE_SIGNATURE(CAttributeManager_AttribHookFloat, "client.dll", "4C 8B DC 49 89 5B ? 49 89 6B ? 56 57 41 54 41 56 41 57 48 83 EC ? 48 8B 3D ? ? ? ? 4C 8D 35", 0x0);

class IBaseInterface
{
public:
	virtual	~IBaseInterface() {}
};

typedef void* (*CreateInterfaceFn)(const char* pName, int* pReturnCode);

namespace SDK
{
	inline float AttribHookValue(float value, const char* name, void* econent, void* buffer = 0, bool isGlobalConstString = true)
	{
		return S::CAttributeManager_AttribHookFloat.Call<float>(value, name, econent, buffer, isGlobalConstString);
	}
}