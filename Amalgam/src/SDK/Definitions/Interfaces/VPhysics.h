#pragma once
#include "../Misc/IAppSystem.h"
#include "../Types.h"

#define k_flMaxVelocity 2000.0f
#define k_flMaxAngularVelocity 360.0f * 10.0f
#define DEFAULT_MIN_FRICTION_MASS 10.0f
#define DEFAULT_MAX_FRICTION_MASS 2500.0f

struct vcollide_t;

enum
{
	COLLIDE_POLY = 0,
	COLLIDE_MOPP = 1,
	COLLIDE_BALL = 2,
	COLLIDE_VIRTUAL = 3,
};

class IPhysCollide
{
public:
	virtual ~IPhysCollide() {}
	//virtual void AddReference() = 0;
	//virtual void ReleaseReference() = 0;

	// get a surface manager 
	virtual void* CreateSurfaceManager(short&) const = 0;
	virtual void GetAllLedges(void* ledges) const = 0;
	virtual unsigned int GetSerializationSize() const = 0;
	virtual unsigned int SerializeToBuffer(char* pDest, bool bSwap = false) const = 0;
	virtual int GetVCollideIndex() const = 0;
	virtual Vec3 GetMassCenter() const = 0;
	virtual void SetMassCenter(const Vec3& massCenter) = 0;
	virtual Vec3 GetOrthographicAreas() const = 0;
	virtual void SetOrthographicAreas(const Vec3& areas) = 0;
	virtual float GetSphereRadius() const = 0;
	virtual void OutputDebugInfo() const = 0;
};

#define LEAFMAP_HAS_CUBEMAP					0x0001
#define LEAFMAP_HAS_SINGLE_VERTEX_SPAN		0x0002
#define LEAFMAP_HAS_MULTIPLE_VERTEX_SPANS	0x0004
struct leafmap_t
{
	void* pLeaf;
	unsigned short vertCount;
	byte	flags;
	byte	spanCount;
	unsigned short startVert[8];

	void SetHasCubemap()
	{
		flags = LEAFMAP_HAS_CUBEMAP;
	}

	void SetSingleVertexSpan(int startVertIndex, int vertCountIn)
	{
		flags = 0;
		flags |= LEAFMAP_HAS_SINGLE_VERTEX_SPAN;
		startVert[0] = startVertIndex;
		vertCount = vertCountIn;
	}

	int MaxSpans()
	{
		return sizeof(startVert) - sizeof(startVert[0]);
	}
	const byte* GetSpans() const
	{
		return reinterpret_cast<const byte*>(&startVert[1]);
	}
	byte* GetSpans()
	{
		return reinterpret_cast<byte*>(&startVert[1]);
	}

	void SetRLESpans(int startVertIndex, int spanCountIn, byte* pSpans)
	{
		flags = 0;
		if (spanCountIn > MaxSpans())
			return;
		if (spanCountIn == 1)
		{
			SetSingleVertexSpan(startVertIndex, pSpans[0]);
			return;
		}
		// write out a run length encoded list of verts to include in this model
		flags |= LEAFMAP_HAS_MULTIPLE_VERTEX_SPANS;
		startVert[0] = startVertIndex;
		vertCount = 0;
		spanCount = spanCountIn;
		byte* pSpanOut = GetSpans();
		for (int i = 0; i < spanCountIn; i++)
		{
			pSpanOut[i] = pSpans[i];
			if (!(i & 1))
			{
				vertCount += pSpans[i];
			}
		}
	}

	inline bool HasSpans() const { return (flags & (LEAFMAP_HAS_SINGLE_VERTEX_SPAN | LEAFMAP_HAS_MULTIPLE_VERTEX_SPANS)) ? true : false; }
	inline bool HasCubemap() const { return (flags & LEAFMAP_HAS_CUBEMAP) ? true : false; }
	inline bool HasSingleVertexSpan() const { return (flags & LEAFMAP_HAS_SINGLE_VERTEX_SPAN) ? true : false; }
	inline bool HasRLESpans() const { return (flags & LEAFMAP_HAS_MULTIPLE_VERTEX_SPANS) ? true : false; }
};


struct collidemap_t
{
	int				leafCount;
	leafmap_t		leafmap[1];
};

class CPhysCollide : public IPhysCollide
{
public:
	static CPhysCollide* UnserializeFromBuffer(const char* pBuffer, unsigned int size, int index, bool swap = false);
	virtual const void* GetCompactSurface() const { return NULL; }
	virtual Vec3 GetOrthographicAreas() const { return Vec3(1, 1, 1); }
	virtual float GetSphereRadius() const { return 0; }
	virtual void ComputeOrthographicAreas(float epsilon) {}
	virtual void SetOrthographicAreas(const Vec3& areas) {}
	virtual const collidemap_t* GetCollideMap() const { return NULL; }
};

// ------------------------------------------------------------------------------------
// UNITS:
// ------------------------------------------------------------------------------------
// NOTE:  Coordinates are in HL units.  1 unit == 1 inch.  X is east (forward), Y is north (left), Z is up (up)
// QAngle are pitch (around y), Yaw (around Z), Roll (around X)
// AngularImpulse are exponetial maps (an axis in HL units scaled by a "twist" angle in degrees)
//		They can be transformed like normals/covectors and added linearly
// mass is kg, volume is in^3, acceleration is in/s^2, velocity is in/s

// density is kg/m^3 (water ~= 998 at room temperature)
// preferably, these would be in kg/in^3, but the range of those numbers makes them not very human readable
// having water be about 1000 is really convenient for data entry.
// Since volume is in in^3 and density is in kg/m^3: 
//	density = (mass / volume) * CUBIC_METERS_PER_CUBIC_INCH
// Force is applied using impulses (kg*in/s)
// Torque is applied using impulses (kg*degrees/s)
// ------------------------------------------------------------------------------------

#define METERS_PER_INCH					(0.0254f)
#define CUBIC_METERS_PER_CUBIC_INCH		(METERS_PER_INCH*METERS_PER_INCH*METERS_PER_INCH)
// 2.2 lbs / kg
#define POUNDS_PER_KG	(2.2f)
#define KG_PER_POUND	(1.0f/POUNDS_PER_KG)

