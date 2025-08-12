#include "Memory.h"

#include <format>
#include <Psapi.h>
#include <Zydis/Zydis.h>

#define INRANGE(x, a, b) (x >= a && x <= b) 
#define GetBits(x) (INRANGE((x & (~0x20)),'A','F') ? ((x & (~0x20)) - 'A' + 0xA) : (INRANGE(x,'0','9') ? x - '0' : 0))
#define GetBytes(x) (GetBits(x[0]) << 4 | GetBits(x[1]))

std::vector<byte> CMemory::PatternToByte(const char* szPattern)
{
	std::vector<byte> vPattern = {};

	const auto pStart = const_cast<char*>(szPattern);
	const auto pEnd = const_cast<char*>(szPattern) + strlen(szPattern);
	for (char* pCurrent = pStart; pCurrent < pEnd; ++pCurrent)
		vPattern.push_back(byte(std::strtoul(pCurrent, &pCurrent, 16)));

	return vPattern;
}

std::vector<int> CMemory::PatternToInt(const char* szPattern)
{
	std::vector<int> vPattern = {};

	const auto pStart = const_cast<char*>(szPattern);
	const auto pEnd = const_cast<char*>(szPattern) + strlen(szPattern);
	for (char* pCurrent = pStart; pCurrent < pEnd; ++pCurrent)
	{
		if (*pCurrent == '?') // Is current byte a wildcard? Simply ignore that that byte later
		{
			++pCurrent;
			if (*pCurrent == '?') // Check if following byte is also a wildcard
				++pCurrent;

			vPattern.push_back(-1);
		}
		else
			vPattern.push_back(std::strtoul(pCurrent, &pCurrent, 16));
	}

	return vPattern;
}

uintptr_t CMemory::FindSignature(const char* szModule, const char* szPattern)
{
	if (const auto hModule = GetModuleHandle(szModule))
	{
		// Get module information to search in the given module
		MODULEINFO lpModuleInfo;
		if (!GetModuleInformation(GetCurrentProcess(), hModule, &lpModuleInfo, sizeof(MODULEINFO)))
			return 0x0;

		// The region where we will search for the byte sequence
		const auto dwImageSize = lpModuleInfo.SizeOfImage;

		// Check if the image is faulty
		if (!dwImageSize)
			return 0x0;

		// Convert IDA-Style signature to a byte sequence
		const auto vPattern = PatternToInt(szPattern);
		const auto iPatternSize = vPattern.size();
		const int* iPatternBytes = vPattern.data();

		const auto pImageBytes = reinterpret_cast<byte*>(hModule);

		// Now loop through all bytes and check if the byte sequence matches
		for (auto i = 0ul; i < dwImageSize - iPatternSize; ++i)
		{
			auto bFound = true;

			// Go through all bytes from the signature and check if it matches
			for (auto j = 0ul; j < iPatternSize; ++j)
			{
				if (pImageBytes[i + j] != iPatternBytes[j] // Bytes don't match
					&& iPatternBytes[j] != -1)             // Byte isn't a wildcard either
				{
					bFound = false;
					break;
				}
			}

			if (bFound)
				return uintptr_t(&pImageBytes[i]);
		}

		return 0x0;
	}

	return 0x0;
}

std::string CMemory::GetModuleName(uintptr_t uAddress)
{
	HMODULE hModule;
	char cModuleName[MAX_PATH];

	if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
		GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
		(LPCSTR)uAddress, &hModule))
	{
		GetModuleBaseNameA(GetCurrentProcess(), hModule, cModuleName, MAX_PATH);
		return cModuleName;
	}

	return "Unknown";
}

uintptr_t CMemory::FindSignatureAtAddress(uintptr_t uAddress, const char* szPattern, uintptr_t uSkipAddress, bool* bRetFound)
{
	if (const auto hMod = GetModuleHandleA(GetModuleName(uAddress).c_str()))
	{
		// Get module information to search in the given module
		MODULEINFO lpModuleInfo;
		if (!GetModuleInformation(GetCurrentProcess(), hMod, &lpModuleInfo, sizeof(MODULEINFO)))
			return 0x0;

		// The region where we will search for the byte sequence
		auto dwImageSize = lpModuleInfo.SizeOfImage;

		// Check if the image is faulty
		if (!dwImageSize)
			return 0x0;

		DWORD dwSubtract = uAddress - reinterpret_cast<uintptr_t>(hMod);
		dwImageSize -= dwSubtract;

		// Convert IDA-Style signature to a byte sequence
		const auto vPattern = PatternToInt(szPattern);
		const auto iPatternSize = vPattern.size();
		const int* iPatternBytes = vPattern.data();

		const auto pImageBytes = reinterpret_cast<byte*>(uAddress);
		// Now loop through all bytes and check if the byte sequence matches

		for (auto i = 0ul; i < dwImageSize - iPatternSize; ++i)
		{
			auto bFound = true;

			// Go through all bytes from the signature and check if it matches
			for (auto j = 0ul; j < iPatternSize; ++j)
			{
				if (pImageBytes[i + j] != iPatternBytes[j] // Bytes don't match
					&& iPatternBytes[j] != -1)             // Byte isn't a wildcard either
				{
					bFound = false;
					break;
				}
			}

			if (uSkipAddress && uSkipAddress == uintptr_t(&pImageBytes[i]))
			{
				if (bRetFound)
					*bRetFound = bFound;
				continue;
			}

			if (bFound)
				return uintptr_t(&pImageBytes[i]);
		}

		return 0x0;
	}

	return 0x0;
}

