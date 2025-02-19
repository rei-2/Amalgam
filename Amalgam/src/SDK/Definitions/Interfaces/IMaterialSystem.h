#pragma once
#include "Interface.h"
#include "../Misc/Deformations.h"
#include "../Misc/IAppSystem.h"
#include "../Misc/IColorCorrectionSystem.h"
#include "../Misc/ImageFormat.h"
#include "../Misc/IMaterialSystemHardwareConfig.h"
#include "../Misc/IRefCounted.h"
#include "../Misc/LightDesc.h"
#include "../Misc/TextureGroupNames.h"
#include "../Misc/VTF.h"
#include "../Types.h"

#define OVERBRIGHT 2.0f
#define OO_OVERBRIGHT ( 1.0f / 2.0f )
#define GAMMA 2.2f
#define TEXGAMMA 2.2f

class IMaterial;
class IMesh;
class IVertexBuffer;
class IIndexBuffer;
struct MaterialSystem_Config_t;
class ITexture;
class ITextureCompositor;
struct MaterialSystemHardwareIdentifier_t;
class KeyValues;
class IShader;
class IVertexTexture;
class IMorph;
class IMatRenderContext;
class ICallQueue;
struct MorphWeight_t;
class IFileList;

typedef uint64 VertexFormat_t;

#define ABSOLUTE_MINIMUM_DXLEVEL 80

enum ShaderParamType_t
{
	SHADER_PARAM_TYPE_TEXTURE,
	SHADER_PARAM_TYPE_INTEGER,
	SHADER_PARAM_TYPE_COLOR,
	SHADER_PARAM_TYPE_VEC2,
	SHADER_PARAM_TYPE_VEC3,
	SHADER_PARAM_TYPE_VEC4,
	SHADER_PARAM_TYPE_ENVMAP,
	SHADER_PARAM_TYPE_FLOAT,
	SHADER_PARAM_TYPE_BOOL,
	SHADER_PARAM_TYPE_FOURCC,
	SHADER_PARAM_TYPE_MATRIX,
	SHADER_PARAM_TYPE_MATERIAL,
	SHADER_PARAM_TYPE_STRING,
	SHADER_PARAM_TYPE_MATRIX4X2
};

enum MaterialMatrixMode_t
{
	MATERIAL_VIEW = 0,
	MATERIAL_PROJECTION,
	MATERIAL_TEXTURE0,
	MATERIAL_TEXTURE1,
	MATERIAL_TEXTURE2,
	MATERIAL_TEXTURE3,
	MATERIAL_TEXTURE4,
	MATERIAL_TEXTURE5,
	MATERIAL_TEXTURE6,
	MATERIAL_TEXTURE7,
	MATERIAL_MODEL,
	NUM_MATRIX_MODES = MATERIAL_MODEL + 1,
	NUM_TEXTURE_TRANSFORMS = MATERIAL_TEXTURE7 - MATERIAL_TEXTURE0 + 1
};

const int NUM_MODEL_TRANSFORMS = 53;
const int MATERIAL_MODEL_MAX = MATERIAL_MODEL + NUM_MODEL_TRANSFORMS;

enum MaterialPrimitiveType_t
{
	MATERIAL_POINTS = 0x0,
	MATERIAL_LINES,
	MATERIAL_TRIANGLES,
	MATERIAL_TRIANGLE_STRIP,
	MATERIAL_LINE_STRIP,
	MATERIAL_LINE_LOOP,
	MATERIAL_POLYGON,
	MATERIAL_QUADS,
	MATERIAL_INSTANCED_QUADS,
	MATERIAL_HETEROGENOUS
};

enum MaterialPropertyTypes_t
{
	MATERIAL_PROPERTY_NEEDS_LIGHTMAP = 0,
	MATERIAL_PROPERTY_OPACITY,
	MATERIAL_PROPERTY_REFLECTIVITY,
	MATERIAL_PROPERTY_NEEDS_BUMPED_LIGHTMAPS
};

enum MaterialPropertyOpacityTypes_t
{
	MATERIAL_ALPHATEST = 0,
	MATERIAL_OPAQUE,
	MATERIAL_TRANSLUCENT
};

enum MaterialBufferTypes_t
{
	MATERIAL_FRONT = 0,
	MATERIAL_BACK
};

enum MaterialCullMode_t
{
	MATERIAL_CULLMODE_CCW,
	MATERIAL_CULLMODE_CW
};

enum MaterialIndexFormat_t
{
	MATERIAL_INDEX_FORMAT_UNKNOWN = -1,
	MATERIAL_INDEX_FORMAT_16BIT = 0,
	MATERIAL_INDEX_FORMAT_32BIT
};

enum MaterialFogMode_t
{
	MATERIAL_FOG_NONE,
	MATERIAL_FOG_LINEAR,
	MATERIAL_FOG_LINEAR_BELOW_FOG_Z
};

enum MaterialHeightClipMode_t
{
	MATERIAL_HEIGHTCLIPMODE_DISABLE,
	MATERIAL_HEIGHTCLIPMODE_RENDER_ABOVE_HEIGHT,
	MATERIAL_HEIGHTCLIPMODE_RENDER_BELOW_HEIGHT
};

enum MaterialNonInteractiveMode_t
{
	MATERIAL_NON_INTERACTIVE_MODE_NONE = -1,
	MATERIAL_NON_INTERACTIVE_MODE_STARTUP = 0,
	MATERIAL_NON_INTERACTIVE_MODE_LEVEL_LOAD,
	MATERIAL_NON_INTERACTIVE_MODE_COUNT
};