// convert from pounds to kg
#define lbs2kg(x)		((x)*KG_PER_POUND)
#define kg2lbs(x)		((x)*POUNDS_PER_KG)

const float VPHYSICS_MIN_MASS = 0.1f;
const float VPHYSICS_MAX_MASS = 5e4f;

class IPhysicsShadowController;
class IPhysicsObject
{
public:
	virtual ~IPhysicsObject(void) {}
	virtual bool IsStatic() const = 0;
	virtual bool IsAsleep() const = 0;
	virtual bool IsTrigger() const = 0;
	virtual bool IsFluid() const = 0;
	virtual bool IsHinged() const = 0;
	virtual bool IsCollisionEnabled() const = 0;
	virtual bool IsGravityEnabled() const = 0;
	virtual bool IsDragEnabled() const = 0;
	virtual bool IsMotionEnabled() const = 0;
	virtual bool IsMoveable() const = 0;
	virtual bool IsAttachedToConstraint(bool bExternalOnly) const = 0;
	virtual void EnableCollisions(bool enable) = 0;
	virtual void EnableGravity(bool enable) = 0;
	virtual void EnableDrag(bool enable) = 0;
	virtual void EnableMotion(bool enable) = 0;
	virtual void SetGameData(void* pGameData) = 0;
	virtual void* GetGameData(void) const = 0;
	virtual void SetGameFlags(unsigned short userFlags) = 0;
	virtual unsigned short GetGameFlags(void) const = 0;
	virtual void SetGameIndex(unsigned short gameIndex) = 0;
	virtual unsigned short GetGameIndex(void) const = 0;
	virtual void SetCallbackFlags(unsigned short callbackflags) = 0;
	virtual unsigned short GetCallbackFlags(void) const = 0;
	virtual void Wake(void) = 0;
	virtual void Sleep(void) = 0;
	virtual void RecheckCollisionFilter() = 0;
	virtual void RecheckContactPoints() = 0;
	virtual void SetMass(float mass) = 0;
	virtual float GetMass(void) const = 0;
	virtual float GetInvMass(void) const = 0;
	virtual Vec3 GetInertia(void) const = 0;
	virtual Vec3 GetInvInertia(void) const = 0;
	virtual void SetInertia(const Vec3& inertia) = 0;
	virtual void SetDamping(const float* speed, const float* rot) = 0;
	virtual void GetDamping(float* speed, float* rot) const = 0;
	virtual void SetDragCoefficient(float* pDrag, float* pAngularDrag) = 0;
	virtual void SetBuoyancyRatio(float ratio) = 0;
	virtual int GetMaterialIndex() const = 0;
	virtual void SetMaterialIndex(int materialIndex) = 0;
	virtual unsigned int GetContents() const = 0;
	virtual void SetContents(unsigned int contents) = 0;
	virtual float GetSphereRadius() const = 0;
	virtual float GetEnergy() const = 0;
	virtual Vec3 GetMassCenterLocalSpace() const = 0;
	virtual void SetPosition(const Vec3& worldPosition, const QAngle& angles, bool isTeleport) = 0;
	virtual void SetPositionMatrix(const matrix3x4& matrix, bool isTeleport) = 0;
	virtual void GetPosition(Vec3* worldPosition, QAngle* angles) const = 0;
	virtual void GetPositionMatrix(matrix3x4* positionMatrix) const = 0;
	virtual void SetVelocity(const Vec3* velocity, const Vec3* angularVelocity) = 0;
	virtual void SetVelocityInstantaneous(const Vec3* velocity, const Vec3* angularVelocity) = 0;
	virtual void GetVelocity(Vec3* velocity, Vec3* angularVelocity) const = 0;
	virtual void AddVelocity(const Vec3* velocity, const Vec3* angularVelocity) = 0;
	virtual void GetVelocityAtPoint(const Vec3& worldPosition, Vec3* pVelocity) const = 0;
	virtual void GetImplicitVelocity(Vec3* velocity, Vec3* angularVelocity) const = 0;
	virtual void LocalToWorld(Vec3* worldPosition, const Vec3& localPosition) const = 0;
	virtual void WorldToLocal(Vec3* localPosition, const Vec3& worldPosition) const = 0;
	virtual void LocalToWorldVector(Vec3* worldVector, const Vec3& localVector) const = 0;
	virtual void WorldToLocalVector(Vec3* localVector, const Vec3& worldVector) const = 0;
	virtual void ApplyForceCenter(const Vec3& forceVector) = 0;
	virtual void ApplyForceOffset(const Vec3& forceVector, const Vec3& worldPosition) = 0;
	virtual void ApplyTorqueCenter(const Vec3& torque) = 0;
	virtual void CalculateForceOffset(const Vec3& forceVector, const Vec3& worldPosition, Vec3* centerForce, Vec3* centerTorque) const = 0;
	virtual void CalculateVelocityOffset(const Vec3& forceVector, const Vec3& worldPosition, Vec3* centerVelocity, Vec3* centerAngularVelocity) const = 0;
	virtual float CalculateLinearDrag(const Vec3& unitDirection) const = 0;
	virtual float CalculateAngularDrag(const Vec3& objectSpaceRotationAxis) const = 0;
	virtual bool GetContactPoint(Vec3* contactPoint, IPhysicsObject** contactObject) const = 0;
	virtual void SetShadow(float maxSpeed, float maxAngularSpeed, bool allowPhysicsMovement, bool allowPhysicsRotation) = 0;
	virtual void UpdateShadow(const Vec3& targetPosition, const QAngle& targetAngles, bool tempDisableGravity, float timeOffset) = 0;
	virtual int GetShadowPosition(Vec3* position, QAngle* angles) const = 0;
	virtual /*IPhysicsShadowController*/void* GetShadowController(void) const = 0;
	virtual void RemoveShadowController() = 0;
	virtual float ComputeShadowControl(/*const hlshadowcontrol_params_t &*/void* params, float secondsToArrival, float dt) = 0;
	virtual const CPhysCollide* GetCollide(void) const = 0;
	virtual const char* GetName() const = 0;
	virtual void BecomeTrigger() = 0;
	virtual void RemoveTrigger() = 0;
	virtual void BecomeHinged(int localAxis) = 0;
	virtual void RemoveHinged() = 0;
	virtual /*IPhysicsFrictionSnapshot*/void* CreateFrictionSnapshot() = 0;
	virtual void DestroyFrictionSnapshot(/*IPhysicsFrictionSnapshot*/void* pSnapshot) = 0;
	virtual void OutputDebugInfo() const = 0;

