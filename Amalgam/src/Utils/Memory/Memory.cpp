#include "Memory.h"
#include <format>
#include <Psapi.h>

#define INRANGE(x, a, b) (x >= a && x <= b) 
#define GetBits(x) (INRANGE((x & (~0x20)),'A','F') ? ((x & (~0x20)) - 'A' + 0xA) : (INRANGE(x,'0','9') ? x - '0' : 0))
#define GetBytes(x) (GetBits(x[0]) << 4 | GetBits(x[1]))

typedef void* (*InstantiateInterfaceFn)();

struct InterfaceInit_t
{
	InstantiateInterfaceFn m_pInterface = nullptr;
	const char* m_pszInterfaceName = nullptr;
	InterfaceInit_t* m_pNextInterface = nullptr;
};

std::vector<byte> CMemory::PatternToByte(const char* szPattern)
{
	std::vector<byte> bytes = {};
	const auto start = const_cast<char*>(szPattern);
	const auto end = const_cast<char*>(szPattern) + strlen(szPattern);

	for (char* current = start; current < end; ++current)
		bytes.push_back(byte(std::strtoul(current, &current, 16)));

	return bytes;
}

std::vector<int> CMemory::PatternToInt(const char* szPattern)
{
	std::vector<int> bytes = {};
	const auto start = const_cast<char*>(szPattern);
	const auto end = const_cast<char*>(szPattern) + strlen(szPattern);

	for (char* current = start; current < end; ++current)
	{
		if (*current == '?') // Is current byte a wildcard? Simply ignore that that byte later
		{
			++current;
			if (*current == '?') // Check if following byte is also a wildcard
				++current;

			bytes.push_back(-1);
		}
		else
			bytes.push_back(std::strtoul(current, &current, 16));
	}

	return bytes;
}

uintptr_t CMemory::FindSignature(const char* szModule, const char* szPattern)
{
	if (const auto hMod = GetModuleHandleA(szModule))
	{
		// Get module information to search in the given module
		MODULEINFO module_info;
		if (!GetModuleInformation(GetCurrentProcess(), hMod, &module_info, sizeof(MODULEINFO)))
			return {};

		// The region where we will search for the byte sequence
		const auto image_size = module_info.SizeOfImage;

		// Check if the image is faulty
		if (!image_size)
			return {};

		// Convert IDA-Style signature to a byte sequence
		const auto pattern_bytes = PatternToInt(szPattern);
		const auto signature_size = pattern_bytes.size();
		const int* signature_bytes = pattern_bytes.data();

		const auto image_bytes = reinterpret_cast<byte*>(hMod);

		// Now loop through all bytes and check if the byte sequence matches
		for (auto i = 0ul; i < image_size - signature_size; ++i)
		{
			auto byte_sequence_found = true;

			// Go through all bytes from the signature and check if it matches
			for (auto j = 0ul; j < signature_size; ++j)
			{
				if (image_bytes[i + j] != signature_bytes[j] // Bytes don't match
					&& signature_bytes[j] != -1)             // Byte isn't a wildcard either
				{
					byte_sequence_found = false;
					break;
				}
			}

			if (byte_sequence_found)
				return { reinterpret_cast<uintptr_t>(&image_bytes[i]) };
		}

		return {};
	}

	return 0x0;
}

using CreateInterfaceFn = void* (*)(const char* pName, int* pReturnCode);

PVOID CMemory::FindInterface(const char* szModule, const char* szObject)
{
	const auto hModule = GetModuleHandleA(szModule);
	if (!hModule)
		return nullptr;

	const auto createFn = reinterpret_cast<CreateInterfaceFn>(GetProcAddress(hModule, "CreateInterface"));
	if (!createFn)
		return nullptr;

	return createFn(szObject, nullptr);
}