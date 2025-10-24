#pragma once
#include "../Macros/Macros.h"
#include "../../SDK/Definitions/Misc/ConVar.h"
#include "../Hash/FNV1A.h"
#include <unordered_map>

class CConVars
{
private:
	std::unordered_map<uint32_t, ConVar*> m_mCVarMap = {};
	std::unordered_map<ConCommandBase*, int> m_mFlagMap = {};

	bool m_bUnlocked = false;

public:
	bool Unlock();
	bool Restore();
	ConVar* FindVar(const char* sCVar);
};

ADD_FEATURE_CUSTOM(CConVars, ConVars, U);