	void* m_pGameData;
	void* m_pObject;
	const CPhysCollide* m_pCollide;
	IPhysicsShadowController* m_pShadow;

	Vec3			m_dragBasis;
	Vec3			m_angDragBasis;

	// these 5 should pack into a short
	// pack new bools here
	bool			m_shadowTempGravityDisable : 5;
	bool			m_hasTouchedDynamic : 1;
	bool			m_asleepSinceCreation : 1;
	bool			m_forceSilentDelete : 1;
	unsigned char	m_sleepState : 2;
	unsigned char	m_hingedAxis : 3;
	unsigned char	m_collideType : 3;
	unsigned short	m_gameIndex;

private:
	unsigned short	m_materialIndex;
	unsigned short	m_activeIndex;

	unsigned short	m_callbacks;
	unsigned short	m_gameFlags;
	unsigned int	m_contentsMask;

	float			m_volume;
	float			m_buoyancyRatio;
	float			m_dragCoefficient;
	float			m_angDragCoefficient;
};
class IPhysicsEnvironment;
class IPhysicsSurfaceProps;
class IPhysicsConstraint;
class IPhysicsConstraintGroup;
class IPhysicsFluidController;
class IPhysicsSpring;
class IPhysicsVehicleController;
class IConvexInfo;
class IPhysicsObjectPairHash;
class IPhysicsCollisionSet;
class IPhysicsPlayerController;
class IPhysicsFrictionSnapshot;

struct Ray_t;
struct constraint_ragdollparams_t;
struct constraint_hingeparams_t;
struct constraint_fixedparams_t;
struct constraint_ballsocketparams_t;
struct constraint_slidingparams_t;
struct constraint_pulleyparams_t;
struct constraint_lengthparams_t;
struct constraint_groupparams_t;

struct vehicleparams_t;

struct fluidparams_t;
struct springparams_t;
struct objectparams_t
{
	Vec3* massCenterOverride;
	float		mass;
	float		inertia;
	float		damping;
	float		rotdamping;
	float		rotInertiaLimit;
	const char* pName;				// used only for debugging
	void* pGameData;
	float		volume;
	float		dragCoefficient;
	bool		enableCollisions;
};
struct debugcollide_t;
class CGameTrace;
typedef CGameTrace trace_t;
struct physics_stats_t;
struct physics_performanceparams_t
{
	int		maxCollisionsPerObjectPerTimestep;		// object will be frozen after this many collisions (visual hitching vs. CPU cost)
	int		maxCollisionChecksPerTimestep;			// objects may penetrate after this many collision checks (can be extended in AdditionalCollisionChecksThisTick)
	float	maxVelocity;							// limit world space linear velocity to this (in / s)
	float	maxAngularVelocity;						// limit world space angular velocity to this (degrees / s)
	float	lookAheadTimeObjectsVsWorld;			// predict collisions this far (seconds) into the future
	float	lookAheadTimeObjectsVsObject;			// predict collisions this far (seconds) into the future
	float	minFrictionMass;						// min mass for friction solves (constrains dynamic range of mass to improve stability)
	float	maxFrictionMass;						// mas mass for friction solves

	void Defaults()
	{
		maxCollisionsPerObjectPerTimestep = 6;
		maxCollisionChecksPerTimestep = 250;
		maxVelocity = k_flMaxVelocity;
		maxAngularVelocity = k_flMaxAngularVelocity;
		lookAheadTimeObjectsVsWorld = 1.0f;
		lookAheadTimeObjectsVsObject = 0.5f;
		minFrictionMass = DEFAULT_MIN_FRICTION_MASS;
		maxFrictionMass = DEFAULT_MAX_FRICTION_MASS;
	}
};
struct virtualmeshparams_t;

//enum PhysInterfaceId_t;
struct physsaveparams_t;
struct physrestoreparams_t;
struct physprerestoreparams_t;

enum PhysInterfaceId_t
{
	PIID_UNKNOWN,
	PIID_IPHYSICSOBJECT,
	PIID_IPHYSICSFLUIDCONTROLLER,
	PIID_IPHYSICSSPRING,
	PIID_IPHYSICSCONSTRAINTGROUP,
	PIID_IPHYSICSCONSTRAINT,
	PIID_IPHYSICSSHADOWCONTROLLER,
	PIID_IPHYSICSPLAYERCONTROLLER,
	PIID_IPHYSICSMOTIONCONTROLLER,
	PIID_IPHYSICSVEHICLECONTROLLER,
	PIID_IPHYSICSGAMETRACE,

	PIID_NUM_TYPES
};


class ISave;
class IRestore;


#define VPHYSICS_DEBUG_OVERLAY_INTERFACE_VERSION	"VPhysicsDebugOverlay001"

