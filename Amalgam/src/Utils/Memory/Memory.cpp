#include "Memory.h"
#include <format>
#include <Psapi.h>

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
	if (const auto hMod = GetModuleHandleA(szModule))
	{
		// Get module information to search in the given module
		MODULEINFO lpModuleInfo;
		if (!GetModuleInformation(GetCurrentProcess(), hMod, &lpModuleInfo, sizeof(MODULEINFO)))
			return {};

		// The region where we will search for the byte sequence
		const auto dwImageSize = lpModuleInfo.SizeOfImage;

		// Check if the image is faulty
		if (!dwImageSize)
			return {};

		// Convert IDA-Style signature to a byte sequence
		const auto vPattern = PatternToInt(szPattern);
		const auto iPatternSize = vPattern.size();
		const int* iPatternBytes = vPattern.data();

		const auto pImageBytes = reinterpret_cast<byte*>(hMod);

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

using CreateInterfaceFn = void*(*)(const char* pName, int* pReturnCode);

PVOID CMemory::FindInterface(const char* szModule, const char* szObject)
{
	const auto hModule = GetModuleHandleA(szModule);
	if (!hModule)
		return nullptr;

	const auto fnCreateInterface = reinterpret_cast<CreateInterfaceFn>(GetProcAddress(hModule, "CreateInterface"));
	if (!fnCreateInterface)
		return nullptr;

	return fnCreateInterface(szObject, nullptr);
}