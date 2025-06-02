#pragma once
#include "../Feature/Feature.h"
#include "../../SDK/Definitions/Misc/dt_recv.h"

class CNetVars
{
public:
	int GetOffset(RecvTable* pTable, const char* szNetVar);
	int GetNetVar(const char* szClass, const char* szNetVar);

	RecvProp* GetProp(RecvTable* pTable, const char* szNetVar);
	RecvProp* GetNetProp(const char* szClass, const char* szNetVar);
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

#define NETVAR_ARRAY(_name, type, table, name) inline type& _name(int iIndex) \
{ \
	static int nOffset = U::NetVars.GetNetVar(table, name); \
	return *reinterpret_cast<type*>(uintptr_t(this) + nOffset + iIndex * sizeof(type)); \
}

#define NETVAR_ARRAY_OFF(_name, type, table, name, offset) inline type& _name(int iIndex) \
{ \
	static int nOffset = U::NetVars.GetNetVar(table, name) + offset; \
	return *reinterpret_cast<type*>(uintptr_t(this) + nOffset + iIndex * sizeof(type)); \
}