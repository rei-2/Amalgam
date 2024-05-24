#pragma once
#include "CommonMacros.h"

template< class T >
T Clamp(T const& val, T const& minVal, T const& maxVal)
{
	if (val < minVal)
		return minVal;
	else if (val > maxVal)
		return maxVal;
	else
		return val;
}

template< class T >
T Min(T const& val1, T const& val2)
{
	return val1 < val2 ? val1 : val2;
}

template< class T >
T Max(T const& val1, T const& val2)
{
	return val1 > val2 ? val1 : val2;
}

typedef int qboolean;
typedef unsigned long ULONG;
typedef unsigned char BYTE;
typedef unsigned char byte;
typedef unsigned short word;
typedef wchar_t ucs2;

enum ThreeState_t
{
	TRS_FALSE,
	TRS_TRUE,
	TRS_NONE
};

typedef float vec_t;

struct color24
{
	byte r, g, b;
};

typedef struct color32_s
{
	byte r, g, b, a;
} color32;

struct colorVec
{
	unsigned r, g, b, a;
};

struct vrect_t
{
	int x, y, width, height;
	vrect_t* pnext;
};

struct Rect_t
{
	int x, y;
	int width, height;
};

struct wrect_t
{
	int	left;
	int right;
	int top;
	int bottom;
};

struct interval_t
{
	float start;
	float range;
};

using uint32 = unsigned int;
typedef unsigned char uint8;
typedef signed char int8;
typedef __int16 int16;
typedef unsigned __int16 uint16;
typedef __int32 int32;
typedef unsigned __int32 uint32;
typedef __int64 int64;
typedef unsigned __int64 uint64;
typedef uint32 RTime32;
typedef float float32;
typedef double float64;
typedef unsigned int uint;