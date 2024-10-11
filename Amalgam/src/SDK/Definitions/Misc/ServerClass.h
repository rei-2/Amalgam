#pragma once
#include "dt_send.h"

class ServerClass
{
public:
	const char* GetName() { return m_pNetworkName; }

public:
	const char* m_pNetworkName;
	SendTable* m_pTable;
	ServerClass* m_pNext;
	int m_ClassID;
	int	m_InstanceBaselineIndex;
};