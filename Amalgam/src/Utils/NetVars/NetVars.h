#pragma once
#include "../Feature/Feature.h"
#include "../../SDK/Definitions/Misc/dt_recv.h"
#include <cstdint>

class CNetVars
{
public:
	int GetOffset(RecvTable* pTable, const char* szNetVar);
	int GetNetVar(const char* szClass, const char* szNetVar);
};

ADD_FEATURE_CUSTOM(CNetVars, NetVars, U);

#define NETVAR(_name, type, table, name) inline type& _name() \
{ \
	static int nOffset = U::NetVars.GetNetVar(table, name); \
	return *reinterpret_cast<type*>(uintptr_t(this) + nOffset); \
}

#define NETVAR_OFF(_name, type, table, name, offset) inline type& _name() \
{ \
	static int nOffset = U::NetVars.GetNetVar(table, name) + offset; \
	return *reinterpret_cast<type*>(uintptr_t(this) + nOffset); \
}

#define OFFSET(name, type, offset) inline type& name() \
{ \
	return *reinterpret_cast<type*>(uintptr_t(this) + offset); \
}

#define VIRTUAL(name, type, fn, base, index) inline type name() \
{ \
	return reinterpret_cast<type(*)(fn)>(U::Memory.GetVFunc(base, index))(base); \
}

#define CONDGET(name, conditions, cond) inline bool name() \
{ \
	return (conditions & cond); \
}