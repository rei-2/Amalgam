#pragma once
#include "CBasePlayer.h"
#include "../Misc/Activity.h"

template< class T, int minValue, int maxValue, int startValue >
class CRangeCheckedVar
{
public:

	inline CRangeCheckedVar()
	{
		m_Val = startValue;
	}

	inline CRangeCheckedVar(const T& value)
	{
		*this = value;
	}

	T GetRaw() const
	{
		return m_Val;
	}

	inline void Clamp()
	{
		if (m_Val < minValue)
			m_Val = minValue;

		else if (m_Val > maxValue)
			m_Val = maxValue;
	}

	inline operator const T& () const
	{
		return m_Val;
	}

	inline CRangeCheckedVar<T, minValue, maxValue, startValue>& operator=(const T& value)
	{
		//RangeCheck(value, minValue, maxValue);
		m_Val = value;
		return *this;
	}

	inline CRangeCheckedVar<T, minValue, maxValue, startValue>& operator+=(const T& value)
	{
		return (*this = m_Val + value);
	}

	inline CRangeCheckedVar<T, minValue, maxValue, startValue>& operator-=(const T& value)
	{
		return (*this = m_Val - value);
	}

	inline CRangeCheckedVar<T, minValue, maxValue, startValue>& operator*=(const T& value)
	{
		return (*this = m_Val * value);
	}

	inline CRangeCheckedVar<T, minValue, maxValue, startValue>& operator/=(const T& value)
	{
		return (*this = m_Val / value);
	}

private:
	T m_Val;
};

class CAnimationLayer
{
public:
	CRangeCheckedVar<int, -1, 65535, 0>	m_nSequence;
	CRangeCheckedVar<float, -2, 2, 0> m_flPrevCycle;
	CRangeCheckedVar<float, -5, 5, 0> m_flWeight;
	int m_nOrder;
	CRangeCheckedVar<float, -50, 50, 1> m_flPlaybackRate;
	CRangeCheckedVar<float, -2, 2, 0> m_flCycle;
	float m_flLayerAnimtime;
	float m_flLayerFadeOuttime;
	float m_flBlendIn;
	float m_flBlendOut;
	bool m_bClientBlend;
};

typedef enum
{
	LEGANIM_9WAY,
	LEGANIM_8WAY,
	LEGANIM_GOLDSRC
} LegAnimType_t;

enum PlayerAnimEvent_t
{
	PLAYERANIMEVENT_ATTACK_PRIMARY,
	PLAYERANIMEVENT_ATTACK_SECONDARY,
	PLAYERANIMEVENT_ATTACK_GRENADE,
	PLAYERANIMEVENT_RELOAD,
	PLAYERANIMEVENT_RELOAD_LOOP,
	PLAYERANIMEVENT_RELOAD_END,
	PLAYERANIMEVENT_JUMP,
	PLAYERANIMEVENT_SWIM,
	PLAYERANIMEVENT_DIE,
	PLAYERANIMEVENT_FLINCH_CHEST,
	PLAYERANIMEVENT_FLINCH_HEAD,
	PLAYERANIMEVENT_FLINCH_LEFTARM,
	PLAYERANIMEVENT_FLINCH_RIGHTARM,
	PLAYERANIMEVENT_FLINCH_LEFTLEG,
	PLAYERANIMEVENT_FLINCH_RIGHTLEG,
	PLAYERANIMEVENT_DOUBLEJUMP,
	PLAYERANIMEVENT_CANCEL,
	PLAYERANIMEVENT_SPAWN,
	PLAYERANIMEVENT_SNAP_YAW,
	PLAYERANIMEVENT_CUSTOM,
	PLAYERANIMEVENT_CUSTOM_GESTURE,
	PLAYERANIMEVENT_CUSTOM_SEQUENCE,
	PLAYERANIMEVENT_CUSTOM_GESTURE_SEQUENCE,
	PLAYERANIMEVENT_ATTACK_PRE,
	PLAYERANIMEVENT_ATTACK_POST,
	PLAYERANIMEVENT_GRENADE1_DRAW,
	PLAYERANIMEVENT_GRENADE2_DRAW,
	PLAYERANIMEVENT_GRENADE1_THROW,
	PLAYERANIMEVENT_GRENADE2_THROW,
	PLAYERANIMEVENT_VOICE_COMMAND_GESTURE,
	PLAYERANIMEVENT_DOUBLEJUMP_CROUCH,
	PLAYERANIMEVENT_STUN_BEGIN,
	PLAYERANIMEVENT_STUN_MIDDLE,
	PLAYERANIMEVENT_STUN_END,
	PLAYERANIMEVENT_PASSTIME_THROW_BEGIN,
	PLAYERANIMEVENT_PASSTIME_THROW_MIDDLE,
	PLAYERANIMEVENT_PASSTIME_THROW_END,
	PLAYERANIMEVENT_PASSTIME_THROW_CANCEL,
	PLAYERANIMEVENT_ATTACK_PRIMARY_SUPER,
	PLAYERANIMEVENT_COUNT
};

