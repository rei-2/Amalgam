#include "../SDK/SDK.h"

MAKE_SIGNATURE(CPredictionCopy_CopyFields, "client.dll", "4C 89 4C 24 ? 4C 89 44 24 ? 89 54 24", 0x0);

enum
{
	PC_EVERYTHING = 0,
	PC_NON_NETWORKED_ONLY,
	PC_NETWORKED_ONLY,
};

#define LIST_FIELD(t1, t2) \
case FIELD_##t2: \
{ \
	auto pPredicted = reinterpret_cast<t1*>(pOutputData); \
	auto pNetwork = reinterpret_cast<t1*>(pInputData); \
	for (int i = 0; i < fieldSize; i++) \
	{ \
		auto& tPredicted = pPredicted[i]; \
		auto& tNetwork = pNetwork[i]; \
		if (!I::EngineVGui->IsGameUIVisible()) \
		{ \
			SDK::Output(std::format("FIELD_"#t2"_{}", i).c_str(), std::format("{}: {}, {}" /*"{}: {:#x}, {:#x}; {}, {}"*/, \
				pCurrentField->fieldName ? pCurrentField->fieldName : "(null)", \
				tPredicted, tNetwork, \
				uintptr_t(pOutputData), uintptr_t(pInputData) \
			).c_str(), Color_t(bLocal ? 255 : 0, 0, 255)); \
		} \
	} \
	break; \
}

MAKE_HOOK(CPredictionCopy_CopyFields, S::CPredictionCopy_CopyFields(), void,
	void* rcx, int chain_count, datamap_t* pRootMap, typedescription_t* pFields, int fieldCount)
{
	if (auto pLocal = H::Entities.GetLocal())
	{
		datamap_t* pDescMap = U::Memory.CallVirtual<15, datamap_t*>(pLocal);
		while (pDescMap && pDescMap->baseMap && pDescMap != pRootMap)
			pDescMap = pDescMap->baseMap;
		bool bLocal = pDescMap == pRootMap;

		//if (bLocal)
		{
			for (int i = 0; i < fieldCount; i++)
			{
				auto pCurrentField = &pFields[i];
				int flags = pCurrentField->flags;

				if (pCurrentField->override_count == chain_count)
					continue;

				if (pCurrentField->fieldType != FIELD_EMBEDDED)
				{
					if (flags & FTYPEDESC_PRIVATE)
						continue;

					int nType = *reinterpret_cast<int*>(rcx);
					if (nType == PC_NON_NETWORKED_ONLY && (flags & FTYPEDESC_INSENDTABLE)
						|| nType == PC_NETWORKED_ONLY && !(flags & FTYPEDESC_INSENDTABLE))
						continue;
				}

				void* pDest = *reinterpret_cast<void**>(uintptr_t(rcx) + 8);
				void* pSrc = *reinterpret_cast<void**>(uintptr_t(rcx) + 16);
				int nDestOffsetIndex = *reinterpret_cast<int*>(uintptr_t(rcx) + 24);
				int nSrcOffsetIndex = *reinterpret_cast<int*>(uintptr_t(rcx) + 28);

				int fieldOffsetDest = pCurrentField->fieldOffset[nDestOffsetIndex];
				int fieldOffsetSrc = pCurrentField->fieldOffset[nSrcOffsetIndex];
				int fieldSize = pCurrentField->fieldSize;

				void* pOutputData = pOutputData = (void*)((char*)pDest + fieldOffsetDest);
				void* pInputData = pInputData = (void*)((char*)pSrc + fieldOffsetSrc);

				switch (pCurrentField->fieldType)
				{
				LIST_FIELD(int, INTEGER);
				LIST_FIELD(float, FLOAT);
				}
			}
		}
	}

	CALL_ORIGINAL(rcx, chain_count, pRootMap, pFields, fieldCount);
}