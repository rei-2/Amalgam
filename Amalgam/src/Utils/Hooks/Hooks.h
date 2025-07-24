#pragma once
#include "../Macros/Macros.h"
#include <MinHook/MinHook.h>
#include <unordered_map>
#include <string>

class CHook
{
public:
	void* m_pOriginal = nullptr;
	void* m_pInitFunc = nullptr;

public:
	CHook(const std::string& sName, void* pInitFunc);

	inline void Create(void* pSrc, void* pDst)
	{
		MH_CreateHook(pSrc, pDst, &m_pOriginal);
	}

	template <typename T>
	inline T As() const
	{
		return reinterpret_cast<T>(m_pOriginal);
	}

	template <typename T, typename... Args>
	inline T Call(Args... args) const
	{
		return reinterpret_cast<T(__fastcall*)(Args...)>(m_pOriginal)(args...);
	}
};

#define MAKE_HOOK(name, address, type, ...) \
namespace Hooks \
{ \
	namespace name \
	{ \
		void Init(); \
		inline CHook Hook(#name, Init); \
		using FN = type(__fastcall*)(__VA_ARGS__); \
		type __fastcall Func(__VA_ARGS__); \
	} \
} \
void Hooks::name::Init() { Hook.Create(reinterpret_cast<void*>(address), Func); } \
type __fastcall Hooks::name::Func(__VA_ARGS__)

#define CALL_ORIGINAL Hook.As<FN>()

class CHooks
{
private:
	bool m_bFailed = false;

public:
	std::unordered_map<std::string, CHook*> m_mHooks = {};

public:
	bool Initialize();
	bool Unload();
};

ADD_FEATURE_CUSTOM(CHooks, Hooks, U);