#pragma once
#include <cstdint>
#include <cstddef>
#include <string.h>

using FNV1A_t = std::uint32_t;

namespace FNV1A
{
	inline constexpr std::uint32_t ullBasis = 0x811C9DC5;
	inline constexpr std::uint32_t ullPrime = 0x1000193;

	// compile-time hashes
	constexpr FNV1A_t HashConst(const char* szString, const FNV1A_t uValue = ullBasis) noexcept
	{
		return (szString[0] == '\0') ? uValue : HashConst(&szString[1], (uValue ^ FNV1A_t(szString[0])) * ullPrime);
	}

	// runtime hashes
	inline FNV1A_t Hash(const char* szString)
	{
		FNV1A_t uHashed = ullBasis;

		for (std::size_t i = 0U; i < strlen(szString); ++i)
		{
			uHashed ^= szString[i];
			uHashed *= ullPrime;
		}

		return uHashed;
	}
}