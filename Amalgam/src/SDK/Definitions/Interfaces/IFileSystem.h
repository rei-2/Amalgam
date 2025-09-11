#pragma once
#include "../Misc/IAppSystem.h"
#include "../Main/MD5.h"

typedef void* FileHandle_t;
typedef void* (*FSAllocFunc_t)(const char* pszFilename, unsigned nBytes);
typedef uint32_t PathTypeQuery_t;
typedef int FileFindHandle_t;
typedef void* FileNameHandle_t;
struct FSAsyncControl_t__ { int unused; }; typedef struct FSAsyncControl_t__* FSAsyncControl_t;
struct FSAsyncFile_t__ { int unused; }; typedef struct FSAsyncFile_t__* FSAsyncFile_t;
typedef int WaitForResourcesHandle_t;
typedef void (*FileSystemLoggingFunc_t)(const char* fileName, const char* accessType);
typedef void (*FSDirtyDiskReportFunc_t)();
typedef void* FileCacheHandle_t;

enum FileSystemSeek_t
{
	FILESYSTEM_SEEK_HEAD = SEEK_SET,
	FILESYSTEM_SEEK_CURRENT = SEEK_CUR,
	FILESYSTEM_SEEK_TAIL = SEEK_END,
};
enum FilesystemMountRetval_t
{
	FILESYSTEM_MOUNT_OK = 0,
	FILESYSTEM_MOUNT_FAILED,
};
enum SearchPathAdd_t
{
	PATH_ADD_TO_HEAD,
	PATH_ADD_TO_TAIL,
};
enum PathTypeFilter_t
{
	FILTER_NONE = 0,
	FILTER_CULLPACK = 1,
	FILTER_CULLNONPACK = 2,
};
enum FSAsyncStatus_t
{
	FSASYNC_ERR_NOT_MINE = -8,
	FSASYNC_ERR_RETRY_LATER = -7,
	FSASYNC_ERR_ALIGNMENT = -6,
	FSASYNC_ERR_FAILURE = -5,
	FSASYNC_ERR_READING = -4,
	FSASYNC_ERR_NOMEMORY = -3,
	FSASYNC_ERR_UNKNOWNID = -2,
	FSASYNC_ERR_FILEOPEN = -1,
	FSASYNC_OK = 0,
	FSASYNC_STATUS_PENDING,
	FSASYNC_STATUS_INPROGRESS,
	FSASYNC_STATUS_ABORTED,
	FSASYNC_STATUS_UNSERVICED,
};
enum FileWarningLevel_t
{
	FILESYSTEM_WARNING = -1,
	FILESYSTEM_WARNING_QUIET = 0,
	FILESYSTEM_WARNING_REPORTUNCLOSED,
	FILESYSTEM_WARNING_REPORTUSAGE,
	FILESYSTEM_WARNING_REPORTALLACCESSES,
	FILESYSTEM_WARNING_REPORTALLACCESSES_READ,
	FILESYSTEM_WARNING_REPORTALLACCESSES_READWRITE,
	FILESYSTEM_WARNING_REPORTALLACCESSES_ASYNC,
};
enum DVDMode_t
{
	DVDMODE_OFF = 0,
	DVDMODE_STRICT = 1,
	DVDMODE_DEV = 2,
};
enum EPureServerFileClass
{
	ePureServerFileClass_Unknown = -1,
	ePureServerFileClass_Any = 0,
	ePureServerFileClass_AnyTrusted,
	ePureServerFileClass_CheckHash,
};
enum ECacheCRCType
{
	k_eCacheCRCType_SingleFile,
	k_eCacheCRCType_Directory,
	k_eCacheCRCType_Directory_Recursive
};
enum EFileCRCStatus
{
	k_eFileCRCStatus_CantOpenFile,
	k_eFileCRCStatus_GotCRC,
	k_eFileCRCStatus_FileInVPK
};

struct FileAsyncRequest_t;
struct FileSystemStatistics;
struct FileHash_t;
class CUtlBuffer;
class CSysModule;
class IThreadPool;
class KeyValues;
class IAsyncFileFetch;
class IPureServerWhitelist;
class IFileList;
class CUnverifiedFileHash;
class CMemoryFileBacking;
typedef void (*FSAsyncCallbackFunc_t)(const FileAsyncRequest_t& request, int nBytesRead, FSAsyncStatus_t err);

