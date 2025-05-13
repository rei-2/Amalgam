#include "KeyValues.h"

#include "../../SDK.h"

#define Q_ARRAYSIZE(A) (sizeof(A)/sizeof((A)[0]))

MAKE_SIGNATURE(KeyValues_LoadFromBuffer, "engine.dll", "4C 89 4C 24 ? 48 89 4C 24 ? 55 56", 0x0);
MAKE_SIGNATURE(KeyValues_Initialize, "engine.dll", "40 53 48 83 EC ? 48 8B D9 C7 01", 0x0);
MAKE_SIGNATURE(KeyValues_GetSymbolForStringClassic, "engine.dll", "48 89 5C 24 ? 57 48 83 EC ? 0F B6 DA 48 8B F9", 0x0);
MAKE_SIGNATURE(KeyValues_GetStringForSymbolClassic, "engine.dll", "40 53 48 83 EC ? 8B D9 FF 15", 0x0);
MAKE_SIGNATURE(KeyValues_FindKey, "client.dll", "48 8B C4 53 57 41 56", 0x0);

int UnicodeToUTF8(const wchar_t* unicode, char* ansi, int ansiBufferSize)
{
	int result = WideCharToMultiByte(CP_UTF8, 0, unicode, -1, ansi, ansiBufferSize, NULL, NULL);
	ansi[ansiBufferSize - 1] = 0;
	return result;
}

int UTF8ToUnicode(const char* ansi, wchar_t* unicode, int unicodeBufferSizeInBytes)
{
	int chars = MultiByteToWideChar(CP_UTF8, 0, ansi, -1, unicode, unicodeBufferSizeInBytes / sizeof(wchar_t));
	unicode[(unicodeBufferSizeInBytes / sizeof(wchar_t)) - 1] = 0;
	return chars;
}


bool KeyValues::LoadFromBuffer(char const* resource_name, const char* buffer, void* file_system, const char* path_id)
{
	return S::KeyValues_LoadFromBuffer.Call<bool>(this, resource_name, buffer, file_system, path_id);
}

void KeyValues::Initialize(const char* name)
{
	S::KeyValues_Initialize.Call<KeyValues*>(this, name);
}

KeyValues::KeyValues(const char* name)
{
	Initialize(name);
}

void* KeyValues::operator new(size_t iAllocSize)
{
	return I::KeyValuesSystem->AllocKeyValuesMemory((int)iAllocSize);
}

void* KeyValues::operator new(size_t iAllocSize, int nBlockUse, const char* pFileName, int nLine)
{
	void* p = I::KeyValuesSystem->AllocKeyValuesMemory((int)iAllocSize);
	return p;
}

void KeyValues::operator delete(void* pMem)
{
	I::KeyValuesSystem->FreeKeyValuesMemory(pMem);
}

void KeyValues::operator delete(void* pMem, int nBlockUse, const char* pFileName, int nLine)
{
	I::KeyValuesSystem->FreeKeyValuesMemory(pMem);
}

KeyValues *KeyValues::FindKey(const char *keyName, bool bCreate)
{
	return S::KeyValues_FindKey.Call<KeyValues*>(this, keyName, bCreate);
}


const char* KeyValues::GetName() const
{
	return S::KeyValues_GetStringForSymbolClassic.Call<const char*>(m_iKeyName);
}

void KeyValues::SetName(const char* setName)
{
	m_iKeyName = S::KeyValues_GetSymbolForStringClassic.Call<int>(setName, true);
}


int KeyValues::GetInt(const char* keyName, int defaultValue)
{
	KeyValues* dat = FindKey(keyName, false);
	if (dat)
	{
		switch (dat->m_iDataType)
		{
		case TYPE_STRING:
			return atoi(dat->m_sValue);
		case TYPE_WSTRING:
			return _wtoi(dat->m_wsValue);
		case TYPE_FLOAT:
			return (int)dat->m_flValue;
		case TYPE_UINT64:
			return 0;
		case TYPE_INT:
		case TYPE_PTR:
		default:
			return dat->m_iValue;
		};
	}
	return defaultValue;
}

uint64_t KeyValues::GetUint64(const char* keyName, uint64_t defaultValue)
{
	KeyValues* dat = FindKey(keyName, false);
	if (dat)
	{
		switch (dat->m_iDataType)
		{
		case TYPE_STRING:
			return (uint64_t)_atoi64(dat->m_sValue);
		case TYPE_WSTRING:
			return _wtoi64(dat->m_wsValue);
		case TYPE_FLOAT:
			return (int)dat->m_flValue;
		case TYPE_UINT64:
			return *((uint64_t*)dat->m_sValue);
		case TYPE_INT:
		case TYPE_PTR:
		default:
			return dat->m_iValue;
		};
	}
	return defaultValue;
}

