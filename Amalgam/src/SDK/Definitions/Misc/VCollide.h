#pragma once

class CPhysCollide;

struct vcollide_t
{
	unsigned short solidCount : 15;
	unsigned short isPacked : 1;
	unsigned short descSize;
	CPhysCollide** solids;
	char* pKeyValues;
};