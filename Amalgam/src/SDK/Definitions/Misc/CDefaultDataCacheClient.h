#pragma once
#include "BaseTypes.h"

typedef uint32 DataCacheClientID_t;

enum DataCacheNotificationType_t
{
	DC_NONE,
	DC_AGE_DISCARD,
	DC_FLUSH_DISCARD,
	DC_REMOVED,
	DC_RELOCATE,
	DC_PRINT_INF0,
};

struct DataCacheNotification_t
{
	DataCacheNotificationType_t type;
	const char* pszSectionName;
	DataCacheClientID_t			clientId;
	const void* pItemData;
	unsigned					nItemSize;
};

class IDataCacheClient
{
public:
	virtual bool HandleCacheNotification(const DataCacheNotification_t& notification) = 0;
	virtual bool GetItemName(DataCacheClientID_t clientId, const void* pItem, char* pDest, unsigned nMaxLen) = 0;
};

//-------------------------------------

class CDefaultDataCacheClient : public IDataCacheClient
{
public:
	virtual bool HandleCacheNotification(const DataCacheNotification_t& notification)
	{
		switch (notification.type)
		{
		case DC_AGE_DISCARD:
		case DC_FLUSH_DISCARD:
		case DC_REMOVED:
		default:
			//Assert(0);
			return false;
		}
		return false;
	}

	virtual bool GetItemName(DataCacheClientID_t clientId, const void* pItem, char* pDest, unsigned nMaxLen)
	{
		return false;
	}
};