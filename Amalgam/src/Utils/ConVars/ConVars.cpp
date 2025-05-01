#include "ConVars.h"

#include "../../SDK/Definitions/Interfaces/ICVar.h"

void CConVars::Initialize()
{
	ConCommandBase* pCmdBase = I::CVar->GetCommands();
	while (pCmdBase != nullptr)
	{
		mFlagMap[pCmdBase] = pCmdBase->m_nFlags;
		pCmdBase->m_nFlags &= ~(FCVAR_HIDDEN | FCVAR_DEVELOPMENTONLY | FCVAR_CHEAT | FCVAR_NOT_CONNECTED);
		pCmdBase = pCmdBase->m_pNext;
	}
}

void CConVars::Unload()
{
	for (auto& [pCmdBase, nFlags] : mFlagMap)
	{
		if (pCmdBase)
			pCmdBase->m_nFlags = nFlags;
	}
}

ConVar* CConVars::FindVar(const char* sCVar)
{
	auto uHash = FNV1A::Hash32(sCVar);
	if (!mCVarMap.contains(uHash))
		mCVarMap[uHash] = I::CVar->FindVar(sCVar);
	return mCVarMap[uHash];
}