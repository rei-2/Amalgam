#pragma once

template< typename Type, bool IS_ARRAY >
struct CInterpolatedVarEntryBase
{
	float changetime;
	int count;
	Type* value;
};

template<typename T>
class CSimpleRingBuffer
{
public:
	T* m_pElements;
	unsigned short m_maxElement;
	unsigned short m_firstElement;
	unsigned short m_count;
	unsigned short m_growSize;
};

class IInterpolatedVar
{
public:
	virtual ~IInterpolatedVar() {}
	virtual void Setup(void* pValue, int type) = 0;
	virtual void SetInterpolationAmount(float seconds) = 0;
	virtual void NoteLastNetworkedValue() = 0;
	virtual bool NoteChanged(float changetime, bool bUpdateLastNetworkedValue) = 0;
	virtual void Reset() = 0;
	virtual int Interpolate(float currentTime) = 0;
	virtual int GetType() const = 0;
	virtual void RestoreToLastNetworked() = 0;
	virtual void Copy(IInterpolatedVar* pSrc) = 0;
	virtual const char* GetDebugName() = 0;
	virtual void SetDebugName(const char* pName) = 0;
	virtual void SetDebug(bool bDebug) = 0;
};

template<typename Type, bool IS_ARRAY>
class CInterpolatedVarArrayBase
{
protected:
	typedef CInterpolatedVarEntryBase<Type, IS_ARRAY> CInterpolatedVarEntry;
	typedef CSimpleRingBuffer<CInterpolatedVarEntry> CVarHistory;

public:
	IInterpolatedVar* m_vTable;
	Type* m_pValue;
	CVarHistory m_VarHistory;
	Type* m_LastNetworkedValue;
	float m_LastNetworkedTime;
	byte m_fType;
	byte m_nMaxCount;
	byte* m_bLooping;
	float m_InterpolationAmount;
	const char* m_pDebugName;
	bool m_bDebug : 1;
};

template<typename Type, int COUNT>
class CInterpolatedVarArray : public CInterpolatedVarArrayBase<Type, true >
{

};

template<typename Type>
class CInterpolatedVar : public CInterpolatedVarArrayBase< Type, false >
{

};