#pragma once
#include "BaseTypes.h"
#include "Datamap.h"
#include "LocalFlexController.h"
#include "../Main/UtlVector.h"
#include "../Types.h"

#define STUDIO_ENABLE_PERF_COUNTERS
#define STUDIO_SEQUENCE_ACTIVITY_LOOKUPS_ARE_SLOW 0 
#define STUDIO_SEQUENCE_ACTIVITY_LAZY_INITIALIZE 1

class IMaterial;
class IMesh;
class IMorph;
struct virtualmodel_t;
struct vertexFileHeader_t;
struct thinModelVertices_t;
struct StripHeader_t;

#define STUDIO_VERSION 48

#define MAXSTUDIOTRIANGLES 65536
#define MAXSTUDIOVERTS 65536
#define	MAXSTUDIOFLEXVERTS 10000
#define MAXSTUDIOSKINS 32
#define MAXSTUDIOBONES 128
#define MAXSTUDIOFLEXDESC 1024
#define MAXSTUDIOFLEXCTRL 96
#define MAXSTUDIOPOSEPARAM 24
#define MAXSTUDIOBONECTRLS 4
#define MAXSTUDIOANIMBLOCKS 256
#define MAXSTUDIOBONEBITS 7
#define MAX_NUM_BONES_PER_VERT 3

#define NEW_EVENT_STYLE ( 1 << 10 )

struct mstudiodata_t
{
	int count;
	int offset;
};

#define STUDIO_PROC_AXISINTERP 1
#define STUDIO_PROC_QUATINTERP 2
#define STUDIO_PROC_AIMATBONE 3
#define STUDIO_PROC_AIMATATTACH 4
#define STUDIO_PROC_JIGGLE 5

struct mstudioaxisinterpbone_t
{
	int control;
	int axis;
	Vector pos[6];
	Quaternion quat[6];
};

struct mstudioquatinterpinfo_t
{
	float inv_tolerance;
	Quaternion trigger;
	Vector pos;
	Quaternion quat;
};

struct mstudioquatinterpbone_t
{
	int control;
	int numtriggers;
	int triggerindex;
	inline mstudioquatinterpinfo_t* pTrigger(int i) const { return  (mstudioquatinterpinfo_t*)(((byte*)this) + triggerindex) + i; };
};

#define JIGGLE_IS_FLEXIBLE 0x01
#define JIGGLE_IS_RIGID	0x02
#define JIGGLE_HAS_YAW_CONSTRAINT 0x04
#define JIGGLE_HAS_PITCH_CONSTRAINT	0x08
#define JIGGLE_HAS_ANGLE_CONSTRAINT	0x10
#define JIGGLE_HAS_LENGTH_CONSTRAINT 0x20
#define JIGGLE_HAS_BASE_SPRING 0x40
#define JIGGLE_IS_BOING 0x80

struct mstudiojigglebone_t
{
	int flags;
	float length;
	float tipMass;
	float yawStiffness;
	float yawDamping;
	float pitchStiffness;
	float pitchDamping;
	float alongStiffness;
	float alongDamping;
	float angleLimit;
	float minYaw;
	float maxYaw;
	float yawFriction;
	float yawBounce;
	float minPitch;
	float maxPitch;
	float pitchFriction;
	float pitchBounce;
	float baseMass;
	float baseStiffness;
	float baseDamping;
	float baseMinLeft;
	float baseMaxLeft;
	float baseLeftFriction;
	float baseMinUp;
	float baseMaxUp;
	float baseUpFriction;
	float baseMinForward;
	float baseMaxForward;
	float baseForwardFriction;
	float boingImpactSpeed;
	float boingImpactAngle;
	float boingDampingRate;
	float boingFrequency;
	float boingAmplitude;
};

struct mstudioaimatbone_t
{
	int parent;
	int aim;
	Vector aimvector;
	Vector upvector;
	Vector basepos;
};

struct mstudiobone_t
{
	int sznameindex;
	inline char* const pszName(void) const { return ((char*)this) + sznameindex; }
	int parent;
	int bonecontroller[6];
	Vector pos;
	Quaternion quat;
	RadianEuler rot;
	Vector posscale;
	Vector rotscale;
	matrix3x4 poseToBone;
	Quaternion qAlignment;
	int flags;
	int proctype;
	int procindex;
	mutable int physicsbone;
	inline void* pProcedure() const { if (procindex == 0) return NULL; else return  (void*)(((byte*)this) + procindex); };
	int surfacepropidx;
	inline char* const pszSurfaceProp(void) const { return ((char*)this) + surfacepropidx; }
	int contents;
	int unused[8];
};

struct mstudiolinearbone_t
{
	int numbones;
	int flagsindex;
	inline int flags(int i) const { return *((int*)(((byte*)this) + flagsindex) + i); };
	inline int* pflags(int i) { return ((int*)(((byte*)this) + flagsindex) + i); };
	int	parentindex;
	inline int parent(int i) const { return *((int*)(((byte*)this) + parentindex) + i); };
	int	posindex;
	inline Vector pos(int i) const { return *((Vector*)(((byte*)this) + posindex) + i); };
	int quatindex;
	inline Quaternion quat(int i) const { return *((Quaternion*)(((byte*)this) + quatindex) + i); };
	int rotindex;
	inline RadianEuler rot(int i) const { return *((RadianEuler*)(((byte*)this) + rotindex) + i); };
	int posetoboneindex;
	inline matrix3x4* poseToBone(int i) const { return ((matrix3x4*)(((byte*)this) + posetoboneindex) + i); };
	int	posscaleindex;
	inline Vector posscale(int i) const { return *((Vector*)(((byte*)this) + posscaleindex) + i); };
	int	rotscaleindex;
	inline Vector rotscale(int i) const { return *((Vector*)(((byte*)this) + rotscaleindex) + i); };
	int	qalignmentindex;
	inline Quaternion qalignment(int i) const { return *((Quaternion*)(((byte*)this) + qalignmentindex) + i); };
	int unused[6];
};