class IVPhysicsDebugOverlay
{
public:
	virtual void AddEntityTextOverlay(int ent_index, int line_offset, float duration, int r, int g, int b, int a, const char* format, ...) = 0;
	virtual void AddBoxOverlay(const Vec3& origin, const Vec3& mins, const Vec3& max, QAngle const& orientation, int r, int g, int b, int a, float duration) = 0;
	virtual void AddTriangleOverlay(const Vec3& p1, const Vec3& p2, const Vec3& p3, int r, int g, int b, int a, bool noDepthTest, float duration) = 0;
	virtual void AddLineOverlay(const Vec3& origin, const Vec3& dest, int r, int g, int b, bool noDepthTest, float duration) = 0;
	virtual void AddTextOverlay(const Vec3& origin, float duration, const char* format, ...) = 0;
	virtual void AddTextOverlay(const Vec3& origin, int line_offset, float duration, const char* format, ...) = 0;
	virtual void AddScreenTextOverlay(float flXPos, float flYPos, float flDuration, int r, int g, int b, int a, const char* text) = 0;
	virtual void AddSweptBoxOverlay(const Vec3& start, const Vec3& end, const Vec3& mins, const Vec3& max, const QAngle& angles, int r, int g, int b, int a, float flDuration) = 0;
	virtual void AddTextOverlayRGB(const Vec3& origin, int line_offset, float duration, float r, float g, float b, float alpha, const char* format, ...) = 0;
};

class IPhysics : public IAppSystem
{
public:
	virtual IPhysicsEnvironment* CreateEnvironment(void) = 0;
	virtual void DestroyEnvironment(IPhysicsEnvironment*) = 0;
	virtual IPhysicsEnvironment* GetActiveEnvironmentByIndex(int index) = 0;

	// Creates a fast hash of pairs of objects
	// Useful for maintaining a table of object relationships like pairs that do not collide.
	virtual IPhysicsObjectPairHash* CreateObjectPairHash() = 0;
	virtual void						DestroyObjectPairHash(IPhysicsObjectPairHash* pHash) = 0;

	// holds a cache of these by id.  So you can get by id to search for the previously created set
	// UNDONE: Sets are currently limited to 32 elements.  More elements will return NULL in create.
	// NOTE: id is not allowed to be zero.
	virtual IPhysicsCollisionSet* FindOrCreateCollisionSet(unsigned int id, int maxElementCount) = 0;
	virtual IPhysicsCollisionSet* FindCollisionSet(unsigned int id) = 0;
	virtual void						DestroyAllCollisionSets() = 0;
};


// CPhysConvex is a single convex solid
class CPhysConvex;
// CPhysPolysoup is an abstract triangle soup mesh
class CPhysPolysoup;
class ICollisionQuery;
class IVPhysicsKeyParser;
struct convertconvexparams_t;
class CPackedPhysicsDescription;

class CPolyhedron;

// UNDONE: Find a better place for this?  Should be in collisionutils, but it's needs VPHYSICS' solver.
struct truncatedcone_t
{
	Vec3	origin;
	Vec3	normal;
	float	h;			// height of the cone (hl units)
	float	theta;		// cone angle (degrees)
};


#define VPHYSICS_COLLISION_INTERFACE_VERSION	"VPhysicsCollision007"

class IPhysicsCollision
{
public:
	virtual ~IPhysicsCollision(void) {}

	// produce a convex element from verts (convex hull around verts)
	virtual CPhysConvex* ConvexFromVerts(Vec3** pVerts, int vertCount) = 0;
	// produce a convex element from planes (csg of planes)
	virtual CPhysConvex* ConvexFromPlanes(float* pPlanes, int planeCount, float mergeDistance) = 0;
	// calculate volume of a convex element
	virtual float			ConvexVolume(CPhysConvex* pConvex) = 0;

	virtual float			ConvexSurfaceArea(CPhysConvex* pConvex) = 0;
	// store game-specific data in a convex solid
	virtual void			SetConvexGameData(CPhysConvex* pConvex, unsigned int gameData) = 0;
	// If not converted, free the convex elements with this call
	virtual void			ConvexFree(CPhysConvex* pConvex) = 0;
	virtual CPhysConvex* BBoxToConvex(const Vec3& mins, const Vec3& maxs) = 0;
	// produce a convex element from a convex polyhedron
	virtual CPhysConvex* ConvexFromConvexPolyhedron(const CPolyhedron& ConvexPolyhedron) = 0;
	// produce a set of convex triangles from a convex polygon, normal is assumed to be on the side with forward point ordering, which should be clockwise, output will need to be able to hold exactly (iPointCount-2) convexes
	virtual void			ConvexesFromConvexPolygon(const Vec3& vPolyNormal, const Vec3* pPoints, int iPointCount, CPhysConvex** pOutput) = 0;

	// concave objects
	// create a triangle soup
	virtual CPhysPolysoup* PolysoupCreate(void) = 0;
	// destroy the container and memory
	virtual void			PolysoupDestroy(CPhysPolysoup* pSoup) = 0;
	// add a triangle to the soup
	virtual void			PolysoupAddTriangle(CPhysPolysoup* pSoup, const Vec3& a, const Vec3& b, const Vec3& c, int materialIndex7bits) = 0;
	// convert the convex into a compiled collision model
	virtual CPhysCollide* ConvertPolysoupToCollide(CPhysPolysoup* pSoup, bool useMOPP) = 0;

	// Convert an array of convex elements to a compiled collision model (this deletes the convex elements)
	virtual CPhysCollide* ConvertConvexToCollide(CPhysConvex** pConvex, int convexCount) = 0;
	virtual CPhysCollide* ConvertConvexToCollideParams(CPhysConvex** pConvex, int convexCount, const convertconvexparams_t& convertParams) = 0;
	// Free a collide that was created with ConvertConvexToCollide()
	virtual void			DestroyCollide(CPhysCollide* pCollide) = 0;

	// Get the memory size in bytes of the collision model for serialization
	virtual int				CollideSize(CPhysCollide* pCollide) = 0;
	// serialize the collide to a block of memory
	virtual int				CollideWrite(char* pDest, CPhysCollide* pCollide, bool bSwap = false) = 0;
	// unserialize the collide from a block of memory
	virtual CPhysCollide* UnserializeCollide(char* pBuffer, int size, int index) = 0;

	// compute the volume of a collide
	virtual float			CollideVolume(CPhysCollide* pCollide) = 0;
	// compute surface area for tools
	virtual float			CollideSurfaceArea(CPhysCollide* pCollide) = 0;

	// Get the support map for a collide in the given direction
	virtual Vec3			CollideGetExtent(const CPhysCollide* pCollide, const Vec3& collideOrigin, const QAngle& collideAngles, const Vec3& direction) = 0;

