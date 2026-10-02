#include "Memory.h"

#include <format>
#include <Psapi.h>

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
	int iMatches = 0;
	return FindSignature(szModule, szPattern, iMatches);
}

uintptr_t CMemory::FindSignature(const char* szModule, const char* szPattern, int& iMatches)
{
	iMatches = 0;

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

		uintptr_t uResult = 0x0;

		// Now loop through all bytes and check if the byte sequence matches
		for (auto i = 0ul; i < dwImageSize - iPatternSize; ++i)
		{
			auto bFound = true;

			// Go through each byte of the signature and check if it matches
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
			{
				++iMatches;
				if (!uResult)
					uResult = uintptr_t(&pImageBytes[i]);
			}
		}

		return uResult;
	}

	return 0x0;
}

uintptr_t CMemory::FindUniqueSignature(const char* szModule, const char* szPattern)
{
	int iMatches = 0;
	const auto uResult = FindSignature(szModule, szPattern, iMatches);
	return iMatches == 1 ? uResult : 0x0;
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

uintptr_t CMemory::GetOffsetFromBase(uintptr_t uAddress)
{
	HMODULE hModule;
	if (!GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, LPCSTR(uAddress), &hModule))
		return -1;

	uintptr_t uBase = uintptr_t(hModule);
	return uAddress - uBase;
}