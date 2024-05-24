#pragma once
#include "../Interfaces/IMaterialSystem.h"

class IMaterialVar;
class ITexture;
class IMaterialProxy;

#define	VERTEX_POSITION 0x0001
#define	VERTEX_NORMAL 0x0002
#define	VERTEX_COLOR 0x0004
#define	VERTEX_SPECULAR 0x0008
#define	VERTEX_TANGENT_S 0x0010
#define	VERTEX_TANGENT_T 0x0020
#define	VERTEX_TANGENT_SPACE ( VERTEX_TANGENT_S | VERTEX_TANGENT_T )
#define	VERTEX_WRINKLE 0x0040
#define	VERTEX_BONE_INDEX 0x0080
#define	VERTEX_FORMAT_VERTEX_SHADER 0x0100
#define	VERTEX_FORMAT_USE_EXACT_FORMAT 0x0200
#define	VERTEX_FORMAT_COMPRESSED 0x400
#define	VERTEX_LAST_BIT 10
#define	VERTEX_BONE_WEIGHT_BIT (VERTEX_LAST_BIT + 1)
#define	USER_DATA_SIZE_BIT (VERTEX_LAST_BIT + 4)
#define	TEX_COORD_SIZE_BIT (VERTEX_LAST_BIT + 7)
#define	VERTEX_BONE_WEIGHT_MASK ( 0x7 << VERTEX_BONE_WEIGHT_BIT )
#define	USER_DATA_SIZE_MASK ( 0x7 << USER_DATA_SIZE_BIT )
#define	VERTEX_FORMAT_FIELD_MASK 0x0FF
#define	VERTEX_FORMAT_UNKNOWN 0

#define VERTEX_BONEWEIGHT( _n ) ((_n) << VERTEX_BONE_WEIGHT_BIT)
#define VERTEX_USERDATA_SIZE( _n ) ((_n) << USER_DATA_SIZE_BIT)
#define VERTEX_TEXCOORD_MASK( _coord ) (( 0x7ULL ) << ( TEX_COORD_SIZE_BIT + 3 * (_coord) ))

enum VertexElement_t
{
	VERTEX_ELEMENT_NONE = -1,
	VERTEX_ELEMENT_POSITION = 0,
	VERTEX_ELEMENT_NORMAL = 1,
	VERTEX_ELEMENT_COLOR = 2,
	VERTEX_ELEMENT_SPECULAR = 3,
	VERTEX_ELEMENT_TANGENT_S = 4,
	VERTEX_ELEMENT_TANGENT_T = 5,
	VERTEX_ELEMENT_WRINKLE = 6,
	VERTEX_ELEMENT_BONEINDEX = 7,
	VERTEX_ELEMENT_BONEWEIGHTS1 = 8,
	VERTEX_ELEMENT_BONEWEIGHTS2 = 9,
	VERTEX_ELEMENT_BONEWEIGHTS3 = 10,
	VERTEX_ELEMENT_BONEWEIGHTS4 = 11,
	VERTEX_ELEMENT_USERDATA1 = 12,
	VERTEX_ELEMENT_USERDATA2 = 13,
	VERTEX_ELEMENT_USERDATA3 = 14,
	VERTEX_ELEMENT_USERDATA4 = 15,
	VERTEX_ELEMENT_TEXCOORD1D_0 = 16,
	VERTEX_ELEMENT_TEXCOORD1D_1 = 17,
	VERTEX_ELEMENT_TEXCOORD1D_2 = 18,
	VERTEX_ELEMENT_TEXCOORD1D_3 = 19,
	VERTEX_ELEMENT_TEXCOORD1D_4 = 20,
	VERTEX_ELEMENT_TEXCOORD1D_5 = 21,
	VERTEX_ELEMENT_TEXCOORD1D_6 = 22,
	VERTEX_ELEMENT_TEXCOORD1D_7 = 23,
	VERTEX_ELEMENT_TEXCOORD2D_0 = 24,
	VERTEX_ELEMENT_TEXCOORD2D_1 = 25,
	VERTEX_ELEMENT_TEXCOORD2D_2 = 26,
	VERTEX_ELEMENT_TEXCOORD2D_3 = 27,
	VERTEX_ELEMENT_TEXCOORD2D_4 = 28,
	VERTEX_ELEMENT_TEXCOORD2D_5 = 29,
	VERTEX_ELEMENT_TEXCOORD2D_6 = 30,
	VERTEX_ELEMENT_TEXCOORD2D_7 = 31,
	VERTEX_ELEMENT_TEXCOORD3D_0 = 32,
	VERTEX_ELEMENT_TEXCOORD3D_1 = 33,
	VERTEX_ELEMENT_TEXCOORD3D_2 = 34,
	VERTEX_ELEMENT_TEXCOORD3D_3 = 35,
	VERTEX_ELEMENT_TEXCOORD3D_4 = 36,
	VERTEX_ELEMENT_TEXCOORD3D_5 = 37,
	VERTEX_ELEMENT_TEXCOORD3D_6 = 38,
	VERTEX_ELEMENT_TEXCOORD3D_7 = 39,
	VERTEX_ELEMENT_TEXCOORD4D_0 = 40,
	VERTEX_ELEMENT_TEXCOORD4D_1 = 41,
	VERTEX_ELEMENT_TEXCOORD4D_2 = 42,
	VERTEX_ELEMENT_TEXCOORD4D_3 = 43,
	VERTEX_ELEMENT_TEXCOORD4D_4 = 44,
	VERTEX_ELEMENT_TEXCOORD4D_5 = 45,
	VERTEX_ELEMENT_TEXCOORD4D_6 = 46,
	VERTEX_ELEMENT_TEXCOORD4D_7 = 47,
	VERTEX_ELEMENT_NUMELEMENTS = 48
};

