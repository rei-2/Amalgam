#include "ConVars.h"

#include "../../SDK/Definitions/Interfaces/ICVar.h"

bool CConVars::Unlock()
{
	if (!m_bUnlocked)
	{
		ConCommandBase* pCmdBase = I::CVar->GetCommands();
		while (pCmdBase != nullptr)
		{
			m_mFlagMap[pCmdBase] = pCmdBase->m_nFlags;
			pCmdBase->m_nFlags &= ~(FCVAR_HIDDEN | FCVAR_DEVELOPMENTONLY | FCVAR_CHEAT | FCVAR_NOT_CONNECTED);
			pCmdBase = pCmdBase->m_pNext;
		}
		m_bUnlocked = true;

		return true;
	}
	return false;
}

bool CConVars::Restore()
{
	if (m_bUnlocked)
	{
		for (auto& [pCmdBase, nFlags] : m_mFlagMap)
			pCmdBase->m_nFlags = nFlags;
		m_mFlagMap.clear();
		m_bUnlocked = false;

		return true;
	}
	return false;
}

ConVar* CConVars::FindVar(const char* sCVar)
{
	auto uHash = FNV1A::Hash32(sCVar);
	if (!m_mCVarMap.contains(uHash))
		m_mCVarMap[uHash] = I::CVar->FindVar(sCVar);
	return m_mCVarMap[uHash];
}