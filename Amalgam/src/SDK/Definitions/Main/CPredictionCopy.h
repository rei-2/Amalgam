#pragma once
#include "../Misc/Datamap.h"
#include "../../../Utils/Signatures/Signatures.h"

MAKE_SIGNATURE(CPredictionCopy_TransferData, "client.dll", "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 56 48 83 EC ? 8B 3D", 0x0);

#define PC_DATA_PACKED			true
#define PC_DATA_NORMAL			false

enum
{
	PC_EVERYTHING = 0,
	PC_NON_NETWORKED_ONLY,
	PC_NETWORKED_ONLY,
};

typedef void (*FN_FIELD_COMPARE)(const char* classname, const char* fieldname, const char* fieldtype,
	bool networked, bool noterrorchecked, bool differs, bool withintolerance, const char* value);

class CPredictionCopy
{
public:
	typedef enum
	{
		DIFFERS = 0,
		IDENTICAL,
		WITHINTOLERANCE,
	} difftype_t;

	CPredictionCopy(int type, void* dest, bool dest_packed, void const* src, bool src_packed,
		bool counterrors = false, bool reporterrors = false, bool performcopy = true,
		bool describefields = false, FN_FIELD_COMPARE func = nullptr)
	{
		m_nType = type;
		m_pDest = dest;
		m_pSrc = src;
		m_nDestOffsetIndex = dest_packed ? TD_OFFSET_PACKED : TD_OFFSET_NORMAL;
		m_nSrcOffsetIndex = src_packed ? TD_OFFSET_PACKED : TD_OFFSET_NORMAL;
		m_bErrorCheck = counterrors;
		m_bReportErrors = reporterrors;
		m_bPerformCopy = performcopy;
		m_bDescribeFields = describefields;

		m_pCurrentField = nullptr;
		m_pCurrentMap = nullptr;
		m_pCurrentClassName = nullptr;
		m_bShouldReport = false;
		m_bShouldDescribe = false;
		m_nErrorCount = 0;

		m_FieldCompareFunc = func;
	}

	int TransferData(const char* operation, int entindex, datamap_t* dmap)
	{
		return S::CPredictionCopy_TransferData.Call<int>(this, operation, entindex, dmap);
	}

public:
	int m_nType;
	void* m_pDest;
	void const* m_pSrc;
	int m_nDestOffsetIndex;
	int m_nSrcOffsetIndex;

	bool m_bErrorCheck;
	bool m_bReportErrors;
	bool m_bDescribeFields;
	typedescription_t* m_pCurrentField;
	char const* m_pCurrentClassName;
	datamap_t* m_pCurrentMap;
	bool m_bShouldReport;
	bool m_bShouldDescribe;
	int m_nErrorCount;
	bool m_bPerformCopy;

	FN_FIELD_COMPARE	m_FieldCompareFunc;

	typedescription_t* m_pWatchField;
	char const* m_pOperation;
};