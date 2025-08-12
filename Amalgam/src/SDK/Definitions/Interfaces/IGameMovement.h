#pragma once
#include "IMoveHelper.h"

class CTFPlayer;

class CMoveData
{
public:
	bool m_bFirstRunOfFunctions : 1;
	bool m_bGameCodeMovedPlayer : 1;
	EntityHandle_t m_nPlayerHandle;
	int m_nImpulseCommand;
	QAngle m_vecViewAngles;
	QAngle m_vecAbsViewAngles;
	int m_nButtons;
	int m_nOldButtons;
	float m_flForwardMove;
	float m_flOldForwardMove;
	float m_flSideMove;
	float m_flUpMove;
	float m_flMaxSpeed;
	float m_flClientMaxSpeed;
	Vector m_vecVelocity;
	QAngle m_vecAngles;
	QAngle m_vecOldAngles;
	float m_outStepHeight;
	Vector m_outWishVel;
	Vector m_outJumpVel;
	Vector m_vecConstraintCenter;
	float m_flConstraintRadius;
	float m_flConstraintWidth;
	float m_flConstraintSpeedFactor;
	Vector m_vecAbsOrigin;
};

class CBasePlayer;

class IGameMovement
{
public:
	virtual ~IGameMovement(void) {}

	virtual void	ProcessMovement(CBasePlayer* pPlayer, CMoveData* pMove) = 0;
	virtual void	StartTrackPredictionErrors(CBasePlayer* pPlayer) = 0;
	virtual void	FinishTrackPredictionErrors(CBasePlayer* pPlayer) = 0;
	virtual void	DiffPrint(char const* fmt, ...) = 0;
	virtual Vector	GetPlayerMins(bool ducked) const = 0;
	virtual Vector	GetPlayerMaxs(bool ducked) const = 0;
	virtual Vector	GetPlayerViewOffset(bool ducked) const = 0;
};

class CGameMovement : public IGameMovement
{
public:
	virtual			~CGameMovement(void) {}

	virtual void	ProcessMovement(CBasePlayer* pPlayer, CMoveData* pMove) = 0;
	virtual void	StartTrackPredictionErrors(CBasePlayer* pPlayer) = 0;
	virtual void	FinishTrackPredictionErrors(CBasePlayer* pPlayer) = 0;
	virtual void	DiffPrint(char const* fmt, ...) = 0;
	virtual Vector	GetPlayerMins(bool ducked) const = 0;
	virtual Vector	GetPlayerMaxs(bool ducked) const = 0;
	virtual Vector	GetPlayerViewOffset(bool ducked) const = 0;

	typedef enum
	{
		GROUND = 0,
		STUCK,
		LADDER
	} IntervalType_t;

	virtual void	TracePlayerBBox(const Vector& start, const Vector& end, unsigned int fMask, int collisionGroup, trace_t& pm) = 0;
	virtual void	TryTouchGround(const Vector& start, const Vector& end, const Vector& mins, const Vector& maxs, unsigned int fMask, int collisionGroup, trace_t& pm) = 0;
	virtual unsigned int	PlayerSolidMask(bool brushOnly = false) = 0;
	virtual void	PlayerMove(void) = 0;
	virtual float	CalcRoll(const QAngle& angles, const Vector& velocity, float rollangle, float rollspeed) = 0;
	virtual	void	DecayPunchAngle(void) = 0;
	virtual void	CheckWaterJump(void) = 0;
	virtual void	WaterMove(void) = 0;
	virtual void	AirAccelerate(Vector& wishdir, float wishspeed, float accel) = 0;
	virtual void	AirMove(void) = 0;
	virtual float	GetAirSpeedCap(void) = 0;
	virtual bool	CanAccelerate() = 0;
	virtual void	Accelerate(Vector& wishdir, float wishspeed, float accel) = 0;
	virtual void	WalkMove(void) = 0;
	virtual void	FullWalkMove() = 0;
	virtual void	OnJump(float fImpulse) = 0;
	virtual void	OnLand(float fVelocity) = 0;
	virtual void	OnTryPlayerMoveCollision(trace_t& tr) = 0;
	virtual Vector	GetPlayerMins(void) const = 0;
	virtual Vector	GetPlayerMaxs(void) const = 0;
	virtual int		GetCheckInterval(IntervalType_t type) = 0;
	virtual bool	CheckJumpButton(void) = 0;
	virtual void    FullTossMove(void) = 0;
	virtual void	FullLadderMove() = 0;
	virtual int		TryPlayerMove(Vector* pFirstDest = NULL, trace_t* pFirstTrace = NULL, float flSlideMultiplier = 0.f) = 0;
	virtual bool	LadderMove(void) = 0;
	virtual bool	OnLadder(trace_t& trace) = 0;
	virtual float	LadderDistance(void) const = 0;
	virtual unsigned int	LadderMask(void) const = 0;
	virtual float	ClimbSpeed(void) const = 0;
	virtual float	LadderLateralMultiplier(void) const = 0;
	virtual int		CheckStuck(void) = 0;
	virtual bool	CheckWater(void) = 0;
	virtual void	CategorizePosition(void) = 0;
	virtual void	CheckParameters(void) = 0;
	virtual	void	ReduceTimers(void) = 0;
	virtual void	CheckFalling(void) = 0;
	virtual void	PlayerRoughLandingEffects(float fvol) = 0;
	virtual void	Duck(void) = 0;
	virtual void	HandleDuckingSpeedCrop() = 0;
	virtual void	FinishUnDuck(void) = 0;
	virtual void	FinishDuck(void) = 0;
	virtual bool	CanUnduck() = 0;
	virtual CBaseHandle	TestPlayerPosition(const Vector& pos, int collisionGroup, trace_t& pm) = 0;
	virtual void	SetGroundEntity(trace_t* pm) = 0;
	virtual void	StepMove(Vector& vecDestination, trace_t& trace) = 0;
	virtual bool	GameHasLadders() const = 0;