	// Get an AABB for an oriented collision model
	virtual void			CollideGetAABB(Vec3* pMins, Vec3* pMaxs, const CPhysCollide* pCollide, const Vec3& collideOrigin, const QAngle& collideAngles) = 0;

	virtual void			CollideGetMassCenter(CPhysCollide* pCollide, Vec3* pOutMassCenter) = 0;
	virtual void			CollideSetMassCenter(CPhysCollide* pCollide, const Vec3& massCenter) = 0;
	// get the approximate cross-sectional area projected orthographically on the bbox of the collide
	// NOTE: These are fractional areas - unitless.  Basically this is the fraction of the OBB on each axis that
	// would be visible if the object were rendered orthographically.
	// NOTE: This has been precomputed when the collide was built or this function will return 1,1,1
	virtual Vec3			CollideGetOrthographicAreas(const CPhysCollide* pCollide) = 0;
	virtual void			CollideSetOrthographicAreas(CPhysCollide* pCollide, const Vec3& areas) = 0;

	// query the vcollide index in the physics model for the instance
	virtual int				CollideIndex(const CPhysCollide* pCollide) = 0;

	// Convert a bbox to a collide
	virtual CPhysCollide* BBoxToCollide(const Vec3& mins, const Vec3& maxs) = 0;
	virtual int				GetConvexesUsedInCollideable(const CPhysCollide* pCollideable, CPhysConvex** pOutputArray, int iOutputArrayLimit) = 0;


	// Trace an AABB against a collide
	virtual void TraceBox(const Vec3& start, const Vec3& end, const Vec3& mins, const Vec3& maxs, const CPhysCollide* pCollide, const Vec3& collideOrigin, const QAngle& collideAngles, trace_t* ptr) = 0;
	virtual void TraceBox(const Ray_t& ray, const CPhysCollide* pCollide, const Vec3& collideOrigin, const QAngle& collideAngles, trace_t* ptr) = 0;
	virtual void TraceBox(const Ray_t& ray, unsigned int contentsMask, IConvexInfo* pConvexInfo, const CPhysCollide* pCollide, const Vec3& collideOrigin, const QAngle& collideAngles, trace_t* ptr) = 0;

	// Trace one collide against another
	virtual void TraceCollide(const Vec3& start, const Vec3& end, const CPhysCollide* pSweepCollide, const QAngle& sweepAngles, const CPhysCollide* pCollide, const Vec3& collideOrigin, const QAngle& collideAngles, trace_t* ptr) = 0;

	// relatively slow test for box vs. truncated cone
	virtual bool			IsBoxIntersectingCone(const Vec3& boxAbsMins, const Vec3& boxAbsMaxs, const truncatedcone_t& cone) = 0;

	// loads a set of solids into a vcollide_t
	virtual void			VCollideLoad(vcollide_t* pOutput, int solidCount, const char* pBuffer, int size, bool swap = false) = 0;
	// destroyts the set of solids created by VCollideLoad
	virtual void			VCollideUnload(vcollide_t* pVCollide) = 0;

	// begins parsing a vcollide.  NOTE: This keeps pointers to the text
	// If you free the text and call members of IVPhysicsKeyParser, it will crash
	virtual IVPhysicsKeyParser* VPhysicsKeyParserCreate(const char* pKeyData) = 0;
	// Free the parser created by VPhysicsKeyParserCreate
	virtual void			VPhysicsKeyParserDestroy(IVPhysicsKeyParser* pParser) = 0;

	// creates a list of verts from a collision mesh
	virtual int				CreateDebugMesh(CPhysCollide const* pCollisionModel, Vec3** outVerts) = 0;
	// destroy the list of verts created by CreateDebugMesh
	virtual void			DestroyDebugMesh(int vertCount, Vec3* outVerts) = 0;

	// create a queryable version of the collision model
	virtual ICollisionQuery* CreateQueryModel(CPhysCollide* pCollide) = 0;
	// destroy the queryable version
	virtual void			DestroyQueryModel(ICollisionQuery* pQuery) = 0;

	virtual IPhysicsCollision* ThreadContextCreate(void) = 0;
	virtual void			ThreadContextDestroy(IPhysicsCollision* pThreadContex) = 0;

	virtual CPhysCollide* CreateVirtualMesh(const virtualmeshparams_t& params) = 0;
	virtual bool			SupportsVirtualMesh() = 0;


	virtual bool			GetBBoxCacheSize(int* pCachedSize, int* pCachedCount) = 0;


	// extracts a polyhedron that defines a CPhysConvex's shape
	virtual CPolyhedron* PolyhedronFromConvex(CPhysConvex* const pConvex, bool bUseTempPolyhedron) = 0;

	// dumps info about the collide to Msg()
	virtual void			OutputDebugInfo(const CPhysCollide* pCollide) = 0;
	virtual unsigned int	ReadStat(int statID) = 0;
};

MAKE_INTERFACE_VERSION(IPhysicsCollision, PhysicsCollision, "vphysics.dll", "VPhysicsCollision007");

// this can be used to post-process a collision model
class ICollisionQuery
{
public:
	virtual ~ICollisionQuery() {}
	// number of convex pieces in the whole solid
	virtual int		ConvexCount(void) = 0;
	// triangle count for this convex piece
	virtual int		TriangleCount(int convexIndex) = 0;
	// get the stored game data
	virtual unsigned int GetGameData(int convexIndex) = 0;
	// Gets the triangle's verts to an array
	virtual void	GetTriangleVerts(int convexIndex, int triangleIndex, Vec3* verts) = 0;

	// UNDONE: This doesn't work!!!
	virtual void	SetTriangleVerts(int convexIndex, int triangleIndex, const Vec3* verts) = 0;

	// returns the 7-bit material index
	virtual int		GetTriangleMaterialIndex(int convexIndex, int triangleIndex) = 0;
	// sets a 7-bit material index for this triangle
	virtual void	SetTriangleMaterialIndex(int convexIndex, int triangleIndex, int index7bits) = 0;
};

