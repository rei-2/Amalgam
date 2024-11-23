#include "../SDK/SDK.h"

MAKE_SIGNATURE(DataTable_Warning, "engine.dll", "48 89 4C 24 ? 48 89 54 24 ? 4C 89 44 24 ? 4C 89 4C 24 ? B8 ? ? ? ? E8 ? ? ? ? 48 2B E0 4C 8B C1 4C 8D 8C 24 ? ? ? ? 48 8D 4C 24 ? 8D 50", 0x0);

MAKE_HOOK(DataTable_Warning, S::DataTable_Warning(), void,
	const char* pInMessage, ...)
{
	// dont send datatable warnings
}