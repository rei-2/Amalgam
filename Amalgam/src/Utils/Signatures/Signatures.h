#pragma once
#include "../Feature/Feature.h"
#include <cstdint>
#include <vector>

class CSignature
{
private:
	uintptr_t m_dwVal = 0x0;
	const char* m_pszDLLName = {};
	const char* m_pszSignature = {};
	int m_nOffset = 0;
	const char* m_pszName = {};

public:
	CSignature(const char* sDLLName, const char* sSignature, int nOffset, const char* sName);

	bool Initialize();

	inline uintptr_t operator()()
	{
		return m_dwVal;
	}

	template <typename T>
	inline T As()
	{
		return reinterpret_cast<T>(m_dwVal);
	}

	template <typename T, typename... Args> 
	inline T Call(Args... args) const
	{
		return reinterpret_cast<T(__fastcall*)(Args...)>(m_dwVal)(args...);
	}
};

#define MAKE_SIGNATURE(name, dll, sig, offset) namespace S { inline CSignature name(dll, sig, offset, #name); }

class CSignatures
{
private:
	std::vector<CSignature*> m_vSignatures = {};
	bool m_bFailed = false;

public:
	bool Initialize();

	inline void AddSignature(CSignature* pSignature)
	{
		m_vSignatures.push_back(pSignature);
	}
};

ADD_FEATURE_CUSTOM(CSignatures, Signatures, U);