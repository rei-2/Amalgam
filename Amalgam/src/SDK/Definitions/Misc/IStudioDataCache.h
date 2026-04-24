#pragma once
#include "IAppSystem.h"

struct studiohdr_t;
struct vertexFileHeader_t;

class IStudioDataCache : public IAppSystem
{
public:
	virtual bool VerifyHeaders(studiohdr_t* pStudioHdr) = 0;
	virtual vertexFileHeader_t* CacheVertexData(studiohdr_t* pStudioHdr) = 0;
};