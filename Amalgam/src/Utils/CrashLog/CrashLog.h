#pragma once
#include <Windows.h>

namespace CrashLog
{
	LONG APIENTRY ExceptionFilter(PEXCEPTION_POINTERS Info);
}