enum StudioBoneFlexComponent_t
{
	STUDIO_BONE_FLEX_INVALID = -1,
	STUDIO_BONE_FLEX_TX = 0,
	STUDIO_BONE_FLEX_TY = 1,
	STUDIO_BONE_FLEX_TZ = 2
};

struct mstudioboneflexdrivercontrol_t
{
	int m_nBoneComponent;
	int m_nFlexControllerIndex;
	float m_flMin;
	float m_flMax;
};

struct mstudioboneflexdriver_t
{
	int m_nBoneIndex;
	int m_nControlCount;
	int m_nControlIndex;

	inline mstudioboneflexdrivercontrol_t* pBoneFlexDriverControl(int i) const
	{
		return (mstudioboneflexdrivercontrol_t*)(((byte*)this) + m_nControlIndex) + i;
	}

	int unused[3];
};

#define BONE_CALCULATE_MASK	0x1F
#define BONE_PHYSICALLY_SIMULATED 0x01
#define BONE_PHYSICS_PROCEDURAL 0x02
#define BONE_ALWAYS_PROCEDURAL 0x04
#define BONE_SCREEN_ALIGN_SPHERE 0x08
#define BONE_SCREEN_ALIGN_CYLINDER	0x10
#define BONE_USED_MASK 0x0007FF00
#define BONE_USED_BY_ANYTHING 0x0007FF00
#define BONE_USED_BY_HITBOX 0x00000100
#define BONE_USED_BY_ATTACHMENT 0x00000200
#define BONE_USED_BY_VERTEX_MASK 0x0003FC00
#define BONE_USED_BY_VERTEX_LOD0 0x00000400
#define BONE_USED_BY_VERTEX_LOD1 0x00000800	
#define BONE_USED_BY_VERTEX_LOD2 0x00001000  
#define BONE_USED_BY_VERTEX_LOD3 0x00002000
#define BONE_USED_BY_VERTEX_LOD4 0x00004000
#define BONE_USED_BY_VERTEX_LOD5 0x00008000
#define BONE_USED_BY_VERTEX_LOD6 0x00010000
#define BONE_USED_BY_VERTEX_LOD7 0x00020000
#define BONE_USED_BY_BONE_MERGE 0x00040000
#define BONE_USED_BY_VERTEX_AT_LOD(lod) ( BONE_USED_BY_VERTEX_LOD0 << (lod) )
#define BONE_USED_BY_ANYTHING_AT_LOD(lod) ( ( BONE_USED_BY_ANYTHING & ~BONE_USED_BY_VERTEX_MASK ) | BONE_USED_BY_VERTEX_AT_LOD(lod) )

#define MAX_NUM_LODS 8

#define BONE_TYPE_MASK 0x00F00000
#define BONE_FIXED_ALIGNMENT 0x00100000
#define BONE_HAS_SAVEFRAME_POS 0x00200000
#define BONE_HAS_SAVEFRAME_ROT 0x00400000

struct mstudiobonecontroller_t
{
	int bone;
	int type;
	float start;
	float end;
	int rest;
	int inputfield;
	int unused[8];
};

struct mstudiobbox_t
{
	int bone;
	int group;
	Vector bbmin;
	Vector bbmax;
	int szhitboxnameindex;
	Vec3 angle;
	float m_radius;
	uint8_t _padding[0x10];

	const char* pszHitboxName()
	{
		if (szhitboxnameindex == 0)
			return "";

		return ((const char*)this) + szhitboxnameindex;
	}
};

struct mstudiomodelgroup_t
{
	int szlabelindex;
	inline char* const pszLabel(void) const { return ((char*)this) + szlabelindex; }
	int sznameindex;
	inline char* const pszName(void) const { return ((char*)this) + sznameindex; }
};

struct mstudiomodelgrouplookup_t
{
	int modelgroup;
	int indexwithingroup;
};

struct mstudioevent_t
{
	float cycle;
	int event;
	int type;
	inline const char* pszOptions(void) const { return options; }
	char options[64];
	int szeventindex;
	inline char* const pszEventName(void) const { return ((char*)this) + szeventindex; }
};

#define	ATTACHMENT_FLAG_WORLD_ALIGN 0x10000

struct mstudioattachment_t
{
	int sznameindex;
	inline char* const pszName(void) const { return ((char*)this) + sznameindex; }
	unsigned int flags;
	int localbone;
	matrix3x4 local;
	int unused[8];
};

#define IK_SELF 1
#define IK_WORLD 2
#define IK_GROUND 3
#define IK_RELEASE 4
#define IK_ATTACHMENT 5
#define IK_UNLATCH 6

struct mstudioikerror_t
{
	Vector pos;
	Quaternion q;
};

union mstudioanimvalue_t;

struct mstudiocompressedikerror_t
{
	float scale[6];
	short offset[6];
	inline mstudioanimvalue_t* pAnimvalue(int i) const { if (offset[i] > 0) return  (mstudioanimvalue_t*)(((byte*)this) + offset[i]); else return NULL; };
};

struct mstudioikrule_t
{
	int index;
	int type;
	int chain;
	int bone;
	int slot;
	float height;
	float radius;
	float floor;
	Vector pos;
	Quaternion q;
	int compressedikerrorindex;
	inline mstudiocompressedikerror_t* pCompressedError() const { return (mstudiocompressedikerror_t*)(((byte*)this) + compressedikerrorindex); };
	int unused2;
	int iStart;
	int ikerrorindex;
	inline mstudioikerror_t* pError(int i) const { return  (ikerrorindex) ? (mstudioikerror_t*)(((byte*)this) + ikerrorindex) + (i - iStart) : NULL; };
	float start;
	float peak;
	float tail;
	float end;
	float unused3;
	float contact;
	float drop;
	float top;
	int unused6;
	int unused7;
	int unused8;
	int szattachmentindex;
	inline char* const pszAttachment(void) const { return ((char*)this) + szattachmentindex; }
	int unused[7];
};

