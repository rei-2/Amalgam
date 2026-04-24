#pragma once
#include "../Misc/IMDLCache.h"
#include "../Misc/IStudioDataCache.h"
#include "../Misc/CDefaultDataCacheClient.h"
#include "../Misc/VCollide.h"
#include "../Misc/Studio.h"
#include "../Misc/CUtlDict.h"
#include "../Misc/CUtlLinkedList.h"

class IDataCacheSection;
typedef void* DataCacheHandle_t;

struct studiodata_t
{
	DataCacheHandle_t m_MDLCache;
	vcollide_t m_VCollisionData;
	studiohwdata_t m_HardwareData;
#if defined( USE_HARDWARE_CACHE )
	DataCacheHandle_t m_HardwareDataCache;
#endif
	unsigned short m_nFlags;
	short m_nRefCount;
	virtualmodel_t* m_pVirtualModel;
	int m_nAnimBlockCount;
	DataCacheHandle_t* m_pAnimBlock;
	unsigned long* m_iFakeAnimBlockStall;
	DataCacheHandle_t m_VertexCache;
	bool m_VertexDataIsCompressed;
	int m_nAutoplaySequenceCount;
	unsigned short* m_pAutoplaySequenceList;
	void* m_pUserData;
};

struct AsyncInfo_t
{
	FSAsyncControl_t	hControl;
	MDLHandle_t			hModel;
	MDLCacheDataType_t	type;
	int					iAnimBlock;
};

class CThreadFastMutex
{
public:
	volatile uint32 m_ownerID;
	int m_depth;
};

class CMDLCache : public CTier3AppSystem<IMDLCache>, public IStudioDataCache, public CDefaultDataCacheClient
{
	typedef CTier3AppSystem<IMDLCache> BaseClass;

public:
	virtual bool Connect(CreateInterfaceFn factory) = 0;
	virtual void Disconnect() = 0;
	virtual void* QueryInterface(const char* pInterfaceName) = 0;
	virtual InitReturnVal_t Init() = 0;
	virtual void Shutdown() = 0;
	virtual MDLHandle_t FindMDL(const char* pMDLRelativePath) = 0;
	virtual int AddRef(MDLHandle_t handle) = 0;
	virtual int Release(MDLHandle_t handle) = 0;
	virtual int GetRef(MDLHandle_t handle) = 0;
	virtual void MarkAsLoaded(MDLHandle_t handle) = 0;
	virtual studiohdr_t* GetStudioHdr(MDLHandle_t handle) = 0;
	virtual studiohwdata_t* GetHardwareData(MDLHandle_t handle) = 0;
	virtual vcollide_t* GetVCollide(MDLHandle_t handle) = 0;
	virtual vcollide_t* GetVCollideEx(MDLHandle_t handle, bool synchronousLoad = true) = 0;
	virtual unsigned char* GetAnimBlock(MDLHandle_t handle, int nBlock) = 0;
	virtual virtualmodel_t* GetVirtualModel(MDLHandle_t handle) = 0;
	virtual virtualmodel_t* GetVirtualModelFast(const studiohdr_t* pStudioHdr, MDLHandle_t handle) = 0;
	virtual int GetAutoplayList(MDLHandle_t handle, unsigned short** pOut) = 0;
	virtual void TouchAllData(MDLHandle_t handle) = 0;
	virtual void SetUserData(MDLHandle_t handle, void* pData) = 0;
	virtual void* GetUserData(MDLHandle_t handle) = 0;
	virtual bool IsErrorModel(MDLHandle_t handle) = 0;
	virtual void SetCacheNotify(IMDLCacheNotify* pNotify) = 0;
	virtual vertexFileHeader_t* GetVertexData(MDLHandle_t handle) = 0;
	virtual void Flush(MDLCacheFlush_t nFlushFlags = MDLCACHE_FLUSH_ALL) = 0;
	virtual void Flush(MDLHandle_t handle, int nFlushFlags = MDLCACHE_FLUSH_ALL) = 0;
	virtual const char* GetModelName(MDLHandle_t handle) = 0;
	virtual void BeginLock() = 0;
	virtual void EndLock() = 0;
	virtual int* GetFrameUnlockCounterPtrOLD() = 0;
	virtual int* GetFrameUnlockCounterPtr(MDLCacheDataType_t type) = 0;
	virtual void FinishPendingLoads() = 0;
	virtual bool GetVCollideSize(MDLHandle_t handle, int* pVCollideSize) = 0;
	virtual void BeginMapLoad() = 0;
	virtual void EndMapLoad() = 0;
	virtual void InitPreloadData(bool rebuild) = 0;
	virtual void ShutdownPreloadData() = 0;
	virtual bool IsDataLoaded(MDLHandle_t handle, MDLCacheDataType_t type) = 0;
	virtual studiohdr_t* LockStudioHdr(MDLHandle_t handle) = 0;
	virtual void UnlockStudioHdr(MDLHandle_t handle) = 0;
	virtual bool PreloadModel(MDLHandle_t handle) = 0;
	virtual void ResetErrorModelStatus(MDLHandle_t handle) = 0;
	virtual void MarkFrame() = 0;
	virtual bool HandleCacheNotification(const DataCacheNotification_t& notification) = 0;
	virtual bool GetItemName(DataCacheClientID_t clientId, const void* pItem, char* pDest, unsigned nMaxLen) = 0;
	virtual bool GetAsyncLoad(MDLCacheDataType_t type) = 0;
	virtual bool SetAsyncLoad(MDLCacheDataType_t type, bool bAsync) = 0;

public:
	IDataCacheSection* m_pModelCacheSection;
	IDataCacheSection* m_pMeshCacheSection;
	IDataCacheSection* m_pAnimBlockCacheSection;
	int m_nModelCacheFrameLocks;
	int m_nMeshCacheFrameLocks;
	CUtlDict<studiodata_t*, MDLHandle_t> m_MDLDict;
	IMDLCacheNotify* m_pCacheNotify;
	CUtlFixedLinkedList<AsyncInfo_t> m_PendingAsyncs;
	CThreadFastMutex m_QueuedLoadingMutex;
	CThreadFastMutex m_AsyncMutex;
	bool m_bLostVideoMemory : 1;
	bool m_bConnected : 1;
	bool m_bInitialized : 1;
};

MAKE_INTERFACE_VERSION(CMDLCache, MDLCache, "datacache.dll", "MDLCache004");