#define MATERIAL_MORPH_DECAL ( (IMorph*)1 )

enum MaterialThreadMode_t
{
	MATERIAL_SINGLE_THREADED,
	MATERIAL_QUEUED_SINGLE_THREADED,
	MATERIAL_QUEUED_THREADED
};

enum MaterialContextType_t
{
	MATERIAL_HARDWARE_CONTEXT,
	MATERIAL_QUEUED_CONTEXT,
	MATERIAL_NULL_CONTEXT
};

enum MaterialFindContext_t
{
	MATERIAL_FINDCONTEXT_NONE,
	MATERIAL_FINDCONTEXT_ISONAMODEL
};

#define CREATERENDERTARGETFLAGS_HDR 0x00000001
#define CREATERENDERTARGETFLAGS_AUTOMIPMAP 0x00000002
#define CREATERENDERTARGETFLAGS_UNFILTERABLE_OK 0x00000004
#define CREATERENDERTARGETFLAGS_NOEDRAM 0x00000008
#define CREATERENDERTARGETFLAGS_TEMP 0x00000010

enum StencilOperation_t
{
	STENCILOPERATION_KEEP = 1,
	STENCILOPERATION_ZERO = 2,
	STENCILOPERATION_REPLACE = 3,
	STENCILOPERATION_INCRSAT = 4,
	STENCILOPERATION_DECRSAT = 5,
	STENCILOPERATION_INVERT = 6,
	STENCILOPERATION_INCR = 7,
	STENCILOPERATION_DECR = 8,
	STENCILOPERATION_FORCE_DWORD = 0x7fffffff
};

enum StencilComparisonFunction_t
{
	STENCILCOMPARISONFUNCTION_NEVER = 1,
	STENCILCOMPARISONFUNCTION_LESS = 2,
	STENCILCOMPARISONFUNCTION_EQUAL = 3,
	STENCILCOMPARISONFUNCTION_LESSEQUAL = 4,
	STENCILCOMPARISONFUNCTION_GREATER = 5,
	STENCILCOMPARISONFUNCTION_NOTEQUAL = 6,
	STENCILCOMPARISONFUNCTION_GREATEREQUAL = 7,
	STENCILCOMPARISONFUNCTION_ALWAYS = 8,
	STENCILCOMPARISONFUNCTION_FORCE_DWORD = 0x7fffffff
};

enum MorphFormatFlags_t
{
	MORPH_POSITION = 0x0001,
	MORPH_NORMAL = 0x0002,
	MORPH_WRINKLE = 0x0004,
	MORPH_SPEED = 0x0008,
	MORPH_SIDE = 0x0010
};

typedef unsigned int MorphFormat_t;

enum StandardLightmap_t
{
	MATERIAL_SYSTEM_LIGHTMAP_PAGE_WHITE = -1,
	MATERIAL_SYSTEM_LIGHTMAP_PAGE_WHITE_BUMP = -2,
	MATERIAL_SYSTEM_LIGHTMAP_PAGE_USER_DEFINED = -3
};

struct MaterialSystem_SortInfo_t
{
	IMaterial* material;
	int lightmapPageID;
};

#define MAX_FB_TEXTURES 4

enum
{
	MATERIAL_ADAPTER_NAME_LENGTH = 512
};

struct MaterialAdapterInfo_t
{
	char m_pDriverName[MATERIAL_ADAPTER_NAME_LENGTH];
	unsigned int m_VendorID;
	unsigned int m_DeviceID;
	unsigned int m_SubSysID;
	unsigned int m_Revision;
	int m_nDXSupportLevel;
	int m_nMaxDXSupportLevel;
	unsigned int m_nDriverVersionHigh;
	unsigned int m_nDriverVersionLow;
};

struct MaterialVideoMode_t
{
	int m_Width;
	int m_Height;
	ImageFormat m_Format;
	int m_RefreshRate;
};

struct FlashlightState_t
{
	Vector m_vecLightOrigin;
	Quaternion m_quatOrientation;
	float m_NearZ;
	float m_FarZ;
	float m_fHorizontalFOVDegrees;
	float m_fVerticalFOVDegrees;
	float m_fQuadraticAtten;
	float m_fLinearAtten;
	float m_fConstantAtten;
	float m_Color[4];
	ITexture* m_pSpotlightTexture;
	int m_nSpotlightTextureFrame;
	bool m_bEnableShadows;
	bool m_bDrawShadowFrustum;
	float m_flShadowMapResolution;
	float m_flShadowFilterSize;
	float m_flShadowSlopeScaleDepthBias;
	float m_flShadowDepthBias;
	float m_flShadowJitterSeed;
	float m_flShadowAtten;
	int m_nShadowQuality;
	bool m_bScissor;
	int m_nLeft;
	int m_nTop;
	int m_nRight;
	int m_nBottom;
};

class IAsyncTextureOperationReceiver : public IRefCounted
{
public:
	virtual void OnAsyncCreateComplete(ITexture* pTex, void* pExtraArgs) = 0;
	virtual void OnAsyncFindComplete(ITexture* pTex, void* pExtraArgs) = 0;
	virtual void OnAsyncMapComplete(ITexture* pTex, void* pExtraArgs, void* pMemory, int nPitch) = 0;
	virtual void OnAsyncReadbackBegin(ITexture* pDst, ITexture* pSrc, void* pExtraArgs) = 0;
	virtual int GetRefCount() const = 0;
};

