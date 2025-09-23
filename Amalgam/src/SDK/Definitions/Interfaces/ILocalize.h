#pragma once
#include "Interface.h"

typedef unsigned long StringIndex_t;

class KeyValues;

class ILocalize
{
public:
	virtual bool AddFile(const char* fileName, const char* pPathID = NULL, bool bIncludeFallbackSearchPaths = false) = 0;
	virtual void RemoveAll() = 0;
	virtual wchar_t* Find(char const* tokenName) = 0;
	virtual StringIndex_t FindIndex(const char* tokenName) = 0;
	virtual const char* GetNameByIndex(StringIndex_t index) = 0;
	virtual wchar_t* GetValueByIndex(StringIndex_t index) = 0;
	virtual StringIndex_t GetFirstStringIndex() = 0;
	virtual StringIndex_t GetNextStringIndex(StringIndex_t index) = 0;
	virtual void AddString(const char* tokenName, wchar_t* unicodeString, const char* fileName) = 0;
	virtual void SetValueByIndex(StringIndex_t index, wchar_t* newValue) = 0;
	virtual bool SaveToFile(const char* fileName) = 0;
	virtual int GetLocalizationFileCount() = 0;
	virtual const char* GetLocalizationFileName(int index) = 0;
	virtual const char* GetFileNameByIndex(StringIndex_t index) = 0;
	virtual void ReloadLocalizationFiles() = 0;
	virtual const char* FindAsUTF8(const char* pchTokenName) = 0;
	virtual void ConstructString(wchar_t* unicodeOutput, int unicodeBufferSizeInBytes, const char* tokenName, KeyValues* localizationVariables) = 0;
	virtual void ConstructString(wchar_t* unicodeOutput, int unicodeBufferSizeInBytes, StringIndex_t unlocalizedTextSymbol, KeyValues* localizationVariables) = 0;
};

MAKE_INTERFACE_VERSION(ILocalize, Localize, "vgui2.dll", "VGUI_Localize005");