#define COMPRESSED_NORMALS_SEPARATETANGENTS_SHORT2 0
#define COMPRESSED_NORMALS_COMBINEDTANGENTS_UBYTE4 1
#define COMPRESSED_NORMALS_TYPE COMPRESSED_NORMALS_COMBINEDTANGENTS_UBYTE4

enum MaterialVarFlags_t
{
	MATERIAL_VAR_DEBUG = (1 << 0),
	MATERIAL_VAR_NO_DEBUG_OVERRIDE = (1 << 1),
	MATERIAL_VAR_NO_DRAW = (1 << 2),
	MATERIAL_VAR_USE_IN_FILLRATE_MODE = (1 << 3),
	MATERIAL_VAR_VERTEXCOLOR = (1 << 4),
	MATERIAL_VAR_VERTEXALPHA = (1 << 5),
	MATERIAL_VAR_SELFILLUM = (1 << 6),
	MATERIAL_VAR_ADDITIVE = (1 << 7),
	MATERIAL_VAR_ALPHATEST = (1 << 8),
	MATERIAL_VAR_MULTIPASS = (1 << 9),
	MATERIAL_VAR_ZNEARER = (1 << 10),
	MATERIAL_VAR_MODEL = (1 << 11),
	MATERIAL_VAR_FLAT = (1 << 12),
	MATERIAL_VAR_NOCULL = (1 << 13),
	MATERIAL_VAR_NOFOG = (1 << 14),
	MATERIAL_VAR_IGNOREZ = (1 << 15),
	MATERIAL_VAR_DECAL = (1 << 16),
	MATERIAL_VAR_ENVMAPSPHERE = (1 << 17),
	MATERIAL_VAR_NOALPHAMOD = (1 << 18),
	MATERIAL_VAR_ENVMAPCAMERASPACE = (1 << 19),
	MATERIAL_VAR_BASEALPHAENVMAPMASK = (1 << 20),
	MATERIAL_VAR_TRANSLUCENT = (1 << 21),
	MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK = (1 << 22),
	MATERIAL_VAR_NEEDS_SOFTWARE_SKINNING = (1 << 23),
	MATERIAL_VAR_OPAQUETEXTURE = (1 << 24),
	MATERIAL_VAR_ENVMAPMODE = (1 << 25),
	MATERIAL_VAR_SUPPRESS_DECALS = (1 << 26),
	MATERIAL_VAR_HALFLAMBERT = (1 << 27),
	MATERIAL_VAR_WIREFRAME = (1 << 28),
	MATERIAL_VAR_ALLOWALPHATOCOVERAGE = (1 << 29),
	MATERIAL_VAR_IGNORE_ALPHA_MODULATION = (1 << 30)
};

enum MaterialVarFlags2_t
{
	MATERIAL_VAR2_LIGHTING_UNLIT = 0,
	MATERIAL_VAR2_LIGHTING_VERTEX_LIT = (1 << 1),
	MATERIAL_VAR2_LIGHTING_LIGHTMAP = (1 << 2),
	MATERIAL_VAR2_LIGHTING_BUMPED_LIGHTMAP = (1 << 3),
	MATERIAL_VAR2_LIGHTING_MASK = (MATERIAL_VAR2_LIGHTING_VERTEX_LIT | MATERIAL_VAR2_LIGHTING_LIGHTMAP | MATERIAL_VAR2_LIGHTING_BUMPED_LIGHTMAP),
	MATERIAL_VAR2_DIFFUSE_BUMPMAPPED_MODEL = (1 << 4),
	MATERIAL_VAR2_USES_ENV_CUBEMAP = (1 << 5),
	MATERIAL_VAR2_NEEDS_TANGENT_SPACES = (1 << 6),
	MATERIAL_VAR2_NEEDS_SOFTWARE_LIGHTING = (1 << 7),
	MATERIAL_VAR2_BLEND_WITH_LIGHTMAP_ALPHA = (1 << 8),
	MATERIAL_VAR2_NEEDS_BAKED_LIGHTING_SNAPSHOTS = (1 << 9),
	MATERIAL_VAR2_USE_FLASHLIGHT = (1 << 10),
	MATERIAL_VAR2_USE_FIXED_FUNCTION_BAKED_LIGHTING = (1 << 11),
	MATERIAL_VAR2_NEEDS_FIXED_FUNCTION_FLASHLIGHT = (1 << 12),
	MATERIAL_VAR2_USE_EDITOR = (1 << 13),
	MATERIAL_VAR2_NEEDS_POWER_OF_TWO_FRAME_BUFFER_TEXTURE = (1 << 14),
	MATERIAL_VAR2_NEEDS_FULL_FRAME_BUFFER_TEXTURE = (1 << 15),
	MATERIAL_VAR2_IS_SPRITECARD = (1 << 16),
	MATERIAL_VAR2_USES_VERTEXID = (1 << 17),
	MATERIAL_VAR2_SUPPORTS_HW_SKINNING = (1 << 18),
	MATERIAL_VAR2_SUPPORTS_FLASHLIGHT = (1 << 19)
};

