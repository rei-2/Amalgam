#include "Signatures.h"

#include "../Memory/Memory.h"
#include <string>
#include <format>

CSignature::CSignature(const char* sDLLName, const char* sSignature, int nOffset, const char* sName)
{
	m_pszDLLName = sDLLName;
	m_pszSignature = sSignature;
	m_nOffset = nOffset;
	m_pszName = sName;

	U::Signatures.AddSignature(this);
}

void CSignature::Initialize()
{
	m_dwVal = U::Memory.FindSignature(m_pszDLLName, m_pszSignature);
	if (!m_dwVal)
		OutputDebugStringA(std::format("CSignature::Initialize() failed to initialize:\n  {}\n  {}\n  {}\n", m_pszName, m_pszDLLName, m_pszSignature).c_str());

	m_dwVal += m_nOffset;
}

void CSignatures::Initialize()
{
	for (auto Signature : m_vecSignatures)
	{
		if (!Signature)
			continue;

		Signature->Initialize();
	}
}