struct GestureSlot_t
{
	int m_iGestureSlot;
	Activity m_iActivity;
	bool m_bAutoKill;
	bool m_bActive;
	CAnimationLayer* m_pAnimLayer;
};

struct MultiPlayerPoseData_t
{
	int m_iMoveX;
	int m_iMoveY;
	int m_iAimYaw;
	int m_iAimPitch;
	int m_iBodyHeight;
	int m_iMoveYaw;
	int m_iMoveScale;
	float m_flEstimateYaw;
	float m_flLastAimTurnTime;

	void Init()
	{
		m_iMoveX = 0;
		m_iMoveY = 0;
		m_iAimYaw = 0;
		m_iAimPitch = 0;
		m_iBodyHeight = 0;
		m_iMoveYaw = 0;
		m_iMoveScale = 0;
		m_flEstimateYaw = 0.0f;
		m_flLastAimTurnTime = 0.0f;
	}
};

struct DebugPlayerAnimData_t
{
	float m_flSpeed;
	float m_flAimPitch;
	float m_flAimYaw;
	float m_flBodyHeight;
	Vector2D m_vecMoveYaw;
};

struct MultiPlayerMovementData_t
{
	float m_flWalkSpeed;
	float m_flRunSpeed;
	float m_flSprintSpeed;
	float m_flBodyYawRate;
};

enum
{
	GESTURE_SLOT_ATTACK_AND_RELOAD,
	GESTURE_SLOT_GRENADE,
	GESTURE_SLOT_JUMP,
	GESTURE_SLOT_SWIM,
	GESTURE_SLOT_FLINCH,
	GESTURE_SLOT_VCD,
	GESTURE_SLOT_CUSTOM,
	GESTURE_SLOT_COUNT
};

class IInterpolatedVar
{
public:
	virtual ~IInterpolatedVar() = 0;
	virtual void Setup(void* pValue, int type) = 0;
	virtual void SetInterpolationAmount(float seconds) = 0;
	virtual void NoteLastNetworkedValue() = 0;
	virtual bool NoteChanged(float changetime, bool bUpdateLastNetworkedValue) = 0;
	virtual void Reset() = 0;
	virtual int Interpolate(float currentTime) = 0;
	virtual int  GetType() const = 0;
	virtual void RestoreToLastNetworked() = 0;
	virtual void Copy(IInterpolatedVar* pSrc) = 0;
	virtual const char* GetDebugName() = 0;
	virtual void SetDebugName(const char* pName) = 0;
	virtual void SetDebug(bool bDebug) = 0;
};

class CInterpolatedVar
{
public:
	IInterpolatedVar* vtable;
	float* m_pValue;
	float* m_pElements;
	unsigned short m_maxElement;
	unsigned short m_firstElement;
	unsigned short m_count;
	unsigned short m_growSize;
	float* m_LastNetworkedValue;
	float m_LastNetworkedTime;
	byte m_fType;
	byte m_nMaxCount;
	byte* m_bLooping;
	float m_InterpolationAmount;
	const char* m_pDebugName;
	bool m_bDebug : 1;
};

