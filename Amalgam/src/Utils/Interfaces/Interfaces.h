#pragma once
#include "../Macros/Macros.h"
#include <vector>

struct InterfaceInit_t
{
	void** m_pPtr;
	const char* m_sDLL;
	const char* m_sName;
	int8_t m_nType; // 0: find interface, 1: get export, 2: sig scan
	int8_t m_nOffset;
	int8_t m_nDereferenceCount;

	InterfaceInit_t(void** pPtr, const char* sDLL, const char* sName, int8_t nType, int8_t nOffset = 0, int8_t nDereferenceCount = 0);
};

#define MAKE_INTERFACE_VERSION(type, symbol, dll, version) namespace I { inline type *symbol = nullptr; } \
namespace MAKE_INTERFACE_SCOPE \
{\
	inline InterfaceInit_t symbol##InterfaceInit_t(reinterpret_cast<void**>(&I::symbol), dll, version, 0); \
}

#define MAKE_INTERFACE_EXPORT(type, symbol, dll, name, deref) namespace I { inline type *symbol = nullptr; } \
namespace MAKE_INTERFACE_SCOPE \
{\
	inline InterfaceInit_t symbol##InterfaceInit_t(reinterpret_cast<void**>(&I::symbol), dll, name, 1, 0, deref); \
}

#define MAKE_INTERFACE_SIGNATURE(type, symbol, dll, signature, offset, deref) namespace I { inline type *symbol = nullptr; } \
namespace MAKE_INTERFACE_SCOPE \
{\
	inline InterfaceInit_t symbol##InterfaceInit_t(reinterpret_cast<void**>(&I::symbol), dll, signature, 2, offset, deref); \
}

#define MAKE_INTERFACE_NULL(type, symbol) namespace I { inline type *symbol = nullptr; }

class CInterfaces
{
private:
	std::vector<InterfaceInit_t*> m_vInterfaces = {};
	bool m_bFailed = false;

public:
	bool Initialize();

	inline void AddInterface(InterfaceInit_t* pInterface)
	{
		m_vInterfaces.push_back(pInterface);
	}
};

ADD_FEATURE_CUSTOM(CInterfaces, Interfaces, U);