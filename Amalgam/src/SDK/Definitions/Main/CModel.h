#pragma once
#include "Ray.h"
#include "../Misc/BSPFlags.h"
#include "../Misc/VCollide.h"
#include "../Misc/CUtlString.h"

#define	AREA_SOLID 1
#define	AREA_TRIGGERS 2
#define NUMSIDES_BOXBRUSH 0xFFFF
#define	MAXLIGHTMAPS 4

#define SURFDRAW_NOLIGHT		0x00000001
#define	SURFDRAW_NODE			0x00000002
#define	SURFDRAW_SKY			0x00000004
#define SURFDRAW_BUMPLIGHT		0x00000008
#define SURFDRAW_NODRAW			0x00000010
#define SURFDRAW_TRANS			0x00000020
#define SURFDRAW_PLANEBACK		0x00000040
#define SURFDRAW_DYNAMIC		0x00000080
#define SURFDRAW_TANGENTSPACE	0x00000100
#define SURFDRAW_NOCULL			0x00000200
#define SURFDRAW_HASLIGHTSYTLES 0x00000400
#define SURFDRAW_HAS_DISP		0x00000800
#define SURFDRAW_ALPHATEST		0x00001000
#define SURFDRAW_NOSHADOWS		0x00002000
#define SURFDRAW_NODECALS		0x00004000
#define SURFDRAW_HAS_PRIMS		0x00008000
#define SURFDRAW_WATERSURFACE	0x00010000
#define SURFDRAW_UNDERWATER		0x00020000
#define SURFDRAW_ABOVEWATER		0x00040000
#define SURFDRAW_HASDLIGHT		0x00080000
#define SURFDRAW_DLIGHTPASS		0x00100000
#define SURFDRAW_UNUSED2		0x00200000
#define SURFDRAW_VERTCOUNT_MASK	0xFF000000
#define SURFDRAW_SORTGROUP_MASK	0x00C00000

#define SURFDRAW_VERTCOUNT_SHIFT	24
#define SURFDRAW_SORTGROUP_SHIFT	22

class IMaterial;
class ITexture;
class CEngineSprite;
class IDispInfo;
struct cplane_t;
struct edict_t;
struct model_t;
typedef unsigned short WorldDecalHandle_t;
typedef unsigned short ShadowDecalHandle_t;
typedef unsigned short OverlayFragmentHandle_t;
typedef void* FileNameHandle_t;
typedef unsigned short MDLHandle_t;
typedef void* HDISPINFOARRAY;
typedef void*/*CCubeMap<LightShadowZBufferSample_t, SHADOW_ZBUF_RES>*/ lightzbuffer_t;

enum modtype_t
{
	mod_bad = 0,
	mod_brush,
	mod_sprite,
	mod_studio
};

enum emittype_t
{
	emit_surface,
	emit_point,
	emit_spotlight,
	emit_skylight,
	emit_quakelight,
	emit_skyambient,
};

struct ColorRGBExp32
{
	byte r, g, b;
	signed char exponent;
};

struct CompressedLightCube
{
	ColorRGBExp32 m_Color[6];
};

struct darea_t
{
	int numareaportals;
	int firstareaportal;
};

struct dareaportal_t
{
	unsigned short m_PortalKey;
	unsigned short otherarea;
	unsigned short m_FirstClipPortalVert;
	unsigned short m_nClipPortalVerts;
	int planenum;
};

struct dvis_t
{
	int numclusters;
	int bitofs[8][2];
};

struct doccluderdata_t
{
	int flags;
	int firstpoly;
	int polycount;
	Vector mins;
	Vector maxs;
	int area;
};

struct doccluderpolydata_t
{
	int firstvertexindex;
	int vertexcount;
	int planenum;
};

struct dvertex_t
{
	Vector point;
};

struct dedge_t
{
	unsigned short v[2];
};

struct dface_t
{
	unsigned short planenum;
	byte side;
	byte onNode;
	int firstedge;
	short numedges;
	short texinfo;
	//union
	//{
	short dispinfo;
	short surfaceFogVolumeID;
	//};
	byte styles[MAXLIGHTMAPS];
	int lightofs;
	float area;
	int m_LightmapTextureMinsInLuxels[2];
	int m_LightmapTextureSizeInLuxels[2];
	int origFace;
	unsigned short m_NumPrims;
	unsigned short	firstPrimID;
	unsigned int	smoothingGroups;
};

