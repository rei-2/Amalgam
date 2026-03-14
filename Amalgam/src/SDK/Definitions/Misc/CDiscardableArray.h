#pragma once
#include "CUtlBuffer.h"
#include "../Interfaces/IFileSystem.h"

//-----------------------------------------------------------------------------
// A dumpable version of RangeValidatedArray
//-----------------------------------------------------------------------------
template <class T>
class CDiscardableArray
{
public:
	CDiscardableArray()
	{
		m_nCount = 0;
		m_nOffset = -1;
		memset(m_pFilename, 0, sizeof(m_pFilename));
	}

	~CDiscardableArray()
	{
	}

	void Init(char* pFilename, int nOffset, int nCount, void* pData = NULL)
	{
		if (m_buf.TellPut())
		{
			m_buf.Purge();
		}

		m_nCount = nCount;
		strcpy_s(m_pFilename, pFilename);
		m_nOffset = nOffset;

		// can preload as required
		if (pData)
		{
			m_buf.Put(pData, nCount);
		}
	}

	// Either get or load the array as needed:
	T* Get()
	{
		if (!m_buf.TellPut())
		{
			if (!I::FileSystem->ReadFile(m_pFilename, NULL, m_buf, sizeof(T) * m_nCount, m_nOffset))
			{
				return NULL;
			}
		}

		return (T*)m_buf.PeekGet();
	}

	void Discard()
	{
		// TODO(johns): Neutered -- Tear this class out. This can no longer be discarded with transparently compressed
		//              BSPs. This is on the range of 500k of memory, and is probably overly complex for the savings in
		//              the current era.
		// m_buf.Purge();
	}

protected:
	int			m_nCount;
	CUtlBuffer	m_buf;
	char		m_pFilename[256];
	int			m_nOffset;
};