enum MaterialInitFlags_t
{
	MATERIAL_INIT_ALLOCATE_FULLSCREEN_TEXTURE = 0x2,
	MATERIAL_INIT_REFERENCE_RASTERIZER = 0x4
};

enum MaterialRenderTargetDepth_t
{
	MATERIAL_RT_DEPTH_SHARED = 0x0,
	MATERIAL_RT_DEPTH_SEPARATE = 0x1,
	MATERIAL_RT_DEPTH_NONE = 0x2,
	MATERIAL_RT_DEPTH_ONLY = 0x3
};

enum RestoreChangeFlags_t
{
	MATERIAL_RESTORE_VERTEX_FORMAT_CHANGED = 0x1
};

enum RenderTargetSizeMode_t
{
	RT_SIZE_NO_CHANGE = 0,
	RT_SIZE_DEFAULT = 1,
	RT_SIZE_PICMIP = 2,
	RT_SIZE_HDR = 3,
	RT_SIZE_FULL_FRAME_BUFFER = 4,
	RT_SIZE_OFFSCREEN = 5,
	RT_SIZE_FULL_FRAME_BUFFER_ROUNDED_UP = 6,
	RT_SIZE_REPLAY_SCREENSHOT = 7,
	RT_SIZE_LITERAL = 8,
	RT_SIZE_LITERAL_PICMIP = 9
};

enum RenderBackend_t
{
	RENDER_BACKEND_UNKNOWN,
	RENDER_BACKEND_D3D9,
	RENDER_BACKEND_TOGL,
	RENDER_BACKEND_VULKAN,
	RENDER_BACKEND_NULL,
};

typedef void (*MaterialBufferReleaseFunc_t)();
typedef void (*MaterialBufferRestoreFunc_t)(int nChangeFlags);
typedef void (*ModeChangeCallbackFunc_t)(void);

typedef int VertexBufferHandle_t;
typedef unsigned short MaterialHandle_t;

using OcclusionQueryObjectHandle_t = void*;
#define INVALID_OCCLUSION_QUERY_OBJECT_HANDLE ( (OcclusionQueryObjectHandle_t)0 )

class IMaterialProxyFactory;
class ITexture;
class IMaterialSystemHardwareConfig;
class CShadowMgr;

using MaterialLock_t = void*;

