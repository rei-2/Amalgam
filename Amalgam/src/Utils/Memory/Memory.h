#pragma once
#include "../Macros/Macros.h"
#include <Windows.h>
#include <cstdint>
#include <vector>
#include <string>

class CMemory
{
public:
	std::vector<byte> PatternToByte(const char* szPattern);
	std::vector<int> PatternToInt(const char* szPattern);
	uintptr_t FindSignature(const char* szModule, const char* szPattern);
	PVOID FindInterface(const char* szModule, const char* szObject);
	std::string GetModuleOffset(void* pAddress) { return GetModuleOffset(uintptr_t(pAddress)); };
	std::string GetModuleOffset(uintptr_t uAddress);

	inline void* GetVirtual(void* p, size_t i)
	{
		auto vTable = *static_cast<void***>(p);
		return vTable[i];
	}

	template <size_t I, typename T, typename... Args>
	inline T CallVirtual(void* p, Args... args) const
	{
		auto vTable = *static_cast<void***>(p);
		return reinterpret_cast<T(__fastcall*)(void*, Args...)>(vTable[I])(p, args...);
	}

	template <size_t I, typename T, typename... Args>
	inline T CallVirtual(uintptr_t u, Args... args) const
	{
		auto p = reinterpret_cast<void*>(u);
		auto vTable = *static_cast<void***>(p);
		return reinterpret_cast<T(__fastcall*)(void*, Args...)>(vTable[I])(p, args...);
	}

	inline uintptr_t RelToAbs(uintptr_t uAddress, uintptr_t uOffset = 3)
	{
		return *reinterpret_cast<int32_t*>(uAddress + uOffset) + uAddress + sizeof(int32_t) + uOffset;
	}

	template <typename T>
	inline T GetModuleExport(const char* szModule, const char* szExport)
	{
		if (const auto hModule = GetModuleHandle(szModule))
			return reinterpret_cast<T>(GetProcAddress(hModule, szExport));
		return reinterpret_cast<T>(nullptr);
	}
};

ADD_FEATURE_CUSTOM(CMemory, Memory, U);

#define OFFSET(name, type, offset) inline type& name() \
{ \
	return *reinterpret_cast<type*>(uintptr_t(this) + offset); \
}

#define CONDGET(name, conditions, cond) inline bool name() \
{ \
	return conditions & cond; \
}

#define VIRTUAL(name, type, index, ...) inline type name() \
{ \
	return U::Memory.CallVirtual<index, type>(##__VA_ARGS__); \
}

#define VIRTUAL_ARGS(name, type, index, args, ...) inline type name##args \
{ \
	return U::Memory.CallVirtual<index, type>(##__VA_ARGS__); \
}