struct mstudioiklock_t
{
	int chain;
	float flPosWeight;
	float flLocalQWeight;
	int flags;
	int unused[4];
};


struct mstudiolocalhierarchy_t
{
	int iBone;
	int iNewParent;
	float start;
	float peak;
	float tail;
	float end;
	int iStart;
	int localanimindex;
	inline mstudiocompressedikerror_t* pLocalAnim() const { return (mstudiocompressedikerror_t*)(((byte*)this) + localanimindex); };
	int unused[4];
};

union mstudioanimvalue_t
{
	struct
	{
		byte valid;
		byte total;
	} num;
	short value;
};

struct mstudioanim_valueptr_t
{
	short offset[3];
	inline mstudioanimvalue_t* pAnimvalue(int i) const { if (offset[i] > 0) return  (mstudioanimvalue_t*)(((byte*)this) + offset[i]); else return NULL; };
};

#define STUDIO_ANIM_RAWPOS 0x01
#define STUDIO_ANIM_RAWROT 0x02
#define STUDIO_ANIM_ANIMPOS	0x04
#define STUDIO_ANIM_ANIMROT	0x08
#define STUDIO_ANIM_DELTA 0x10
#define STUDIO_ANIM_RAWROT2	0x20

struct mstudioanim_t
{
	byte bone;
	byte flags;
	inline byte* pData(void) const { return (((byte*)this) + sizeof(struct mstudioanim_t)); };
	inline mstudioanim_valueptr_t* pRotV(void) const { return (mstudioanim_valueptr_t*)(pData()); };
	inline mstudioanim_valueptr_t* pPosV(void) const { return (mstudioanim_valueptr_t*)(pData()) + ((flags & STUDIO_ANIM_ANIMROT) != 0); };
	short nextoffset;
	inline mstudioanim_t* pNext(void) const { if (nextoffset != 0) return  (mstudioanim_t*)(((byte*)this) + nextoffset); else return NULL; };
};

struct mstudiomovement_t
{
	int endframe;
	int motionflags;
	float v0;
	float v1;
	float angle;
	Vector vector;
	Vector position;
};

struct studiohdr_t;

struct mstudioanimblock_t
{
	int datastart;
	int dataend;
};

struct mstudioanimsections_t
{
	int animblock;
	int animindex;
};

struct mstudioanimdesc_t
{
	int baseptr;
	inline studiohdr_t* pStudiohdr(void) const { return (studiohdr_t*)(((byte*)this) + baseptr); }
	int sznameindex;
	inline char* const pszName(void) const { return ((char*)this) + sznameindex; }
	float fps;
	int flags;
	int numframes;
	int nummovements;
	int movementindex;
	inline mstudiomovement_t* const pMovement(int i) const { return (mstudiomovement_t*)(((byte*)this) + movementindex) + i; };
	int unused1[6];
	int animblock;
	int animindex;
	int numikrules;
	int ikruleindex;
	int animblockikruleindex;
	int numlocalhierarchy;
	int localhierarchyindex;
	int sectionindex;
	int sectionframes;
	inline mstudioanimsections_t* const pSection(int i) const { return (mstudioanimsections_t*)(((byte*)this) + sectionindex) + i; }
	short zeroframespan;
	short zeroframecount;
	int zeroframeindex;
	byte* pZeroFrameData() const { if (zeroframeindex) return (((byte*)this) + zeroframeindex); else return NULL; };
	mutable float zeroframestalltime;
};

struct mstudioikrule_t;

struct mstudioautolayer_t
{
	short iSequence;
	short iPose;
	int flags;
	float start;
	float peak;
	float tail;
	float end;
};

struct mstudioactivitymodifier_t
{
	int sznameindex;
	inline char* pszName() { return (sznameindex) ? (char*)(((byte*)this) + sznameindex) : NULL; }
};

struct mstudioseqdesc_t
{
	int baseptr;
	inline studiohdr_t* pStudiohdr(void) const { return (studiohdr_t*)(((byte*)this) + baseptr); }
	int szlabelindex;
	inline char* const pszLabel(void) const { return ((char*)this) + szlabelindex; }
	int szactivitynameindex;
	inline char* const pszActivityName(void) const { return ((char*)this) + szactivitynameindex; }
	int flags;
	int activity;
	int actweight;
	int numevents;
	int eventindex;
	inline mstudioevent_t* pEvent(int i) const { return (mstudioevent_t*)(((byte*)this) + eventindex) + i; };
	Vector bbmin;
	Vector bbmax;
	int numblends;
	int animindexindex;

	inline int anim(int x, int y) const
	{
		if (x >= groupsize[0])
		{
			x = groupsize[0] - 1;
		}

		if (y >= groupsize[1])
		{
			y = groupsize[1] - 1;
		}

		int offset = y * groupsize[0] + x;
		short* blends = (short*)(((byte*)this) + animindexindex);
		int value = (int)blends[offset];
		return value;
	}

