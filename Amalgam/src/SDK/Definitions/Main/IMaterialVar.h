#pragma once
#include "../Misc/BaseTypes.h"
#include "../Types.h"

class IMaterial;
class VMatrix;
class ITexture;

#define MAKE_MATERIALVAR_FOURCC(ch0, ch1, ch2, ch3) ((unsigned long)(ch0) | ((unsigned long)(ch1) << 8) | ((unsigned long)(ch2) << 16) | ((unsigned long)(ch3) << 24 ))
#define FOURCC_UNKNOWN	MAKE_MATERIALVAR_FOURCC('U','N','K','N')

enum MaterialVarType_t
{
	MATERIAL_VAR_TYPE_FLOAT = 0,
	MATERIAL_VAR_TYPE_STRING,
	MATERIAL_VAR_TYPE_VECTOR,
	MATERIAL_VAR_TYPE_TEXTURE,
	MATERIAL_VAR_TYPE_INT,
	MATERIAL_VAR_TYPE_FOURCC,
	MATERIAL_VAR_TYPE_UNDEFINED,
	MATERIAL_VAR_TYPE_MATRIX,
	MATERIAL_VAR_TYPE_MATERIAL
};

typedef unsigned short MaterialVarSym_t;
typedef unsigned long FourCC;

class IMaterialVar
{
private:
	char* m_pStringVal;
	int m_intVal;
	Vector4D m_VecVal;
	uint8 m_Type : 4;
	uint8 m_nNumVectorComps : 3;
	uint8 m_bFakeMaterialVar : 1;
	uint8 m_nTempIndex;
	//CUtlSymbol m_Name;

public:
	virtual ITexture* GetTextureValue(void) = 0;
	virtual char const* GetName(void) const = 0;
	virtual MaterialVarSym_t GetNameAsSymbol() const = 0;
	virtual void SetFloatValue(float val) = 0;
	virtual void SetIntValue(int val) = 0;
	virtual void SetStringValue(char const* val) = 0;
	virtual char const* GetStringValue(void) const = 0;
	virtual void SetFourCCValue(FourCC type, void* pData) = 0;
	virtual void GetFourCCValue(FourCC* type, void** ppData) = 0;
	virtual void SetVecValue(float const* val, int numcomps) = 0;
	virtual void SetVecValue(float x, float y) = 0;
	virtual void SetVecValue(float x, float y, float z) = 0;
	virtual void SetVecValue(float x, float y, float z, float w) = 0;
	virtual void GetLinearVecValue(float* val, int numcomps) const = 0;
	virtual void SetTextureValue(ITexture*) = 0;
	virtual IMaterial* GetMaterialValue(void) = 0;
	virtual void SetMaterialValue(IMaterial*) = 0;
	virtual bool IsDefined() const = 0;
	virtual void SetUndefined() = 0;
	virtual void SetMatrixValue(VMatrix const& matrix) = 0;
	virtual const VMatrix& GetMatrixValue() = 0;
	virtual bool MatrixIsIdentity() const = 0;
	virtual void CopyFrom(IMaterialVar* pMaterialVar) = 0;
	virtual void SetValueAutodetectType(char const* val) = 0;
	virtual IMaterial* GetOwningMaterial() = 0;
	virtual void SetVecComponentValue(float fVal, int nComponent) = 0;
	virtual int GetIntValueInternal(void) const = 0;
	virtual float GetFloatValueInternal(void) const = 0;
	virtual float const* GetVecValueInternal() const = 0;
	virtual void GetVecValueInternal(float* val, int numcomps) const = 0;
	virtual int VectorSizeInternal() const = 0;
};