class IMaterialSystem : public IAppSystem
{
public:
	virtual bool Connect(CreateInterfaceFn factory) = 0;
	virtual void Disconnect() = 0;
	virtual void* QueryInterface(const char* pInterfaceName) = 0;
	virtual InitReturnVal_t Init() = 0;
	virtual void Shutdown() = 0;
	virtual CreateInterfaceFn	Init(char const* pShaderAPIDLL, IMaterialProxyFactory* pMaterialProxyFactory, CreateInterfaceFn fileSystemFactory, CreateInterfaceFn cvarFactory = NULL) = 0;
	virtual void SetShaderAPI(char const* pShaderAPIDLL) = 0;
	virtual void SetAdapter(int nAdapter, int nFlags) = 0;
	virtual void ModInit() = 0;
	virtual void ModShutdown() = 0;
	virtual void SetThreadMode(MaterialThreadMode_t mode, int nServiceThread = -1) = 0;
	virtual MaterialThreadMode_t GetThreadMode() = 0;
	virtual bool IsRenderThreadSafe() = 0;
	virtual void ExecuteQueued() = 0;
	virtual IMaterialSystemHardwareConfig* GetHardwareConfig(const char* pVersion, int* returnCode) = 0;
	virtual bool UpdateConfig(bool bForceUpdate) = 0;
	virtual bool OverrideConfig(const MaterialSystem_Config_t& config, bool bForceUpdate) = 0;
	virtual const MaterialSystem_Config_t& GetCurrentConfigForVideoCard() const = 0;
	virtual bool GetRecommendedConfigurationInfo(int nDXLevel, KeyValues* pKeyValues) = 0;
	virtual int GetDisplayAdapterCount() const = 0;
	virtual int GetCurrentAdapter() const = 0;
	virtual void GetDisplayAdapterInfo(int adapter, MaterialAdapterInfo_t& info) const = 0;
	virtual int GetModeCount(int adapter) const = 0;
	virtual void GetModeInfo(int adapter, int mode, MaterialVideoMode_t& info) const = 0;
	virtual void AddModeChangeCallBack(ModeChangeCallbackFunc_t func) = 0;
	virtual void GetDisplayMode(MaterialVideoMode_t& mode) const = 0;
	virtual bool SetMode(void* hwnd, const MaterialSystem_Config_t& config) = 0;
	virtual bool SupportsMSAAMode(int nMSAAMode) = 0;
	virtual const MaterialSystemHardwareIdentifier_t& GetVideoCardIdentifier(void) const = 0;
	virtual void SpewDriverInfo() const = 0;
	virtual void GetDXLevelDefaults(uint& max_dxlevel, uint& recommended_dxlevel) = 0;
	virtual void GetBackBufferDimensions(int& width, int& height) const = 0;
	virtual ImageFormat GetBackBufferFormat() const = 0;
	virtual bool SupportsHDRMode(HDRType_t nHDRModede) = 0;
	virtual bool AddView(void* hwnd) = 0;
	virtual void RemoveView(void* hwnd) = 0;
	virtual void SetView(void* hwnd) = 0;
	virtual void BeginFrame(float frameTime) = 0;
	virtual void EndFrame() = 0;
	virtual void Flush(bool flushHardware = false) = 0;
	virtual void SwapBuffers() = 0;
	virtual void EvictManagedResources() = 0;
	virtual void ReleaseResources(void) = 0;
	virtual void ReacquireResources(void) = 0;
	virtual void AddReleaseFunc(MaterialBufferReleaseFunc_t func) = 0;
	virtual void RemoveReleaseFunc(MaterialBufferReleaseFunc_t func) = 0;
	virtual void AddRestoreFunc(MaterialBufferRestoreFunc_t func) = 0;
	virtual void RemoveRestoreFunc(MaterialBufferRestoreFunc_t func) = 0;
	virtual void ResetTempHWMemory(bool bExitingLevel = false) = 0;
	virtual void HandleDeviceLost() = 0;
	virtual int ShaderCount() const = 0;
	virtual int GetShaders(int nFirstShader, int nMaxCount, IShader** ppShaderList) const = 0;
	virtual int ShaderFlagCount() const = 0;
	virtual const char* ShaderFlagName(int nIndex) const = 0;
	virtual void GetShaderFallback(const char* pShaderName, char* pFallbackShader, int nFallbackLength) = 0;
	virtual IMaterialProxyFactory* GetMaterialProxyFactory() = 0;
	virtual void SetMaterialProxyFactory(IMaterialProxyFactory* pFactory) = 0;
	virtual void EnableEditorMaterials() = 0;
	virtual void SetInStubMode(bool bInStubMode) = 0;
	virtual void DebugPrintUsedMaterials(const char* pSearchSubString, bool bVerbose) = 0;
	virtual void DebugPrintUsedTextures(void) = 0;
	virtual void ToggleSuppressMaterial(char const* pMaterialName) = 0;
	virtual void ToggleDebugMaterial(char const* pMaterialName) = 0;
	virtual bool UsingFastClipping(void) = 0;
	virtual int StencilBufferBits(void) = 0;
	virtual void UncacheAllMaterials() = 0;
	virtual void UncacheUnusedMaterials(bool bRecomputeStateSnapshots = false) = 0;
	virtual void CacheUsedMaterials() = 0;
	virtual void ReloadTextures() = 0;
	virtual void ReloadMaterials(const char* pSubString = NULL) = 0;
	virtual IMaterial* CreateMaterial(const char* pMaterialName, KeyValues* pVMTKeyValues) = 0;
	virtual IMaterial* FindMaterial(char const* pMaterialName, const char* pTextureGroupName, bool complain = true, const char* pComplainPrefix = NULL) = 0;
	virtual bool IsMaterialLoaded(char const* pMaterialName) = 0;
	virtual MaterialHandle_t FirstMaterial() const = 0;
	virtual MaterialHandle_t NextMaterial(MaterialHandle_t h) const = 0;
	virtual MaterialHandle_t InvalidMaterial() const = 0;
	virtual IMaterial* GetMaterial(MaterialHandle_t h) const = 0;
	virtual int GetNumMaterials() const = 0;
	virtual void SetAsyncTextureLoadCache(void* hFileCache) = 0;
	virtual ITexture* FindTexture(char const* pTextureName, const char* pTextureGroupName, bool complain = true, int nAdditionalCreationFlags = 0) = 0;
	virtual bool IsTextureLoaded(char const* pTextureName) const = 0;
	virtual ITexture* CreateProceduralTexture(const char* pTextureName, const char* pTextureGroupName, int w, int h, ImageFormat fmt, int nFlags) = 0;
	virtual void BeginRenderTargetAllocation() = 0;
	virtual void EndRenderTargetAllocation() = 0;
	virtual ITexture* CreateRenderTargetTexture(int w, int h, RenderTargetSizeMode_t sizeMode, ImageFormat	format, MaterialRenderTargetDepth_t depth = MATERIAL_RT_DEPTH_SHARED) = 0;
	virtual ITexture* CreateNamedRenderTargetTextureEx(const char* pRTName, int w, int h, RenderTargetSizeMode_t sizeMode, ImageFormat format, MaterialRenderTargetDepth_t depth = MATERIAL_RT_DEPTH_SHARED, unsigned int textureFlags = TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT, unsigned int renderTargetFlags = 0) = 0;
	virtual ITexture* CreateNamedRenderTargetTexture(const char* pRTName, int w, int h, RenderTargetSizeMode_t sizeMode, ImageFormat format, MaterialRenderTargetDepth_t depth = MATERIAL_RT_DEPTH_SHARED, bool bClampTexCoords = true, bool bAutoMipMap = false) = 0;
	virtual ITexture* CreateNamedRenderTargetTextureEx2(const char* pRTName, int w, int h, RenderTargetSizeMode_t sizeMode, ImageFormat format, MaterialRenderTargetDepth_t depth = MATERIAL_RT_DEPTH_SHARED, unsigned int textureFlags = TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT, unsigned int renderTargetFlags = 0) = 0;
	virtual void BeginLightmapAllocation() = 0;
	virtual void EndLightmapAllocation() = 0;
	virtual int AllocateLightmap(int width, int height, int offsetIntoLightmapPage[2], IMaterial* pMaterial) = 0;
	virtual int AllocateWhiteLightmap(IMaterial* pMaterial) = 0;
	virtual void UpdateLightmap(int lightmapPageID, int lightmapSize[2], int offsetIntoLightmapPage[2], float* pFloatImage, float* pFloatImageBump1, float* pFloatImageBump2, float* pFloatImageBump3) = 0;
	virtual int GetNumSortIDs() = 0;
	virtual void GetSortInfo(MaterialSystem_SortInfo_t* sortInfoArray) = 0;
	virtual void GetLightmapPageSize(int lightmap, int* width, int* height) const = 0;
	virtual void ResetMaterialLightmapPageInfo() = 0;
	virtual void ClearBuffers(bool bClearColor, bool bClearDepth, bool bClearStencil = false) = 0;
	virtual IMatRenderContext* GetRenderContext() = 0;
	virtual bool SupportsShadowDepthTextures(void) = 0;
	virtual void BeginUpdateLightmaps(void) = 0;
	virtual void EndUpdateLightmaps(void) = 0;
	virtual MaterialLock_t Lock() = 0;
	virtual void Unlock(MaterialLock_t) = 0;
	virtual ImageFormat GetShadowDepthTextureFormat() = 0;
	virtual bool SupportsFetch4(void) = 0;
	virtual IMatRenderContext* CreateRenderContext(MaterialContextType_t type) = 0;
	virtual IMatRenderContext* SetRenderContext(IMatRenderContext*) = 0;
	virtual bool SupportsCSAAMode(int nNumSamples, int nQualityLevel) = 0;
	virtual void RemoveModeChangeCallBack(ModeChangeCallbackFunc_t func) = 0;
	virtual IMaterial* FindProceduralMaterial(const char* pMaterialName, const char* pTextureGroupName, KeyValues* pVMTKeyValues) = 0;
	virtual ImageFormat GetNullTextureFormat() = 0;
	virtual void AddTextureAlias(const char* pAlias, const char* pRealName) = 0;
	virtual void RemoveTextureAlias(const char* pAlias) = 0;
	virtual int AllocateDynamicLightmap(int lightmapSize[2], int* pOutOffsetIntoPage, int frameID) = 0;
	virtual void SetExcludedTextures(const char* pScriptName) = 0;
	virtual void UpdateExcludedTextures(void) = 0;
	virtual bool IsInFrame() const = 0;
	virtual void CompactMemory() = 0;
	virtual void ReloadFilesInList(IFileList* pFilesToReload) = 0;
	virtual	bool AllowThreading(bool bAllow, int nServiceThread) = 0;
	virtual IMaterial* FindMaterialEx(char const* pMaterialName, const char* pTextureGroupName, int nContext, bool complain = true, const char* pComplainPrefix = NULL) = 0;
	virtual void SetRenderTargetFrameBufferSizeOverrides(int nWidth, int nHeight) = 0;
	virtual void GetRenderTargetFrameBufferDimensions(int& nWidth, int& nHeight) = 0;
	virtual char* GetDisplayDeviceName() const = 0;
	virtual ITexture* CreateTextureFromBits(int w, int h, int mips, ImageFormat fmt, int srcBufferSize, byte* srcBits) = 0;
	virtual void OverrideRenderTargetAllocation(bool rtAlloc) = 0;
	virtual ITextureCompositor* NewTextureCompositor(int w, int h, const char* pCompositeName, int nTeamNum, uint64 randomSeed, KeyValues* stageDesc, uint32 texCompositeCreateFlags = 0) = 0;
	virtual void AsyncFindTexture(const char* pFilename, const char* pTextureGroupName, IAsyncTextureOperationReceiver* pRecipient, void* pExtraArgs, bool bComplain = true, int nAdditionalCreationFlags = 0) = 0;
	virtual ITexture* CreateNamedTextureFromBitsEx(const char* pName, const char* pTextureGroupName, int w, int h, int mips, ImageFormat fmt, int srcBufferSize, byte* srcBits, int nFlags) = 0;
	virtual bool AddTextureCompositorTemplate(const char* pName, KeyValues* pTmplDesc, int nTexCompositeTemplateFlags = 0) = 0;
	virtual bool VerifyTextureCompositorTemplates() = 0;
	virtual RenderBackend_t GetRenderBackend() const = 0;
	virtual void SuspendTextureStreaming() = 0;
	virtual void ResumeTextureStreaming() = 0;
};

