#pragma once

#define DBGFLAG_ASSERT

template <class T>
class CRangeValidatedArray
{
public:
	void Attach(int nCount, T* pData);
	void Detach();

	T& operator[](int i);
	const T& operator[](int i) const;

	T* Base();

private:
	T* m_pArray;

#ifdef DBGFLAG_ASSERT
	int m_nCount;
#endif
};

template <class T>
inline T& CRangeValidatedArray<T>::operator[](int i)
{
	//Assert((i >= 0) && (i < m_nCount));
	return m_pArray[i];
}

template <class T>
inline const T& CRangeValidatedArray<T>::operator[](int i) const
{
	//Assert((i >= 0) && (i < m_nCount));
	return m_pArray[i];
}

template <class T>
inline void CRangeValidatedArray<T>::Attach(int nCount, T* pData)
{
	m_pArray = pData;

#ifdef DBGFLAG_ASSERT
	m_nCount = nCount;
#endif
}

template <class T>
inline void CRangeValidatedArray<T>::Detach()
{
	m_pArray = NULL;

#ifdef DBGFLAG_ASSERT
	m_nCount = 0;
#endif
}

template <class T>
inline T* CRangeValidatedArray<T>::Base()
{
	return m_pArray;
}