#pragma once
#include "../Feature/Feature.h"
#include <Windows.h>
#include <cstdint>
#include <vector>

class CMemory
{
public:
	std::vector<byte> PatternToByte(const char* szPattern);
	std::vector<int> PatternToInt(const char* szPattern);
	uintptr_t FindSignature(const char* szModule, const char* szPattern);
	PVOID FindInterface(const char* szModule, const char* szObject);

	inline void* GetVFunc(void* instance, size_t index)
	{
		const auto vtable = *static_cast<void***>(instance);
		return vtable[index];
	}

	inline uintptr_t RelToAbs(const uintptr_t address)
	{
		return *reinterpret_cast<std::int32_t*>(address + 0x3) + address + 0x7;
	}

	template <typename T>
	inline T GetModuleExport(const char* szModule, const char* szExport)
	{
		if (auto hModule = GetModuleHandle(szModule))
			return reinterpret_cast<T>(GetProcAddress(hModule, szExport));
		return reinterpret_cast<T>(nullptr);
	}
};

ADD_FEATURE_CUSTOM(CMemory, Memory, U)