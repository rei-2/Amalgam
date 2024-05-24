#pragma once

enum HDRType_t
{
	HDR_TYPE_NONE,
	HDR_TYPE_INTEGER,
	HDR_TYPE_FLOAT
};

enum VertexCompressionType_t
{
	VERTEX_COMPRESSION_INVALID = 0xFFFFFFFF,
	VERTEX_COMPRESSION_NONE = 0,
	VERTEX_COMPRESSION_ON = 1
};

#define DEFCONFIGMETHOD( ret_type, method, xbox_return_value ) virtual ret_type method const = 0;

class IMaterialSystemHardwareConfig
{
public:
	DEFCONFIGMETHOD(bool, HasDestAlphaBuffer(), true);
	DEFCONFIGMETHOD(bool, HasStencilBuffer(), true);
	virtual int GetFrameBufferColorDepth() const = 0;
	virtual int GetSamplerCount() const = 0;
	virtual bool HasSetDeviceGammaRamp() const = 0;
	DEFCONFIGMETHOD(bool, SupportsCompressedTextures(), true);
	virtual VertexCompressionType_t SupportsCompressedVertices() const = 0;
	virtual bool SupportsNormalMapCompression() const { return false; }
	DEFCONFIGMETHOD(bool, SupportsVertexAndPixelShaders(), true);
	DEFCONFIGMETHOD(bool, SupportsPixelShaders_1_4(), true);
	DEFCONFIGMETHOD(bool, SupportsStaticControlFlow(), true);
	DEFCONFIGMETHOD(bool, SupportsPixelShaders_2_0(), true);
	DEFCONFIGMETHOD(bool, SupportsVertexShaders_2_0(), true);
	virtual int MaximumAnisotropicLevel() const = 0;
	virtual int MaxTextureWidth() const = 0;
	virtual int MaxTextureHeight() const = 0;
	virtual int TextureMemorySize() const = 0;
	virtual bool SupportsOverbright() const = 0;
	virtual bool SupportsCubeMaps() const = 0;
	virtual bool SupportsMipmappedCubemaps() const = 0;
	virtual bool SupportsNonPow2Textures() const = 0;
	virtual int GetTextureStageCount() const = 0;
	virtual int NumVertexShaderConstants() const = 0;
	virtual int NumPixelShaderConstants() const = 0;
	virtual int MaxNumLights() const = 0;
	virtual bool SupportsHardwareLighting() const = 0;
	virtual int MaxBlendMatrices() const = 0;
	virtual int MaxBlendMatrixIndices() const = 0;
	virtual int MaxTextureAspectRatio() const = 0;
	virtual int MaxVertexShaderBlendMatrices() const = 0;
	virtual int MaxUserClipPlanes() const = 0;
	virtual bool UseFastClipping() const = 0;
	DEFCONFIGMETHOD(int, GetDXSupportLevel(), 98);
	virtual const char* GetShaderDLLName() const = 0;
	virtual bool ReadPixelsFromFrontBuffer() const = 0;
	virtual bool PreferDynamicTextures() const = 0;
	DEFCONFIGMETHOD(bool, SupportsHDR(), true);
	virtual bool HasProjectedBumpEnv() const = 0;
	virtual bool SupportsSpheremapping() const = 0;
	virtual bool NeedsAAClamp() const = 0;
	virtual bool NeedsATICentroidHack() const = 0;
	virtual bool SupportsColorOnSecondStream() const = 0;
	virtual bool SupportsStaticPlusDynamicLighting() const = 0;
	virtual bool PreferReducedFillrate() const = 0;
	virtual int	 GetMaxDXSupportLevel() const = 0;
	virtual bool SpecifiesFogColorInLinearSpace() const = 0;
	DEFCONFIGMETHOD(bool, SupportsSRGB(), true);
	DEFCONFIGMETHOD(bool, FakeSRGBWrite(), false);
	DEFCONFIGMETHOD(bool, CanDoSRGBReadFromRTs(), true);
	virtual bool SupportsGLMixedSizeTargets() const = 0;
	virtual bool IsAAEnabled() const = 0;
	virtual int GetVertexTextureCount() const = 0;
	virtual int GetMaxVertexTextureDimension() const = 0;
	virtual int MaxTextureDepth() const = 0;
	virtual HDRType_t GetHDRType() const = 0;
	virtual HDRType_t GetHardwareHDRType() const = 0;
	DEFCONFIGMETHOD(bool, SupportsPixelShaders_2_b(), true);
	virtual bool SupportsStreamOffset() const = 0;
	virtual int StencilBufferBits() const = 0;
	virtual int MaxViewports() const = 0;
	virtual void OverrideStreamOffsetSupport(bool bOverrideEnabled, bool bEnableSupport) = 0;
	virtual int GetShadowFilterMode() const = 0;
	virtual int NeedsShaderSRGBConversion() const = 0;
	DEFCONFIGMETHOD(bool, UsesSRGBCorrectBlending(), true);
	virtual bool SupportsShaderModel_3_0() const = 0;
	virtual bool HasFastVertexTextures() const = 0;
	virtual int MaxHWMorphBatchCount() const = 0;
	DEFCONFIGMETHOD(bool, ActuallySupportsPixelShaders_2_b(), true);
	virtual bool SupportsHDRMode(HDRType_t nHDRMode) const = 0;
	virtual bool GetHDREnabled(void) const = 0;
	virtual void SetHDREnabled(bool bEnable) = 0;
	virtual bool SupportsBorderColor(void) const = 0;
	virtual bool SupportsFetch4(void) const = 0;
	virtual bool CanStretchRectFromTextures(void) const = 0;
};