#pragma once
#include "BaseTypes.h"
#include "ImageFormat.h"

class CUtlBuffer;
struct Rect_t;
class IFileSystem;

enum CompiledVtfFlags
{
	TEXTUREFLAGS_POINTSAMPLE = 0x00000001,
	TEXTUREFLAGS_TRILINEAR = 0x00000002,
	TEXTUREFLAGS_CLAMPS = 0x00000004,
	TEXTUREFLAGS_CLAMPT = 0x00000008,
	TEXTUREFLAGS_ANISOTROPIC = 0x00000010,
	TEXTUREFLAGS_HINT_DXT5 = 0x00000020,
	TEXTUREFLAGS_SRGB = 0x00000040,
	TEXTUREFLAGS_NORMAL = 0x00000080,
	TEXTUREFLAGS_NOMIP = 0x00000100,
	TEXTUREFLAGS_NOLOD = 0x00000200,
	TEXTUREFLAGS_ALL_MIPS = 0x00000400,
	TEXTUREFLAGS_PROCEDURAL = 0x00000800,
	TEXTUREFLAGS_ONEBITALPHA = 0x00001000,
	TEXTUREFLAGS_EIGHTBITALPHA = 0x00002000,
	TEXTUREFLAGS_ENVMAP = 0x00004000,
	TEXTUREFLAGS_RENDERTARGET = 0x00008000,
	TEXTUREFLAGS_DEPTHRENDERTARGET = 0x00010000,
	TEXTUREFLAGS_NODEBUGOVERRIDE = 0x00020000,
	TEXTUREFLAGS_SINGLECOPY = 0x00040000,
	TEXTUREFLAGS_STAGING_MEMORY = 0x00080000,
	TEXTUREFLAGS_IMMEDIATE_CLEANUP = 0x00100000,
	TEXTUREFLAGS_IGNORE_PICMIP = 0x00200000,
	TEXTUREFLAGS_UNUSED_00400000 = 0x00400000,
	TEXTUREFLAGS_NODEPTHBUFFER = 0x00800000,
	TEXTUREFLAGS_UNUSED_01000000 = 0x01000000,
	TEXTUREFLAGS_CLAMPU = 0x02000000,
	TEXTUREFLAGS_VERTEXTEXTURE = 0x04000000,
	TEXTUREFLAGS_SSBUMP = 0x08000000,
	TEXTUREFLAGS_UNUSED_10000000 = 0x10000000,
	TEXTUREFLAGS_BORDER = 0x20000000,
	TEXTUREFLAGS_STREAMABLE_COARSE = 0x40000000,
	TEXTUREFLAGS_STREAMABLE_FINE = 0x80000000,
	TEXTUREFLAGS_STREAMABLE = (TEXTUREFLAGS_STREAMABLE_COARSE | TEXTUREFLAGS_STREAMABLE_FINE)
};

enum VersionedVtfFlags
{
	VERSIONED_VTF_FLAGS_MASK_7_3 = ~0xD1780400
};


struct VtfProcessingOptions
{
	uint32 cbSize;

	enum Flags0
	{
		OPT_DECAY_R = 0x00000001,
		OPT_DECAY_G = 0x00000002,
		OPT_DECAY_B = 0x00000004,
		OPT_DECAY_A = 0x00000008,
		OPT_DECAY_EXP_R = 0x00000010,
		OPT_DECAY_EXP_G = 0x00000020,
		OPT_DECAY_EXP_B = 0x00000040,
		OPT_DECAY_EXP_A = 0x00000080,
		OPT_NOCOMPRESS = 0x00000100,
		OPT_NORMAL_DUDV = 0x00000200,
		OPT_FILTER_NICE = 0x00000400,
		OPT_SET_ALPHA_ONEOVERMIP = 0x00001000,
		OPT_PREMULT_COLOR_ONEOVERMIP = 0x00002000,
		OPT_MIP_ALPHATEST = 0x00004000
	};

	uint32 flags0;
	uint8 clrDecayGoal[4];
	uint8 numNotDecayMips[4];
	float fDecayExponentBase[4];
};

enum CubeMapFaceIndex_t
{
	CUBEMAP_FACE_RIGHT = 0,
	CUBEMAP_FACE_LEFT,
	CUBEMAP_FACE_BACK,
	CUBEMAP_FACE_FRONT,
	CUBEMAP_FACE_UP,
	CUBEMAP_FACE_DOWN,
	CUBEMAP_FACE_SPHEREMAP,
	CUBEMAP_FACE_COUNT
};

enum LookDir_t
{
	LOOK_DOWN_X = 0,
	LOOK_DOWN_NEGX,
	LOOK_DOWN_Y,
	LOOK_DOWN_NEGY,
	LOOK_DOWN_Z,
	LOOK_DOWN_NEGZ
};

