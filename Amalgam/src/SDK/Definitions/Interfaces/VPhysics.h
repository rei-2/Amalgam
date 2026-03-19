#pragma once
#include "../Misc/IAppSystem.h"
#include "../Types.h"

#define AIR_DENSITY	2
#define k_flMaxVelocity 2000.0f
#define k_flMaxAngularVelocity 360.0f * 10.0f
#define DEFAULT_MIN_FRICTION_MASS 10.0f
#define DEFAULT_MAX_FRICTION_MASS 2500.0f
#define LEAFMAP_HAS_CUBEMAP					0x0001
#define LEAFMAP_HAS_SINGLE_VERTEX_SPAN		0x0002
#define LEAFMAP_HAS_MULTIPLE_VERTEX_SPANS	0x0004
#define METERS_PER_INCH					(0.0254f)
#define CUBIC_METERS_PER_CUBIC_INCH		(METERS_PER_INCH*METERS_PER_INCH*METERS_PER_INCH)
#define POUNDS_PER_KG	(2.2f)
#define KG_PER_POUND	(1.0f/POUNDS_PER_KG)
#define lbs2kg(x)		((x)*KG_PER_POUND)
#define kg2lbs(x)		((x)*POUNDS_PER_KG)

const float VPHYSICS_MIN_MASS = 0.1f;
const float VPHYSICS_MAX_MASS = 5e4f;

class IPhysicsShadowController;
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
class CGameTrace;
class ISave;
class IRestore;
class CPhysConvex;
class CPhysPolysoup;
class IVPhysicsKeyParser;
class CPackedPhysicsDescription;
class CPhysicsEventHandler;
class CPhysicsSimObject;
class IPhysicsMotionController;
struct Ray_t;
struct vcollide_t;
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
struct debugcollide_t;
struct physics_stats_t;
struct virtualmeshparams_t;
struct physsaveparams_t;
struct physrestoreparams_t;
struct physprerestoreparams_t;
struct convertconvexparams_t;
typedef CGameTrace trace_t;

enum
{
	COLLIDE_POLY = 0,
	COLLIDE_MOPP = 1,
	COLLIDE_BALL = 2,
	COLLIDE_VIRTUAL = 3,
};

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

enum PhysicsTraceType_t
{
	VPHYSICS_TRACE_EVERYTHING = 0,
	VPHYSICS_TRACE_STATIC_ONLY,
	VPHYSICS_TRACE_MOVING_ONLY,
	VPHYSICS_TRACE_TRIGGERS_ONLY,
	VPHYSICS_TRACE_STATIC_AND_MOVING,
};

struct leafmap_t
{
	void* pLeaf;
	unsigned short vertCount;
	byte flags;
	byte spanCount;
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
	int leafCount;
	leafmap_t leafmap[1];
};

struct objectparams_t
{
	Vec3* massCenterOverride;
	float mass;
	float inertia;
	float damping;
	float rotdamping;
	float rotInertiaLimit;
	const char* pName;
	void* pGameData;
	float volume;
	float dragCoefficient;
	bool enableCollisions;
};

struct physics_performanceparams_t
{
	int maxCollisionsPerObjectPerTimestep;
	int maxCollisionChecksPerTimestep;
	float maxVelocity;
	float maxAngularVelocity;
	float lookAheadTimeObjectsVsWorld;
	float lookAheadTimeObjectsVsObject;
	float minFrictionMass;
	float maxFrictionMass;

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

struct truncatedcone_t
{
	Vec3 origin;
	Vec3 normal;
	float h;
	float theta;
};

struct hlshadowcontrol_params_t
{
	Vec3 targetPosition;
	QAngle targetRotation;
	float maxAngular;
	float maxDampAngular;
	float maxSpeed;
	float maxDampSpeed;
	float dampFactor;
	float teleportDistance;
};

struct Polyhedron_IndexedLine_t
{
	unsigned short iPointIndices[2];
};

struct Polyhedron_IndexedLineReference_t
{
	unsigned short iLineIndex;
	unsigned char iEndPointIndex;
};

struct Polyhedron_IndexedPolygon_t
{
	unsigned short iFirstIndex;
	unsigned short iIndexCount;
	Vector polyNormal;
};

class CPolyhedron
{
public:
	Vector* pVertices;
	Polyhedron_IndexedLine_t* pLines;
	Polyhedron_IndexedLineReference_t* pIndices;
	Polyhedron_IndexedPolygon_t* pPolygons;