	int movementindex;
	int groupsize[2];
	int paramindex[2];
	float paramstart[2];
	float paramend[2];
	int paramparent;
	float fadeintime;
	float fadeouttime;
	int localentrynode;
	int localexitnode;
	int nodeflags;
	float entryphase;
	float exitphase;
	float lastframe;
	int nextseq;
	int pose;
	int numikrules;
	int numautolayers;
	int autolayerindex;
	inline mstudioautolayer_t* pAutolayer(int i) const { return (mstudioautolayer_t*)(((byte*)this) + autolayerindex) + i; };
	int weightlistindex;
	inline float* pBoneweight(int i) const { return ((float*)(((byte*)this) + weightlistindex) + i); };
	inline float weight(int i) const { return *(pBoneweight(i)); };
	int posekeyindex;
	float* pPoseKey(int iParam, int iAnim) const { return (float*)(((byte*)this) + posekeyindex) + iParam * groupsize[0] + iAnim; }
	float poseKey(int iParam, int iAnim) const { return *(pPoseKey(iParam, iAnim)); }
	int numiklocks;
	int iklockindex;
	inline mstudioiklock_t* pIKLock(int i) const { return (mstudioiklock_t*)(((byte*)this) + iklockindex) + i; };
	int keyvalueindex;
	int keyvaluesize;
	inline const char* KeyValueText(void) const { return keyvaluesize != 0 ? ((char*)this) + keyvalueindex : NULL; }
	int cycleposeindex;
	int activitymodifierindex;
	int numactivitymodifiers;
	inline mstudioactivitymodifier_t* pActivityModifier(int i) const { return activitymodifierindex != 0 ? (mstudioactivitymodifier_t*)(((byte*)this) + activitymodifierindex) + i : NULL; };
	int unused[5];
};

struct mstudioposeparamdesc_t
{
	int sznameindex;
	inline char* const pszName(void) const { return ((char*)this) + sznameindex; }
	int flags;
	float start;
	float end;
	float loop;
};

struct mstudioflexdesc_t
{
	int szFACSindex;
	inline char* const pszFACS(void) const { return ((char*)this) + szFACSindex; }
};

struct mstudioflexcontroller_t
{
	int sztypeindex;
	inline char* const pszType(void) const { return ((char*)this) + sztypeindex; }
	int sznameindex;
	inline char* const pszName(void) const { return ((char*)this) + sznameindex; }
	mutable int localToGlobal;
	float min;
	float max;
};

enum FlexControllerRemapType_t
{
	FLEXCONTROLLER_REMAP_PASSTHRU = 0,
	FLEXCONTROLLER_REMAP_2WAY,
	FLEXCONTROLLER_REMAP_NWAY,
	FLEXCONTROLLER_REMAP_EYELID
};

class CStudioHdr;
struct mstudioflexcontrollerui_t
{
	int sznameindex;
	inline char* const pszName(void) const { return ((char*)this) + sznameindex; }

	int szindex0;
	int szindex1;
	int szindex2;

	inline const mstudioflexcontroller_t* pController(void) const
	{
		return !stereo ? (mstudioflexcontroller_t*)((char*)this + szindex0) : NULL;
	}
	inline char* const	pszControllerName(void) const { return !stereo ? pController()->pszName() : NULL; }

	inline const mstudioflexcontroller_t* pLeftController(void) const
	{
		return stereo ? (mstudioflexcontroller_t*)((char*)this + szindex0) : NULL;
	}
	inline char* const	pszLeftName(void) const { return stereo ? pLeftController()->pszName() : NULL; }

	inline const mstudioflexcontroller_t* pRightController(void) const
	{
		return stereo ? (mstudioflexcontroller_t*)((char*)this + szindex1) : NULL;
	}

	inline char* const	pszRightName(void) const { return stereo ? pRightController()->pszName() : NULL; }

	inline const mstudioflexcontroller_t* pNWayValueController(void) const
	{
		return remaptype == FLEXCONTROLLER_REMAP_NWAY ? (mstudioflexcontroller_t*)((char*)this + szindex2) : NULL;
	}
	inline char* const	pszNWayValueName(void) const { return remaptype == FLEXCONTROLLER_REMAP_NWAY ? pNWayValueController()->pszName() : NULL; }
	inline int Count() const { return (stereo ? 2 : 1) + (remaptype == FLEXCONTROLLER_REMAP_NWAY ? 1 : 0); }
	unsigned char remaptype;
	bool stereo;
	byte unused[2];
};

struct mstudiovertanim_t
{
	unsigned short index;
	byte speed;
	byte side;

	/*union
	{
		short delta[3];
		float16 flDelta[3];
	};

	union
	{
		short ndelta[3];
		float16 flNDelta[3];
	};*/
};

struct mstudiovertanim_wrinkle_t : public mstudiovertanim_t
{
	short wrinkledelta;
};

enum StudioVertAnimType_t
{
	STUDIO_VERT_ANIM_NORMAL = 0,
	STUDIO_VERT_ANIM_WRINKLE
};

struct mstudioflex_t
{
	int flexdesc;
	float target0;
	float target1;
	float target2;
	float target3;
	int numverts;
	int vertindex;
	inline mstudiovertanim_t* pVertanim(int i) const { return (mstudiovertanim_t*)(((byte*)this) + vertindex) + i; };
	inline mstudiovertanim_wrinkle_t* pVertanimWrinkle(int i) const { return  (mstudiovertanim_wrinkle_t*)(((byte*)this) + vertindex) + i; };
	inline byte* pBaseVertanim() const { return ((byte*)this) + vertindex; };
	inline int	VertAnimSizeBytes() const { return (vertanimtype == STUDIO_VERT_ANIM_NORMAL) ? sizeof(mstudiovertanim_t) : sizeof(mstudiovertanim_wrinkle_t); }
	int flexpair;
	unsigned char vertanimtype;
	unsigned char unusedchar[3];
	int unused[6];
};

struct mstudioflexop_t
{
	int	 op;
	union
	{
		int index;
		float value;
	} d;
};

struct mstudioflexrule_t
{
	int flex;
	int numops;
	int opindex;
	inline mstudioflexop_t* iFlexOp(int i) const { return  (mstudioflexop_t*)(((byte*)this) + opindex) + i; };
};

