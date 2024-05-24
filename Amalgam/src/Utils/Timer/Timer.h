#pragma once
#include <chrono>

/*
 *	Credits to cathook (nullifiedcat)
 */

class Timer
{
private:
	std::chrono::steady_clock::time_point Last;

public:
	Timer();
	bool Check(unsigned ms) const;
	bool Run(unsigned ms);
	inline void Update();
};