//-----------------------------------------------------------------------------
// Purpose: Ray traces from game engine.
//-----------------------------------------------------------------------------
class IPhysicsGameTrace
{
public:
	virtual void VehicleTraceRay(const Ray_t& ray, void* pVehicle, trace_t* pTrace) = 0;
	virtual	void VehicleTraceRayWithWater(const Ray_t& ray, void* pVehicle, trace_t* pTrace) = 0;
	virtual bool VehiclePointInWater(const Vec3& vecPoint) = 0;
};

// The caller should implement this to return contents masks per convex on a collide
class IConvexInfo
{
public:
	virtual unsigned int GetContents(int convexGameData) = 0;
};

class CPhysicsEventHandler;
class IPhysicsCollisionData
{
public:
	virtual void GetSurfaceNormal(Vec3& out) = 0;		// normal points toward second object (object index 1)
	virtual void GetContactPoint(Vec3& out) = 0;		// contact point of collision (in world space)
	virtual void GetContactSpeed(Vec3& out) = 0;		// speed of surface 1 relative to surface 0 (in world space)
};


struct vcollisionevent_t
{
	IPhysicsObject* pObjects[2];
	int				surfaceProps[2];
	bool			isCollision;
	bool			isShadowCollision;
	float			deltaCollisionTime;

	float			collisionSpeed;				// only valid at postCollision
	IPhysicsCollisionData* pInternalData;		// may change pre/post collision
};

class IPhysicsCollisionEvent
{
public:
	// returns the two objects that collided, time between last collision of these objects
	// and an opaque data block of collision information
	// NOTE: PreCollision/PostCollision ALWAYS come in matched pairs!!!
	virtual void PreCollision(vcollisionevent_t* pEvent) = 0;
	virtual void PostCollision(vcollisionevent_t* pEvent) = 0;

	// This is a scrape event.  The object has scraped across another object consuming the indicated energy
	virtual void Friction(IPhysicsObject* pObject, float energy, int surfaceProps, int surfacePropsHit, IPhysicsCollisionData* pData) = 0;

	virtual void StartTouch(IPhysicsObject* pObject1, IPhysicsObject* pObject2, IPhysicsCollisionData* pTouchData) = 0;
	virtual void EndTouch(IPhysicsObject* pObject1, IPhysicsObject* pObject2, IPhysicsCollisionData* pTouchData) = 0;

	virtual void FluidStartTouch(IPhysicsObject* pObject, IPhysicsFluidController* pFluid) = 0;
	virtual void FluidEndTouch(IPhysicsObject* pObject, IPhysicsFluidController* pFluid) = 0;

	virtual void PostSimulationFrame() = 0;

	virtual void ObjectEnterTrigger(IPhysicsObject* pTrigger, IPhysicsObject* pObject) {}
	virtual void ObjectLeaveTrigger(IPhysicsObject* pTrigger, IPhysicsObject* pObject) {}
};


class IPhysicsObjectEvent
{
public:
	// these can be used to optimize out queries on sleeping objects
	// Called when an object is woken after sleeping
	virtual void ObjectWake(IPhysicsObject* pObject) = 0;
	// called when an object goes to sleep (no longer simulating)
	virtual void ObjectSleep(IPhysicsObject* pObject) = 0;
};

class IPhysicsConstraintEvent
{
public:
	// the constraint is now inactive, the game code is required to delete it or re-activate it.
	virtual void ConstraintBroken(IPhysicsConstraint*) = 0;
};

struct hlshadowcontrol_params_t
{
	Vec3			targetPosition;
	QAngle			targetRotation;
	float			maxAngular;
	float			maxDampAngular;
	float			maxSpeed;
	float			maxDampSpeed;
	float			dampFactor;
	float			teleportDistance;
};

// UNDONE: At some point allow this to be parameterized using hlshadowcontrol_params_t.
// All of the infrastructure is in place to do that.

class IPhysicsShadowController
{
public:
	virtual ~IPhysicsShadowController(void) {}

	virtual void Update(const Vec3& position, const QAngle& angles, float timeOffset) = 0;
	virtual void MaxSpeed(float maxSpeed, float maxAngularSpeed) = 0;
	virtual void StepUp(float height) = 0;

	// If the teleport distance is non-zero, the object will be teleported to 
	// the target location when the error exceeds this quantity.
	virtual void SetTeleportDistance(float teleportDistance) = 0;
	virtual bool AllowsTranslation() = 0;
	virtual bool AllowsRotation() = 0;

	// There are two classes of shadow objects:
	// 1) Game physics controlled, shadow follows game physics (this is the default)
	// 2) Physically controlled - shadow position is a target, but the game hasn't guaranteed that the space can be occupied by this object
	virtual void SetPhysicallyControlled(bool isPhysicallyControlled) = 0;
	virtual bool IsPhysicallyControlled() = 0;
	virtual void GetLastImpulse(Vec3* pOut) = 0;
	virtual void UseShadowMaterial(bool bUseShadowMaterial) = 0;
	virtual void ObjectMaterialChanged(int materialIndex) = 0;


	//Basically get the last inputs to IPhysicsShadowController::Update(), returns last input to timeOffset in Update()
	virtual float GetTargetPosition(Vec3* pPositionOut, QAngle* pAnglesOut) = 0;

	virtual float GetTeleportDistance(void) = 0;
	virtual void GetMaxSpeed(float* pMaxSpeedOut, float* pMaxAngularSpeedOut) = 0;
};

class CPhysicsSimObject;
class IPhysicsMotionController;

// Callback for simulation
class IMotionEvent
{
public:
	// These constants instruct the simulator as to how to apply the values copied to linear & angular
	// GLOBAL/LOCAL refer to the coordinate system of the values, whereas acceleration/force determine whether or not
	// mass is divided out (forces must be divided by mass to compute acceleration)
	enum simresult_e { SIM_NOTHING = 0, SIM_LOCAL_ACCELERATION, SIM_LOCAL_FORCE, SIM_GLOBAL_ACCELERATION, SIM_GLOBAL_FORCE };
	virtual simresult_e	Simulate(IPhysicsMotionController* pController, IPhysicsObject* pObject, float deltaTime, Vec3& linear, Vec3& angular) = 0;
};

