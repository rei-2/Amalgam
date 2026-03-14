#pragma once
#include "IClientUnknown.h"
#include "IClientRenderable.h"
#include "ICollideable.h"
#include "CBaseHandle.h"

typedef unsigned short SpatialPartitionHandle_t;

class CStaticProp : public IClientUnknown, public IClientRenderable, public ICollideable
{
public:
	virtual void SetRefEHandle(const CBaseHandle& handle) = 0;
	virtual const CBaseHandle& GetRefEHandle() const = 0;
	virtual IClientUnknown* GetIClientUnknown() = 0;
	virtual ICollideable* GetCollideable() = 0;
	virtual IClientNetworkable* GetClientNetworkable() = 0;
	virtual IClientRenderable* GetClientRenderable() = 0;
	virtual IClientEntity* GetIClientEntity() = 0;
	virtual CBaseEntity* GetBaseEntity() = 0;
	virtual IClientThinkable* GetClientThinkable() = 0;
	virtual const Vector& OBBMinsPreScaled() const = 0;
	virtual const Vector& OBBMaxsPreScaled() const = 0;
	virtual const Vector& OBBMins() const = 0;
	virtual const Vector& OBBMaxs() const = 0;
	virtual bool TestCollision(const Ray_t& ray, unsigned int fContentsMask, trace_t& tr) = 0;
	virtual bool TestHitboxes(const Ray_t& ray, unsigned int fContentsMask, trace_t& tr) = 0;
	virtual int GetCollisionModelIndex() = 0;
	virtual const model_t* GetCollisionModel() = 0;
	virtual const Vector& GetCollisionOrigin() const = 0;
	virtual const QAngle& GetCollisionAngles() const = 0;
	virtual const matrix3x4& CollisionToWorldTransform() const = 0;
	virtual SolidType_t GetSolid() const = 0;
	virtual int GetSolidFlags() const = 0;
	virtual IHandleEntity* GetEntityHandle() = 0;
	virtual int GetCollisionGroup() const = 0;
	virtual void WorldSpaceTriggerBounds(Vector* pVecWorldMins, Vector* pVecWorldMaxs) const = 0;
	virtual void WorldSpaceSurroundingBounds(Vector* pVecWorldMins, Vector* pVecWorldMaxs) = 0;
	virtual bool ShouldTouchTrigger(int triggerSolidFlags) const = 0;
	virtual const matrix3x4* GetRootParentToWorldTransform() const = 0;
	virtual int GetBody() = 0;
	virtual int GetSkin() = 0;
	virtual const Vector& GetRenderOrigin() = 0;
	virtual const QAngle& GetRenderAngles() = 0;
	virtual bool ShouldDraw() = 0;
	virtual bool IsTransparent(void) = 0;
	virtual bool IsTwoPass(void) = 0;
	virtual void OnThreadedDrawSetup() = 0;
	virtual const model_t* GetModel() = 0;
	virtual int DrawModel(int flags) = 0;
	virtual void ComputeFxBlend() = 0;
	virtual int GetFxBlend() = 0;
	virtual void GetColorModulation(float* color) = 0;
	virtual bool LODTest() = 0;
	virtual bool SetupBones(matrix3x4* pBoneToWorldOut, int nMaxBones, int boneMask, float currentTime) = 0;
	virtual void SetupWeights(const matrix3x4* pBoneToWorld, int nFlexWeightCount, float* pFlexWeights, float* pFlexDelayedWeights) = 0;
	virtual bool UsesFlexDelayedWeights() = 0;
	virtual void DoAnimationEvents(void) = 0;
	virtual IPVSNotify* GetPVSNotifyInterface() = 0;
	virtual void GetRenderBounds(Vector& mins, Vector& maxs) = 0;
	virtual void GetRenderBoundsWorldspace(Vector& mins, Vector& maxs) = 0;
	virtual bool ShouldCacheRenderInfo() = 0;
	virtual bool ShouldReceiveProjectedTextures(int flags) = 0;
	virtual bool GetShadowCastDistance(float* pDist, ShadowType_t shadowType) const = 0;
	virtual bool GetShadowCastDirection(Vector* pDirection, ShadowType_t shadowType) const = 0;
	virtual bool UsesPowerOfTwoFrameBufferTexture() = 0;
	virtual bool UsesFullFrameBufferTexture() = 0;
	virtual ClientShadowHandle_t GetShadowHandle() = 0;
	virtual ClientRenderHandle_t& RenderHandle() = 0;
	virtual void RecordToolMessage() = 0;
	virtual void GetShadowRenderBounds(Vector& mins, Vector& maxs, ShadowType_t shadowType) = 0;
	virtual bool IsShadowDirty() = 0;
	virtual void MarkShadowDirty(bool bDirty) = 0;
	virtual IClientRenderable* GetShadowParent() = 0;
	virtual IClientRenderable* FirstShadowChild() = 0;
	virtual IClientRenderable* NextShadowPeer() = 0;
	virtual ShadowType_t ShadowCastType() = 0;
	virtual void CreateModelInstance() = 0;
	virtual ModelInstanceHandle_t GetModelInstance() = 0;
	virtual int LookupAttachment(const char* pAttachmentName) = 0;
	virtual bool GetAttachment(int number, Vector& origin, QAngle& angles) = 0;
	virtual bool GetAttachment(int number, matrix3x4& matrix) = 0;
	virtual bool IgnoresZBuffer(void) const = 0;
	virtual float* GetRenderClipPlane(void) = 0;
	virtual const matrix3x4& RenderableToWorldTransform() = 0;

	Vector m_Origin;
	QAngle m_Angles;
	model_t* m_pModel;
	SpatialPartitionHandle_t m_Partition;
	ModelInstanceHandle_t m_ModelInstance;
	unsigned char m_Alpha;
	unsigned char m_nSolidType;
	unsigned char m_Skin;
	unsigned char m_Flags;
	unsigned short m_FirstLeaf;
	unsigned short m_LeafCount;
	CBaseHandle m_EntHandle;
	ClientRenderHandle_t m_RenderHandle;
	unsigned short m_FadeIndex;
	float m_flForcedFadeScale;
	Vector m_RenderBBoxMin;
	Vector m_RenderBBoxMax;
	matrix3x4 m_ModelToWorld;
	float m_flRadius;
	Vector m_WorldRenderBBoxMin;
	Vector m_WorldRenderBBoxMax;
	Vector m_LightingOrigin;
};
