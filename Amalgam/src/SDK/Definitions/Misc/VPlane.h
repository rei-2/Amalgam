#pragma once
#include "BaseTypes.h"
#include "../Types.h"

typedef int SideType;

#define	SIDE_FRONT 0
#define	SIDE_BACK 1
#define	SIDE_ON 2

#define VP_EPSILON 0.01f

class VPlane
{
public:
	Vector m_Normal;
	vec_t m_Dist;
};