struct mstudioboneweight_t
{
	float weight[MAX_NUM_BONES_PER_VERT];
	char bone[MAX_NUM_BONES_PER_VERT];
	byte numbones;
};

struct mstudiovertex_t
{
	mstudioboneweight_t	m_BoneWeights;
	Vector m_vecPosition;
	Vector m_vecNormal;
	Vector2D m_vecTexCoord;
};

struct mstudiotexture_t
{
	int sznameindex;
	inline char* const pszName(void) const { return ((char*)this) + sznameindex; }
	int flags;
	int used;
	int unused1;
	IMaterial* material;
	void* clientmaterial;
	int unused[10];
};

struct mstudioeyeball_t
{
	int sznameindex;
	inline char* const pszName(void) const { return ((char*)this) + sznameindex; }
	int bone;
	Vector org;
	float zoffset;
	float radius;
	Vector up;
	Vector forward;
	int texture;
	int unused1;
	float iris_scale;
	int unused2;
	int upperflexdesc[3];
	int lowerflexdesc[3];
	float uppertarget[3];
	float lowertarget[3];
	int	upperlidflexdesc;
	int	lowerlidflexdesc;
	int	unused[4];
	bool m_bNonFACS;
	char unused3[3];
	int unused4[7];
};

struct mstudioiklink_t
{
	int bone;
	Vector kneeDir;
	Vector unused0;
};

struct mstudioikchain_t
{
	int sznameindex;
	inline char* const pszName(void) const { return ((char*)this) + sznameindex; }
	int linktype;
	int numlinks;
	int linkindex;
	inline mstudioiklink_t* pLink(int i) const { return (mstudioiklink_t*)(((byte*)this) + linkindex) + i; };
};

struct mstudioiface_t
{
	unsigned short a, b, c;
};

struct mstudiomodel_t;

struct mstudio_modelvertexdata_t
{
	void* pVertexData;
	void* pTangentData;
};

struct mstudio_meshvertexdata_t
{
	mstudio_modelvertexdata_t* modelvertexdata;
	int numLODVertexes[MAX_NUM_LODS];
};

struct mstudiomesh_t
{
	int material;
	int modelindex;
	int numvertices;
	int vertexoffset;
	int numflexes;
	int flexindex;
	inline mstudioflex_t* pFlex(int i) const { return (mstudioflex_t*)(((byte*)this) + flexindex) + i; };
	int materialtype;
	int materialparam;
	int meshid;
	Vector center;
	mstudio_meshvertexdata_t vertexdata;
	int unused[8];
};

struct mstudiomodel_t
{
	inline const char* pszName(void) const { return name; }
	char name[64];
	int type;
	float boundingradius;
	int nummeshes;
	int meshindex;
	inline mstudiomesh_t* pMesh(int i) const { return (mstudiomesh_t*)(((byte*)this) + meshindex) + i; };
	int numvertices;
	int vertexindex;
	int tangentsindex;
	int numattachments;
	int attachmentindex;
	int numeyeballs;
	int eyeballindex;
	inline  mstudioeyeball_t* pEyeball(int i) { return (mstudioeyeball_t*)(((byte*)this) + eyeballindex) + i; };
	mstudio_modelvertexdata_t vertexdata;
	int unused[8];
};

enum studiomeshgroupflags_t
{
	MESHGROUP_IS_FLEXED = 0x1,
	MESHGROUP_IS_HWSKINNED = 0x2,
	MESHGROUP_IS_DELTA_FLEXED = 0x4
};

struct studiomeshgroup_t
{
	IMesh* m_pMesh;
	int m_NumStrips;
	int m_Flags;
	StripHeader_t* m_pStripData;
	unsigned short* m_pGroupIndexToMeshIndex;
	int m_NumVertices;
	int* m_pUniqueTris;
	unsigned short* m_pIndices;
	bool m_MeshNeedsRestore;
	short m_ColorMeshID;
	IMorph* m_pMorph;
	inline unsigned short MeshIndex(int i) const { return m_pGroupIndexToMeshIndex[m_pIndices[i]]; }
};

struct studiomeshdata_t
{
	int					m_NumGroup;
	studiomeshgroup_t* m_pMeshGroup;
};

struct studioloddata_t
{
	studiomeshdata_t* m_pMeshData;
	float m_SwitchPoint;
	int numMaterials;
	IMaterial** ppMaterials;
	int* pMaterialFlags;
	int* m_pHWMorphDecalBoneRemap;
	int m_nDecalBoneCount;
};

struct studiohwdata_t
{
	int m_RootLOD;
	int m_NumLODs;
	studioloddata_t* m_pLODs;
	int m_NumStudioMeshes;

	inline float LODMetric(float unitSphereSize) const { return (unitSphereSize != 0.0f) ? (100.0f / unitSphereSize) : 0.0f; }
};

struct mstudiobodyparts_t
{
	int sznameindex;
	inline char* const pszName(void) const { return ((char*)this) + sznameindex; }
	int nummodels;
	int base;
	int modelindex;
	inline mstudiomodel_t* pModel(int i) const { return (mstudiomodel_t*)(((byte*)this) + modelindex) + i; };
};


struct mstudiomouth_t
{
	int bone;
	Vector forward;
	int flexdesc;
};

struct mstudiohitboxset_t
{
	int sznameindex;
	inline char* const	pszName(void) const { return ((char*)this) + sznameindex; }
	int numhitboxes;
	int hitboxindex;
	inline mstudiobbox_t* pHitbox(int i) const { return (mstudiobbox_t*)(((byte*)this) + hitboxindex) + i; };
};

