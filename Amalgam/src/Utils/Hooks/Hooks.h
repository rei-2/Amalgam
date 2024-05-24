#pragma once
#include "../Feature/Feature.h"
#include <MinHook/MinHook.h>
#include <unordered_map>
#include <string>

class CHook
{
public:
	void* m_pOriginal = nullptr;
	void* m_pInitFunc = nullptr;

public:
	CHook(std::string sName, void* pInitFunc);

	inline void Create(void* pSrc, void* pDst)
	{
		MH_CreateHook(pSrc, pDst, &m_pOriginal);
	}

	template <typename fn> inline fn Original()
	{
		return reinterpret_cast<fn>(m_pOriginal);
	}
};

#define MAKE_HOOK(name, address, type, callconvo, ...) namespace Hooks \
{\
	namespace name\
	{\
		void Init(); \
		inline CHook Hook(#name, Init); \
		using FN = type(callconvo *)(__VA_ARGS__); \
		type callconvo Func(__VA_ARGS__); \
	}\
} \
void Hooks::name::Init() { Hook.Create(reinterpret_cast<void*>(address), Func); } \
type callconvo Hooks::name::Func(__VA_ARGS__)

class CHooks
{
public:
	std::unordered_map<std::string, CHook*> m_mHooks = {};

public:
	void Initialize();
	void Unload();
};

ADD_FEATURE_CUSTOM(CHooks, Hooks, U);

#define CALL_ORIGINAL Hook.Original<FN>()