float KeyValues::GetFloat(const char* keyName, float defaultValue)
{
	KeyValues* dat = FindKey(keyName, false);
	if (dat)
	{
		switch (dat->m_iDataType)
		{
		case TYPE_STRING:
			return (float)atof(dat->m_sValue);
		case TYPE_WSTRING:
			return (float)_wtof(dat->m_wsValue);
		case TYPE_FLOAT:
			return dat->m_flValue;
		case TYPE_INT:
			return (float)dat->m_iValue;
		case TYPE_UINT64:
			return (float)(*((uint64_t*)dat->m_sValue));
		case TYPE_PTR:
		default:
			return 0.0f;
		};
	}
	return defaultValue;
}

const char* KeyValues::GetString(const char* keyName, const char* defaultValue)
{
	KeyValues* dat = FindKey(keyName, false);
	if (dat)
	{
		char buf[64];
		switch (dat->m_iDataType)
		{
		case TYPE_FLOAT:
			snprintf(buf, sizeof(buf), "%f", dat->m_flValue);
			SetString(keyName, buf);
			break;
		case TYPE_PTR:
			snprintf(buf, sizeof(buf), "%lld", (int64_t)(size_t)dat->m_pValue);
			SetString(keyName, buf);
			break;
		case TYPE_INT:
			snprintf(buf, sizeof(buf), "%d", dat->m_iValue);
			SetString(keyName, buf);
			break;
		case TYPE_UINT64:
			snprintf(buf, sizeof(buf), "%lld", *((uint64_t*)(dat->m_sValue)));
			SetString(keyName, buf);
			break;
		case TYPE_WSTRING:
		{
			char wideBuf[512];
			int result = UnicodeToUTF8(dat->m_wsValue, wideBuf, 512);
			if (result)
			{
				SetString(keyName, wideBuf);
			}
			else
			{
				return defaultValue;
			}
			break;
		}
		case TYPE_STRING:
			break;
		default:
			return defaultValue;
		};

		return dat->m_sValue;
	}
	return defaultValue;
}

const wchar_t* KeyValues::GetWString(const char* keyName, const wchar_t* defaultValue)
{
	KeyValues* dat = FindKey(keyName, false);
	if (dat)
	{
		wchar_t wbuf[64];
		switch (dat->m_iDataType)
		{
		case TYPE_FLOAT:
			swprintf(wbuf, Q_ARRAYSIZE(wbuf), L"%f", dat->m_flValue);
			SetWString(keyName, wbuf);
			break;
		case TYPE_PTR:
			swprintf(wbuf, Q_ARRAYSIZE(wbuf), L"%lld", (int64_t)(size_t)dat->m_pValue);
			SetWString(keyName, wbuf);
			break;
		case TYPE_INT:
			swprintf(wbuf, Q_ARRAYSIZE(wbuf), L"%d", dat->m_iValue);
			SetWString(keyName, wbuf);
			break;
		case TYPE_UINT64:
		{
			swprintf(wbuf, Q_ARRAYSIZE(wbuf), L"%lld", *((uint64_t*)(dat->m_sValue)));
			SetWString(keyName, wbuf);
		}
		break;

		case TYPE_WSTRING:
			break;
		case TYPE_STRING:
		{
			const auto bufSize = strlen(dat->m_sValue) + 1;
			wchar_t* pWBuf = new wchar_t[bufSize];
			const int result = UTF8ToUnicode(dat->m_sValue, pWBuf, static_cast<int>(bufSize) * sizeof(wchar_t));
			if (result >= 0)
			{
				SetWString(keyName, pWBuf);
			}
			else
			{
				delete[] pWBuf;
				return defaultValue;
			}
			delete[] pWBuf;
			break;
		}
		default:
			return defaultValue;
		};

		return (const wchar_t*)dat->m_wsValue;
	}
	return defaultValue;
}

void* KeyValues::GetPtr(const char* keyName, void* defaultValue)
{
	KeyValues* dat = FindKey(keyName, false);
	if (dat)
	{
		switch (dat->m_iDataType)
		{
		case TYPE_PTR:
			return dat->m_pValue;

		case TYPE_WSTRING:
		case TYPE_STRING:
		case TYPE_FLOAT:
		case TYPE_INT:
		case TYPE_UINT64:
		default:
			return NULL;
		};
	}
	return defaultValue;
}

