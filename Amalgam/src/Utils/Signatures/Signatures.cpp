#include "Signatures.h"

#include "../Memory/Memory.h"
#include "../../Core/Core.h"
#include <string>
#include <format>

CSignature::CSignature(const char* sDLLName, const char* sSignature, int nOffset, const char* sName)
{
	m_dwVal = 0x0;
	m_sDLLName = sDLLName;
	m_sSignature = sSignature;
	m_nOffset = nOffset;
	m_sName = sName;

	U::Signatures.AddSignature(this);
}

bool CSignature::Initialize()
{
	m_dwVal = U::Memory.FindSignature(m_sDLLName, m_sSignature);
	if (!m_dwVal)
	{
		U::Core.AppendFailText(std::format("CSignature::Initialize() failed to initialize:\n  {}\n  {}\n  {}", m_sName, m_sDLLName, m_sSignature).c_str());
		return false;
	}

	m_dwVal += m_nOffset;
	return true;
}

bool CSignatures::Initialize()
{
	for (auto pSignature : m_vSignatures)
	{
		if (!pSignature)
			continue;

		if (!pSignature->Initialize())
			m_bFailed = true;
	}

	return !m_bFailed;
}