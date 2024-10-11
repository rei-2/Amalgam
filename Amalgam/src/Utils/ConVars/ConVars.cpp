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

ConVar* CConVars::FindVar(const char* cvarname)
{
	if (!mCVarMap.contains(FNV1A::Hash32Const(cvarname)))
		mCVarMap[FNV1A::Hash32Const(cvarname)] = I::CVar->FindVar(cvarname);
	return mCVarMap[FNV1A::Hash32Const(cvarname)];
}