#pragma once
#include "../Feature/Feature.h"
#include "../../SDK/Definitions/Misc/ConVar.h"
#include "../Hash/FNV1A.h"
#include <unordered_map>

class CConVars
{
private:
	std::unordered_map<uint32_t, ConVar*> mCVarMap;
	std::unordered_map<ConCommandBase*, int> mFlagMap;

public:
	void Initialize();
	void Unload();
	ConVar* FindVar(const char* sCVar);
};

ADD_FEATURE_CUSTOM(CConVars, ConVars, U);