	unsigned short iVertexCount;
	unsigned short iLineCount;
	unsigned short iIndexCount;
	unsigned short iPolygonCount;

	virtual ~CPolyhedron(void) = 0;
	virtual void Release(void) = 0;

	Vector Center(void)
	{
		if (!iVertexCount)
			return Vec3();

		Vector vAABBMin, vAABBMax;
		vAABBMin = vAABBMax = pVertices[0];
		for (int i = 1; i != iVertexCount; i++)
		{
			Vector& vPoint = pVertices[i];
			if (vPoint.x < vAABBMin.x)
				vAABBMin.x = vPoint.x;
			if (vPoint.y < vAABBMin.y)
				vAABBMin.y = vPoint.y;
			if (vPoint.z < vAABBMin.z)
				vAABBMin.z = vPoint.z;

			if (vPoint.x > vAABBMax.x)
				vAABBMax.x = vPoint.x;
			if (vPoint.y > vAABBMax.y)
				vAABBMax.y = vPoint.y;
			if (vPoint.z > vAABBMax.z)
				vAABBMax.z = vPoint.z;
		}
		return (vAABBMin + vAABBMax) / 2;
	}
};

class ICollisionQuery
{
public:
	virtual ~ICollisionQuery() {}
	virtual int ConvexCount(void) = 0;
	virtual int TriangleCount(int convexIndex) = 0;
	virtual unsigned int GetGameData(int convexIndex) = 0;
	virtual void GetTriangleVerts(int convexIndex, int triangleIndex, Vec3* verts) = 0;
	virtual void SetTriangleVerts(int convexIndex, int triangleIndex, const Vec3* verts) = 0;
	virtual int GetTriangleMaterialIndex(int convexIndex, int triangleIndex) = 0;
	virtual void SetTriangleMaterialIndex(int convexIndex, int triangleIndex, int index7bits) = 0;
};

class IPhysCollide
{
public:
	virtual ~IPhysCollide() {}
	//virtual void AddReference() = 0;
	//virtual void ReleaseReference() = 0;
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

class CPhysCollide : public IPhysCollide
{
public:
	virtual const void* GetCompactSurface() const = 0;
	virtual Vec3 GetOrthographicAreas() const = 0;
	virtual float GetSphereRadius() const = 0;
	virtual void ComputeOrthographicAreas(float epsilon) = 0;
	virtual void SetOrthographicAreas(const Vec3& areas) = 0;
	virtual const collidemap_t* GetCollideMap() const = 0;
};

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
	virtual IPhysicsShadowController* GetShadowController(void) const = 0;
	virtual void RemoveShadowController() = 0;
	virtual float ComputeShadowControl(const hlshadowcontrol_params_t& params, float secondsToArrival, float dt) = 0;
	virtual const CPhysCollide* GetCollide(void) const = 0;
	virtual const char* GetName() const = 0;
	virtual void BecomeTrigger() = 0;
	virtual void RemoveTrigger() = 0;
	virtual void BecomeHinged(int localAxis) = 0;
	virtual void RemoveHinged() = 0;
	virtual IPhysicsFrictionSnapshot* CreateFrictionSnapshot() = 0;
	virtual void DestroyFrictionSnapshot(IPhysicsFrictionSnapshot* pSnapshot) = 0;
	virtual void OutputDebugInfo() const = 0;