MAKE_INTERFACE_VERSION(IMaterialSystem, MaterialSystem, "materialsystem.dll", "VMaterialSystem082");

class IMatRenderContext : public IRefCounted
{
public:
	virtual void BeginRender() = 0;
	virtual void EndRender() = 0;
	virtual void Flush(bool flushHardware = false) = 0;
	virtual void BindLocalCubemap(ITexture* pTexture) = 0;
	virtual void SetRenderTarget(ITexture* pTexture) = 0;
	virtual ITexture* GetRenderTarget(void) = 0;
	virtual void GetRenderTargetDimensions(int& width, int& height) const = 0;
	virtual void Bind(IMaterial* material, void* proxyData = 0) = 0;
	virtual void BindLightmapPage(int lightmapPageID) = 0;
	virtual void DepthRange(float zNear, float zFar) = 0;
	virtual void ClearBuffers(bool bClearColor, bool bClearDepth, bool bClearStencil = false) = 0;
	virtual void ReadPixels(int x, int y, int width, int height, unsigned char* data, ImageFormat dstFormat) = 0;
	virtual void SetAmbientLight(float r, float g, float b) = 0;
	virtual void SetLight(int lightNum, const LightDesc_t& desc) = 0;
	virtual void SetAmbientLightCube(Vector4D cube[6]) = 0;
	virtual void CopyRenderTargetToTexture(ITexture* pTexture) = 0;
	virtual void SetFrameBufferCopyTexture(ITexture* pTexture, int textureIndex = 0) = 0;
	virtual ITexture* GetFrameBufferCopyTexture(int textureIndex) = 0;
	virtual void MatrixMode(MaterialMatrixMode_t matrixMode) = 0;
	virtual void PushMatrix(void) = 0;
	virtual void PopMatrix(void) = 0;
	virtual void LoadMatrix(VMatrix const& matrix) = 0;
	virtual void LoadMatrix(matrix3x4 const& matrix) = 0;
	virtual void MultMatrix(VMatrix const& matrix) = 0;
	virtual void MultMatrix(matrix3x4 const& matrix) = 0;
	virtual void MultMatrixLocal(VMatrix const& matrix) = 0;
	virtual void MultMatrixLocal(matrix3x4 const& matrix) = 0;
	virtual void GetMatrix(MaterialMatrixMode_t matrixMode, VMatrix* matrix) = 0;
	virtual void GetMatrix(MaterialMatrixMode_t matrixMode, matrix3x4* matrix) = 0;
	virtual void LoadIdentity(void) = 0;
	virtual void Ortho(double left, double top, double right, double bottom, double zNear, double zFar) = 0;
	virtual void PerspectiveX(double fovx, double aspect, double zNear, double zFar) = 0;
	virtual void PickMatrix(int x, int y, int width, int height) = 0;
	virtual void Rotate(float angle, float x, float y, float z) = 0;
	virtual void Translate(float x, float y, float z) = 0;
	virtual void Scale(float x, float y, float z) = 0;
	virtual void Viewport(int x, int y, int width, int height) = 0;
	virtual void GetViewport(int& x, int& y, int& width, int& height) const = 0;
	virtual void CullMode(MaterialCullMode_t cullMode) = 0;
	virtual void SetHeightClipMode(MaterialHeightClipMode_t nHeightClipMode) = 0;
	virtual void SetHeightClipZ(float z) = 0;
	virtual void FogMode(MaterialFogMode_t fogMode) = 0;
	virtual void FogStart(float fStart) = 0;
	virtual void FogEnd(float fEnd) = 0;
	virtual void SetFogZ(float fogZ) = 0;
	virtual MaterialFogMode_t GetFogMode(void) = 0;
	virtual void FogColor3f(float r, float g, float b) = 0;
	virtual void FogColor3fv(float const* rgb) = 0;
	virtual void FogColor3ub(unsigned char r, unsigned char g, unsigned char b) = 0;
	virtual void FogColor3ubv(unsigned char const* rgb) = 0;
	virtual void GetFogColor(unsigned char* rgb) = 0;
	virtual void SetNumBoneWeights(int numBones) = 0;
	virtual IMesh* CreateStaticMesh(VertexFormat_t fmt, const char* pTextureBudgetGroup, IMaterial* pMaterial = NULL) = 0;
	virtual void DestroyStaticMesh(IMesh* mesh) = 0;
	virtual IMesh* GetDynamicMesh(bool buffered = true, IMesh* pVertexOverride = 0, IMesh* pIndexOverride = 0, IMaterial* pAutoBind = 0) = 0;
	virtual IVertexBuffer* CreateStaticVertexBuffer(VertexFormat_t fmt, int nVertexCount, const char* pTextureBudgetGroup) = 0;
	virtual IIndexBuffer* CreateStaticIndexBuffer(MaterialIndexFormat_t fmt, int nIndexCount, const char* pTextureBudgetGroup) = 0;
	virtual void DestroyVertexBuffer(IVertexBuffer*) = 0;
	virtual void DestroyIndexBuffer(IIndexBuffer*) = 0;
	virtual IVertexBuffer* GetDynamicVertexBuffer(int streamID, VertexFormat_t vertexFormat, bool bBuffered = true) = 0;
	virtual IIndexBuffer* GetDynamicIndexBuffer(MaterialIndexFormat_t fmt, bool bBuffered = true) = 0;
	virtual void BindVertexBuffer(int streamID, IVertexBuffer* pVertexBuffer, int nOffsetInBytes, int nFirstVertex, int nVertexCount, VertexFormat_t fmt, int nRepetitions = 1) = 0;
	virtual void BindIndexBuffer(IIndexBuffer* pIndexBuffer, int nOffsetInBytes) = 0;
	virtual void Draw(MaterialPrimitiveType_t primitiveType, int firstIndex, int numIndices) = 0;
	virtual int  SelectionMode(bool selectionMode) = 0;
	virtual void SelectionBuffer(unsigned int* pBuffer, int size) = 0;
	virtual void ClearSelectionNames() = 0;
	virtual void LoadSelectionName(int name) = 0;
	virtual void PushSelectionName(int name) = 0;
	virtual void PopSelectionName() = 0;
	virtual void ClearColor3ub(unsigned char r, unsigned char g, unsigned char b) = 0;
	virtual void ClearColor4ub(unsigned char r, unsigned char g, unsigned char b, unsigned char a) = 0;
	virtual void OverrideDepthEnable(bool bEnable, bool bDepthEnable) = 0;
	virtual void DrawScreenSpaceQuad(IMaterial* pMaterial) = 0;
	virtual void SyncToken(const char* pToken) = 0;
	virtual float ComputePixelWidthOfSphere(const Vector& origin, float flRadius) = 0;
	virtual OcclusionQueryObjectHandle_t CreateOcclusionQueryObject(void) = 0;
	virtual void DestroyOcclusionQueryObject(OcclusionQueryObjectHandle_t) = 0;
	virtual void BeginOcclusionQueryDrawing(OcclusionQueryObjectHandle_t) = 0;
	virtual void EndOcclusionQueryDrawing(OcclusionQueryObjectHandle_t) = 0;
	virtual int OcclusionQuery_GetNumPixelsRendered(OcclusionQueryObjectHandle_t) = 0;
	virtual void SetFlashlightMode(bool bEnable) = 0;
	virtual void SetFlashlightState(const FlashlightState_t& state, const VMatrix& worldToTexture) = 0;
	virtual MaterialHeightClipMode_t GetHeightClipMode() = 0;
	virtual float ComputePixelDiameterOfSphere(const Vector& vecAbsOrigin, float flRadius) = 0;
	virtual void EnableUserClipTransformOverride(bool bEnable) = 0;
	virtual void UserClipTransform(const VMatrix& worldToView) = 0;
	virtual bool GetFlashlightMode() const = 0;
	virtual void ResetOcclusionQueryObject(OcclusionQueryObjectHandle_t) = 0;
	virtual void Unused3() {}
	virtual IMorph* CreateMorph(MorphFormat_t format, const char* pDebugName) = 0;
	virtual void DestroyMorph(IMorph* pMorph) = 0;
	virtual void BindMorph(IMorph* pMorph) = 0;
	virtual void SetFlexWeights(int nFirstWeight, int nCount, const MorphWeight_t* pWeights) = 0;
	virtual void Unused4() {};
	virtual void Unused5() {};
	virtual void Unused6() {};
	virtual void Unused7() {};
	virtual void Unused8() {};
	virtual void ReadPixelsAndStretch(Rect_t* pSrcRect, Rect_t* pDstRect, unsigned char* pBuffer, ImageFormat dstFormat, int nDstStride) = 0;
	virtual void GetWindowSize(int& width, int& height) const = 0;