struct mstudiosrcbonetransform_t
{
	int sznameindex;
	inline const char* pszName(void) const { return ((char*)this) + sznameindex; }
	matrix3x4	pretransform;
	matrix3x4	posttransform;
};

class virtualgroup_t
{
public:
	virtualgroup_t(void) { cache = NULL; };
	void* cache;
	CUtlVector< int > boneMap;
	CUtlVector< int > masterBone;
	CUtlVector< int > masterSeq;
	CUtlVector< int > masterAnim;
	CUtlVector< int > masterAttachment;
	CUtlVector< int > masterPose;
	CUtlVector< int > masterNode;
};

struct virtualsequence_t
{
	int	flags;
	int activity;
	int group;
	int index;
};

struct virtualgeneric_t
{
	int group;
	int index;
};

struct virtualmodel_t;
struct thinModelVertices_t;

#define MODEL_VERTEX_FILE_ID (('V'<<24)+('S'<<16)+('D'<<8)+'I')
#define MODEL_VERTEX_FILE_VERSION 4
#define MODEL_VERTEX_FILE_THIN_ID (('V'<<24)+('C'<<16)+('D'<<8)+'I')

struct vertexFileHeader_t
{
	int id;
	int version;
	int checksum;
	int numLODs;
	int numLODVertexes[MAX_NUM_LODS];
	int numFixups;
	int fixupTableStart;
	int vertexDataStart;
	int tangentDataStart;

public:
	const mstudiovertex_t* GetVertexData() const
	{
		if ((id == MODEL_VERTEX_FILE_ID) && (vertexDataStart != 0))
			return (mstudiovertex_t*)(vertexDataStart + (byte*)this);
		else
			return NULL;
	}

	const Vector4D* GetTangentData() const
	{
		if ((id == MODEL_VERTEX_FILE_ID) && (tangentDataStart != 0))
			return (Vector4D*)(tangentDataStart + (byte*)this);
		else
			return NULL;
	}

	const thinModelVertices_t* GetThinVertexData() const
	{
		if ((id == MODEL_VERTEX_FILE_THIN_ID) && (vertexDataStart != 0))
			return (thinModelVertices_t*)(vertexDataStart + (byte*)this);
		else
			return NULL;
	}
};

struct vertexFileFixup_t
{
	int lod;
	int sourceVertexID;
	int numVertexes;
};

#define STUDIOHDR_FLAGS_AUTOGENERATED_HITBOX 0x00000001
#define STUDIOHDR_FLAGS_USES_ENV_CUBEMAP 0x00000002
#define STUDIOHDR_FLAGS_FORCE_OPAQUE 0x00000004
#define STUDIOHDR_FLAGS_TRANSLUCENT_TWOPASS 0x00000008
#define STUDIOHDR_FLAGS_STATIC_PROP 0x00000010
#define STUDIOHDR_FLAGS_USES_FB_TEXTURE 0x00000020
#define STUDIOHDR_FLAGS_HASSHADOWLOD 0x00000040
#define STUDIOHDR_FLAGS_USES_BUMPMAPPING 0x00000080
#define STUDIOHDR_FLAGS_USE_SHADOWLOD_MATERIALS 0x00000100
#define STUDIOHDR_FLAGS_OBSOLETE 0x00000200
#define STUDIOHDR_FLAGS_UNUSED 0x00000400
#define STUDIOHDR_FLAGS_NO_FORCED_FADE 0x00000800
#define STUDIOHDR_FLAGS_FORCE_PHONEME_CROSSFADE 0x00001000
#define STUDIOHDR_FLAGS_CONSTANT_DIRECTIONAL_LIGHT_DOT 0x00002000
#define STUDIOHDR_FLAGS_FLEXES_CONVERTED 0x00004000
#define STUDIOHDR_FLAGS_BUILT_IN_PREVIEW_MODE 0x00008000
#define STUDIOHDR_FLAGS_AMBIENT_BOOST 0x00010000
#define STUDIOHDR_FLAGS_DO_NOT_CAST_SHADOWS 0x00020000
#define STUDIOHDR_FLAGS_CAST_TEXTURE_SHADOWS 0x00040000
#define STUDIOHDR_FLAGS_VERT_ANIM_FIXED_POINT_SCALE 0x00200000

struct studiohdr2_t
{
	int numsrcbonetransform;
	int srcbonetransformindex;
	int	illumpositionattachmentindex;
	inline int IllumPositionAttachmentIndex() const { return illumpositionattachmentindex; }
	float flMaxEyeDeflection;
	inline float MaxEyeDeflection() const { return flMaxEyeDeflection != 0.0f ? flMaxEyeDeflection : 0.866f; }
	int linearboneindex;
	inline mstudiolinearbone_t* pLinearBones() const { return (linearboneindex) ? (mstudiolinearbone_t*)(((byte*)this) + linearboneindex) : NULL; }
	int sznameindex;
	inline char* pszName() { return (sznameindex) ? (char*)(((byte*)this) + sznameindex) : NULL; }
	int m_nBoneFlexDriverCount;
	int m_nBoneFlexDriverIndex;
	inline mstudioboneflexdriver_t* pBoneFlexDriver(int i) const { return (mstudioboneflexdriver_t*)(((byte*)this) + m_nBoneFlexDriverIndex) + i; }
	int reserved[56];
};

struct studiohdr_t
{
	int id;
	int version;
	int checksum;
	inline const char* pszName(void) const { if (studiohdr2index && pStudioHdr2()->pszName()) return pStudioHdr2()->pszName(); else return name; }
	char name[64];
	int length;
	Vector eyeposition;
	Vector illumposition;
	Vector hull_min;
	Vector hull_max;
	Vector view_bbmin;
	Vector view_bbmax;
	int flags;
	int numbones;
	int boneindex;
	inline mstudiobone_t* pBone(int i) const { return (mstudiobone_t*)(((byte*)this) + boneindex) + i; };
	int numbonecontrollers;
	int bonecontrollerindex;
	inline mstudiobonecontroller_t* pBonecontroller(int i) const { return (mstudiobonecontroller_t*)(((byte*)this) + bonecontrollerindex) + i; };
	int numhitboxsets;
	int hitboxsetindex;

