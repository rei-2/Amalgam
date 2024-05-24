#include "NetVars.h"

#include "../Hash/FNV1A.h"

int CNetVars::GetOffset(RecvTable* pTable, const char* szNetVar)
{
	auto uHash = FNV1A::Hash(szNetVar);
	for (int i = 0; i < pTable->m_nProps; i++)
	{
		RecvProp Prop = pTable->m_pProps[i];

		if (uHash == FNV1A::Hash(Prop.m_pVarName))
			return Prop.GetOffset();

		if (auto DataTable = Prop.GetDataTable())
		{
			if (auto nOffset = GetOffset(DataTable, szNetVar))
				return nOffset + Prop.GetOffset();
		}
	}

	return 0;
}

int CNetVars::GetNetVar(const char* szClass, const char* szNetVar)
{
	ClientClass* pClasses = I::BaseClientDLL->GetAllClasses();

	auto uHash = FNV1A::Hash(szClass);
	for (auto pCurrNode = pClasses; pCurrNode; pCurrNode = pCurrNode->m_pNext)
	{
		if (uHash == FNV1A::Hash(pCurrNode->m_pNetworkName))
			return GetOffset(pCurrNode->m_pRecvTable, szNetVar);
	}

	return 0;
}