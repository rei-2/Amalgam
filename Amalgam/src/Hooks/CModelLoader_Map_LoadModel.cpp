#include "../SDK/SDK.h"

#include "../Features/World/World.h"

MAKE_SIGNATURE(CModelLoader_Map_LoadModel, "engine.dll", "48 8B C4 48 89 58 ? 48 89 50 ? 48 89 48 ? 55 56 57 41 54 41 55 41 56 41 57 48 8D A8 ? ? ? ? 48 81 EC ? ? ? ? FF 05", 0x0);
MAKE_SIGNATURE(R_LevelInit, "engine.dll", "48 83 EC ? 48 8D 0D ? ? ? ? FF 15 ? ? ? ? 48 8D 0D ? ? ? ? FF 15", 0x0);
MAKE_SIGNATURE(CM_FreeMap, "engine.dll", "48 8D 0D ? ? ? ? E9 ? ? ? ? CC CC CC CC 48 89 5C 24 ? 57 48 83 EC ? 45 33 C9", 0x0);

MAKE_HOOK(CModelLoader_Map_LoadModel, S::CModelLoader_Map_LoadModel(), void,
	void* rcx, model_t* mod)
{
	DEBUG_RETURN(CModelLoader_Map_LoadModel, rcx, mod);

	CALL_ORIGINAL(rcx, mod);

	F::World.CacheBoxBrushes();
	F::World.CachePlaneBrushes();
	F::World.CacheEntities();
}

MAKE_HOOK(R_LevelInit, S::R_LevelInit(), void,
	)
{
	DEBUG_RETURN(R_LevelInit);

	CALL_ORIGINAL();

	F::World.CacheProps();
}

MAKE_HOOK(CM_FreeMap, S::CM_FreeMap(), void,
	)
{
	DEBUG_RETURN(CM_FreeMap);

	CALL_ORIGINAL();

	F::World.Uncache();
}