bool KeyValues::GetBool(const char* keyName, bool defaultValue)
{
	if (FindKey(keyName))
	{
		/*if (optGotDefault)
			(*optGotDefault) = false;*/

		return 0 != GetInt(keyName, 0);
	}

	/*if (optGotDefault)
		(*optGotDefault) = true;*/

	return defaultValue;
}

Color_t KeyValues::GetColor(const char* keyName)
{
	Color_t color{ 0, 0, 0, 0 };
	KeyValues* dat = FindKey(keyName, false);
	if (dat)
	{
		if (dat->m_iDataType == TYPE_COLOR)
		{
			color.r = dat->m_Color[0];
			color.g = dat->m_Color[1];
			color.b = dat->m_Color[2];
			color.a = dat->m_Color[3];
		}
		else if (dat->m_iDataType == TYPE_FLOAT)
		{
			color.r = dat->m_flValue;
		}
		else if (dat->m_iDataType == TYPE_INT)
		{
			color.r = dat->m_iValue;
		}
		else if (dat->m_iDataType == TYPE_STRING)
		{
			float a = 0.0f, b = 0.0f, c = 0.0f, d = 0.0f;
			sscanf_s(dat->m_sValue, "%f %f %f %f", &a, &b, &c, &d);
			color.r = (unsigned char)a;
			color.g = (unsigned char)b;
			color.b = (unsigned char)c;
			color.a = (unsigned char)d;
		}
	}
	return color;
}

bool KeyValues::IsEmpty(const char* keyName)
{
	KeyValues* dat = FindKey(keyName, false);
	if (!dat)
		return true;

	if (dat->m_iDataType == TYPE_NONE && dat->m_pSub == NULL)
		return true;

	return false;
}


void KeyValues::SetWString(const char* keyName, const wchar_t* value)
{
	KeyValues* dat = FindKey(keyName, true);
	if (dat)
	{
		delete[] dat->m_wsValue;
		delete[] dat->m_sValue;
		dat->m_sValue = NULL;

		if (!value)
		{
			value = L"";
		}

		int len = int(wcslen(value));
		dat->m_wsValue = new wchar_t[len + 1];
		memcpy(dat->m_wsValue, value, (len + 1) * sizeof(wchar_t));

		dat->m_iDataType = TYPE_WSTRING;
	}
}

void KeyValues::SetString(const char* keyName, const char* value)
{
	KeyValues* dat = FindKey(keyName, true);

	if (dat)
	{
		if (dat->m_iDataType == TYPE_STRING && dat->m_sValue == value)
		{
			return;
		}

		delete[] dat->m_sValue;
		delete[] dat->m_wsValue;
		dat->m_wsValue = NULL;

		if (!value)
		{
			value = "";
		}

		int len = int(strlen(value));
		dat->m_sValue = new char[len + 1];
		memcpy(dat->m_sValue, value, len + 1);

		dat->m_iDataType = TYPE_STRING;
	}
}

void KeyValues::SetInt(const char* keyName, int value)
{
	KeyValues* dat = FindKey(keyName, true);

	if (dat)
	{
		dat->m_iValue = value;
		dat->m_iDataType = TYPE_INT;
	}
}

void KeyValues::SetUint64(const char* keyName, uint64_t value)
{
	KeyValues* dat = FindKey(keyName, true);

	if (dat)
	{
		delete[] dat->m_sValue;
		delete[] dat->m_wsValue;
		dat->m_wsValue = NULL;

		dat->m_sValue = new char[sizeof(uint64_t)];
		*((uint64_t*)dat->m_sValue) = value;
		dat->m_iDataType = TYPE_UINT64;
	}
}

void KeyValues::SetFloat(const char* keyName, float value)
{
	KeyValues* dat = FindKey(keyName, true);

	if (dat)
	{
		dat->m_flValue = value;
		dat->m_iDataType = TYPE_FLOAT;
	}
}

void KeyValues::SetPtr(const char* keyName, void* value)
{
	KeyValues* dat = FindKey(keyName, true);

	if (dat)
	{
		dat->m_pValue = value;
		dat->m_iDataType = TYPE_PTR;
	}
}

void KeyValues::SetColor(const char* keyName, Color_t value)
{
	KeyValues* dat = FindKey(keyName, true);

	if (dat)
	{
		dat->m_iDataType = TYPE_COLOR;
		dat->m_Color[0] = value.r;
		dat->m_Color[1] = value.g;
		dat->m_Color[2] = value.b;
		dat->m_Color[3] = value.a;
	}
}

void KeyValues::SetBool(const char* keyName, bool value)
{
	SetInt(keyName, value ? 1 : 0);
}

void KeyValues::DeleteThis()
{
	delete this;
}