	void* m_pGameData;
	void* m_pObject;
	const CPhysCollide* m_pCollide;
	IPhysicsShadowController* m_pShadow;
	Vec3 m_dragBasis;
	Vec3 m_angDragBasis;
	bool m_shadowTempGravityDisable : 5;
	bool m_hasTouchedDynamic : 1;
	bool m_asleepSinceCreation : 1;
	bool m_forceSilentDelete : 1;
	unsigned char m_sleepState : 2;
	unsigned char m_hingedAxis : 3;
	unsigned char m_collideType : 3;
	unsigned short m_gameIndex;
	unsigned short m_materialIndex;
	unsigned short m_activeIndex;
	unsigned short m_callbacks;
	unsigned short m_gameFlags;
	unsigned int m_contentsMask;
	float m_volume;
	float m_buoyancyRatio;
	float m_dragCoefficient;
	float m_angDragCoefficient;
};

// "VPhysicsDebugOverlay001"
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

class IPhysicsGameTrace
{
public:
	virtual void VehicleTraceRay(const Ray_t& ray, void* pVehicle, trace_t* pTrace) = 0;
	virtual void VehicleTraceRayWithWater(const Ray_t& ray, void* pVehicle, trace_t* pTrace) = 0;
	virtual bool VehiclePointInWater(const Vec3& vecPoint) = 0;
};

class IConvexInfo
{
public:
	virtual unsigned int GetContents(int convexGameData) = 0;
};

class IPhysicsCollisionData
{
public:
	virtual void GetSurfaceNormal(Vec3& out) = 0;
	virtual void GetContactPoint(Vec3& out) = 0;
	virtual void GetContactSpeed(Vec3& out) = 0;
};

struct vcollisionevent_t
{
	IPhysicsObject* pObjects[2];
	int surfaceProps[2];
	bool isCollision;
	bool isShadowCollision;
	float deltaCollisionTime;
	float collisionSpeed;
	IPhysicsCollisionData* pInternalData;
};

class IPhysicsCollisionEvent
{
public:
	virtual void PreCollision(vcollisionevent_t* pEvent) = 0;
	virtual void PostCollision(vcollisionevent_t* pEvent) = 0;
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
	virtual void ObjectWake(IPhysicsObject* pObject) = 0;
	virtual void ObjectSleep(IPhysicsObject* pObject) = 0;
};

class IPhysicsConstraintEvent
{
public:
	virtual void ConstraintBroken(IPhysicsConstraint*) = 0;
};

class IPhysicsShadowController
{
public:
	virtual ~IPhysicsShadowController(void) {}
	virtual void Update(const Vec3& position, const QAngle& angles, float timeOffset) = 0;
	virtual void MaxSpeed(float maxSpeed, float maxAngularSpeed) = 0;
	virtual void StepUp(float height) = 0;
	virtual void SetTeleportDistance(float teleportDistance) = 0;
	virtual bool AllowsTranslation() = 0;
	virtual bool AllowsRotation() = 0;
	virtual void SetPhysicallyControlled(bool isPhysicallyControlled) = 0;
	virtual bool IsPhysicallyControlled() = 0;
	virtual void GetLastImpulse(Vec3* pOut) = 0;
	virtual void UseShadowMaterial(bool bUseShadowMaterial) = 0;
	virtual void ObjectMaterialChanged(int materialIndex) = 0;
	virtual float GetTargetPosition(Vec3* pPositionOut, QAngle* pAnglesOut) = 0;
	virtual float GetTeleportDistance(void) = 0;
	virtual void GetMaxSpeed(float* pMaxSpeedOut, float* pMaxAngularSpeedOut) = 0;
};

class IMotionEvent
{
public:
	enum simresult_e { SIM_NOTHING = 0, SIM_LOCAL_ACCELERATION, SIM_LOCAL_FORCE, SIM_GLOBAL_ACCELERATION, SIM_GLOBAL_FORCE };
	virtual simresult_e Simulate(IPhysicsMotionController* pController, IPhysicsObject* pObject, float deltaTime, Vec3& linear, Vec3& angular) = 0;
};

class IPhysicsMotionController
{
public:
	virtual ~IPhysicsMotionController(void) {}
	virtual void SetEventHandler(IMotionEvent* handler) = 0;
	virtual void AttachObject(IPhysicsObject* pObject, bool checkIfAlreadyAttached) = 0;
	virtual void DetachObject(IPhysicsObject* pObject) = 0;
	virtual int CountObjects(void) = 0;
	virtual void GetObjects(IPhysicsObject** pObjectList) = 0;
	virtual void ClearObjects(void) = 0;
	virtual void WakeObjects(void) = 0;
	enum priority_t
	{
		LOW_PRIORITY = 0,
		MEDIUM_PRIORITY = 1,
		HIGH_PRIORITY = 2,
	};
	virtual void SetPriority(priority_t priority) = 0;
};

class IPhysicsCollisionSolver
{
public:
	virtual int ShouldCollide(IPhysicsObject* pObj0, IPhysicsObject* pObj1, void* pGameData0, void* pGameData1) = 0;
	virtual int ShouldSolvePenetration(IPhysicsObject* pObj0, IPhysicsObject* pObj1, void* pGameData0, void* pGameData1, float dt) = 0;
	virtual bool ShouldFreezeObject(IPhysicsObject* pObject) = 0;
	virtual int AdditionalCollisionChecksThisTick(int currentChecksDone) = 0;
	virtual bool ShouldFreezeContacts(IPhysicsObject** pObjectList, int objectCount) = 0;
};

class IPhysicsTraceFilter
{
public:
	virtual bool ShouldHitObject(IPhysicsObject* pObject, int contentsMask) = 0;
	virtual PhysicsTraceType_t GetTraceType() const = 0;
};

class IPhysicsEnvironment
{
public:
	virtual ~IPhysicsEnvironment(void) {}
	virtual void SetDebugOverlay(CreateInterfaceFn debugOverlayFactory) = 0;
	virtual IVPhysicsDebugOverlay* GetDebugOverlay(void) = 0;
	virtual void SetGravity(const Vec3& gravityVector) = 0;
	virtual void GetGravity(Vec3* pGravityVector) const = 0;
	virtual void SetAirDensity(float density) = 0;
	virtual float GetAirDensity(void) const = 0;
	virtual IPhysicsObject* CreatePolyObject(const CPhysCollide* pCollisionModel, int materialIndex, const Vec3& position, const QAngle& angles, objectparams_t* pParams) = 0;
	virtual IPhysicsObject* CreatePolyObjectStatic(const CPhysCollide* pCollisionModel, int materialIndex, const Vec3& position, const QAngle& angles, objectparams_t* pParams) = 0;
	virtual IPhysicsObject* CreateSphereObject(float radius, int materialIndex, const Vec3& position, const QAngle& angles, objectparams_t* pParams, bool isStatic) = 0;
	virtual void DestroyObject(IPhysicsObject*) = 0;
	virtual IPhysicsFluidController* CreateFluidController(IPhysicsObject* pFluidObject, fluidparams_t* pParams) = 0;
	virtual void DestroyFluidController(IPhysicsFluidController*) = 0;
	virtual IPhysicsSpring* CreateSpring(IPhysicsObject* pObjectStart, IPhysicsObject* pObjectEnd, springparams_t* pParams) = 0;
	virtual void DestroySpring(IPhysicsSpring*) = 0;
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
	virtual void DestroyShadowController(IPhysicsShadowController*) = 0;
	virtual IPhysicsPlayerController* CreatePlayerController(IPhysicsObject* pObject) = 0;
	virtual void DestroyPlayerController(IPhysicsPlayerController*) = 0;
	virtual IPhysicsMotionController* CreateMotionController(IMotionEvent* pHandler) = 0;
	virtual void DestroyMotionController(IPhysicsMotionController* pController) = 0;
	virtual IPhysicsVehicleController* CreateVehicleController(IPhysicsObject* pVehicleBodyObject, const vehicleparams_t& params, unsigned int nVehicleType, IPhysicsGameTrace* pGameTrace) = 0;
	virtual void DestroyVehicleController(IPhysicsVehicleController*) = 0;
	virtual void SetCollisionSolver(IPhysicsCollisionSolver* pSolver) = 0;
	virtual void Simulate(float deltaTime) = 0;
	virtual bool IsInSimulation() const = 0;
	virtual float GetSimulationTimestep() const = 0;
	virtual void SetSimulationTimestep(float timestep) = 0;
	virtual float GetSimulationTime() const = 0;
	virtual void ResetSimulationClock() = 0;
	virtual float GetNextFrameTime(void) const = 0;
	virtual void SetCollisionEventHandler(IPhysicsCollisionEvent* pCollisionEvents) = 0;
	virtual void SetObjectEventHandler(IPhysicsObjectEvent* pObjectEvents) = 0;
	virtual void SetConstraintEventHandler(IPhysicsConstraintEvent* pConstraintEvents) = 0;
	virtual void SetQuickDelete(bool bQuick) = 0;
	virtual int GetActiveObjectCount() const = 0;
	virtual void GetActiveObjects(IPhysicsObject** pOutputObjectList) const = 0;
	virtual const IPhysicsObject** GetObjectList(int* pOutputObjectCount) const = 0;
	virtual bool TransferObject(IPhysicsObject* pObject, IPhysicsEnvironment* pDestinationEnvironment) = 0;
	virtual void CleanupDeleteList(void) = 0;
	virtual void EnableDeleteQueue(bool enable) = 0;
	virtual bool Save(const physsaveparams_t& params) = 0;
	virtual void PreRestore(const physprerestoreparams_t& params) = 0;
	virtual bool Restore(const physrestoreparams_t& params) = 0;
	virtual void PostRestore() = 0;
	virtual bool IsCollisionModelUsed(CPhysCollide* pCollide) const = 0;
	virtual void TraceRay(const Ray_t& ray, unsigned int fMask, IPhysicsTraceFilter* pTraceFilter, trace_t* pTrace) = 0;
	virtual void SweepCollideable(const CPhysCollide* pCollide, const Vec3& vecAbsStart, const Vec3& vecAbsEnd, const QAngle& vecAngles, unsigned int fMask, IPhysicsTraceFilter* pTraceFilter, trace_t* pTrace) = 0;
	virtual void GetPerformanceSettings(physics_performanceparams_t* pOutput) const = 0;
	virtual void SetPerformanceSettings(const physics_performanceparams_t* pSettings) = 0;
	virtual void ReadStats(physics_stats_t* pOutput) = 0;
	virtual void ClearStats() = 0;
	virtual unsigned int GetObjectSerializeSize(IPhysicsObject* pObject) const = 0;
	virtual void SerializeObjectToBuffer(IPhysicsObject* pObject, unsigned char* pBuffer, unsigned int bufferSize) = 0;
	virtual IPhysicsObject* UnserializeObjectFromBuffer(void* pGameData, unsigned char* pBuffer, unsigned int bufferSize, bool enableCollisions) = 0;
	virtual void EnableConstraintNotify(bool bEnable) = 0;
	virtual void DebugCheckContacts(void) = 0;
};

class IPhysicsCollision
{
public:
	virtual ~IPhysicsCollision(void) {}
	virtual CPhysConvex* ConvexFromVerts(Vec3** pVerts, int vertCount) = 0;
	virtual CPhysConvex* ConvexFromPlanes(float* pPlanes, int planeCount, float mergeDistance) = 0;
	virtual float ConvexVolume(CPhysConvex* pConvex) = 0;
	virtual float ConvexSurfaceArea(CPhysConvex* pConvex) = 0;
	virtual void SetConvexGameData(CPhysConvex* pConvex, unsigned int gameData) = 0;
	virtual void ConvexFree(CPhysConvex* pConvex) = 0;
	virtual CPhysConvex* BBoxToConvex(const Vec3& mins, const Vec3& maxs) = 0;
	virtual CPhysConvex* ConvexFromConvexPolyhedron(const CPolyhedron& ConvexPolyhedron) = 0;
	virtual void ConvexesFromConvexPolygon(const Vec3& vPolyNormal, const Vec3* pPoints, int iPointCount, CPhysConvex** pOutput) = 0;
	virtual CPhysPolysoup* PolysoupCreate(void) = 0;
	virtual void PolysoupDestroy(CPhysPolysoup* pSoup) = 0;
	virtual void PolysoupAddTriangle(CPhysPolysoup* pSoup, const Vec3& a, const Vec3& b, const Vec3& c, int materialIndex7bits) = 0;
	virtual CPhysCollide* ConvertPolysoupToCollide(CPhysPolysoup* pSoup, bool useMOPP) = 0;
	virtual CPhysCollide* ConvertConvexToCollide(CPhysConvex** pConvex, int convexCount) = 0;
	virtual CPhysCollide* ConvertConvexToCollideParams(CPhysConvex** pConvex, int convexCount, const convertconvexparams_t& convertParams) = 0;
	virtual void DestroyCollide(CPhysCollide* pCollide) = 0;
	virtual int CollideSize(CPhysCollide* pCollide) = 0;
	virtual int CollideWrite(char* pDest, CPhysCollide* pCollide, bool bSwap = false) = 0;
	virtual CPhysCollide* UnserializeCollide(char* pBuffer, int size, int index) = 0;
	virtual float CollideVolume(CPhysCollide* pCollide) = 0;
	virtual float CollideSurfaceArea(CPhysCollide* pCollide) = 0;
	virtual Vec3 CollideGetExtent(const CPhysCollide* pCollide, const Vec3& collideOrigin, const QAngle& collideAngles, const Vec3& direction) = 0;
	virtual void CollideGetAABB(Vec3* pMins, Vec3* pMaxs, const CPhysCollide* pCollide, const Vec3& collideOrigin, const QAngle& collideAngles) = 0;
	virtual void CollideGetMassCenter(CPhysCollide* pCollide, Vec3* pOutMassCenter) = 0;
	virtual void CollideSetMassCenter(CPhysCollide* pCollide, const Vec3& massCenter) = 0;
	virtual Vec3 CollideGetOrthographicAreas(const CPhysCollide* pCollide) = 0;
	virtual void CollideSetOrthographicAreas(CPhysCollide* pCollide, const Vec3& areas) = 0;
	virtual int CollideIndex(const CPhysCollide* pCollide) = 0;
	virtual CPhysCollide* BBoxToCollide(const Vec3& mins, const Vec3& maxs) = 0;
	virtual int GetConvexesUsedInCollideable(const CPhysCollide* pCollideable, CPhysConvex** pOutputArray, int iOutputArrayLimit) = 0;
	virtual void TraceBox(const Vec3& start, const Vec3& end, const Vec3& mins, const Vec3& maxs, const CPhysCollide* pCollide, const Vec3& collideOrigin, const QAngle& collideAngles, trace_t* ptr) = 0;
	virtual void TraceBox(const Ray_t& ray, const CPhysCollide* pCollide, const Vec3& collideOrigin, const QAngle& collideAngles, trace_t* ptr) = 0;
	virtual void TraceBox(const Ray_t& ray, unsigned int contentsMask, IConvexInfo* pConvexInfo, const CPhysCollide* pCollide, const Vec3& collideOrigin, const QAngle& collideAngles, trace_t* ptr) = 0;
	virtual void TraceCollide(const Vec3& start, const Vec3& end, const CPhysCollide* pSweepCollide, const QAngle& sweepAngles, const CPhysCollide* pCollide, const Vec3& collideOrigin, const QAngle& collideAngles, trace_t* ptr) = 0;
	virtual bool IsBoxIntersectingCone(const Vec3& boxAbsMins, const Vec3& boxAbsMaxs, const truncatedcone_t& cone) = 0;
	virtual void VCollideLoad(vcollide_t* pOutput, int solidCount, const char* pBuffer, int size, bool swap = false) = 0;
	virtual void VCollideUnload(vcollide_t* pVCollide) = 0;
	virtual IVPhysicsKeyParser* VPhysicsKeyParserCreate(const char* pKeyData) = 0;
	virtual void VPhysicsKeyParserDestroy(IVPhysicsKeyParser* pParser) = 0;
	virtual int CreateDebugMesh(CPhysCollide const* pCollisionModel, Vec3** outVerts) = 0;
	virtual void DestroyDebugMesh(int vertCount, Vec3* outVerts) = 0;
	virtual ICollisionQuery* CreateQueryModel(CPhysCollide* pCollide) = 0;
	virtual void DestroyQueryModel(ICollisionQuery* pQuery) = 0;
	virtual IPhysicsCollision* ThreadContextCreate(void) = 0;
	virtual void ThreadContextDestroy(IPhysicsCollision* pThreadContex) = 0;
	virtual CPhysCollide* CreateVirtualMesh(const virtualmeshparams_t& params) = 0;
	virtual bool SupportsVirtualMesh() = 0;
	virtual bool GetBBoxCacheSize(int* pCachedSize, int* pCachedCount) = 0;
	virtual CPolyhedron* PolyhedronFromConvex(CPhysConvex* const pConvex, bool bUseTempPolyhedron) = 0;
	virtual void OutputDebugInfo(const CPhysCollide* pCollide) = 0;
	virtual unsigned int ReadStat(int statID) = 0;
};

class IPhysics : public IAppSystem
{
public:
	virtual IPhysicsEnvironment* CreateEnvironment(void) = 0;
	virtual void DestroyEnvironment(IPhysicsEnvironment*) = 0;
	virtual IPhysicsEnvironment* GetActiveEnvironmentByIndex(int index) = 0;
	virtual IPhysicsObjectPairHash* CreateObjectPairHash() = 0;
	virtual void DestroyObjectPairHash(IPhysicsObjectPairHash* pHash) = 0;
	virtual IPhysicsCollisionSet* FindOrCreateCollisionSet(unsigned int id, int maxElementCount) = 0;
	virtual IPhysicsCollisionSet* FindCollisionSet(unsigned int id) = 0;
	virtual void DestroyAllCollisionSets() = 0;
};

MAKE_INTERFACE_VERSION(IPhysicsCollision, PhysicsCollision, "vphysics.dll", "VPhysicsCollision007");
MAKE_INTERFACE_VERSION(IPhysics, Physics, "vphysics.dll", "VPhysics031");