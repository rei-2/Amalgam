#include "BytePatches.h"

BytePatch::BytePatch(const char* sModule, const char* sAddress, int iOffset, const char* sPatch)
{
	m_sModule = sModule;
	m_sAddress = sAddress;
	m_iOffset = iOffset;

	auto vPatch = U::Memory.PatternToByte(sPatch);
	m_vPatch = vPatch;
	m_iSize = vPatch.size();
	m_vOriginal.resize(m_iSize);
}

void BytePatch::Write(std::vector<byte>& bytes)
{
	DWORD flNewProtect, flOldProtect;
	VirtualProtect(m_pAddress, m_iSize, PAGE_EXECUTE_READWRITE, &flNewProtect);
	memcpy(m_pAddress, bytes.data(), m_iSize);
	VirtualProtect(m_pAddress, m_iSize, flNewProtect, &flOldProtect);
}

void BytePatch::Initialize()
{
	if (m_bIsPatched)
		return;

	m_pAddress = LPVOID(U::Memory.FindSignature(m_sModule, m_sAddress));
	if (!m_pAddress)
		return;

	m_pAddress = LPVOID(uintptr_t(m_pAddress) + m_iOffset);

	DWORD flNewProtect, flOldProtect;
	VirtualProtect(m_pAddress, m_iSize, PAGE_EXECUTE_READWRITE, &flNewProtect);
	memcpy(m_vOriginal.data(), m_pAddress, m_iSize);
	VirtualProtect(m_pAddress, m_iSize, flNewProtect, &flOldProtect);

	Write(m_vPatch);
	m_bIsPatched = true;
}

void BytePatch::Unload()
{
	if (!m_bIsPatched)
		return;

	Write(m_vOriginal);
	m_bIsPatched = false;
}



void CBytePatches::Initialize()
{
	m_vPatches = {
		BytePatch("engine.dll", "0F 82 ? ? ? ? 4A 63 84 2F", 0x0, "90 90 90 90 90 90") // skybox fix
	};

	for (auto& patch : m_vPatches)
		patch.Initialize();
}

void CBytePatches::Unload()
{
	for (auto& patch : m_vPatches)
		patch.Unload();
}