	mstudiohitboxset_t* pHitboxSet(int i) const
	{
		return (mstudiohitboxset_t*)(((byte*)this) + hitboxsetindex) + i;
	};

	inline mstudiobbox_t* pHitbox(int i, int set) const
	{
		mstudiohitboxset_t const* s = pHitboxSet(set);
		if (!s)
			return NULL;

		return s->pHitbox(i);
	};

	inline int iHitboxCount(int set) const
	{
		mstudiohitboxset_t const* s = pHitboxSet(set);
		if (!s)
			return 0;

		return s->numhitboxes;
	};

	int numlocalanim;
	int localanimindex;
	inline mstudioanimdesc_t* pLocalAnimdesc(int i) const { if (i < 0 || i >= numlocalanim) i = 0; return (mstudioanimdesc_t*)(((byte*)this) + localanimindex) + i; };
	int numlocalseq;
	int localseqindex;
	inline mstudioseqdesc_t* pLocalSeqdesc(int i) const { if (i < 0 || i >= numlocalseq) i = 0; return (mstudioseqdesc_t*)(((byte*)this) + localseqindex) + i; };
	int activitylistversion;
	int eventsindexed;
	int numtextures;
	int textureindex;
	inline mstudiotexture_t* pTexture(int i) const { return (mstudiotexture_t*)(((byte*)this) + textureindex) + i; };
	int numcdtextures;
	int cdtextureindex;
	inline char* pCdtexture(int i) const { return (((char*)this) + *((int*)(((byte*)this) + cdtextureindex) + i)); };
	int numskinref;
	int numskinfamilies;
	int skinindex;
	inline short* pSkinref(int i) const { return (short*)(((byte*)this) + skinindex) + i; };
	int numbodyparts;
	int bodypartindex;
	inline mstudiobodyparts_t* pBodypart(int i) const { return (mstudiobodyparts_t*)(((byte*)this) + bodypartindex) + i; };
	int numlocalattachments;
	int localattachmentindex;
	inline mstudioattachment_t* pLocalAttachment(int i) const { return (mstudioattachment_t*)(((byte*)this) + localattachmentindex) + i; };
	int numlocalnodes;
	int localnodeindex;
	int localnodenameindex;
	inline char* pszLocalNodeName(int iNode) const { return (((char*)this) + *((int*)(((byte*)this) + localnodenameindex) + iNode)); }
	inline byte* pLocalTransition(int i) const { return (byte*)(((byte*)this) + localnodeindex) + i; };
	int numflexdesc;
	int flexdescindex;
	inline mstudioflexdesc_t* pFlexdesc(int i) const { return (mstudioflexdesc_t*)(((byte*)this) + flexdescindex) + i; };
	int numflexcontrollers;
	int flexcontrollerindex;
	inline mstudioflexcontroller_t* pFlexcontroller(LocalFlexController_t i) const { return (mstudioflexcontroller_t*)(((byte*)this) + flexcontrollerindex) + i; };
	int numflexrules;
	int flexruleindex;
	inline mstudioflexrule_t* pFlexRule(int i) const { return (mstudioflexrule_t*)(((byte*)this) + flexruleindex) + i; };
	int numikchains;
	int ikchainindex;
	inline mstudioikchain_t* pIKChain(int i) const { return (mstudioikchain_t*)(((byte*)this) + ikchainindex) + i; };
	int nummouths;
	int mouthindex;
	inline mstudiomouth_t* pMouth(int i) const { return (mstudiomouth_t*)(((byte*)this) + mouthindex) + i; };
	int numlocalposeparameters;
	int localposeparamindex;
	inline mstudioposeparamdesc_t* pLocalPoseParameter(int i) const { return (mstudioposeparamdesc_t*)(((byte*)this) + localposeparamindex) + i; };
	int surfacepropindex;
	inline char* const pszSurfaceProp(void) const { return ((char*)this) + surfacepropindex; }
	int keyvalueindex;
	int keyvaluesize;
	inline const char* KeyValueText(void) const { return keyvaluesize != 0 ? ((char*)this) + keyvalueindex : NULL; }
	int numlocalikautoplaylocks;
	int localikautoplaylockindex;
	inline mstudioiklock_t* pLocalIKAutoplayLock(int i) const { return (mstudioiklock_t*)(((byte*)this) + localikautoplaylockindex) + i; };
	float mass;
	int contents;
	int numincludemodels;
	int includemodelindex;
	inline mstudiomodelgroup_t* pModelGroup(int i) const { return (mstudiomodelgroup_t*)(((byte*)this) + includemodelindex) + i; };
	void* virtualModel;
	int szanimblocknameindex;
	inline char* const pszAnimBlockName(void) const { return ((char*)this) + szanimblocknameindex; }
	int numanimblocks;
	int animblockindex;
	inline mstudioanimblock_t* pAnimBlock(int i) const { return (mstudioanimblock_t*)(((byte*)this) + animblockindex) + i; };
	void* animblockModel;
	int bonetablebynameindex;
	inline const byte* GetBoneTableSortedByName() const { return (byte*)this + bonetablebynameindex; }
	void* pVertexBase;
	void* pIndexBase;
	byte constdirectionallightdot;
	byte rootLOD;
	byte numAllowedRootLODs;
	byte unused[1];
	int unused4;
	int numflexcontrollerui;
	int flexcontrolleruiindex;
	mstudioflexcontrollerui_t* pFlexControllerUI(int i) const { return (mstudioflexcontrollerui_t*)(((byte*)this) + flexcontrolleruiindex) + i; }
	float flVertAnimFixedPointScale;
	inline float VertAnimFixedPointScale() const { return (flags & STUDIOHDR_FLAGS_VERT_ANIM_FIXED_POINT_SCALE) ? flVertAnimFixedPointScale : 1.0f / 4096.0f; }
	int unused3[1];
	int studiohdr2index;
	studiohdr2_t* pStudioHdr2() const { return (studiohdr2_t*)(((byte*)this) + studiohdr2index); }
	int NumSrcBoneTransforms() const { return studiohdr2index ? pStudioHdr2()->numsrcbonetransform : 0; }
	const mstudiosrcbonetransform_t* SrcBoneTransform(int i) const { return (mstudiosrcbonetransform_t*)(((byte*)this) + pStudioHdr2()->srcbonetransformindex) + i; }
	inline int IllumPositionAttachmentIndex() const { return studiohdr2index ? pStudioHdr2()->IllumPositionAttachmentIndex() : 0; }
	inline float MaxEyeDeflection() const { return studiohdr2index ? pStudioHdr2()->MaxEyeDeflection() : 0.866f; }
	inline mstudiolinearbone_t* pLinearBones() const { return studiohdr2index ? pStudioHdr2()->pLinearBones() : NULL; }
	inline int BoneFlexDriverCount() const { return studiohdr2index ? pStudioHdr2()->m_nBoneFlexDriverCount : 0; }
	inline const mstudioboneflexdriver_t* BoneFlexDriver(int i) const { return studiohdr2index ? pStudioHdr2()->pBoneFlexDriver(i) : NULL; }
	int unused2[1];
};