using CreateInterfaceFn = void*(*)(const char* pName, int* pReturnCode);

PVOID CMemory::FindInterface(const char* szModule, const char* szObject)
{
	const auto CreateInterface = GetModuleExport<CreateInterfaceFn>(szModule, "CreateInterface");
	return CreateInterface(szObject, nullptr);
}

std::string CMemory::GetModuleOffset(uintptr_t uAddress)
{
	HMODULE hModule;
	if (!GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, LPCSTR(uAddress), &hModule))
		return std::format("{:#x}", uAddress);

	uintptr_t uBase = uintptr_t(hModule);
	if (char buffer[MAX_PATH]; GetModuleBaseName(GetCurrentProcess(), hModule, buffer, sizeof(buffer) / sizeof(char)))
		return std::format("{}+{:#x}", buffer, uAddress - uBase);

	return std::format("{:#x}+{:#x}", uBase, uAddress - uBase);
}

// AI failed me so i had to learn this shit by myself
// Some parts were taken from IDA-Fusion plugin
std::string CMemory::GenerateSignatureAtAddress(uintptr_t uAddress, size_t maxLength)
{
	std::vector<std::pair<byte, bool>> vBytes;
	std::string sPattern;
	std::string sModule = GetModuleName(uAddress);

	uintptr_t uMinAddr, uMaxAddr;
	if (const auto hMod = GetModuleHandleA(sModule.c_str()))
	{
		uMinAddr = (uintptr_t)hMod;

		MODULEINFO lpModuleInfo;
		if (GetModuleInformation(GetCurrentProcess(), hMod, &lpModuleInfo, sizeof(MODULEINFO)) && lpModuleInfo.SizeOfImage)
			uMaxAddr = uMinAddr + lpModuleInfo.SizeOfImage;
	}

	if (!uMaxAddr)
		return {};

	ZydisDecoder tDecoder;
	if (ZYAN_FAILED(ZydisDecoderInit(&tDecoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_STACK_WIDTH_64)))
		return {};

	uintptr_t uLastFound = uMinAddr;
	uintptr_t uCurrentAddr = uAddress;
	
	while (uCurrentAddr - uAddress < maxLength && uCurrentAddr < uMaxAddr)
	{
		ZydisDecodedInstruction tInstruction;
		if (ZYAN_FAILED(ZydisDecoderDecodeInstruction(&tDecoder, nullptr, (void*)(uCurrentAddr), 15,
			&tInstruction)))
			break;

		// Works
		byte uOffset = tInstruction.raw.disp.offset + tInstruction.raw.imm->offset;
		for (int i = 0; i < tInstruction.length; i++)
			vBytes.push_back({ *(byte*)(uCurrentAddr + i), uOffset && i >= uOffset });

		// Attempt to search for this signature, if nothing is found then we have a unique signature
		{
			std::string sTempPattern;
			sPattern.clear();

			for (auto [uByte, bWildcard] : vBytes)
			{
				if (bWildcard)
					sTempPattern.append("? ");
				else
					sTempPattern.append(std::format("{:02X} ", uByte));
			}
			sTempPattern.pop_back();

			// Try to find it at original address
			bool bFoundAtOriginal = false;
			FindSignatureAtAddress(uAddress, sTempPattern.c_str(), uAddress, &bFoundAtOriginal);

			uintptr_t uFound = FindSignatureAtAddress(uLastFound, sTempPattern.c_str(), uAddress);
			if (bFoundAtOriginal && !uFound)
			{
				sPattern = sTempPattern;
				break;
			}
			// Failsafe in case we can no longer find it at the original address
			else if (!bFoundAtOriginal)
				break;

			sPattern = std::format("Not a unique signature ({})", sTempPattern);

			// Update the last found address so we dont have to scan that region anymore
			uLastFound = uFound;
		}

		uCurrentAddr += tInstruction.length;
	}

	return sPattern;
}