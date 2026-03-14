#pragma once
#include "Interface.h"
#include "../Main/CModel.h"
#include "../Misc/CRangeValidatedArray.h"
#include "../Misc/CDiscardableArray.h"

#define	MAX_QPATH 96

class CCollisionBSPData
{
public:
	cnode_t* map_rootnode;
	char map_name[MAX_QPATH];
	static csurface_t nullsurface;
	int numbrushsides;
	CRangeValidatedArray<cbrushside_t> map_brushsides;
	int numboxbrushes;
	CRangeValidatedArray<cboxbrush_t> map_boxbrushes;
	int numplanes;
	CRangeValidatedArray<cplane_t> map_planes;
	int numnodes;
	CRangeValidatedArray<cnode_t> map_nodes;
	int numleafs;
	CRangeValidatedArray<cleaf_t> map_leafs;
	int emptyleaf, solidleaf;
	int numleafbrushes;
	CRangeValidatedArray<unsigned short> map_leafbrushes;
	int numcmodels;
	CRangeValidatedArray<cmodel_t> map_cmodels;
	int numbrushes;
	CRangeValidatedArray<cbrush_t> map_brushes;
	int numdisplist;
	CRangeValidatedArray<unsigned short> map_dispList;
	int numvisibility;
	dvis_t* map_vis;
	int numentitychars;
	CDiscardableArray<char> map_entitystring;
	int numareas;
	CRangeValidatedArray<carea_t> map_areas;
	int numareaportals;
	CRangeValidatedArray<dareaportal_t> map_areaportals;
	int numclusters;
	char* map_nullname;
	int numtextures;
	char* map_texturenames;
	CRangeValidatedArray<csurface_t> map_surfaces;
	int floodvalid;
	int numportalopen;
	CRangeValidatedArray<bool> portalopen;
};

MAKE_INTERFACE_SIGNATURE(CCollisionBSPData, BSPData, "engine.dll", "48 8D 15 ? ? ? ? 4C 03 05", 0x0, 0);