	enum
	{
		MAX_PC_CACHE_SLOTS = 3,
	};

	CBasePlayer*	player;
	CMoveData*		mv;
	int				m_nOldWaterLevel;
	float			m_flWaterEntryTime;
	int				m_nOnLadder;
	Vector			m_vecForward;
	Vector			m_vecRight;
	Vector			m_vecUp;
	int				m_CachedGetPointContents[MAX_PLAYERS_ARRAY_SAFE][MAX_PC_CACHE_SLOTS];
	Vector			m_CachedGetPointContentsPoint[MAX_PLAYERS_ARRAY_SAFE][MAX_PC_CACHE_SLOTS];
	Vector			m_vecProximityMins;
	Vector			m_vecProximityMaxs;
	float			m_fFrameTime;
	int				m_iSpeedCropped;
	float			m_flStuckCheckTime[MAX_PLAYERS_ARRAY_SAFE][2];
};

class CTFGameMovement : public CGameMovement
{
public:
	virtual void	PlayerMove() = 0;
	virtual unsigned int	PlayerSolidMask(bool brushOnly = false) = 0;
	virtual void	ProcessMovement(CBasePlayer* pBasePlayer, CMoveData* pMove) = 0;
	virtual bool	CanAccelerate() = 0;
	virtual bool	CheckJumpButton() = 0;
	virtual int		CheckStuck(void) = 0;
	virtual bool	CheckWater(void) = 0;
	virtual void	WaterMove(void) = 0;
	virtual void	FullWalkMove() = 0;
	virtual void	WalkMove(void) = 0;
	virtual void	AirMove(void) = 0;
	virtual void	FullTossMove(void) = 0;
	virtual void	CategorizePosition(void) = 0;
	virtual void	CheckFalling(void) = 0;
	virtual void	Duck(void) = 0;
	virtual Vector	GetPlayerViewOffset(bool ducked) const = 0;
	virtual float	GetAirSpeedCap(void) = 0;
	virtual void	TracePlayerBBox(const Vector& start, const Vector& end, unsigned int fMask, int collisionGroup, trace_t& pm) = 0;
	virtual CBaseHandle	TestPlayerPosition(const Vector& pos, int collisionGroup, trace_t& pm) = 0;
	virtual void	StepMove(Vector& vecDestination, trace_t& trace) = 0;
	virtual bool	GameHasLadders() const = 0;
	virtual void	SetGroundEntity(trace_t* pm) = 0;
	virtual void	PlayerRoughLandingEffects(float fvol) = 0;
	virtual void	HandleDuckingSpeedCrop(void) = 0;
	virtual void	CheckWaterJump(void) = 0;

	Vector			m_vecWaterPoint;
	CTFPlayer*		m_pTFPlayer;
	bool			m_isPassingThroughEnemies;
};

MAKE_INTERFACE_VERSION(CTFGameMovement, GameMovement, "client.dll", "GameMovement001");