class IDataCache;
class IMDLCache;
class CStudioHdr;

struct flexweight_t
{
	int key;
	float weight;
	float influence;
};

struct flexsetting_t
{
	int nameindex;

	inline char* pszName(void) const
	{
		return (char*)(((byte*)this) + nameindex);
	}

	int obsolete1;
	int numsettings;
	int index;
	int obsolete2;
	int settingindex;
};

struct flexsettinghdr_t
{
	int id;
	int version;
	inline const char* pszName(void) const { return name; }
	char name[64];
	int length;
	int numflexsettings;
	int flexsettingindex;
	inline flexsetting_t* pSetting(int i) const { return (flexsetting_t*)(((byte*)this) + flexsettingindex) + i; };
	int nameindex;
	int numindexes;
	int indexindex;

	inline flexsetting_t* pIndexedSetting(int index) const
	{
		if (index < 0 || index >= numindexes)
		{
			return NULL;
		}

		int i = *((int*)(((byte*)this) + indexindex) + index);

		if (i == -1)
		{
			return NULL;
		}

		return pSetting(i);
	}

	int numkeys;
	int keynameindex;
	inline char* pLocalName(int i) const { return (char*)(((byte*)this) + *((int*)(((byte*)this) + keynameindex) + i)); };
	int keymappingindex;
	inline int* pLocalToGlobal(int i) const { return (int*)(((byte*)this) + keymappingindex) + i; };
	inline int LocalToGlobal(int i) const { return *pLocalToGlobal(i); };
};

#define STUDIO_CONST 1
#define STUDIO_FETCH1 2
#define STUDIO_FETCH2 3
#define STUDIO_ADD 4
#define STUDIO_SUB 5
#define STUDIO_MUL 6
#define STUDIO_DIV 7
#define STUDIO_NEG 8
#define STUDIO_EXP 9
#define STUDIO_OPEN	10
#define STUDIO_CLOSE 11
#define STUDIO_COMMA 12
#define STUDIO_MAX 13
#define STUDIO_MIN 14
#define STUDIO_2WAY_0 15
#define STUDIO_2WAY_1 16
#define STUDIO_NWAY 17
#define STUDIO_COMBO 18
#define STUDIO_DOMINATE	19
#define STUDIO_DME_LOWER_EYELID 20
#define STUDIO_DME_UPPER_EYELID 21
#define STUDIO_X 0x00000001
#define STUDIO_Y 0x00000002	
#define STUDIO_Z 0x00000004
#define STUDIO_XR 0x00000008
#define STUDIO_YR 0x00000010
#define STUDIO_ZR 0x00000020
#define STUDIO_LX 0x00000040
#define STUDIO_LY 0x00000080
#define STUDIO_LZ 0x00000100
#define STUDIO_LXR 0x00000200
#define STUDIO_LYR 0x00000400
#define STUDIO_LZR 0x00000800
#define STUDIO_LINEAR 0x00001000
#define STUDIO_TYPES 0x0003FFFF
#define STUDIO_RLOOP 0x00040000
#define STUDIO_LOOPING 0x0001
#define STUDIO_SNAP	0x0002
#define STUDIO_DELTA 0x0004
#define STUDIO_AUTOPLAY	0x0008
#define STUDIO_POST 0x0010
#define STUDIO_ALLZEROS	0x0020
#define STUDIO_CYCLEPOSE 0x0080
#define STUDIO_REALTIME	0x0100
#define STUDIO_LOCAL 0x0200
#define STUDIO_HIDDEN 0x0400
#define STUDIO_OVERRIDE	0x0800
#define STUDIO_ACTIVITY	0x1000
#define STUDIO_EVENT 0x2000
#define STUDIO_WORLD 0x4000
#define STUDIO_AL_POST 0x0010
#define STUDIO_AL_SPLINE 0x0040
#define STUDIO_AL_XFADE 0x0080
#define STUDIO_AL_NOBLEND 0x0200
#define STUDIO_AL_LOCAL 0x1000
#define STUDIO_AL_POSE 0x4000