class CMultiPlayerAnimState
{
public:
	virtual ~CMultiPlayerAnimState() = 0;
	virtual void ClearAnimationState() = 0;
	virtual void DoAnimationEvent(PlayerAnimEvent_t event, int nData = 0) = 0;
	virtual Activity CalcMainActivity() = 0;
	virtual void Update(float eyeYaw, float eyePitch) = 0;
	virtual void Release() = 0;
	virtual Activity TranslateActivity(Activity actDesired) = 0;
	virtual void SetRunSpeed(float flSpeed) = 0;
	virtual void SetWalkSpeed(float flSpeed) = 0;
	virtual void SetSprintSpeed(float flSpeed) = 0;
	virtual void ShowDebugInfo() = 0;
	virtual void DebugShowAnimState(int iStartLine) = 0;
	virtual void Init(CBasePlayer* pPlayer, MultiPlayerMovementData_t& movementData) = 0;
	virtual int SelectWeightedSequence(Activity activity) = 0;
	virtual void RestartMainSequence() = 0;
	virtual void GetOuterAbsVelocity(Vector& vel) = 0;
	virtual bool HandleJumping(Activity& idealActivity) = 0;
	virtual bool HandleDucking(Activity& idealActivity) = 0;
	virtual bool HandleMoving(Activity& idealActivity) = 0;
	virtual bool HandleSwimming(Activity& idealActivity) = 0;
	virtual bool HandleDying(Activity& idealActivity) = 0;
	virtual void RestartGesture(int iGestureSlot, Activity iGestureActivity, bool bAutoKill = true) = 0;
	virtual float GetGesturePlaybackRate() = 0;
	virtual void PlayFlinchGesture(Activity iActivity) = 0;
	virtual float CalcMovementSpeed(bool* bIsMoving) = 0;
	virtual float CalcMovementPlaybackRate(bool* bIsMoving) = 0;
	virtual void ComputePoseParam_MoveYaw(CStudioHdr* pStudioHdr) = 0;
	virtual void ComputePoseParam_AimPitch(CStudioHdr* pStudioHdr) = 0;
	virtual void ComputePoseParam_AimYaw(CStudioHdr* pStudioHdr) = 0;
	virtual void EstimateYaw() = 0;
	virtual float GetCurrentMaxGroundSpeed() = 0;
	virtual void ComputeSequences(CStudioHdr* pStudioHdr) = 0;
	virtual bool ShouldUpdateAnimState() = 0;

public:
	bool m_bForceAimYaw;
	CUtlVector<GestureSlot_t> m_aGestureSlots;
	CBasePlayer* m_pEntity;
	QAngle m_angRender;
	bool m_bPoseParameterInit;
	MultiPlayerPoseData_t m_PoseParameterData;
	DebugPlayerAnimData_t m_DebugAnimData;
	bool m_bCurrentFeetYawInitialized;
	float m_flLastAnimationStateClearTime;
	float m_flEyeYaw;
	float m_flEyePitch;
	float m_flGoalFeetYaw;
	float m_flCurrentFeetYaw;
	float m_flLastAimTurnTime;
	MultiPlayerMovementData_t m_MovementData;
	bool m_bJumping;
	float m_flJumpStartTime;
	bool m_bFirstJumpFrame;
	bool m_bInSwim;
	bool m_bFirstSwimFrame;
	bool m_bDying;
	bool m_bFirstDyingFrame;
	Activity m_eCurrentMainSequenceActivity;
	int m_nSpecificMainSequence;
	CHandle<void> m_hActiveWeapon;
	float m_flLastGroundSpeedUpdateTime;
	CInterpolatedVar m_iv_flMaxGroundSpeed;
	float m_flMaxGroundSpeed;
	int m_nMovementSequence;
	LegAnimType_t m_LegAnimType;
};

class CTFPlayerAnimState : public CMultiPlayerAnimState
{
public:
	CBaseEntity* GetTFPlayer() { return m_pTFPlayer; }

	virtual void		ClearAnimationState() = 0;
	virtual Activity	TranslateActivity(Activity actDesired) = 0;
	virtual void		Update(float eyeYaw, float eyePitch) = 0;
	virtual void		CheckStunAnimation() = 0;
	virtual Activity	CalcMainActivity() = 0;
	virtual void		ComputePoseParam_AimYaw(CStudioHdr* pStudioHdr) = 0;
	virtual float		GetCurrentMaxGroundSpeed() = 0;
	virtual float		GetGesturePlaybackRate() = 0;
	virtual bool		ShouldUpdateAnimState() = 0;
	virtual void		GetOuterAbsVelocity(Vec3& vel) = 0;
	virtual void		RestartGesture(int iGestureSlot, Activity iGestureActivity, bool bAutoKill = true) = 0;
	void				SetRenderAngles(const Vec3& angles) { m_angRender = angles; }

	CBaseEntity* m_pTFPlayer;
	bool		 m_bInAirWalk;
	float		 m_flHoldDeployedPoseUntilTime;
	float		 m_flTauntMoveX;
	float		 m_flTauntMoveY;
	float		 m_flVehicleLeanVel;
	float		 m_flVehicleLeanPos;
	Vec3		 m_vecSmoothedUp;
};