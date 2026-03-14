#pragma once
#include "CUtlIntrusiveList.h"

#ifndef UTLOBJECTREFERENCE_H
#define UTLOBJECTREFERENCE_H

template<class T> class CUtlReference
{
public:
	FORCEINLINE CUtlReference(void)
	{
		m_pNext = m_pPrev = NULL;
		m_pObject = NULL;
	}

	FORCEINLINE CUtlReference(T* pObj)
	{
		m_pNext = m_pPrev = NULL;
		AddRef(pObj);
	}

	FORCEINLINE ~CUtlReference(void)
	{
		KillRef();
	}

	FORCEINLINE void Set(T* pObj)
	{
		if (m_pObject != pObj)
		{
			KillRef();
			AddRef(pObj);
		}
	}

	FORCEINLINE T* operator()(void) const
	{
		return m_pObject;
	}

	FORCEINLINE operator T* ()
	{
		return m_pObject;
	}

	FORCEINLINE operator const T* () const
	{
		return m_pObject;
	}

	FORCEINLINE T* operator->()
	{
		return m_pObject;
	}

	FORCEINLINE const T* operator->() const
	{
		return m_pObject;
	}

	FORCEINLINE CUtlReference& operator=(const CUtlReference& otherRef)
	{
		Set(otherRef.m_pObject);
		return *this;
	}

	FORCEINLINE CUtlReference& operator=(T* pObj)
	{
		Set(pObj);
		return *this;
	}


	FORCEINLINE bool operator==(const CUtlReference& o) const
	{
		return (o.m_pObject == m_pObject);
	}

public:
	CUtlReference* m_pNext;
	CUtlReference* m_pPrev;

	T* m_pObject;

	FORCEINLINE void AddRef(T* pObj)
	{
		m_pObject = pObj;
		if (pObj)
		{
			pObj->m_References.AddToHead(this);
		}
	}

	FORCEINLINE void KillRef(void)
	{
		if (m_pObject)
		{
			m_pObject->m_References.RemoveNode(this);
			m_pObject = NULL;
		}
	}

};

template<class T> class CUtlReferenceList : public CUtlIntrusiveDList< CUtlReference<T> >
{
public:
	~CUtlReferenceList(void)
	{
		CUtlReference<T>* i = CUtlIntrusiveDList<CUtlReference<T> >::m_pHead;
		while (i)
		{
			CUtlReference<T>* n = i->m_pNext;
			i->m_pNext = NULL;
			i->m_pPrev = NULL;
			i->m_pObject = NULL;
			i = n;
		}
		CUtlIntrusiveDList<CUtlReference<T> >::m_pHead = NULL;
	}
};


//-----------------------------------------------------------------------------
// Put this macro in classes that are referenced by CUtlReference
//-----------------------------------------------------------------------------
#define DECLARE_REFERENCED_CLASS( _className )				\
	private:												\
		CUtlReferenceList< _className > m_References;		\
		template<class T> friend class CUtlReference;

#endif