#define STREAMING_START_MIPMAP 3
#define IMAGE_FORMAT_DEFAULT ((ImageFormat)-2)

class IVTFTexture
{
public:
	virtual ~IVTFTexture() {}
	virtual bool Init(int nWidth, int nHeight, int nDepth, ImageFormat fmt, int nFlags, int iFrameCount, int nForceMipCount = -1) = 0;
	virtual void SetBumpScale(float flScale) = 0;
	virtual void SetReflectivity(const Vector& vecReflectivity) = 0;
	virtual void InitLowResImage(int nWidth, int nHeight, ImageFormat fmt) = 0;
	virtual void* SetResourceData(uint32 eType, void const* pData, size_t nDataSize) = 0;
	virtual void* GetResourceData(uint32 eType, size_t* pDataSize) const = 0;
	virtual bool HasResourceEntry(uint32 eType) const = 0;
	virtual unsigned int GetResourceTypes(uint32* arrTypesBuffer, int numTypesBufferElems) const = 0;
	virtual bool Unserialize(CUtlBuffer& buf, bool bHeaderOnly = false, int nSkipMipLevels = 0) = 0;
	virtual bool Serialize(CUtlBuffer& buf) = 0;
	virtual void LowResFileInfo(int* pStartLocation, int* pSizeInBytes) const = 0;
	virtual void ImageFileInfo(int nFrame, int nFace, int nMip, int* pStartLocation, int* pSizeInBytes) const = 0;
	virtual int FileSize(int nMipSkipCount = 0) const = 0;
	virtual int Width() const = 0;
	virtual int Height() const = 0;
	virtual int Depth() const = 0;
	virtual int MipCount() const = 0;
	virtual int RowSizeInBytes(int nMipLevel) const = 0;
	virtual int FaceSizeInBytes(int nMipLevel) const = 0;
	virtual ImageFormat Format() const = 0;
	virtual int FaceCount() const = 0;
	virtual int FrameCount() const = 0;
	virtual int Flags() const = 0;
	virtual float BumpScale() const = 0;
	virtual int LowResWidth() const = 0;
	virtual int LowResHeight() const = 0;
	virtual ImageFormat LowResFormat() const = 0;
	virtual const Vector& Reflectivity() const = 0;
	virtual bool IsCubeMap() const = 0;
	virtual bool IsNormalMap() const = 0;
	virtual bool IsVolumeTexture() const = 0;
	virtual void ComputeMipLevelDimensions(int iMipLevel, int* pMipWidth, int* pMipHeight, int* pMipDepth) const = 0;
	virtual int ComputeMipSize(int iMipLevel) const = 0;
	virtual void ComputeMipLevelSubRect(Rect_t* pSrcRect, int nMipLevel, Rect_t* pSubRect) const = 0;
	virtual int ComputeFaceSize(int iStartingMipLevel = 0) const = 0;
	virtual int ComputeTotalSize() const = 0;
	virtual unsigned char* ImageData() = 0;
	virtual unsigned char* ImageData(int iFrame, int iFace, int iMipLevel) = 0;
	virtual unsigned char* ImageData(int iFrame, int iFace, int iMipLevel, int x, int y, int z = 0) = 0;
	virtual unsigned char* LowResImageData() = 0;
	virtual	void ConvertImageFormat(ImageFormat fmt, bool bNormalToDUDV) = 0;
	virtual void GenerateSpheremap(LookDir_t lookDir = LOOK_DOWN_Z) = 0;
	virtual void GenerateHemisphereMap(unsigned char* pSphereMapBitsRGBA, int targetWidth, int targetHeight, LookDir_t lookDir, int iFrame) = 0;
	virtual void FixCubemapFaceOrientation() = 0;
	virtual void GenerateMipmaps() = 0;
	virtual void PutOneOverMipLevelInAlpha() = 0;
	virtual void ComputeReflectivity() = 0;
	virtual void ComputeAlphaFlags() = 0;
	virtual bool ConstructLowResImage() = 0;
	virtual void PostProcess(bool bGenerateSpheremap, LookDir_t lookDir = LOOK_DOWN_Z, bool bAllowFixCubemapOrientation = true) = 0;
	virtual void MatchCubeMapBorders(int iStage, ImageFormat finalFormat, bool bSkybox) = 0;
	virtual void SetAlphaTestThreshholds(float flBase, float flHighFreq) = 0;
	virtual void SetPostProcessingSettings(VtfProcessingOptions const* pOptions) = 0;
	virtual bool UnserializeEx(CUtlBuffer& buf, bool bHeaderOnly = false, int nForceFlags = 0, int nSkipMipLevels = 0) = 0;
	virtual void GetMipmapRange(int* pOutFinest, int* pOutCoarsest) = 0;
};