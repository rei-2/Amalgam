#pragma once

class CUtlCharConversion
{
public:
	struct ConversionArray_t
	{
		char m_nActualChar;
		const char* m_pReplacementString;
	};

	CUtlCharConversion(char nEscapeChar, const char* pDelimiter, int nCount, ConversionArray_t* pArray);
	char GetEscapeChar() const;
	const char* GetDelimiter() const;
	int GetDelimiterLength() const;

	const char* GetConversionString(char c) const;
	int GetConversionLength(char c) const;
	int MaxConversionLength() const;

	// Finds a conversion for the passed-in string, returns length
	virtual char FindConversion(const char* pString, int* pLength);

protected:
	struct ConversionInfo_t
	{
		int m_nLength;
		const char* m_pReplacementString;
	};

	char m_nEscapeChar;
	const char* m_pDelimiter;
	int m_nDelimiterLength;
	int m_nCount;
	int m_nMaxConversionLength;
	char m_pList[256];
	ConversionInfo_t m_pReplacements[256];
};