	virtual void DrawScreenSpaceRectangle(
		IMaterial* pMaterial,
		int destx, int desty,
		int width, int height,
		float src_texture_x0, float src_texture_y0,
		float src_texture_x1, float src_texture_y1,
		int src_texture_width, int src_texture_height,
		void* pClientRenderable = NULL,
		int nXDice = 1,
		int nYDice = 1) = 0;

	virtual void LoadBoneMatrix(int boneIndex, const matrix3x4& matrix) = 0;
	virtual void PushRenderTargetAndViewport() = 0;
	virtual void PushRenderTargetAndViewport(ITexture* pTexture) = 0;
	virtual void PushRenderTargetAndViewport(ITexture* pTexture, int nViewX, int nViewY, int nViewW, int nViewH) = 0;
	virtual void PushRenderTargetAndViewport(ITexture* pTexture, ITexture* pDepthTexture, int nViewX, int nViewY, int nViewW, int nViewH) = 0;
	virtual void PopRenderTargetAndViewport(void) = 0;
	virtual void BindLightmapTexture(ITexture* pLightmapTexture) = 0;
	virtual void CopyRenderTargetToTextureEx(ITexture* pTexture, int nRenderTargetID, Rect_t* pSrcRect, Rect_t* pDstRect = NULL) = 0;
	virtual void CopyTextureToRenderTargetEx(int nRenderTargetID, ITexture* pTexture, Rect_t* pSrcRect, Rect_t* pDstRect = NULL) = 0;
	virtual void PerspectiveOffCenterX(double fovx, double aspect, double zNear, double zFar, double bottom, double top, double left, double right) = 0;
	virtual void SetFloatRenderingParameter(int parm_number, float value) = 0;
	virtual void SetIntRenderingParameter(int parm_number, int value) = 0;
	virtual void SetVectorRenderingParameter(int parm_number, Vector const& value) = 0;
	virtual void SetStencilEnable(bool onoff) = 0;
	virtual void SetStencilFailOperation(StencilOperation_t op) = 0;
	virtual void SetStencilZFailOperation(StencilOperation_t op) = 0;
	virtual void SetStencilPassOperation(StencilOperation_t op) = 0;
	virtual void SetStencilCompareFunction(StencilComparisonFunction_t cmpfn) = 0;
	virtual void SetStencilReferenceValue(int ref) = 0;
	virtual void SetStencilTestMask(uint32 msk) = 0;
	virtual void SetStencilWriteMask(uint32 msk) = 0;
	virtual void ClearStencilBufferRectangle(int xmin, int ymin, int xmax, int ymax, int value) = 0;
	virtual void SetRenderTargetEx(int nRenderTargetID, ITexture* pTexture) = 0;
	virtual void PushCustomClipPlane(const float* pPlane) = 0;
	virtual void PopCustomClipPlane(void) = 0;
	virtual void GetMaxToRender(IMesh* pMesh, bool bMaxUntilFlush, int* pMaxVerts, int* pMaxIndices) = 0;
	virtual int GetMaxVerticesToRender(IMaterial* pMaterial) = 0;
	virtual int GetMaxIndicesToRender() = 0;
	virtual void DisableAllLocalLights() = 0;
	virtual int CompareMaterialCombos(IMaterial* pMaterial1, IMaterial* pMaterial2, int lightMapID1, int lightMapID2) = 0;
	virtual IMesh* GetFlexMesh() = 0;
	virtual void SetFlashlightStateEx(const FlashlightState_t& state, const VMatrix& worldToTexture, ITexture* pFlashlightDepthTexture) = 0;
	virtual ITexture* GetLocalCubemap() = 0;
	virtual void ClearBuffersObeyStencil(bool bClearColor, bool bClearDepth) = 0;
	virtual bool EnableClipping(bool bEnable) = 0;
	virtual void GetFogDistances(float* fStart, float* fEnd, float* fFogZ) = 0;
	virtual void BeginPIXEvent(unsigned long color, const char* szName) = 0;
	virtual void EndPIXEvent() = 0;
	virtual void SetPIXMarker(unsigned long color, const char* szName) = 0;
	virtual void BeginBatch(IMesh* pIndices) = 0;
	virtual void BindBatch(IMesh* pVertices, IMaterial* pAutoBind = NULL) = 0;
	virtual void DrawBatch(int firstIndex, int numIndices) = 0;
	virtual void EndBatch() = 0;
	virtual ICallQueue* GetCallQueue() = 0;
	virtual void GetWorldSpaceCameraPosition(Vector* pCameraPos) = 0;
	virtual void GetWorldSpaceCameraVectors(Vector* pVecForward, Vector* pVecRight, Vector* pVecUp) = 0;
	virtual void ResetToneMappingScale(float monoscale) = 0;
	virtual void SetGoalToneMappingScale(float monoscale) = 0;
	virtual void TurnOnToneMapping() = 0;
	virtual void SetToneMappingScaleLinear(const Vector& scale) = 0;
	virtual Vector GetToneMappingScaleLinear(void) = 0;
	virtual void SetShadowDepthBiasFactors(float fSlopeScaleDepthBias, float fDepthBias) = 0;
	virtual void PerformFullScreenStencilOperation(void) = 0;
	virtual void SetLightingOrigin(Vector vLightingOrigin) = 0;
	virtual void SetScissorRect(const int nLeft, const int nTop, const int nRight, const int nBottom, const bool bEnableScissor) = 0;
	virtual void BeginMorphAccumulation() = 0;
	virtual void EndMorphAccumulation() = 0;
	virtual void AccumulateMorph(IMorph* pMorph, int nMorphCount, const MorphWeight_t* pWeights) = 0;
	virtual void PushDeformation(DeformationBase_t const* Deformation) = 0;
	virtual void PopDeformation() = 0;
	virtual int GetNumActiveDeformations() const = 0;
	virtual bool GetMorphAccumulatorTexCoord(Vector2D* pTexCoord, IMorph* pMorph, int nVertex) = 0;
	virtual IMesh* GetDynamicMeshEx(VertexFormat_t vertexFormat, bool bBuffered = true, IMesh* pVertexOverride = 0, IMesh* pIndexOverride = 0, IMaterial* pAutoBind = 0) = 0;
	virtual void FogMaxDensity(float flMaxDensity) = 0;
	virtual IMaterial* GetCurrentMaterial() = 0;
	virtual int  GetCurrentNumBones() const = 0;
	virtual void* GetCurrentProxy() = 0;
	virtual void EnableColorCorrection(bool bEnable) = 0;
	virtual ColorCorrectionHandle_t AddLookup(const char* pName) = 0;
	virtual bool RemoveLookup(ColorCorrectionHandle_t handle) = 0;
	virtual void LockLookup(ColorCorrectionHandle_t handle) = 0;
	virtual void LoadLookup(ColorCorrectionHandle_t handle, const char* pLookupName) = 0;
	virtual void UnlockLookup(ColorCorrectionHandle_t handle) = 0;
	virtual void SetLookupWeight(ColorCorrectionHandle_t handle, float flWeight) = 0;
	virtual void ResetLookupWeights() = 0;
	virtual void SetResetable(ColorCorrectionHandle_t handle, bool bResetable) = 0;
	virtual void SetFullScreenDepthTextureValidityFlag(bool bIsValid) = 0;
	virtual void SetNonInteractivePacifierTexture(ITexture* pTexture, float flNormalizedX, float flNormalizedY, float flNormalizedSize) = 0;
	virtual void SetNonInteractiveTempFullscreenBuffer(ITexture* pTexture, MaterialNonInteractiveMode_t mode) = 0;
	virtual void EnableNonInteractiveMode(MaterialNonInteractiveMode_t mode) = 0;
	virtual void RefreshFrontBufferNonInteractive() = 0;
	virtual void* LockRenderData(int nSizeInBytes) = 0;
	virtual void UnlockRenderData(void* pData) = 0;
	virtual void AddRefRenderData() = 0;
	virtual void ReleaseRenderData() = 0;
	virtual bool IsRenderData(const void* pData) const = 0;
	virtual void PrintfVA(char* fmt, va_list vargs) = 0;
	virtual void Printf(const char* fmt, ...) = 0;
	virtual float Knob(char* knobname, float* setvalue = NULL) = 0;
	virtual void OverrideAlphaWriteEnable(bool bEnable, bool bAlphaWriteEnable) = 0;
	virtual void OverrideColorWriteEnable(bool bOverrideEnable, bool bColorWriteEnable) = 0;
	virtual void ClearBuffersObeyStencilEx(bool bClearColor, bool bClearAlpha, bool bClearDepth) = 0;
	virtual void AsyncCreateTextureFromRenderTarget(ITexture* pSrcRt, const char* pDstName, ImageFormat dstFmt, bool bGenMips, int nAdditionalCreationFlags, IAsyncTextureOperationReceiver* pRecipient, void* pExtraArgs) = 0;
};