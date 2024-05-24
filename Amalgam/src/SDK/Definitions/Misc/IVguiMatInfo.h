#pragma once
#include "IVguiMatInfoVar.h"

class IVguiMatInfo
{
public:
	virtual ~IVguiMatInfo() {}
	virtual IVguiMatInfoVar* FindVarFactory(const char* varName, bool* found) = 0;
	virtual int GetNumAnimationFrames() = 0;
};