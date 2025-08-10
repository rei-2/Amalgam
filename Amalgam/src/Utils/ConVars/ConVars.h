#pragma once
#include "../Macros/Macros.h"
#include "../../SDK/Definitions/Misc/ConVar.h"
#include "../Hash/FNV1A.h"
#include <unordered_map>

class CConVars
{
private:
	std::unordered_map<uint32_t, ConVar*> mCVarMap = {};
	std::unordered_map<ConCommandBase*, int> mFlagMap = {};

public:
	ConVar* FindVar(const char* sCVar);

	void Initialize();
	void Unload();
};

ADD_FEATURE_CUSTOM(CConVars, ConVars, U);