struct dbrush_t
{
	int firstside;
	int numsides;
	int contents;
};

struct dbrushside_t
{
	unsigned short planenum;
	short texinfo;
	short dispinfo;
	short bevel;
};

struct dworldlight_t
{
	Vector origin;
	Vector intensity;
	Vector normal;
	int cluster;
	emittype_t type;
	int style;
	float stopdot;
	float stopdot2;
	float exponent;
	float radius;
	float constant_attn;
	float linear_attn;
	float quadratic_attn;
	int flags;
	int texinfo;
	int owner;
};

struct dleafambientindex_t
{
	unsigned short ambientSampleCount;
	unsigned short firstAmbientSample;
};
typedef dleafambientindex_t mleafambientindex_t;

struct dleafambientlighting_t
{
	CompressedLightCube	cube;
	byte x;
	byte y;
	byte z;
	byte pad;
};
typedef dleafambientlighting_t mleafambientlighting_t;

struct mnode_t
{
	int contents;
	int visframe;
	mnode_t* parent;
	short area;
	short flags;
	VectorAligned m_vecCenter;
	VectorAligned m_vecHalfDiagonal;
	cplane_t* plane;
	mnode_t* children[2];
	unsigned short firstsurface;
	unsigned short numsurfaces;
};

struct mleaf_t
{
	int contents;
	int visframe;
	mnode_t* parent;
	short area;
	short flags;
	VectorAligned m_vecCenter;
	VectorAligned m_vecHalfDiagonal;
	short cluster;
	short leafWaterDataID;
	unsigned short firstmarksurface;
	unsigned short nummarksurfaces;
	short nummarknodesurfaces;
	short unused;
	unsigned short dispListStart;
	unsigned short dispCount;
};

struct mleafwaterdata_t
{
	float surfaceZ;
	float minZ;
	short surfaceTexInfoID;
	short firstLeafIndex;
};

struct mvertex_t
{
	Vector position;
};

struct msurface1_t
{
	int textureMins[2];
	short textureExtents[2];
	struct
	{
		unsigned short numPrims;
		unsigned short firstPrimID;
	} prims;
};

#pragma pack(1)
struct msurface2_t
{
	unsigned int flags;
	//unsigned char vertCount;
	//unsigned char sortGroup;
	cplane_t* plane;
	int firstvertindex;
	WorldDecalHandle_t decals;
	ShadowDecalHandle_t m_ShadowDecals;
	OverlayFragmentHandle_t m_nFirstOverlayFragment;
	short materialSortID;
	unsigned short vertBufferIndex;
	unsigned short m_bDynamicShadowsEnabled : 1;
	unsigned short texinfo : 15;
	IDispInfo* pDispInfo;
	int visframe;
	byte pad[24];
};
#pragma pack()
typedef msurface2_t* SurfaceHandle_t;

struct msurfacelighting_t
{
	short m_LightmapMins[2];
	short m_LightmapExtents[2];
	short m_OffsetIntoLightmapPage[2];
	int m_nLastComputedFrame;
	int m_fDLightBits;
	int m_nDLightFrame;
	unsigned char m_nStyles[MAXLIGHTMAPS];
	ColorRGBExp32* m_pSamples;
};

#pragma pack(1)
struct msurfacenormal_t
{
	unsigned int firstvertnormal;
	//unsigned short firstvertnormal;
	//short fogVolumeID;
};
#pragma pack()

struct mtexinfo_t
{
	Vector4D textureVecsTexelsPerWorldUnits[2];
	Vector4D lightmapVecsLuxelsPerWorldUnits[2];
	float luxelsPerWorldUnit;
	float worldUnitsPerLuxel;
	unsigned short flags;
	unsigned short texinfoFlags;
	IMaterial* material;
};

struct mprimitive_t
{
	int	type;
	unsigned short firstIndex;
	unsigned short indexCount;
	unsigned short firstVert;
	unsigned short vertCount;
};

struct mprimvert_t
{
	Vector pos;
	float texCoord[2];
	float lightCoord[2];
};

struct mcubemapsample_t
{
	Vector origin;
	ITexture* pTexture;
	unsigned char size;
};

struct cleaf_t
{
	int contents;
	short cluster;
	short area : 9;
	short flags : 7;
	unsigned short firstleafbrush;
	unsigned short numleafbrushes;
	unsigned short dispListStart;
	unsigned short dispCount;
};

