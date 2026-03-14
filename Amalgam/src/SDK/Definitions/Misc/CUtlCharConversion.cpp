#include "CUtlCharConversion.h"

#include <cstring>

#pragma warning (disable:4267)

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CUtlCharConversion::CUtlCharConversion(char nEscapeChar, const char* pDelimiter, int nCount, ConversionArray_t* pArray)
{
	m_nEscapeChar = nEscapeChar;
	m_pDelimiter = pDelimiter;
	m_nCount = nCount;
	m_nDelimiterLength = strlen(pDelimiter);
	m_nMaxConversionLength = 0;

	memset(m_pReplacements, 0, sizeof(m_pReplacements));

	for (int i = 0; i < nCount; ++i)
	{
		m_pList[i] = pArray[i].m_nActualChar;
		ConversionInfo_t& info = m_pReplacements[(unsigned char)m_pList[i]];
		//Assert(info.m_pReplacementString == 0);
		info.m_pReplacementString = pArray[i].m_pReplacementString;
		info.m_nLength = strlen(info.m_pReplacementString);
		if (info.m_nLength > m_nMaxConversionLength)
		{
			m_nMaxConversionLength = info.m_nLength;
		}
	}
}


//-----------------------------------------------------------------------------
// Escape character + delimiter
//-----------------------------------------------------------------------------
char CUtlCharConversion::GetEscapeChar() const
{
	return m_nEscapeChar;
}

const char* CUtlCharConversion::GetDelimiter() const
{
	return m_pDelimiter;
}

int CUtlCharConversion::GetDelimiterLength() const
{
	return m_nDelimiterLength;
}


//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
const char* CUtlCharConversion::GetConversionString(char c) const
{
	return m_pReplacements[(unsigned char)c].m_pReplacementString;
}

int CUtlCharConversion::GetConversionLength(char c) const
{
	return m_pReplacements[(unsigned char)c].m_nLength;
}

int CUtlCharConversion::MaxConversionLength() const
{
	return m_nMaxConversionLength;
}


//-----------------------------------------------------------------------------
// Finds a conversion for the passed-in string, returns length
//-----------------------------------------------------------------------------
char CUtlCharConversion::FindConversion(const char* pString, int* pLength)
{
	for (int i = 0; i < m_nCount; ++i)
	{
		if (!strcmp(pString, m_pReplacements[(unsigned char)m_pList[i]].m_pReplacementString))
		{
			*pLength = m_pReplacements[(unsigned char)m_pList[i]].m_nLength;
			return m_pList[i];
		}
	}

	*pLength = 0;
	return '\0';
}