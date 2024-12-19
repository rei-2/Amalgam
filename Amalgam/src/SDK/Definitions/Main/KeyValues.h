#pragma once
#include "../Types.h"
#include <stdint.h>

enum types_t
{
	TYPE_NONE = 0,
	TYPE_STRING,
	TYPE_INT,
	TYPE_FLOAT,
	TYPE_PTR,
	TYPE_WSTRING,
	TYPE_COLOR,
	TYPE_UINT64,
	TYPE_NUMTYPES,
};

class KeyValues
{
private:
	int m_iKeyName;
	char* m_sValue;
	wchar_t* m_wsValue;

	union
	{
		int m_iValue;
		float m_flValue;
		void* m_pValue;
		unsigned char m_Color[4];
	};

	char m_iDataType;
	char m_bHasEscapeSequences;
	char m_bEvaluateConditionals;
	char unused[1];

	KeyValues* m_pPeer;
	KeyValues* m_pSub;
	KeyValues* m_pChain;

public:
	bool LoadFromBuffer(char const* resource_name, const char* buffer, void* file_system = 0, const char* path_id = 0);
	void Initialize(const char* name);
	KeyValues(const char* name);
	KeyValues* FindKey(const char* keyName, bool bCreate = false);

	const char* GetName() const;
	void SetName(const char* setName);
	int GetNameSymbol() const { return m_iKeyName; }

	int GetInt(const char* keyName, int defaultValue = 0);
	uint64_t GetUint64(const char* keyName, uint64_t defaultValue = 0);
	float GetFloat(const char* keyName, float defaultValue = 0.0f);
	const char* GetString(const char* keyName, const char* defaultValue = "");
	const wchar_t* GetWString(const char* keyName, const wchar_t* defaultValue = L"");
	void* GetPtr(const char* keyName, void* defaultValue = (void*)0);
	bool GetBool(const char* keyName, bool defaultValue = false);
	Color_t GetColor(const char* keyName);
	bool IsEmpty(const char* keyName);

	void SetWString(const char* keyName, const wchar_t* value);
	void SetString(const char* keyName, const char* value);
	void SetInt(const char* keyName, int value);
	void SetUint64(const char* keyName, uint64_t value);
	void SetFloat(const char* keyName, float value);
	void SetPtr(const char* keyName, void* value);
	void SetColor(const char* keyName, Color_t value);
	void SetBool(const char* keyName, bool value) { SetInt(keyName, value ? 1 : 0); }
};