class IBaseFileSystem
{
public:
	virtual int				Read(void* pOutput, int size, FileHandle_t file) = 0;
	virtual int				Write(void const* pInput, int size, FileHandle_t file) = 0;
	virtual FileHandle_t	Open(const char* pFileName, const char* pOptions, const char* pathID = 0) = 0;
	virtual void			Close(FileHandle_t file) = 0;
	virtual void			Seek(FileHandle_t file, int pos, FileSystemSeek_t seekType) = 0;
	virtual unsigned int	Tell(FileHandle_t file) = 0;
	virtual unsigned int	Size(FileHandle_t file) = 0;
	virtual unsigned int	Size(const char* pFileName, const char* pPathID = 0) = 0;
	virtual void			Flush(FileHandle_t file) = 0;
	virtual bool			Precache(const char* pFileName, const char* pPathID = 0) = 0;
	virtual bool			FileExists(const char* pFileName, const char* pPathID = 0) = 0;
	virtual bool			IsFileWritable(char const* pFileName, const char* pPathID = 0) = 0;
	virtual bool			SetFileWritable(char const* pFileName, bool writable, const char* pPathID = 0) = 0;
	virtual long			GetFileTime(const char* pFileName, const char* pPathID = 0) = 0;
	virtual bool			ReadFile(const char* pFileName, const char* pPath, CUtlBuffer& buf, int nMaxBytes = 0, int nStartingByte = 0, FSAllocFunc_t pfnAlloc = NULL) = 0;
	virtual bool			WriteFile(const char* pFileName, const char* pPath, CUtlBuffer& buf) = 0;
	virtual bool			UnzipFile(const char* pFileName, const char* pPath, const char* pDestination) = 0;
};

class IFileSystem : public IAppSystem, public IBaseFileSystem
{
public:
	enum KeyValuesPreloadType_t
	{
		TYPE_VMT,
		TYPE_SOUNDEMITTER,
		TYPE_SOUNDSCAPE,
		NUM_PRELOAD_TYPES
	};

