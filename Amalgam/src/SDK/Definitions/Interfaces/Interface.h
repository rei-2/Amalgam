#pragma once
#include "../../../Utils/Interfaces/Interfaces.h"
#include "../../../Utils/Memory/Memory.h"
#include "../../../Utils/Signatures/Signatures.h"
#include "../../../Utils/NetVars/NetVars.h"

class IBaseInterface
{
public:
	virtual	~IBaseInterface() {}
};

typedef void* (*CreateInterfaceFn)(const char* pName, int* pReturnCode);