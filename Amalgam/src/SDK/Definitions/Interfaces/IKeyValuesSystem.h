#pragma once
#include "Interface.h"

typedef int HKeySymbol;
class IBaseFileSystem;
class KeyValues;

class IKeyValuesSystem
{
public:
	virtual void RegisterSizeofKeyValues(int size) = 0;
	virtual void* AllocKeyValuesMemory(int size) = 0;
	virtual void FreeKeyValuesMemory(void* pMem) = 0;
	virtual HKeySymbol GetSymbolForString(const char* name, bool bCreate = true) = 0;
	virtual const char* GetStringForSymbol(HKeySymbol symbol) = 0;
	virtual void AddKeyValuesToMemoryLeakList(void* pMem, HKeySymbol name) = 0;
	virtual void RemoveKeyValuesFromMemoryLeakList(void* pMem) = 0;
	virtual void AddFileKeyValuesToCache(const KeyValues* _kv, const char* resourceName, const char* pathID) = 0;
	virtual bool LoadFileKeyValuesFromCache(KeyValues* _outKv, const char* resourceName, const char* pathID, IBaseFileSystem* filesystem) const = 0;
	virtual void InvalidateCache() = 0;
	virtual void InvalidateCacheForFile(const char* resourceName, const char* pathID) = 0;
	virtual void SetKeyValuesExpressionSymbol(const char* name, bool bValue) = 0;
	virtual bool GetKeyValuesExpressionSymbol(const char* name) = 0;
	virtual HKeySymbol GetSymbolForStringCaseSensitive(HKeySymbol& hCaseInsensitiveSymbol, const char* name, bool bCreate = true) = 0;
};

MAKE_INTERFACE_NULL(IKeyValuesSystem, KeyValuesSystem);