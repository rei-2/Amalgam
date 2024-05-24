#pragma once
#include "../Feature/Feature.h"
#include <Windows.h>

namespace Minidump
{
	LONG WINAPI ExceptionFilter(PEXCEPTION_POINTERS exPtr);
}