	virtual bool			IsSteam() const = 0;
	virtual	FilesystemMountRetval_t MountSteamContent(int nExtraAppId = -1) = 0;
	virtual void			AddSearchPath(const char* pPath, const char* pathID, SearchPathAdd_t addType = PATH_ADD_TO_TAIL) = 0;
	virtual bool			RemoveSearchPath(const char* pPath, const char* pathID = 0) = 0;
	virtual void			RemoveAllSearchPaths(void) = 0;
	virtual void			RemoveSearchPaths(const char* szPathID) = 0;
	virtual void			MarkPathIDByRequestOnly(const char* pPathID, bool bRequestOnly) = 0;
	virtual const char*		RelativePathToFullPath(const char* pFileName, const char* pPathID, char* pDest, int maxLenInChars, PathTypeFilter_t pathFilter = FILTER_NONE, PathTypeQuery_t* pPathType = NULL) = 0;
	virtual int				GetSearchPath(const char* pathID, bool bGetPackFiles, char* pDest, int maxLenInChars) = 0;
	virtual bool			AddPackFile(const char* fullpath, const char* pathID) = 0;
	virtual void			RemoveFile(char const* pRelativePath, const char* pathID = 0) = 0;
	virtual bool			RenameFile(char const* pOldPath, char const* pNewPath, const char* pathID = 0) = 0;
	virtual void			CreateDirHierarchy(const char* path, const char* pathID = 0) = 0;
	virtual bool			IsDirectory(const char* pFileName, const char* pathID = 0) = 0;
	virtual void			FileTimeToString(char* pStrip, int maxCharsIncludingTerminator, long fileTime) = 0;
	virtual void			SetBufferSize(FileHandle_t file, unsigned nBytes) = 0;
	virtual bool			IsOk(FileHandle_t file) = 0;
	virtual bool			EndOfFile(FileHandle_t file) = 0;
	virtual char*			ReadLine(char* pOutput, int maxChars, FileHandle_t file) = 0;
	virtual int				FPrintf(FileHandle_t file, const char* pFormat, ...) = 0;
	virtual CSysModule*		LoadModule(const char* pFileName, const char* pPathID = 0, bool bValidatedDllOnly = true) = 0;
	virtual void			UnloadModule(CSysModule* pModule) = 0;
	virtual const char*		FindFirst(const char* pWildCard, FileFindHandle_t* pHandle) = 0;
	virtual const char*		FindNext(FileFindHandle_t handle) = 0;
	virtual bool			FindIsDirectory(FileFindHandle_t handle) = 0;
	virtual void			FindClose(FileFindHandle_t handle) = 0;
	virtual const char*		FindFirstEx(const char* pWildCard, const char* pPathID, FileFindHandle_t* pHandle) = 0;
	virtual const char*		GetLocalPath(const char* pFileName, char* pDest, int maxLenInChars) = 0;
	virtual bool			FullPathToRelativePath(const char* pFullpath, char* pDest, int maxLenInChars) = 0;
	virtual bool			GetCurrentDirectory(char* pDirectory, int maxlen) = 0;
	virtual FileNameHandle_t	FindOrAddFileName(char const* pFileName) = 0;
	virtual bool				String(const FileNameHandle_t& handle, char* buf, int buflen) = 0;
	virtual FSAsyncStatus_t	AsyncReadMultiple(const FileAsyncRequest_t* pRequests, int nRequests,  FSAsyncControl_t* phControls = NULL) = 0;
	virtual FSAsyncStatus_t	AsyncAppend(const char* pFileName, const void* pSrc, int nSrcBytes, bool bFreeMemory, FSAsyncControl_t* pControl = NULL) = 0;
	virtual FSAsyncStatus_t	AsyncAppendFile(const char* pAppendToFileName, const char* pAppendFromFileName, FSAsyncControl_t* pControl = NULL) = 0;
	virtual void			AsyncFinishAll(int iToPriority = 0) = 0;
	virtual void			AsyncFinishAllWrites() = 0;
	virtual FSAsyncStatus_t	AsyncFlush() = 0;
	virtual bool			AsyncSuspend() = 0;
	virtual bool			AsyncResume() = 0;
	virtual void			AsyncAddFetcher(IAsyncFileFetch* pFetcher) = 0;
	virtual void			AsyncRemoveFetcher(IAsyncFileFetch* pFetcher) = 0;
	virtual FSAsyncStatus_t	AsyncBeginRead(const char* pszFile, FSAsyncFile_t* phFile) = 0;
	virtual FSAsyncStatus_t	AsyncEndRead(FSAsyncFile_t hFile) = 0;
	virtual FSAsyncStatus_t	AsyncFinish(FSAsyncControl_t hControl, bool wait = true) = 0;
	virtual FSAsyncStatus_t	AsyncGetResult(FSAsyncControl_t hControl, void** ppData, int* pSize) = 0;
	virtual FSAsyncStatus_t	AsyncAbort(FSAsyncControl_t hControl) = 0;
	virtual FSAsyncStatus_t	AsyncStatus(FSAsyncControl_t hControl) = 0;
	virtual FSAsyncStatus_t	AsyncSetPriority(FSAsyncControl_t hControl, int newPriority) = 0;
	virtual void			AsyncAddRef(FSAsyncControl_t hControl) = 0;
	virtual void			AsyncRelease(FSAsyncControl_t hControl) = 0;
	virtual WaitForResourcesHandle_t WaitForResources(const char* resourcelist) = 0;
	virtual bool			GetWaitForResourcesProgress(WaitForResourcesHandle_t handle, float* progress /* out */ , bool* complete /* out */) = 0;
	virtual void			CancelWaitForResources(WaitForResourcesHandle_t handle) = 0;
	virtual int				HintResourceNeed(const char* hintlist, int forgetEverything) = 0;
	virtual bool			IsFileImmediatelyAvailable(const char* pFileName) = 0;
	virtual void			GetLocalCopy(const char* pFileName) = 0;
	virtual void			PrintOpenedFiles(void) = 0;
	virtual void			PrintSearchPaths(void) = 0;
	virtual void			SetWarningFunc(void (*pfnWarning)(const char* fmt, ...)) = 0;
	virtual void			SetWarningLevel(FileWarningLevel_t level) = 0;
	virtual void			AddLoggingFunc(void (*pfnLogFunc)(const char* fileName, const char* accessType)) = 0;
	virtual void			RemoveLoggingFunc(FileSystemLoggingFunc_t logFunc) = 0;
	virtual const FileSystemStatistics* GetFilesystemStatistics() = 0;
	virtual FileHandle_t	OpenEx(const char* pFileName, const char* pOptions, unsigned flags = 0, const char* pathID = 0, char** ppszResolvedFilename = NULL) = 0;
	virtual int				ReadEx(void* pOutput, int sizeDest, int size, FileHandle_t file) = 0;
	virtual int				ReadFileEx(const char* pFileName, const char* pPath, void** ppBuf, bool bNullTerminate = false, bool bOptimalAlloc = false, int nMaxBytes = 0, int nStartingByte = 0, FSAllocFunc_t pfnAlloc = NULL) = 0;
	virtual FileNameHandle_t	FindFileName(char const* pFileName) = 0;
	virtual void			SetupPreloadData() = 0;
	virtual void			DiscardPreloadData() = 0;
	virtual void			LoadCompiledKeyValues(KeyValuesPreloadType_t type, char const* archiveFile) = 0;
	virtual KeyValues*		LoadKeyValues(KeyValuesPreloadType_t type, char const* filename, char const* pPathID = 0) = 0;
	virtual bool			LoadKeyValues(KeyValues& head, KeyValuesPreloadType_t type, char const* filename, char const* pPathID = 0) = 0;
	virtual bool			ExtractRootKeyName(KeyValuesPreloadType_t type, char* outbuf, size_t bufsize, char const* filename, char const* pPathID = 0) = 0;
	virtual FSAsyncStatus_t	AsyncWrite(const char* pFileName, const void* pSrc, int nSrcBytes, bool bFreeMemory, bool bAppend = false, FSAsyncControl_t* pControl = NULL) = 0;
	virtual FSAsyncStatus_t	AsyncWriteFile(const char* pFileName, const CUtlBuffer* pSrc, int nSrcBytes, bool bFreeMemory, bool bAppend = false, FSAsyncControl_t* pControl = NULL) = 0;
	virtual FSAsyncStatus_t	AsyncReadMultipleCreditAlloc(const FileAsyncRequest_t* pRequests, int nRequests, const char* pszFile, int line, FSAsyncControl_t* phControls = NULL) = 0;
	virtual bool			GetFileTypeForFullPath(char const* pFullPath, wchar_t* buf, size_t bufSizeInBytes) = 0;
	virtual bool			ReadToBuffer(FileHandle_t hFile, CUtlBuffer& buf, int nMaxBytes = 0, FSAllocFunc_t pfnAlloc = NULL) = 0;
	virtual bool			GetOptimalIOConstraints(FileHandle_t hFile, unsigned* pOffsetAlign, unsigned* pSizeAlign, unsigned* pBufferAlign) = 0;
	virtual void*			AllocOptimalReadBuffer(FileHandle_t hFile, unsigned nSize = 0, unsigned nOffset = 0) = 0;
	virtual void			FreeOptimalReadBuffer(void*) = 0;
	virtual void			BeginMapAccess() = 0;
	virtual void			EndMapAccess() = 0;
	virtual bool			FullPathToRelativePathEx(const char* pFullpath, const char* pPathId, char* pDest, int maxLenInChars) = 0;
	virtual int				GetPathIndex(const FileNameHandle_t& handle) = 0;
	virtual long			GetPathTime(const char* pPath, const char* pPathID) = 0;
	virtual DVDMode_t		GetDVDMode() = 0;
	virtual void			EnableWhitelistFileTracking(bool bEnable, bool bCacheAllVPKHashes, bool bRecalculateAndCheckHashes) = 0;
	virtual void			RegisterFileWhitelist(IPureServerWhitelist* pWhiteList, IFileList** pFilesToReload) = 0;
	virtual void			MarkAllCRCsUnverified() = 0;
	virtual void			CacheFileCRCs(const char* pPathname, ECacheCRCType eType, IFileList* pFilter) = 0;
	virtual EFileCRCStatus	CheckCachedFileHash(const char* pPathID, const char* pRelativeFilename, int nFileFraction, FileHash_t* pFileHash) = 0;
	virtual int				GetUnverifiedFileHashes(CUnverifiedFileHash* pFiles, int nMaxFiles) = 0;
	virtual int				GetWhitelistSpewFlags() = 0;
	virtual void			SetWhitelistSpewFlags(int flags) = 0;
	virtual void			InstallDirtyDiskReportFunc(FSDirtyDiskReportFunc_t func) = 0;
	virtual FileCacheHandle_t CreateFileCache() = 0;
	virtual void			AddFilesToFileCache(FileCacheHandle_t cacheId, const char** ppFileNames, int nFileNames, const char* pPathID) = 0;
	virtual bool			IsFileCacheFileLoaded(FileCacheHandle_t cacheId, const char* pFileName) = 0;
	virtual bool			IsFileCacheLoaded(FileCacheHandle_t cacheId) = 0;
	virtual void			DestroyFileCache(FileCacheHandle_t cacheId) = 0;
	virtual bool			RegisterMemoryFile(CMemoryFileBacking* pFile, CMemoryFileBacking** ppExistingFileWithRef) = 0;
	virtual void			UnregisterMemoryFile(CMemoryFileBacking* pFile) = 0;
	virtual void			CacheAllVPKFileHashes(bool bCacheAllVPKHashes, bool bRecalculateAndCheckHashes) = 0;
	virtual bool			CheckVPKFileHash(int PackFileID, int nPackFileNumber, int nFileFraction, MD5Value_t& md5Value) = 0;
	virtual void			NotifyFileUnloaded(const char* pszFilename, const char* pPathId) = 0;
	virtual bool			GetCaseCorrectFullPath_Ptr(const char* pFullPath, char* pDest, int maxLenInChars) = 0;
	
