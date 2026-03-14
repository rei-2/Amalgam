#pragma once
#include "../../../Utils/Signatures/Signatures.h"
#include <minwindef.h>

MAKE_SIGNATURE(CMapLoadHelper, "engine.dll", "40 53 56 41 56 41 57", 0x0);
MAKE_SIGNATURE(_CMapLoadHelper, "engine.dll", "40 53 48 83 EC ? 48 8B D9 48 8B 49 ? 48 85 C9 74 ? E8 ? ? ? ? 48 8B 53", 0x0);
MAKE_SIGNATURE(CMapLoadHelper_Init, "engine.dll", "48 89 5C 24 ? 57 48 83 EC ? 8B 05 ? ? ? ? 48 8B DA", 0x0);
MAKE_SIGNATURE(CMapLoadHelper_Shutdown, "engine.dll", "48 83 EC ? 8B 05 ? ? ? ? FF C8", 0x0);

#define	HEADER_LUMPS 64

struct model_t;
using byte = unsigned char;

enum
{
	LUMP_ENTITIES = 0,
	LUMP_PLANES = 1,
	LUMP_TEXDATA = 2,
	LUMP_VERTEXES = 3,
	LUMP_VISIBILITY = 4,
	LUMP_NODES = 5,
	LUMP_TEXINFO = 6,
	LUMP_FACES = 7,
	LUMP_LIGHTING = 8,
	LUMP_OCCLUSION = 9,
	LUMP_LEAFS = 10,
	LUMP_FACEIDS = 11,
	LUMP_EDGES = 12,
	LUMP_SURFEDGES = 13,
	LUMP_MODELS = 14,
	LUMP_WORLDLIGHTS = 15,
	LUMP_LEAFFACES = 16,
	LUMP_LEAFBRUSHES = 17,
	LUMP_BRUSHES = 18,
	LUMP_BRUSHSIDES = 19,
	LUMP_AREAS = 20,
	LUMP_AREAPORTALS = 21,
	LUMP_UNUSED0 = 22,
	LUMP_UNUSED1 = 23,
	LUMP_UNUSED2 = 24,
	LUMP_UNUSED3 = 25,
	LUMP_DISPINFO = 26,
	LUMP_ORIGINALFACES = 27,
	LUMP_PHYSDISP = 28,
	LUMP_PHYSCOLLIDE = 29,
	LUMP_VERTNORMALS = 30,
	LUMP_VERTNORMALINDICES = 31,
	LUMP_DISP_LIGHTMAP_ALPHAS = 32,
	LUMP_DISP_VERTS = 33,
	LUMP_DISP_LIGHTMAP_SAMPLE_POSITIONS = 34,
	LUMP_GAME_LUMP = 35,
	LUMP_LEAFWATERDATA = 36,
	LUMP_PRIMITIVES = 37,
	LUMP_PRIMVERTS = 38,
	LUMP_PRIMINDICES = 39,
	LUMP_PAKFILE = 40,
	LUMP_CLIPPORTALVERTS = 41,
	LUMP_CUBEMAPS = 42,
	LUMP_TEXDATA_STRING_DATA = 43,
	LUMP_TEXDATA_STRING_TABLE = 44,
	LUMP_OVERLAYS = 45,
	LUMP_LEAFMINDISTTOWATER = 46,
	LUMP_FACE_MACRO_TEXTURE_INFO = 47,
	LUMP_DISP_TRIS = 48,
	LUMP_PHYSCOLLIDESURFACE = 49,
	LUMP_WATEROVERLAYS = 50,
	LUMP_LEAF_AMBIENT_INDEX_HDR = 51,
	LUMP_LEAF_AMBIENT_INDEX = 52,
	LUMP_LIGHTING_HDR = 53,
	LUMP_WORLDLIGHTS_HDR = 54,
	LUMP_LEAF_AMBIENT_LIGHTING_HDR = 55,
	LUMP_LEAF_AMBIENT_LIGHTING = 56,
	LUMP_XZIPPAKFILE = 57,
	LUMP_FACES_HDR = 58,
	LUMP_MAP_FLAGS = 59,
	LUMP_OVERLAY_FADES = 60,
};

class CMapLoadHelper
{
public:
	CMapLoadHelper(int lumpToLoad)
	{
		S::CMapLoadHelper.Call<CMapLoadHelper*>(this, lumpToLoad);
	}
	~CMapLoadHelper(void)
	{
		S::_CMapLoadHelper.Call<void>(this);
	}

	byte* LumpBase() { return m_pData; }
	int LumpSize() { return m_nLumpSize; }
	int LumpOffset() { return m_nLumpOffset; };
	int LumpVersion() const { return m_nLumpVersion; };
	//const char* GetMapName() { return s_szMapName; }
	//char* GetLoadName();
	//struct worldbrushdata_t* GetMap();

	// Global setup/shutdown
	static SIGNATURE_ARGS(Init, void, CMapLoadHelper, (model_t* pMapModel, const char* loadname), pMapModel, loadname);
	static SIGNATURE(Shutdown, void, CMapLoadHelper);
	//static void InitFromMemory(model_t* pMapModel, const void* pData, int nDataSize);
	//static int GetRefCount();
	//static void FreeLightingLump();
	//static int LumpSize(int lumpId);
	//static int LumpOffset(int lumpId);
	//void LoadLumpElement(int nElemIndex, int nElemSize, void* pData);
	//void LoadLumpData(int offset, int size, void* pData);

public:
	int m_nLumpSize;
	int m_nLumpOffset;
	int m_nLumpVersion;
	byte* m_pRawData;
	byte* m_pData;
	byte* m_pUncompressedData;
	int m_nLumpID;
	char m_szLumpFilename[MAX_PATH];
};