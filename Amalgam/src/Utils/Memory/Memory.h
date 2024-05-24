#pragma once
#include "../Feature/Feature.h"
#include <Windows.h>
#include <cstdint>

class CMemory
{
public:
	std::uintptr_t FindSignature(const char* szModule, const char* szPattern);
	PVOID FindInterface(const char* szModule, const char* szObject);

	inline void* GetVFunc(void* instance, size_t index)
	{
		const auto vtable = *static_cast<void***>(instance);
		return vtable[index];
	}

	inline std::uintptr_t RelToAbs(const std::uintptr_t address)
	{
		return *reinterpret_cast<std::int32_t*>(address + 0x3) + address + 0x7;
	}
};

ADD_FEATURE_CUSTOM(CMemory, Memory, U)