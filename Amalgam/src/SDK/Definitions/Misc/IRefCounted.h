#pragma once

class IRefCounted
{
public:
	virtual int AddRef() = 0;
	virtual int Release() = 0;
};