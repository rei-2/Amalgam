#pragma once

struct inputdata_t;

#define INVALID_TIME (FLT_MAX * -1.0)

typedef enum _fieldtypes
{
	FIELD_VOID = 0,
	FIELD_FLOAT,
	FIELD_STRING,
	FIELD_VECTOR,
	FIELD_QUATERNION,
	FIELD_INTEGER,
	FIELD_BOOLEAN,
	FIELD_SHORT,
	FIELD_CHARACTER,
	FIELD_COLOR32,
	FIELD_EMBEDDED,
	FIELD_CUSTOM,
	FIELD_CLASSPTR,
	FIELD_EHANDLE,
	FIELD_EDICT,
	FIELD_POSITION_VECTOR,
	FIELD_TIME,
	FIELD_TICK,
	FIELD_MODELNAME,
	FIELD_SOUNDNAME,
	FIELD_INPUT,
	FIELD_FUNCTION,
	FIELD_VMATRIX,
	FIELD_VMATRIX_WORLDSPACE,
	FIELD_MATRIX3X4_WORLDSPACE,
	FIELD_INTERVAL,
	FIELD_MODELINDEX,
	FIELD_MATERIALINDEX,
	FIELD_VECTOR2D,
	FIELD_TYPECOUNT
} fieldtype_t;

template <int FIELD_TYPE>
class CDatamapFieldSizeDeducer
{
public:
	enum
	{
		SIZE = 0
	};

	static int FieldSize()
	{
		return 0;
	}
};

#define FIELD_SIZE( _fieldType ) CDatamapFieldSizeDeducer<_fieldType>::SIZE
#define FIELD_BITS( _fieldType ) (FIELD_SIZE( _fieldType ) * 8)

#define ARRAYSIZE2D(p) (sizeof(p)/sizeof(p[0][0]))
#define SIZE_OF_ARRAY(p) _ARRAYSIZE(p)

#define _hacky_datamap_offsetof(s,m)	((size_t)&(((s *)0)->m))

#define FTYPEDESC_GLOBAL 0x0001
#define FTYPEDESC_SAVE 0x0002
#define FTYPEDESC_KEY 0x0004
#define FTYPEDESC_INPUT 0x0008
#define FTYPEDESC_OUTPUT 0x0010
#define FTYPEDESC_FUNCTIONTABLE 0x0020
#define FTYPEDESC_PTR 0x0040
#define FTYPEDESC_OVERRIDE 0x0080
#define FTYPEDESC_INSENDTABLE 0x0100
#define FTYPEDESC_PRIVATE 0x0200
#define FTYPEDESC_NOERRORCHECK 0x0400
#define FTYPEDESC_MODELINDEX 0x0800
#define FTYPEDESC_INDEX 0x1000
#define FTYPEDESC_VIEW_OTHER_PLAYER 0x2000
#define FTYPEDESC_VIEW_OWN_TEAM 0x4000
#define FTYPEDESC_VIEW_NEVER 0x8000
#define TD_MSECTOLERANCE 0.001f

struct typedescription_t;
class ISaveRestoreOps;

typedef void (*inputfunc_t)(inputdata_t& data);

struct datamap_t;
struct typedescription_t;

enum
{
	TD_OFFSET_NORMAL = 0,
	TD_OFFSET_PACKED = 1,
	TD_OFFSET_COUNT
};

struct typedescription_t
{
	fieldtype_t fieldType;
	const char* fieldName;
	int fieldOffset[TD_OFFSET_COUNT];
	unsigned short fieldSize;
	short flags;
	const char* externalName;
	ISaveRestoreOps* pSaveRestoreOps;
	inputfunc_t inputFunc;
	datamap_t* td;
	int fieldSizeInBytes;
	struct typedescription_t* override_field;
	int override_count;
	float fieldTolerance;
};

struct datamap_t
{
	typedescription_t* dataDesc;
	int dataNumFields;
	char const* dataClassName;
	datamap_t* baseMap;
	bool chains_validated;
	bool packed_offsets_computed;
	int packed_size;
};