class IPhysicsMotionController
{
public:
	virtual ~IPhysicsMotionController(void) {}
	virtual void SetEventHandler(IMotionEvent* handler) = 0;
	virtual void AttachObject(IPhysicsObject* pObject, bool checkIfAlreadyAttached) = 0;
	virtual void DetachObject(IPhysicsObject* pObject) = 0;

	// returns the number of objects currently attached to the controller
	virtual int CountObjects(void) = 0;
	// NOTE: pObjectList is an array with at least CountObjects() allocated
	virtual void GetObjects(IPhysicsObject** pObjectList) = 0;
	// detaches all attached objects
	virtual void ClearObjects(void) = 0;
	// wakes up all attached objects
	virtual void WakeObjects(void) = 0;

	enum priority_t
	{
		LOW_PRIORITY = 0,
		MEDIUM_PRIORITY = 1,
		HIGH_PRIORITY = 2,
	};
	virtual void SetPriority(priority_t priority) = 0;
};

// -------------------
// Collision filter function.  Return 0 if objects should not be tested for collisions, nonzero otherwise
// Install with IPhysicsEnvironment::SetCollisionFilter()
// -------------------
class IPhysicsCollisionSolver
{
public:
	virtual int ShouldCollide(IPhysicsObject* pObj0, IPhysicsObject* pObj1, void* pGameData0, void* pGameData1) = 0;
	virtual int ShouldSolvePenetration(IPhysicsObject* pObj0, IPhysicsObject* pObj1, void* pGameData0, void* pGameData1, float dt) = 0;

	// pObject has already done the max number of collisions this tick, should we freeze it to save CPU?
	virtual bool ShouldFreezeObject(IPhysicsObject* pObject) = 0;

	// The system has already done too many collision checks, performance will suffer.
	// How many more should it do?
	virtual int AdditionalCollisionChecksThisTick(int currentChecksDone) = 0;

	// This list of objects is in a connected contact graph that is too large to solve quickly
	// return true to freeze the system, false to solve it
	virtual bool ShouldFreezeContacts(IPhysicsObject** pObjectList, int objectCount) = 0;
};

enum PhysicsTraceType_t
{
	VPHYSICS_TRACE_EVERYTHING = 0,
	VPHYSICS_TRACE_STATIC_ONLY,
	VPHYSICS_TRACE_MOVING_ONLY,
	VPHYSICS_TRACE_TRIGGERS_ONLY,
	VPHYSICS_TRACE_STATIC_AND_MOVING,
};

class IPhysicsTraceFilter
{
public:
	virtual bool ShouldHitObject(IPhysicsObject* pObject, int contentsMask) = 0;
	virtual PhysicsTraceType_t	GetTraceType() const = 0;
};

class IPhysicsEnvironment
{
public:
	virtual ~IPhysicsEnvironment(void) {}

	virtual void SetDebugOverlay(CreateInterfaceFn debugOverlayFactory) = 0;
	virtual IVPhysicsDebugOverlay* GetDebugOverlay(void) = 0;

	// gravity is a 3-vector in in/s^2
	virtual void			SetGravity(const Vec3& gravityVector) = 0;
	virtual void			GetGravity(Vec3* pGravityVector) const = 0;

	// air density is in kg / m^3 (water is 1000)
	// This controls drag, air that is more dense has more drag.
	virtual void			SetAirDensity(float density) = 0;
	virtual float			GetAirDensity(void) const = 0;

	// object creation
	// create a polygonal object.  pCollisionModel was created by the physics builder DLL in a pre-process.
	virtual IPhysicsObject* CreatePolyObject(const CPhysCollide* pCollisionModel, int materialIndex, const Vec3& position, const QAngle& angles, objectparams_t* pParams) = 0;
	// same as above, but this one cannot move or rotate (infinite mass/inertia)
	virtual IPhysicsObject* CreatePolyObjectStatic(const CPhysCollide* pCollisionModel, int materialIndex, const Vec3& position, const QAngle& angles, objectparams_t* pParams) = 0;
	// Create a perfectly spherical object
	virtual IPhysicsObject* CreateSphereObject(float radius, int materialIndex, const Vec3& position, const QAngle& angles, objectparams_t* pParams, bool isStatic) = 0;
	// destroy an object created with CreatePolyObject() or CreatePolyObjectStatic()
	virtual void DestroyObject(IPhysicsObject*) = 0;

	// Create a polygonal fluid body out of the specified collision model
	// This object will affect any other objects that collide with the collision model
	virtual IPhysicsFluidController* CreateFluidController(IPhysicsObject* pFluidObject, fluidparams_t* pParams) = 0;
	// Destroy an object created with CreateFluidController()
	virtual void DestroyFluidController(IPhysicsFluidController*) = 0;

	// Create a simulated spring that connects 2 objects
	virtual IPhysicsSpring* CreateSpring(IPhysicsObject* pObjectStart, IPhysicsObject* pObjectEnd, springparams_t* pParams) = 0;
	virtual void DestroySpring(IPhysicsSpring*) = 0;

