#pragma once
#include "../../Definitions/Misc/ConVar.h"
#include "../../../Utils/Macros/Macros.h"
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
	bool Modify(bool bUnlock);

	ConVar* FindVar(const char* sCVar);
};

ADD_FEATURE_CUSTOM(CConVars, ConVars, H);