	FSAsyncStatus_t AsyncRead(const FileAsyncRequest_t& request, FSAsyncControl_t* phControl = NULL) { return AsyncReadMultiple(&request, 1, phControl); }
	FSAsyncStatus_t AsyncReadCreditAlloc(const FileAsyncRequest_t& request, const char* pszFile, int line, FSAsyncControl_t* phControl = NULL) { return AsyncReadMultipleCreditAlloc(&request, 1, pszFile, line, phControl); }

	template <size_t maxLenInChars> const char* RelativePathToFullPath_safe(const char* pFileName, const char* pPathID, char(&pDest)[maxLenInChars], PathTypeFilter_t pathFilter = FILTER_NONE, PathTypeQuery_t* pPathType = NULL)
	{
		return RelativePathToFullPath(pFileName, pPathID, pDest, (int)maxLenInChars, pathFilter, pPathType);
	}
	template <size_t maxLenInChars> int GetSearchPath_safe(const char* pathID, bool bGetPackFiles, char(&pDest)[maxLenInChars])
	{
		return GetSearchPath(pathID, bGetPackFiles, pDest, (int)maxLenInChars);
	}
	template <size_t maxLenInChars> const char* GetLocalPath_safe(const char* pFileName, char(&pDest)[maxLenInChars])
	{
		return GetLocalPath(pFileName, pDest, (int)maxLenInChars);
	}
	template <size_t maxLenInChars> bool FullPathToRelativePath_safe(const char* pFullpath, char(&pDest)[maxLenInChars])
	{
		return FullPathToRelativePath(pFullpath, pDest, (int)maxLenInChars);
	}
	template <size_t maxLenInChars> bool FullPathToRelativePathEx_safe(const char* pFullpath, char(&pDest)[maxLenInChars])
	{
		return FullPathToRelativePathEx(pFullpath, pDest, (int)maxLenInChars);
	}
	template <size_t maxLenInChars> bool GetCaseCorrectFullPath(const char* pFullPath, char(&pDest)[maxLenInChars])
	{
		return GetCaseCorrectFullPath_Ptr(pFullPath, pDest, (int)maxLenInChars);
	}
};

MAKE_INTERFACE_VERSION(IFileSystem, FileSystem, "filesystem_stdio.dll", "VFileSystem022");