struct carea_t
{
	int numareaportals;
	int firstareaportal;
	int floodnum;
	int floodvalid;
};

struct cplane_t
{
	Vector normal{};
	float dist{};
	byte type{};
	byte signbits{};
	byte pad[2]{};
};

struct csurface_t
{
	const char* name;
	short surfaceProps;
	unsigned short flags;
};

struct cnode_t
{
	cplane_t* plane;
	int children[2];
};

struct cbrush_t
{
	int				contents;
	unsigned short	numsides;
	unsigned short	firstbrushside;

	inline int GetBox() const { return firstbrushside; }
	inline void SetBox(int boxID)
	{
		numsides = NUMSIDES_BOXBRUSH;
		firstbrushside = boxID;
	}
	inline bool IsBox() const { return numsides == NUMSIDES_BOXBRUSH ? true : false; }
};

struct cboxbrush_t
{
	VectorAligned mins;
	VectorAligned maxs;
	unsigned short surfaceIndex[6];
	unsigned short pad2[2];
};

struct cbrushside_t
{
	cplane_t* plane;
	unsigned short surfaceIndex;
	unsigned short bBevel;
};

struct cmodel_t
{
	Vector mins, maxs;
	Vector origin;
	int headnode;
	vcollide_t vcollisionData;
};

struct worldbrushdata_t
{
	int numsubmodels;
	int numplanes;
	cplane_t* planes;
	int numleafs;
	mleaf_t* leafs;
	int numleafwaterdata;
	mleafwaterdata_t* leafwaterdata;
	int numvertexes;
	mvertex_t* vertexes;
	int numoccluders;
	doccluderdata_t* occluders;
	int numoccluderpolys;
	doccluderpolydata_t* occluderpolys;
	int numoccludervertindices;
	int* occludervertindices;
	int numvertnormalindices;
	unsigned short* vertnormalindices;
	int numvertnormals;
	Vector* vertnormals;
	int numnodes;
	mnode_t* nodes;
	unsigned short* m_LeafMinDistToWater;
	int numtexinfo;
	mtexinfo_t* texinfo;
	int numtexdata;
	csurface_t* texdata;
	int numDispInfos;
	HDISPINFOARRAY hDispInfos;
	//int numOrigSurfaces;
	//msurface_t *pOrigSurfaces;
	int numsurfaces;
	msurface1_t* surfaces1;
	msurface2_t* surfaces2;
	msurfacelighting_t* surfacelighting;
	msurfacenormal_t* surfacenormals;
	bool unloadedlightmaps;
	int numvertindices;
	unsigned short* vertindices;
	int nummarksurfaces;
	SurfaceHandle_t* marksurfaces;
	ColorRGBExp32* lightdata;
	int numworldlights;
	dworldlight_t* worldlights;
	lightzbuffer_t* shadowzbuffers;
	int numprimitives;
	mprimitive_t* primitives;
	int numprimverts;
	mprimvert_t* primverts;
	int numprimindices;
	unsigned short* primindices;
	int m_nAreas;
	darea_t* m_pAreas;
	int m_nAreaPortals;
	dareaportal_t* m_pAreaPortals;
	int m_nClipPortalVerts;
	Vector* m_pClipPortalVerts;
	mcubemapsample_t* m_pCubemapSamples;
	int m_nCubemapSamples;
	int m_nDispInfoReferences;
	unsigned short* m_pDispInfoReferences;
	mleafambientindex_t* m_pLeafAmbient;
	mleafambientlighting_t* m_pAmbientSamples;
#if 0
	int numportals;
	mportal_t* portals;
	int numclusters;
	mcluster_t* clusters;
	int numportalverts;
	unsigned short* portalverts;
	int numclusterportals;
	unsigned short* clusterportals;
#endif
};

struct brushdata_t
{
	worldbrushdata_t* pShared;
	int firstmodelsurface, nummodelsurfaces;
	unsigned short renderHandle;
	unsigned short firstnode;
};

struct spritedata_t
{
	int numframes;
	int width;
	int height;
	CEngineSprite* sprite;
};

struct model_t
{
	FileNameHandle_t fnHandle;
	CUtlString strName;
	int nLoadFlags;
	int nServerCount;
	IMaterial** ppMaterials;
	modtype_t type;
	int flags;
	Vector mins, maxs;
	float radius;
	union
	{
		brushdata_t brush;
		MDLHandle_t studio;
		spritedata_t sprite;
	};
};