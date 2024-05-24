#pragma once
#include "../Feature/Feature.h"
#include "../../SDK/Definitions/Interfaces/IVEngineClient.h"
#include "../../SDK/Definitions/Misc/dt_recv.h"
#include <cstdint>

class CNetVars
{
public:
	int GetOffset(RecvTable* pTable, const char* szNetVar);
	int GetNetVar(const char* szClass, const char* szNetVar);
};

ADD_FEATURE_CUSTOM(CNetVars, NetVars, U);

#define NETVAR(_name, type, table, name) type& _name() \
{ \
	static int nOffset = U::NetVars.GetNetVar(table, name); \
	return *reinterpret_cast<type*>(std::uintptr_t(this) + nOffset); \
}

#define NETVAR_OFF(_name, type, table, name, offset) type& _name() \
{ \
	static int nOffset = U::NetVars.GetNetVar(table, name) + offset; \
	return *reinterpret_cast<type*>(std::uintptr_t(this) + nOffset); \
}

#define OFFSET(name, type, offset) type& name() \
{ \
	return *reinterpret_cast<type*>(std::uintptr_t(this) + offset); \
}

#define VIRTUAL(name, type, fn, base, index) type name() \
{ \
	return reinterpret_cast<fn>(U::Memory.GetVFunc(base, index))(base); \
}

#define CONDGET(name, conditions, cond) bool name() \
{ \
	return (conditions & cond); \
}