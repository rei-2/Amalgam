#pragma once
#include "IClientUnknown.h"
#include "../Types.h"

typedef unsigned short ClientShadowHandle_t;

enum
{
	CLIENTSHADOW_INVALID_HANDLE = (ClientShadowHandle_t)~0
};

enum ShadowType_t
{
	SHADOWS_NONE = 0,
	SHADOWS_SIMPLE,
	SHADOWS_RENDER_TO_TEXTURE,
	SHADOWS_RENDER_TO_TEXTURE_DYNAMIC,
	SHADOWS_RENDER_TO_DEPTH_TEXTURE
};

class IPVSNotify
{
public:
	virtual void OnPVSStatusChanged(bool bInPVS) = 0;
};

using ClientRenderHandle_t = void*;
using ModelInstanceHandle_t = unsigned short;
struct model_t;

class IClientRenderable
{
public:
	virtual IClientUnknown* GetIClientUnknown() = 0;
	virtual Vector const& GetRenderOrigin(void) = 0;
	virtual QAngle const& GetRenderAngles(void) = 0;
	virtual bool ShouldDraw(void) = 0;
	virtual bool IsTransparent(void) = 0;
	virtual bool UsesPowerOfTwoFrameBufferTexture() = 0;
	virtual bool UsesFullFrameBufferTexture() = 0;
	virtual ClientShadowHandle_t GetShadowHandle() const = 0;
	virtual ClientRenderHandle_t& RenderHandle() = 0;
	virtual const model_t* GetModel() const = 0;
	virtual int DrawModel(int flags) = 0;
	virtual int	GetBody() = 0;
	virtual void ComputeFxBlend() = 0;
	virtual int	GetFxBlend(void) = 0;
	virtual void GetColorModulation(float* color) = 0;
	virtual bool LODTest() = 0;
	virtual bool SetupBones(matrix3x4* pBoneToWorldOut, int nMaxBones, int boneMask, float currentTime) = 0;
	virtual void SetupWeights(const matrix3x4* pBoneToWorld, int nFlexWeightCount, float* pFlexWeights, float* pFlexDelayedWeights) = 0;
	virtual void DoAnimationEvents(void) = 0;
	virtual IPVSNotify* GetPVSNotifyInterface() = 0;
	virtual void GetRenderBounds(Vector& mins, Vector& maxs) = 0;
	virtual void GetRenderBoundsWorldspace(Vector& mins, Vector& maxs) = 0;
	virtual void GetShadowRenderBounds(Vector& mins, Vector& maxs, ShadowType_t shadowType) = 0;
	virtual bool ShouldReceiveProjectedTextures(int flags) = 0;
	virtual bool GetShadowCastDistance(float* pDist, ShadowType_t shadowType) const = 0;
	virtual bool GetShadowCastDirection(Vector* pDirection, ShadowType_t shadowType) const = 0;
	virtual bool IsShadowDirty() = 0;
	virtual void MarkShadowDirty(bool bDirty) = 0;
	virtual IClientRenderable* GetShadowParent() = 0;
	virtual IClientRenderable* FirstShadowChild() = 0;
	virtual IClientRenderable* NextShadowPeer() = 0;
	virtual ShadowType_t ShadowCastType() = 0;
	virtual void CreateModelInstance() = 0;
	virtual ModelInstanceHandle_t GetModelInstance() = 0;
	virtual const matrix3x4& RenderableToWorldTransform() = 0;
	virtual int LookupAttachment(const char* pAttachmentName) = 0;
	virtual	bool GetAttachment(int number, Vector& origin, QAngle& angles) = 0;
	virtual bool GetAttachment(int number, matrix3x4& matrix) = 0;
	virtual float* GetRenderClipPlane(void) = 0;
	virtual int GetSkin() = 0;
	virtual bool IsTwoPass(void) = 0;
	virtual void OnThreadedDrawSetup() = 0;
	virtual bool UsesFlexDelayedWeights() = 0;
	virtual void RecordToolMessage() = 0;
	virtual bool IgnoresZBuffer(void) const = 0;
};