enum PreviewImageRetVal_t
{
	MATERIAL_PREVIEW_IMAGE_BAD = 0,
	MATERIAL_PREVIEW_IMAGE_OK,
	MATERIAL_NO_PREVIEW_IMAGE
};

class IMaterial
{
public:
	virtual const char* GetName() const = 0;
	virtual const char* GetTextureGroupName() const = 0;
	virtual PreviewImageRetVal_t GetPreviewImageProperties(int* width, int* height, ImageFormat* imageFormat, bool* isTranslucent) const = 0;
	virtual PreviewImageRetVal_t GetPreviewImage(unsigned char* data, int width, int height, ImageFormat imageFormat) const = 0;
	virtual int GetMappingWidth() = 0;
	virtual int GetMappingHeight() = 0;
	virtual int GetNumAnimationFrames() = 0;
	virtual bool InMaterialPage(void) = 0;
	virtual	void GetMaterialOffset(float* pOffset) = 0;
	virtual void GetMaterialScale(float* pScale) = 0;
	virtual IMaterial* GetMaterialPage(void) = 0;
	virtual IMaterialVar* FindVar(const char* varName, bool* found, bool complain = true) = 0;
	virtual void IncrementReferenceCount(void) = 0;
	virtual void DecrementReferenceCount(void) = 0;
	inline void AddRef() { IncrementReferenceCount(); }
	inline void Release() { DecrementReferenceCount(); }
	virtual int GetEnumerationID(void) const = 0;
	virtual void GetLowResColorSample(float s, float t, float* color) const = 0;
	virtual void RecomputeStateSnapshots() = 0;
	virtual bool IsTranslucent() = 0;
	virtual bool IsAlphaTested() = 0;
	virtual bool IsVertexLit() = 0;
	virtual VertexFormat_t	GetVertexFormat() const = 0;
	virtual bool HasProxy(void) const = 0;
	virtual bool UsesEnvCubemap(void) = 0;
	virtual bool NeedsTangentSpace(void) = 0;
	virtual bool NeedsPowerOfTwoFrameBufferTexture(bool bCheckSpecificToThisFrame = true) = 0;
	virtual bool NeedsFullFrameBufferTexture(bool bCheckSpecificToThisFrame = true) = 0;
	virtual bool NeedsSoftwareSkinning(void) = 0;
	virtual void AlphaModulate(float alpha) = 0;
	virtual void ColorModulate(float r, float g, float b) = 0;
	virtual void SetMaterialVarFlag(MaterialVarFlags_t flag, bool on) = 0;
	virtual bool GetMaterialVarFlag(MaterialVarFlags_t flag) const = 0;
	virtual void GetReflectivity(Vector& reflect) = 0;
	virtual bool GetPropertyFlag(MaterialPropertyTypes_t type) = 0;
	virtual bool IsTwoSided() = 0;
	virtual void SetShader(const char* pShaderName) = 0;
	virtual int GetNumPasses(void) = 0;
	virtual int GetTextureMemoryBytes(void) = 0;
	virtual void Refresh() = 0;
	virtual bool NeedsLightmapBlendAlpha(void) = 0;
	virtual bool NeedsSoftwareLighting(void) = 0;
	virtual int ShaderParamCount() const = 0;
	virtual IMaterialVar** GetShaderParams(void) = 0;
	virtual bool IsErrorMaterial() const = 0;
	virtual void SetUseFixedFunctionBakedLighting(bool bEnable) = 0;
	virtual float GetAlphaModulation() = 0;
	virtual void GetColorModulation(float* r, float* g, float* b) = 0;
	virtual MorphFormat_t GetMorphFormat() const = 0;
	virtual IMaterialVar* FindVarFast(char const* pVarName, unsigned int* pToken) = 0;
	virtual void SetShaderAndParams(KeyValues* pKeyValues) = 0;
	virtual const char* GetShaderName() const = 0;
	virtual void DeleteIfUnreferenced() = 0;
	virtual bool IsSpriteCard() = 0;
	virtual void CallBindProxy(void* proxyData) = 0;
	virtual IMaterial* CheckProxyReplacement(void* proxyData) = 0;
	virtual void RefreshPreservingMaterialVars() = 0;
	virtual bool WasReloadedFromWhitelist() = 0;
	virtual bool IsPrecached() const = 0;
};

inline bool IsErrorMaterial(IMaterial* pMat)
{
	return !pMat || pMat->IsErrorMaterial();
}