	// Create a constraint in the space of pReferenceObject which is attached by the constraint to pAttachedObject
	virtual IPhysicsConstraint* CreateRagdollConstraint(IPhysicsObject* pReferenceObject, IPhysicsObject* pAttachedObject, IPhysicsConstraintGroup* pGroup, const constraint_ragdollparams_t& ragdoll) = 0;
	virtual IPhysicsConstraint* CreateHingeConstraint(IPhysicsObject* pReferenceObject, IPhysicsObject* pAttachedObject, IPhysicsConstraintGroup* pGroup, const constraint_hingeparams_t& hinge) = 0;
	virtual IPhysicsConstraint* CreateFixedConstraint(IPhysicsObject* pReferenceObject, IPhysicsObject* pAttachedObject, IPhysicsConstraintGroup* pGroup, const constraint_fixedparams_t& fixed) = 0;
	virtual IPhysicsConstraint* CreateSlidingConstraint(IPhysicsObject* pReferenceObject, IPhysicsObject* pAttachedObject, IPhysicsConstraintGroup* pGroup, const constraint_slidingparams_t& sliding) = 0;
	virtual IPhysicsConstraint* CreateBallsocketConstraint(IPhysicsObject* pReferenceObject, IPhysicsObject* pAttachedObject, IPhysicsConstraintGroup* pGroup, const constraint_ballsocketparams_t& ballsocket) = 0;
	virtual IPhysicsConstraint* CreatePulleyConstraint(IPhysicsObject* pReferenceObject, IPhysicsObject* pAttachedObject, IPhysicsConstraintGroup* pGroup, const constraint_pulleyparams_t& pulley) = 0;
	virtual IPhysicsConstraint* CreateLengthConstraint(IPhysicsObject* pReferenceObject, IPhysicsObject* pAttachedObject, IPhysicsConstraintGroup* pGroup, const constraint_lengthparams_t& length) = 0;

	virtual void DestroyConstraint(IPhysicsConstraint*) = 0;

	virtual IPhysicsConstraintGroup* CreateConstraintGroup(const constraint_groupparams_t& groupParams) = 0;
	virtual void DestroyConstraintGroup(IPhysicsConstraintGroup* pGroup) = 0;

	virtual IPhysicsShadowController* CreateShadowController(IPhysicsObject* pObject, bool allowTranslation, bool allowRotation) = 0;
	virtual void						DestroyShadowController(IPhysicsShadowController*) = 0;

	virtual IPhysicsPlayerController* CreatePlayerController(IPhysicsObject* pObject) = 0;
	virtual void						DestroyPlayerController(IPhysicsPlayerController*) = 0;

	virtual IPhysicsMotionController* CreateMotionController(IMotionEvent* pHandler) = 0;
	virtual void						DestroyMotionController(IPhysicsMotionController* pController) = 0;

	virtual IPhysicsVehicleController* CreateVehicleController(IPhysicsObject* pVehicleBodyObject, const vehicleparams_t& params, unsigned int nVehicleType, IPhysicsGameTrace* pGameTrace) = 0;
	virtual void						DestroyVehicleController(IPhysicsVehicleController*) = 0;

	// install a function to filter collisions/penentration
	virtual void			SetCollisionSolver(IPhysicsCollisionSolver* pSolver) = 0;

	// run the simulator for deltaTime seconds
	virtual void			Simulate(float deltaTime) = 0;
	// true if currently running the simulator (i.e. in a callback during physenv->Simulate())
	virtual bool			IsInSimulation() const = 0;

	// Manage the timestep (period) of the simulator.  The main functions are all integrated with
	// this period as dt.
	virtual float			GetSimulationTimestep() const = 0;
	virtual void			SetSimulationTimestep(float timestep) = 0;

	// returns the current simulation clock's value.  This is an absolute time.
	virtual float			GetSimulationTime() const = 0;
	virtual void			ResetSimulationClock() = 0;
	// returns the current simulation clock's value at the next frame.  This is an absolute time.
	virtual float			GetNextFrameTime(void) const = 0;

	// Collision callbacks (game code collision response)
	virtual void			SetCollisionEventHandler(IPhysicsCollisionEvent* pCollisionEvents) = 0;
	virtual void			SetObjectEventHandler(IPhysicsObjectEvent* pObjectEvents) = 0;
	virtual	void			SetConstraintEventHandler(IPhysicsConstraintEvent* pConstraintEvents) = 0;

	virtual void			SetQuickDelete(bool bQuick) = 0;

	virtual int				GetActiveObjectCount() const = 0;
	virtual void			GetActiveObjects(IPhysicsObject** pOutputObjectList) const = 0;
	virtual const IPhysicsObject** GetObjectList(int* pOutputObjectCount) const = 0;
	virtual bool			TransferObject(IPhysicsObject* pObject, IPhysicsEnvironment* pDestinationEnvironment) = 0;

	virtual void			CleanupDeleteList(void) = 0;
	virtual void			EnableDeleteQueue(bool enable) = 0;

	// Save/Restore methods
	virtual bool			Save(const physsaveparams_t& params) = 0;
	virtual void			PreRestore(const physprerestoreparams_t& params) = 0;
	virtual bool			Restore(const physrestoreparams_t& params) = 0;
	virtual void			PostRestore() = 0;

	// Debugging:
	virtual bool IsCollisionModelUsed(CPhysCollide* pCollide) const = 0;

	// Physics world version of the enginetrace API:
	virtual void TraceRay(const Ray_t& ray, unsigned int fMask, IPhysicsTraceFilter* pTraceFilter, trace_t* pTrace) = 0;
	virtual void SweepCollideable(const CPhysCollide* pCollide, const Vec3& vecAbsStart, const Vec3& vecAbsEnd,
		const QAngle& vecAngles, unsigned int fMask, IPhysicsTraceFilter* pTraceFilter, trace_t* pTrace) = 0;

	// performance tuning
	virtual void GetPerformanceSettings(physics_performanceparams_t* pOutput) const = 0;
	virtual void SetPerformanceSettings(const physics_performanceparams_t* pSettings) = 0;

	// perf/cost statistics
	virtual void ReadStats(physics_stats_t* pOutput) = 0;
	virtual void ClearStats() = 0;

	virtual unsigned int	GetObjectSerializeSize(IPhysicsObject* pObject) const = 0;
	virtual void			SerializeObjectToBuffer(IPhysicsObject* pObject, unsigned char* pBuffer, unsigned int bufferSize) = 0;
	virtual IPhysicsObject* UnserializeObjectFromBuffer(void* pGameData, unsigned char* pBuffer, unsigned int bufferSize, bool enableCollisions) = 0;


	virtual void EnableConstraintNotify(bool bEnable) = 0;
	virtual void DebugCheckContacts(void) = 0;
};

MAKE_INTERFACE_VERSION(IPhysics, Physics, "vphysics.dll", "VPhysics031");