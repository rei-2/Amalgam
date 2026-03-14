#pragma once
#include "../Misc/CUtlVector.h"
#include "../Types.h"

typedef __m128 fltx4;

class __declspec(align(16)) FourVectors
{
public:
	fltx4 x, y, z;
};

template<typename T>
class CDispVector : public CUtlVector<T, CUtlMemoryAligned<T, 16>>
{

};

class CDispCollTri
{
public:
	inline void SetVert(int iPos, int iVert) { m_TriData[iPos].m_Index.uiVert = iVert; }
	inline int GetVert(int iPos) const { return m_TriData[iPos].m_Index.uiVert; }
	inline void SetMin(int iAxis, int iMin) { m_TriData[iAxis].m_Index.uiMin = iMin; }
	inline int GetMin(int iAxis) const { return m_TriData[iAxis].m_Index.uiMin; }
	inline void SetMax(int iAxis, int iMax) { m_TriData[iAxis].m_Index.uiMax = iMax; }
	inline int GetMax(int iAxis) const { return m_TriData[iAxis].m_Index.uiMax; }

	struct index_t
	{
		union
		{
			struct
			{
				unsigned short uiVert : 9;
				unsigned short uiMin : 2;
				unsigned short uiMax : 2;
			} m_Index;
			unsigned short m_IndexDummy;
		};
	};

	index_t m_TriData[3];
	unsigned short m_ucSignBits : 3;
	unsigned short m_ucPlaneType : 3;
	unsigned short m_uiFlags : 5;
	Vector m_vecNormal;
	float m_flDist;
};

class CDispCollNode
{
public:
	FourVectors m_mins;
	FourVectors m_maxs;
};

class CDispCollLeaf
{
public:
	short	m_tris[2];
};

#pragma pack(1)
class CDispCollTriCache
{
public:
	unsigned short m_iCrossX[3];
	unsigned short m_iCrossY[3];
	unsigned short m_iCrossZ[3];
};
#pragma pack()

class CDispCollHelper
{
public:

	float m_flStartFrac;
	float m_flEndFrac;
	Vector m_vecImpactNormal;
	float m_flImpactDist;
};

class CDispCollTree
{
public:
	inline int GetWidth(void) const { return ((1 << m_nPower) + 1); }
	inline int GetHeight(void) const { return ((1 << m_nPower) + 1); }
	inline int GetSize(void) const { return ((1 << m_nPower) + 1) * ((1 << m_nPower) + 1); }
	inline int GetFaceCount(void) const { return ((1 << m_nPower) * (1 << m_nPower) * 2); }

	byte pad0[8];
	Vector m_mins;
	int m_iCounter;
	Vector m_maxs;
	int m_nContents;
	void* m_hCache;
	int m_nPower;
	int m_nFlags;
	Vector m_vecSurfPoints[4];
	Vector m_vecStabDir;
	short m_nSurfaceProps[2];
	CUtlVector<Vector> m_aVerts;
	CDispVector<CDispCollTri> m_aTris;
	CDispVector<CDispCollNode> m_nodes;
	CDispVector<CDispCollLeaf> m_leaves;
	CUtlVector<CDispCollTriCache> m_aTrisCache;
	CUtlVector<Vector> m_aEdgePlanes;
	CDispCollHelper m_Helper;
	unsigned int m_nSize;
};

MAKE_INTERFACE_SIGNATURE(CDispCollTree*, DispCollTrees, "engine.dll", "48 89 05 ? ? ? ? E8 ? ? ? ? 48 8B D0", 0x0, 0, false);
MAKE_INTERFACE_SIGNATURE(int, DispCollTreeCount, "engine.dll", "44 89 35 ? ? ? ? E8", 0x0, 0, false);