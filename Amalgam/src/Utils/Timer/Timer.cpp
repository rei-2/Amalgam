#include "Timer.h"

#include <chrono>

Timer::Timer()
{
	Last = std::chrono::steady_clock::now();
}

bool Timer::Check(unsigned ms) const
{
	const auto currentTime = std::chrono::steady_clock::now();
	return std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - Last).count() >= ms;
}

bool Timer::Run(unsigned ms)
{
	if (Check(ms))
	{
		Update();
		return true;
	}
	return false;
}

inline void Timer::Update()
{
	Last = std::chrono::steady_clock::now();
}