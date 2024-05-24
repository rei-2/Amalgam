#pragma once
#include "../Misc/BaseTypes.h"
#include "../Misc/ImageFormat.h"

class IVTFTexture;
class ITexture;
struct Rect_t;

class ITextureRegenerator
{
public:
	virtual void RegenerateTextureBits(ITexture* pTexture, IVTFTexture* pVTFTexture, Rect_t* pRect) = 0;
	virtual void Release() = 0;
};

class ITexture
{
public:
	virtual const char* GetName(void) const = 0;
	virtual int GetMappingWidth() const = 0;
	virtual int GetMappingHeight() const = 0;
	virtual int GetActualWidth() const = 0;
	virtual int GetActualHeight() const = 0;
	virtual int GetNumAnimationFrames() const = 0;
	virtual bool IsTranslucent() const = 0;
	virtual bool IsMipmapped() const = 0;
	virtual void GetLowResColorSample(float s, float t, float* color) const = 0;
	virtual void* GetResourceData(uint32 eDataType, size_t* pNumBytes) const = 0;
	virtual void IncrementReferenceCount(void) = 0;
	virtual void DecrementReferenceCount(void) = 0;
	inline void AddRef() { IncrementReferenceCount(); }
	inline void Release() { DecrementReferenceCount(); }
	virtual void SetTextureRegenerator(ITextureRegenerator* pTextureRegen) = 0;
	virtual void Download(Rect_t* pRect = 0, int nAdditionalCreationFlags = 0) = 0;
	virtual int GetApproximateVidMemBytes(void) const = 0;
	virtual bool IsError() const = 0;
	virtual bool IsVolumeTexture() const = 0;
	virtual int GetMappingDepth() const = 0;
	virtual int GetActualDepth() const = 0;
	virtual ImageFormat GetImageFormat() const = 0;
	virtual NormalDecodeMode_t GetNormalDecodeMode() const = 0;
	virtual bool IsRenderTarget() const = 0;
	virtual bool IsCubeMap() const = 0;
	virtual bool IsNormalMap() const = 0;
	virtual bool IsProcedural() const = 0;
	virtual void DeleteIfUnreferenced() = 0;
	virtual void SwapContents(ITexture* pOther) = 0;
	virtual unsigned int GetFlags(void) const = 0;
	virtual void ForceLODOverride(int iNumLodsOverrideUpOrDown) = 0;
	virtual bool SaveToFile(const char* fileName) = 0;
	virtual void CopyToStagingTexture(ITexture* pDstTex) = 0;
	virtual void SetErrorTexture(bool bIsErrorTexture) = 0;
};

inline bool IsErrorTexture(ITexture* pTex)
{
	return !pTex || pTex->IsError();
}