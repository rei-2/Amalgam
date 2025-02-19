#pragma once
#include "../SDK/Definitions/Types.h"
#include <windows.h>
#include <unordered_map>
#include <typeinfo>

#define VA_LIST(...) __VA_ARGS__

#define DEFAULT_BIND -1

// forward declartion of ConfigVar
template <class T>
class ConfigVar;

class CVarBase
{
public:
	size_t m_iType;
	std::string m_sName;
	int m_iFlags;

	// getter for ConfigVar
	template <class T>
	ConfigVar<T>* As()
	{
		if (typeid(T).hash_code() != m_iType)
			return nullptr;

		return reinterpret_cast<ConfigVar<T>*>(this);
	}
};

template <class T>
class ConfigVar : public CVarBase
{
public:
	T Default;
	T Value;
	std::unordered_map<int, T> Map;
	ConfigVar(T value, std::string name, int iFlags = 0);
};

inline std::vector<CVarBase*> g_Vars;

template<class T>
inline ConfigVar<T>::ConfigVar(T value, std::string name, int iFlags)
{
	Default = value;
	Value = value;
	Map[DEFAULT_BIND] = value;
	m_iType = typeid(T).hash_code();
	m_sName = name;
	g_Vars.push_back(this);
	m_iFlags = iFlags;
}

#define NAMESPACE_BEGIN(name)\
	namespace name {\
		inline std::string GetNamespace() { return "Vars::" + std::string(#name) + "::"; }\
		inline std::string GetSubname() { return ""; }

#define SUBNAMESPACE_BEGIN(name)\
	namespace name {\
		inline std::string GetSubname() { return std::string(#name) + "::"; }

#define NAMESPACE_END(name)\
	}
#define SUBNAMESPACE_END(name)\
	}

#define CVar(name, value, ...)\
	inline ConfigVar<decltype(value)> name = { value, GetNamespace() + GetSubname() + std::string(#name), __VA_ARGS__ };
#define Enum(name, ...)\
	namespace name##Enum { enum name##Enum { __VA_ARGS__ }; };
#define CVarEnum(name, value, flags, ...)\
	CVar(name, value, flags)\
	Enum(name, __VA_ARGS__)

#define NONE 0
#define VISUAL (1 << 0)
#define NOSAVE (1 << 1)
#define NOBIND (1 << 2)
#define DEBUGVAR NOSAVE

namespace Vars
{
	NAMESPACE_BEGIN(Menu)
		CVar(CheatName, std::string("Amalgam"), VISUAL)
		CVar(CheatPrefix, std::string("[Amalgam]"), VISUAL)
		CVar(MenuPrimaryKey, VK_INSERT, NOBIND)
		CVar(MenuSecondaryKey, VK_F3, NOBIND)

		CVar(ShowBinds, true)
		CVar(BindsDisplay, DragBox_t(), NOBIND)
		CVar(MenuShowsBinds, false, NOBIND)

		CVarEnum(Indicators, 0b00000, NONE, Ticks = 1 << 0, CritHack = 1 << 1, Spectators = 1 << 2, Ping = 1 << 3, Conditions = 1 << 4, SeedPrediction = 1 << 5)
		//CVar(SpectatorAvatars, false, VISUAL)

		CVar(TicksDisplay, DragBox_t(), NOBIND)
		CVar(CritsDisplay, DragBox_t(), NOBIND)
		CVar(SpectatorsDisplay, DragBox_t(), NOBIND)
		CVar(PingDisplay, DragBox_t(), NOBIND)
		CVar(ConditionsDisplay, DragBox_t(), NOBIND)
		CVar(SeedPredictionDisplay, DragBox_t(), NOBIND)

		CVar(Scale, 1.f, NOBIND)
		CVar(CheapText, false)

		SUBNAMESPACE_BEGIN(Theme)
			CVar(Accent, Color_t(255, 101, 101, 255), VISUAL)
			CVar(Background, Color_t(23, 23, 23, 250), VISUAL)
			CVar(Inactive, Color_t(150, 150, 150, 255), VISUAL)
			CVar(Active, Color_t(255, 255, 255, 255), VISUAL)
		SUBNAMESPACE_END(Theme)
	NAMESPACE_END(Menu)

	NAMESPACE_BEGIN(Aimbot)
		SUBNAMESPACE_BEGIN(General)
			CVarEnum(AimType, 0, NONE, Off, Plain, Smooth, Silent, Locking)
			CVarEnum(TargetSelection, 0, NONE, FOV, Distance)
			CVarEnum(Target, 0b0000001, NONE, Players = 1 << 0, Sentry = 1 << 1, Dispenser = 1 << 2, Teleporter = 1 << 3, Stickies = 1 << 4, NPCs = 1 << 5, Bombs = 1 << 6)
			CVarEnum(Ignore, 0b000000000, NONE, Friends = 1 << 0, Party = 1 << 1, Invulnerable = 1 << 2, Cloaked = 1 << 3, Unsimulated = 1 << 4, DeadRinger = 1 << 5, Vaccinator = 1 << 6, Disguised = 1 << 7, Taunting = 1 << 8)
			CVar(AimFOV, 30.f)
			CVar(Smoothing, 25.f)
			CVar(MaxTargets, 2)
			CVar(IgnoreCloakPercentage, 100)
			CVar(TickTolerance, 7)
			CVar(AutoShoot, true)
			CVar(FOVCircle, true)
			CVar(NoSpread, false)

			CVar(HitscanPeek, 1, DEBUGVAR)
			CVar(PeekDTOnly, true, DEBUGVAR)
			CVar(NoSpreadOffset, 0.f, DEBUGVAR)
			CVar(NoSpreadAverage, 5, DEBUGVAR)
			CVar(NoSpreadInterval, 0.1f, DEBUGVAR)
			CVar(NoSpreadBackupInterval, 2.f, DEBUGVAR)
			CVarEnum(AimHoldsFire, 2, DEBUGVAR, False, MinigunOnly, Always)
		SUBNAMESPACE_END(Global)

		SUBNAMESPACE_BEGIN(Hitscan)
			CVarEnum(Hitboxes, 0b00111, NONE, Head = 1 << 0, Pelvis = 1 << 1, Body = 1 << 2, Arms = 1 << 3, Legs = 1 << 4)
			CVarEnum(Modifiers, 0b01000000, NONE, Tapfire = 1 << 0, WaitForHeadshot = 1 << 1, WaitForCharge = 1 << 2, ScopedOnly = 1 << 3, AutoScope = 1 << 4, BodyaimIfLethal = 1 << 5, AutoRev = 1 << 6, ExtinguishTeam = 1 << 7)
			CVar(PointScale, 0.f)
			CVar(TapFireDist, 1000.f)
		SUBNAMESPACE_END(HITSCAN)

		SUBNAMESPACE_BEGIN(Projectile)
			CVarEnum(StrafePrediction, 0b11, NONE, Air = 1 << 0, Ground = 1 << 1)
			CVarEnum(SplashPrediction, 0, NONE, Off, Include, Prefer, Only)
			CVarEnum(AutoDetonate, 0b00, NONE, Stickies = 1 << 0, Flares = 1 << 1, PreventSelfDamage = 1 << 2)
			CVarEnum(AutoAirblast, 0b1000, NONE, Enabled = 1 << 0, RedirectSimple = 1 << 1, RedirectAdvanced = 1 << 2, RespectFOV = 1 << 3)
			CVarEnum(Modifiers, 0b1010, NONE, ChargeWeapon = 1 << 0, CancelCharge = 1 << 1, BodyaimIfLethal = 1 << 2, UsePrimeTime = 1 << 3, AimBlastAtFeet = 1 << 4)
			CVar(PredictionTime, 2.f)
			CVar(Hitchance, 0.f)
			CVar(AutodetRadius, 90.f)
			CVar(SplashRadius, 90.f)
			CVar(AutoRelease, 0.f)

			CVar(GroundSamples, 33, DEBUGVAR)
			CVar(GroundStraightFuzzyValue, 100.f, DEBUGVAR)
			CVar(GroundLowMinimumSamples, 16, DEBUGVAR)
			CVar(GroundHighMinimumSamples, 33, DEBUGVAR)
			CVar(GroundLowMinimumDistance, 0.f, DEBUGVAR)
			CVar(GroundHighMinimumDistance, 2500.f, DEBUGVAR)
			CVar(GroundMaxChanges, 0, DEBUGVAR)
			CVar(GroundMaxChangeTime, 0, DEBUGVAR)
			CVar(GroundNewWeight, 100.f, DEBUGVAR)
			CVar(GroundOldWeight, 100.f, DEBUGVAR)

			CVar(AirSamples, 33, DEBUGVAR)
			CVar(AirStraightFuzzyValue, 10.f, DEBUGVAR)
			CVar(AirLowMinimumSamples, 8, DEBUGVAR)
			CVar(AirHighMinimumSamples, 8, DEBUGVAR)
			CVar(AirLowMinimumDistance, 100000.f, DEBUGVAR)
			CVar(AirHighMinimumDistance, 100000.f, DEBUGVAR)
			CVar(AirMaxChanges, 1, DEBUGVAR)
			CVar(AirMaxChangeTime, 10, DEBUGVAR)
			CVar(AirNewWeight, 100.f, DEBUGVAR)
			CVar(AirOldWeight, 100.f, DEBUGVAR)

			CVar(VelocityAverageCount, 5, DEBUGVAR)
			CVar(VerticalShift, 5.f, DEBUGVAR)
			CVar(LatencyOffset, 0.f, DEBUGVAR)
			CVar(HullIncrease, 0.f, DEBUGVAR)
			CVar(DragOverride, 0.f, DEBUGVAR)
			CVar(TimeOverride, 0.f, DEBUGVAR)
			CVar(HuntsmanLerp, 50.f, DEBUGVAR)
			CVar(HuntsmanLerpLow, 100.f, DEBUGVAR)
			CVar(HuntsmanAdd, 0.f, DEBUGVAR)
			CVar(HuntsmanAddLow, 0.f, DEBUGVAR)
			CVar(HuntsmanClamp, 5.f, DEBUGVAR)
			CVar(SplashGrates, true, DEBUGVAR)
			CVarEnum(RocketSplashMode, 0, DEBUGVAR, Regular, SpecialLight, SpecialHeavy)
			CVar(SplashPoints, 100, DEBUGVAR)
			CVar(SplashCountDirect, 100, DEBUGVAR)
			CVar(SplashCountArc, 5, DEBUGVAR)
			CVar(SplashTraceInterval, 10, DEBUGVAR)
			CVar(DeltaCount, 5, DEBUGVAR)
			CVarEnum(DeltaMode, 0, DEBUGVAR, Average, Max)
			CVar(StrafeDelta, false, DEBUGVAR)
		SUBNAMESPACE_END(Projectile)

		SUBNAMESPACE_BEGIN(Melee)
			CVar(AutoBackstab, true)
			CVar(IgnoreRazorback, true)
			CVar(SwingPrediction, false)
			CVar(WhipTeam, false)

			CVar(SwingTicks, 13, DEBUGVAR)
			CVar(SwingPredictLag, true, DEBUGVAR)
			CVar(BackstabAccountPing, true, DEBUGVAR)
			CVar(BackstabDoubleTest, true, DEBUGVAR)
		SUBNAMESPACE_END(Melee)

		SUBNAMESPACE_BEGIN(Healing)
			CVar(AutoHeal, false)
			CVar(FriendsOnly, false)
			CVar(ActivateOnVoice, false)
		SUBNAMESPACE_END(Healing)
	NAMESPACE_END(AIMBOT)
	
	NAMESPACE_BEGIN(CritHack)
		CVar(ForceCrits, false)
		CVar(AvoidRandom, false)
		CVar(AlwaysMeleeCrit, false)
	NAMESPACE_END(CritHack)

	NAMESPACE_BEGIN(Backtrack)
		CVar(Enabled, false)
		CVar(PreferOnShot, false)
		CVar(Latency, 0)
		CVar(Interp, 0)
		CVar(Window, 185)

		CVar(Offset, 0, DEBUGVAR)
	NAMESPACE_END(Backtrack)

	NAMESPACE_BEGIN(CL_Move)
		SUBNAMESPACE_BEGIN(Doubletap)
			CVar(Doubletap, false)
			CVar(Warp, false)
			CVar(RechargeTicks, false)
			CVar(AntiWarp, true)
			CVar(TickLimit, 22)
			CVar(WarpRate, 22)
			CVar(PassiveRecharge, 0)
			CVar(RechargeLimit, 24)
		SUBNAMESPACE_END(DoubleTap)

		SUBNAMESPACE_BEGIN(Fakelag)
			CVarEnum(Fakelag, 0, NONE, Off, Plain, Random, Adaptive)
			CVar(PlainTicks, 12)
			CVar(RandomTicks, IntRange_t(14, 18));
			CVarEnum(Options, 0b000, NONE, WhileMoving = 1 << 0, WhileUnducking = 1 << 1, WhileAirborne = 1 << 2)
			CVar(UnchokeOnAttack, true)
			CVar(RetainBlastJump, false)

			CVar(RetainSoldierOnly, true, DEBUGVAR)
		SUBNAMESPACE_END(FakeLag)

		CVar(AutoPeek, false)

		CVar(SpeedEnabled, false)
		CVar(SpeedFactor, 1)
	NAMESPACE_END(CL_Move)

	NAMESPACE_BEGIN(AntiHack)
		SUBNAMESPACE_BEGIN(AntiAim)
			CVar(Enabled, false)
			CVarEnum(PitchReal, 0, NONE, None, Up, Down, Zero, Jitter, ReverseJitter)
			CVarEnum(PitchFake, 0, NONE, None, Up, Down, Jitter, ReverseJitter)
			Enum(Yaw, Forward, Left, Right, Backwards, Edge, Jitter, Spin)
			CVar(YawReal, 0)
			CVar(YawFake, 0)
			Enum(YawMode, View, Target)
			CVar(RealYawMode, 0)
			CVar(FakeYawMode, 0)
			CVar(RealYawOffset, 0)
			CVar(FakeYawOffset, 0)
			CVar(RealYawValue, 0)
			CVar(FakeYawValue, 0)
			CVar(SpinSpeed, 15.f)
			CVar(MinWalk, true)
			CVar(AntiOverlap, false)
			CVar(InvalidShootPitch, false)
		SUBNAMESPACE_END(AntiAim)

		SUBNAMESPACE_BEGIN(Resolver)
			CVar(Enabled, false)
			CVar(AutoResolve, false)
			CVar(AutoResolveCheatersOnly, false)
			CVar(AutoResolveHeadshotOnly, false)
			CVar(AutoResolveYawAmount, 90.f)
			CVar(AutoResolvePitchAmount, 90.f)
			CVar(CycleYaw, 0.f)
			CVar(CyclePitch, 0.f)
			CVar(CycleView, false)
			CVar(CycleMinwalk, false)
		SUBNAMESPACE_END(Resolver)
	NAMESPACE_END(AntiHack)

	NAMESPACE_BEGIN(CheaterDetection)
		CVarEnum(Methods, 0b0001, NONE, InvalidPitch = 1 << 0, PacketChoking = 1 << 1, AimFlicking = 1 << 2, DuckSpeed = 1 << 3)
		CVar(DetectionsRequired, 10)
		CVar(MinimumChoking, 20)
		CVar(MinimumFlick, 20.f) // min flick size to suspect
		CVar(MaximumNoise, 1.f) // max different between angles before and after flick
	NAMESPACE_END(CheaterDetection)

	NAMESPACE_BEGIN(ESP)
		CVarEnum(Draw, 0b0, VISUAL, Players = 1 << 0, Buildings = 1 << 1, Projectiles = 1 << 2, Objective = 1 << 3, NPCs = 1 << 4, Health = 1 << 5, Ammo = 1 << 6, Money = 1 << 7, Powerups = 1 << 8, Bombs = 1 << 9, Spellbook = 1 << 10, Gargoyle = 1 << 11)
		CVarEnum(Player, 0b0, VISUAL, Enemy = 1 << 0, Team = 1 << 1, Local = 1 << 2, Prioritized = 1 << 3, Friends = 1 << 4, Party = 1 << 5, Name = 1 << 6, Box = 1 << 7, Distance = 1 << 8, Bones = 1 << 9, HealthBar = 1 << 10, HealthText = 1 << 11, UberBar = 1 << 12, UberText = 1 << 13, ClassIcon = 1 << 14, ClassText = 1 << 15, WeaponIcon = 1 << 16, WeaponText = 1 << 17, Priority = 1 << 18, Labels = 1 << 19, Buffs = 1 << 20, Debuffs = 1 << 21, Misc = 1 << 22, LagCompensation = 1 << 23, Ping = 1 << 24, KDR = 1 << 25)
		CVarEnum(Building, 0b0, VISUAL, Enemy = 1 << 0, Team = 1 << 1, Local = 1 << 2, Prioritized = 1 << 3, Friends = 1 << 4, Party = 1 << 5, Name = 1 << 6, Box = 1 << 7, Distance = 1 << 8, HealthBar = 1 << 9, HealthText = 1 << 10, Owner = 1 << 11, Level = 1 << 12, Flags = 1 << 13)
		CVarEnum(Projectile, 0b0, VISUAL, Enemy = 1 << 0, Team = 1 << 1, Local = 1 << 2, Prioritized = 1 << 3, Friends = 1 << 4, Party = 1 << 5, Name = 1 << 6, Box = 1 << 7, Distance = 1 << 8, Flags = 1 << 9)
		CVarEnum(Objective, 0b0, VISUAL, Enemy = 1 << 0, Team = 1 << 1, Name = 1 << 2, Box = 1 << 3, Distance = 1 << 4, Flags = 1 << 5, IntelReturnTime = 1 << 6)

		CVar(ActiveAlpha, 255, VISUAL)
		CVar(DormantAlpha, 50, VISUAL)
		CVar(DormantPriority, false, VISUAL)
		CVar(DormantTime, 1.f, VISUAL)
	NAMESPACE_END(ESP)

	NAMESPACE_BEGIN(Chams)
		SUBNAMESPACE_BEGIN(Friendly)
			CVar(Players, false, VISUAL)
			CVar(Buildings, false, VISUAL)
			CVar(Ragdolls, false, VISUAL)
			CVar(Projectiles, false, VISUAL)
			
			CVar(Visible, VA_LIST(std::vector<std::pair<std::string, Color_t>>) VA_LIST({ { "Original", {} } }), VISUAL)
			CVar(Occluded, VA_LIST(std::vector<std::pair<std::string, Color_t>>) {}, VISUAL)
		SUBNAMESPACE_END(Friendly)

		SUBNAMESPACE_BEGIN(Enemy)
			CVar(Players, false, VISUAL)
			CVar(Buildings, false, VISUAL)
			CVar(Ragdolls, false, VISUAL)
			CVar(Projectiles, false, VISUAL)

			CVar(Visible, VA_LIST(std::vector<std::pair<std::string, Color_t>>) VA_LIST({ { "Original", {} } }), VISUAL)
			CVar(Occluded, VA_LIST(std::vector<std::pair<std::string, Color_t>>) {}, VISUAL)
		SUBNAMESPACE_END(Enemy)

		SUBNAMESPACE_BEGIN(Player)
			CVar(Local, false, VISUAL)
			CVar(Priority, false, VISUAL)
			CVar(Friend, false, VISUAL)
			CVar(Party, false, VISUAL)
			CVar(Target, false, VISUAL)
		
			CVar(Visible, VA_LIST(std::vector<std::pair<std::string, Color_t>>) VA_LIST({ { "Original", {} } }), VISUAL)
			CVar(Occluded, VA_LIST(std::vector<std::pair<std::string, Color_t>>) {}, VISUAL)
		SUBNAMESPACE_END(Player)

		SUBNAMESPACE_BEGIN(World)
			CVar(NPCs, false, VISUAL)
			CVar(Pickups, false, VISUAL)
			CVar(Objective, false, VISUAL)
			CVar(Powerups, false, VISUAL)
			CVar(Bombs, false, VISUAL)
			CVar(Halloween, false, VISUAL)
		
			CVar(Visible, VA_LIST(std::vector<std::pair<std::string, Color_t>>) VA_LIST({ { "Original", {} } }), VISUAL)
			CVar(Occluded, VA_LIST(std::vector<std::pair<std::string, Color_t>>) {}, VISUAL)
		SUBNAMESPACE_END(World)

		SUBNAMESPACE_BEGIN(Backtrack)
			CVar(Enabled, false, VISUAL)
			CVarEnum(Draw, 0b0, VISUAL, Last, LastFirst, All)
				
			CVar(Visible, VA_LIST(std::vector<std::pair<std::string, Color_t>>) VA_LIST({ { "Original", {} } }), VISUAL)
			//CVar(Occluded, VA_LIST(std::vector<std::pair<std::string, Color_t>>) {}, VISUAL) // unused
		SUBNAMESPACE_END(Backtrack)

		SUBNAMESPACE_BEGIN(FakeAngle)
			CVar(Enabled, false, VISUAL)

			CVar(Visible, VA_LIST(std::vector<std::pair<std::string, Color_t>>) VA_LIST({ { "Original", {} } }), VISUAL)
			//CVar(Occluded, VA_LIST(std::vector<std::pair<std::string, Color_t>>) {}, VISUAL) // unused
		SUBNAMESPACE_END(FakeAngle)

		SUBNAMESPACE_BEGIN(Viewmodel)
			CVar(Weapon, false, VISUAL)
			CVar(Hands, false, VISUAL)
			
			CVar(WeaponVisible, VA_LIST(std::vector<std::pair<std::string, Color_t>>) VA_LIST({ { "Original", {} } }), VISUAL)
			CVar(HandsVisible, VA_LIST(std::vector<std::pair<std::string, Color_t>>) VA_LIST({ { "Original", {} } }), VISUAL)
		SUBNAMESPACE_END(Viewmodel)
	NAMESPACE_END(Chams)

	NAMESPACE_BEGIN(Glow)
		SUBNAMESPACE_BEGIN(Friendly)
			CVar(Players, false, VISUAL)
			CVar(Buildings, false, VISUAL)
			CVar(Ragdolls, false, VISUAL)
			CVar(Projectiles, false, VISUAL)
				
			CVar(Stencil, 0, VISUAL)
			CVar(Blur, 0, VISUAL)
		SUBNAMESPACE_END(Friendly)

		SUBNAMESPACE_BEGIN(Enemy)
			CVar(Players, false, VISUAL)
			CVar(Buildings, false, VISUAL)
			CVar(Ragdolls, false, VISUAL)
			CVar(Projectiles, false, VISUAL)
				
			CVar(Stencil, 0, VISUAL)
			CVar(Blur, 0, VISUAL)
		SUBNAMESPACE_END(Enemy)

		SUBNAMESPACE_BEGIN(Player)
			CVar(Local, false, VISUAL)
			CVar(Priority, false, VISUAL)
			CVar(Friend, false, VISUAL)
			CVar(Party, false, VISUAL)
			CVar(Target, false, VISUAL)
				
			CVar(Stencil, 0, VISUAL)
			CVar(Blur, 0, VISUAL)
		SUBNAMESPACE_END(Player)

		SUBNAMESPACE_BEGIN(World)
			CVar(NPCs, false, VISUAL)
			CVar(Pickups, false, VISUAL)
			CVar(Objective, false, VISUAL)
			CVar(Powerups, false, VISUAL)
			CVar(Bombs, false, VISUAL)
			CVar(Halloween, false, VISUAL)
				
			CVar(Stencil, 0, VISUAL)
			CVar(Blur, 0, VISUAL)
		SUBNAMESPACE_END(World)

		SUBNAMESPACE_BEGIN(Backtrack)
			CVar(Enabled, false, VISUAL)
			CVarEnum(Draw, 0b0, VISUAL, Last, LastFirst, All)
				
			CVar(Stencil, 0, VISUAL)
			CVar(Blur, 0, VISUAL)
		SUBNAMESPACE_END(Backtrack)

		SUBNAMESPACE_BEGIN(FakeAngle)
			CVar(Enabled, false, VISUAL)
				
			CVar(Stencil, 0, VISUAL)
			CVar(Blur, 0, VISUAL)
		SUBNAMESPACE_END(FakeAngle)

		SUBNAMESPACE_BEGIN(Viewmodel)
			CVar(Weapon, false, VISUAL)
			CVar(Hands, false, VISUAL)

			CVar(Stencil, 0, VISUAL)
			CVar(Blur, 0, VISUAL)
		SUBNAMESPACE_END(Viewmodel)
	NAMESPACE_END(GLOW)

	NAMESPACE_BEGIN(Visuals)
		SUBNAMESPACE_BEGIN(Removals)
			CVar(Scope, false, VISUAL)
			CVar(Interpolation, false)
			CVar(Disguises, false, VISUAL)
			CVar(ScreenOverlays, false, VISUAL)
			CVar(Taunts, false, VISUAL)
			CVar(ScreenEffects, false, VISUAL)
			CVar(ViewPunch, false, VISUAL)
			CVar(AngleForcing, false, VISUAL)
			CVar(MOTD, false, VISUAL)
			CVar(ConvarQueries, false, VISUAL)
			CVar(PostProcessing, false, VISUAL)
			CVar(DSP, false, VISUAL)
		SUBNAMESPACE_END(Removals)

		SUBNAMESPACE_BEGIN(UI)
			CVarEnum(StreamerMode, 0, VISUAL, Off, Local, Friends, Party, All)
			CVarEnum(ChatTags, 0b000, VISUAL, Local = 1 << 0, Friends = 1 << 1, Party = 1 << 2, Assigned = 1 << 3)
			CVar(FieldOfView, 0, VISUAL)
			CVar(ZoomFieldOfView, 0, VISUAL)
			CVar(AspectRatio, 0.f, VISUAL)
			CVar(RevealScoreboard, false, VISUAL)
			CVar(ScoreboardUtility, false)
			CVar(ScoreboardColors, false, VISUAL)
			CVar(CleanScreenshots, true)
			CVar(SniperSightlines, false, VISUAL)
			CVar(PickupTimers, false, VISUAL)
		SUBNAMESPACE_END(Viewmodel)

		SUBNAMESPACE_BEGIN(Viewmodel)
			CVar(CrosshairAim, false, VISUAL)
			CVar(ViewmodelAim, false, VISUAL)
			CVar(OffsetX, 0, VISUAL)
			CVar(OffsetY, 0, VISUAL)
			CVar(OffsetZ, 0, VISUAL)
			CVar(Pitch, 0, VISUAL)
			CVar(Yaw, 0, VISUAL)
			CVar(Roll, 0, VISUAL)
			CVar(FieldOfView, 0.f, VISUAL)
			CVar(SwayScale, 0.f, VISUAL)
			CVar(SwayInterp, 0.f, VISUAL)
		SUBNAMESPACE_END(Viewmodel)

		SUBNAMESPACE_BEGIN(Particles)
			CVar(BulletTrail, std::string("Off"), VISUAL)
			CVar(CritTrail, std::string("Off"), VISUAL)
			CVar(MedigunBeam, std::string("Off"), VISUAL)
			CVar(MedigunCharge, std::string("Off"), VISUAL)
			CVar(ProjectileTrail, std::string("Off"), VISUAL)
			CVarEnum(SpellFootsteps, 0, VISUAL, Off, Color, Team, Halloween)
			CVar(DrawIconsThroughWalls, false, VISUAL)
			CVar(DrawDamageNumbersThroughWalls, false, VISUAL)
		SUBNAMESPACE_END(Tracers)

		SUBNAMESPACE_BEGIN(Beams) // as of now, these will stay out of the menu
			CVar(Active, false, VISUAL)
			CVar(BeamColor, Color_t(255, 255, 255, 255), VISUAL)
			CVar(Model, std::string("sprites/physbeam.vmt"), VISUAL)
			CVar(Life, 2.f, VISUAL)
			CVar(Width, 2.f, VISUAL)
			CVar(EndWidth, 2.f, VISUAL)
			CVar(FadeLength, 10.f, VISUAL)
			CVar(Amplitude, 2.f, VISUAL)
			CVar(Brightness, 255.f, VISUAL)
			CVar(Speed, 0.2f, VISUAL)
			CVar(Flags, 0b10000000100000000, VISUAL) // { Reverse, Halobeam, Forever, Is active, End visible, Start visible, Use hitboxes, No tile, Only noise once, Shade out, Shade in, Solid, Sine noise, Fade out, Fade in, End entity, Start entity }
			CVar(Segments, 2, VISUAL)
		SUBNAMESPACE_END(Beams)

		SUBNAMESPACE_BEGIN(Ragdolls)
			CVar(NoRagdolls, false, VISUAL)
			CVar(NoGib, false, VISUAL)
			CVar(Enabled, false, VISUAL)
			CVar(EnemyOnly, false, VISUAL)
			CVarEnum(Effects, 0b0000, VISUAL, Burning = 1 << 0, Electrocuted = 1 << 1, Ash = 1 << 2, Dissolve = 1 << 3)
			CVarEnum(Type, 0, VISUAL, None, Gold, Ice)
			CVar(Force, 1.f, VISUAL)
			CVar(ForceHorizontal, 1.f, VISUAL)
			CVar(ForceVertical, 1.f, VISUAL)
		SUBNAMESPACE_END(RagdollEffects)

		SUBNAMESPACE_BEGIN(Line)
			CVar(Enabled, false, VISUAL)
			CVar(DrawDuration, 5.f, VISUAL)
		SUBNAMESPACE_END(Line)

		SUBNAMESPACE_BEGIN(Simulation)
			Enum(Style, Off, Line, Separators, Spaced, Arrows, Boxes);
			CVar(PlayerPath, 0, VISUAL)
			CVar(ProjectilePath, 0, VISUAL)
			CVar(TrajectoryPath, 0, VISUAL)
			CVar(ShotPath, 0, VISUAL)
			CVarEnum(SplashRadius, 0b0, VISUAL, Simulation = 1 << 0, Priority = 1 << 1, Enemy = 1 << 2, Team = 1 << 3, Local = 1 << 4, Friends = 1 << 5, Party = 1 << 6, Rockets = 1 << 7, Stickies = 1 << 8, Pipes = 1 << 9, ScorchShot = 1 << 10, Trace = 1 << 11)
			CVar(Timed, false, VISUAL)
			CVar(Box, true, VISUAL)
			CVar(SwingLines, false, VISUAL)
			CVar(ProjectileCamera, false, VISUAL)
			CVar(ProjectileWindow, WindowBox_t(), NOBIND)
			CVar(DrawDuration, 5.f, VISUAL)
			CVar(SeparatorSpacing, 4, DEBUGVAR)
			CVar(SeparatorLength, 12, DEBUGVAR)
		SUBNAMESPACE_END(ProjectileTrajectory)

		SUBNAMESPACE_BEGIN(Trajectory)
			CVar(Overwrite, false, DEBUGVAR)
			CVar(OffX, 16.f, DEBUGVAR)
			CVar(OffY, 8.f, DEBUGVAR)
			CVar(OffZ, -6.f, DEBUGVAR)
			CVar(Pipes, true, DEBUGVAR)
			CVar(Hull, 5.f, DEBUGVAR)
			CVar(Speed, 1200.f, DEBUGVAR)
			CVar(Gravity, 1.f, DEBUGVAR)
			CVar(NoSpin, false, DEBUGVAR)
			CVar(LifeTime, 2.2f, DEBUGVAR)
			CVar(UpVelocity, 200.f, DEBUGVAR)
			CVar(AngVelocityX, 600.f, DEBUGVAR)
			CVar(AngVelocityY, -1200.f, DEBUGVAR)
			CVar(AngVelocityZ, 0.f, DEBUGVAR)
			CVar(Drag, 1.f, DEBUGVAR)
			CVar(DragBasisX, 0.003902f, DEBUGVAR)
			CVar(DragBasisY, 0.009962f, DEBUGVAR)
			CVar(DragBasisZ, 0.009962f, DEBUGVAR)
			CVar(AngDragBasisX, 0.003618f, DEBUGVAR)
			CVar(AngDragBasisY, 0.001514f, DEBUGVAR)
			CVar(AngDragBasisZ, 0.001514f, DEBUGVAR)
			CVar(MaxVelocity, 2000.f, DEBUGVAR)
			CVar(MaxAngularVelocity, 3600.f, DEBUGVAR)
		SUBNAMESPACE_END(ProjectileTrajectory)

		SUBNAMESPACE_BEGIN(Hitbox)
			CVarEnum(BonesEnabled, 0b00, VISUAL, OnShot = 1 << 0, OnHit = 1 << 1)
			CVarEnum(BoundsEnabled, 0b000, VISUAL, OnShot = 1 << 0, OnHit = 1 << 1, AimPoint = 1 << 2)
			CVar(DrawDuration, 5.f, VISUAL)
		SUBNAMESPACE_END(Hitbox)

		SUBNAMESPACE_BEGIN(ThirdPerson)
			CVar(Enabled, false, VISUAL)
			CVar(Distance, 200.f, VISUAL)
			CVar(Right, 0.f, VISUAL)
			CVar(Up, 0.f, VISUAL)
			CVar(Crosshair, false, VISUAL)
			CVar(Scale, true, DEBUGVAR)
		SUBNAMESPACE_END(ThirdPerson)

		SUBNAMESPACE_BEGIN(FOVArrows)
			CVar(Enabled, false, VISUAL)
			CVar(Offset, 25, VISUAL)
			CVar(MaxDist, 1000.f, VISUAL)
		SUBNAMESPACE_END(Arrows)

		SUBNAMESPACE_BEGIN(World)
			CVarEnum(Modulations, 0b00000, VISUAL, World = 1 << 0, Sky = 1 << 1, Prop = 1 << 2, Particle = 1 << 3, Fog = 1 << 4)
			CVar(SkyboxChanger, std::string("Off"), VISUAL)
			CVar(WorldTexture, std::string("Default"), VISUAL)
			CVar(NearPropFade, false, VISUAL)
			CVar(NoPropFade, false, VISUAL)
		SUBNAMESPACE_END(World)

		SUBNAMESPACE_BEGIN(Misc)
			CVar(LocalDominationOverride, std::string(""), VISUAL)
			CVar(LocalRevengeOverride, std::string(""), VISUAL)
			CVar(DominationOverride, std::string(""), VISUAL)
			CVar(RevengeOverride, std::string(""), VISUAL)
		SUBNAMESPACE_END(Misc)
	NAMESPACE_END(Visuals)

	NAMESPACE_BEGIN(Radar)
		SUBNAMESPACE_BEGIN(Main)
			CVar(Enabled, false, VISUAL)
			CVar(AlwaysDraw, true, VISUAL)
			CVarEnum(Style, 0, VISUAL, Circle, Rectangle)
			CVar(Window, WindowBox_t(), NOBIND)
			CVar(Range, 1500, VISUAL)
			CVar(BackAlpha, 128, VISUAL)
			CVar(LineAlpha, 255, VISUAL)
		SUBNAMESPACE_END(Main)

		SUBNAMESPACE_BEGIN(Players)
			CVar(Enabled, false, VISUAL)
			CVar(Background, true, VISUAL)
			CVarEnum(Draw, 0b1001010, VISUAL, Local = 1 << 0, Enemy = 1 << 1, Team = 1 << 2, Prioritized = 1 << 3, Friends = 1 << 4, Party = 1 << 5, Cloaked = 1 << 6)
			CVarEnum(IconType, 1, VISUAL, Icons, Portraits, Avatars)
			CVar(IconSize, 24, VISUAL)
			CVar(Health, false, VISUAL)
			CVar(Height, false, VISUAL)
		SUBNAMESPACE_END(Players)

		SUBNAMESPACE_BEGIN(Buildings)
			CVar(Enabled, false, VISUAL)
			CVar(Background, true, VISUAL)
			CVarEnum(Draw, 0b001011, VISUAL, Local = 1 << 0, Enemy = 1 << 1, Team = 1 << 2, Prioritized = 1 << 3, Friends = 1 << 4, Party = 1 << 5)
			CVar(Health, false, VISUAL)
			CVar(IconSize, 18, VISUAL)
		SUBNAMESPACE_END(Buildings)

		SUBNAMESPACE_BEGIN(World)
			CVar(Enabled, false, VISUAL)
			CVar(Background, true, VISUAL)
			CVarEnum(Draw, 0b0000011, VISUAL, Health = 1 << 0, Ammo = 1 << 1, Money = 1 << 2, Bombs = 1 << 3, Powerup = 1 << 4, Spellbook = 1 << 5, Gargoyle = 1 << 6)
			CVar(IconSize, 14, VISUAL)
		SUBNAMESPACE_END(World)
	NAMESPACE_END(Radar)

	NAMESPACE_BEGIN(Misc)
		SUBNAMESPACE_BEGIN(Movement)
			CVarEnum(AutoStrafe, 0, NONE, Off, Legit, Directional)
			CVar(AutoStrafeTurnScale, 0.5f)
			CVar(AutoStrafeMaxDelta, 180.f)
			CVar(Bunnyhop, false)
			CVar(AutoJumpbug, false)
			CVar(AutoRocketJump, false)
			CVar(AutoCTap, false)
			CVar(FastStop, false)
			CVar(FastAccel, false)
			CVar(NoPush, false)
			CVar(CrouchSpeed, false)
			CVar(MovementLock, false)
			CVar(BreakJump, false)
			CVar(ShieldTurnRate, false)

			CVar(TimingOffset, 0, DEBUGVAR)
			CVar(ChokeCount, 1, DEBUGVAR)
			CVar(ApplyAbove, 0, DEBUGVAR)
		SUBNAMESPACE_END(Movement)

		SUBNAMESPACE_BEGIN(Exploits)
			CVar(CheatsBypass, false)
			CVar(BypassPure, false)
			CVar(PingReducer, false)
			CVar(PingTarget, 1)
			CVar(EquipRegionUnlock, false)
		SUBNAMESPACE_END(Exploits)

		SUBNAMESPACE_BEGIN(Automation)
			CVarEnum(AntiBackstab, 0, NONE, Off, Yaw, Pitch, Fake)
			CVar(AntiAFK, false)
			CVar(AntiAutobalance, false)
			CVar(AcceptItemDrops, false)
			CVar(TauntControl, false)
			CVar(KartControl, false)
			CVar(BackpackExpander, true)
			CVar(AutoF2Ignored, false)
			CVar(AutoF1Priority, false)
		SUBNAMESPACE_END(Automation)

		SUBNAMESPACE_BEGIN(Sound)
			CVarEnum(Block, 0b000, NONE, Footsteps = 1 << 0, Noisemaker = 1 << 1, FryingPan = 1 << 2)
			CVar(GiantWeaponSounds, false)
			CVar(HitsoundAlways, false)
		SUBNAMESPACE_END(Sound)

		SUBNAMESPACE_BEGIN(Game)
			CVar(NetworkFix, false)
			CVar(PredictionErrorJitterFix, false)
			CVar(SetupBonesOptimization, false)
			CVar(F2PChatBypass, false)
		SUBNAMESPACE_END(Game)

		SUBNAMESPACE_BEGIN(Queueing)
			CVarEnum(ForceRegions, 0b0, NONE, // i'm not sure all of these are actually used for tf2 servers
				// North America
				DC_ATL = 1 << 0, // Atlanta
				DC_ORD = 1 << 1, // Chicago
				DC_DFW = 1 << 2, // Texas
				DC_LAX = 1 << 3, // Los Angeles
				DC_EAT = 1 << 4, // Moses Lake
				DC_JFK = 1 << 5, // New York
				DC_SEA = 1 << 6, // Seattle
				DC_IAD = 1 << 7, // Virginia
				// Europe
				DC_AMS = 1 << 8, // Amsterdam
				DC_FRA = 1 << 9, // Frankfurt
				DC_HEL = 1 << 10, // Helsinki
				DC_LHR = 1 << 11, // London
				DC_MAD = 1 << 12, // Madrid
				DC_PAR = 1 << 13, // Paris
				DC_STO = 1 << 14, /*& DC_STO2*/ // Stockholm
				DC_VIE = 1 << 15, // Vienna
				DC_WAW = 1 << 16, // Warsaw
				// South America
				DC_EZE = 1 << 17, // Buenos Aires
				DC_LIM = 1 << 18, // Lima
				DC_SCL = 1 << 19, // Santiago
				DC_GRU = 1 << 20, // Sao Paulo
				// Asia
				DC_BOM2 = 1 << 21, // Bombay
				DC_MAA = 1 << 22, // Chennai
				DC_DXB = 1 << 23, // Dubai
				DC_HKG = 1 << 24, // Hong Kong
				DC_MAA2 = 1 << 25, // Madras
				DC_BOM = 1 << 26, // Mumbai
				DC_SEO = 1 << 27, // Seoul
				DC_SGP = 1 << 28, // Singapore
				DC_TYO = 1 << 29, // Tokyo
				// Australia
				DC_SYD = 1 << 30, // Sydney
				// Africa
				DC_JNB = 1 << 31, // Johannesburg
			)
			CVar(FreezeQueue, false)
			CVar(AutoCasualQueue, false)
		SUBNAMESPACE_END(Queueing)

		SUBNAMESPACE_BEGIN(MannVsMachine)
			CVar(InstantRespawn, false)
			CVar(InstantRevive, false)
			CVar(AllowInspect, false)
		SUBNAMESPACE_END(Sound)

		SUBNAMESPACE_BEGIN(Steam)
			CVar(EnableRPC, false)
			CVar(OverrideMenu, false)
			CVarEnum(MatchGroup, 0, NONE, SpecialEvent, MvMMannUp, Competitive, Casual, MvMBootCamp)
			CVar(MapText, std::string("Amalgam"))
			CVar(GroupSize, 1337)
		SUBNAMESPACE_END(Steam)
	NAMESPACE_END(Misc)

	NAMESPACE_BEGIN(Colors)
		CVar(FOVCircle, Color_t(255, 255, 255, 100), VISUAL)
		CVar(Relative, false, VISUAL)
		CVar(TeamRed, Color_t(225, 60, 60, 255), VISUAL)
		CVar(TeamBlu, Color_t(75, 175, 225, 255), VISUAL)
		CVar(Enemy, Color_t(225, 60, 60, 255), VISUAL)
		CVar(Team, Color_t(75, 175, 225, 255), VISUAL)
		CVar(Local, Color_t(255, 255, 255, 255), VISUAL)
		CVar(Target, Color_t(255, 0, 0, 255), VISUAL)
		CVar(Health, Color_t(0, 225, 75, 255), VISUAL)
		CVar(Ammo, Color_t(127, 127, 127, 255), VISUAL)
		CVar(Money, Color_t(0, 150, 75, 255), VISUAL)
		CVar(Powerup, Color_t(255, 175, 0, 255), VISUAL)
		CVar(NPC, Color_t(255, 255, 255, 255), VISUAL)
		CVar(Halloween, Color_t(100, 0, 255, 255), VISUAL)

		CVar(IndicatorGood, Color_t(0, 255, 100, 255), DEBUGVAR)
		CVar(IndicatorMid, Color_t(255, 200, 0, 255), DEBUGVAR)
		CVar(IndicatorBad, Color_t(255, 0, 0, 255), DEBUGVAR)
		CVar(IndicatorMisc, Color_t(75, 175, 255, 255), DEBUGVAR)
		CVar(IndicatorTextGood, Color_t(150, 255, 150, 255), DEBUGVAR)
		CVar(IndicatorTextMid, Color_t(255, 200, 0, 255), DEBUGVAR)
		CVar(IndicatorTextBad, Color_t(255, 150, 150, 255), DEBUGVAR)
		CVar(IndicatorTextMisc, Color_t(100, 255, 255, 255), DEBUGVAR)

		CVar(WorldModulation, Color_t(255, 255, 255, 255), VISUAL)
		CVar(SkyModulation, Color_t(255, 255, 255, 255), VISUAL)
		CVar(PropModulation, Color_t(255, 255, 255, 255), VISUAL)
		CVar(ParticleModulation, Color_t(255, 255, 255, 255), VISUAL)
		CVar(FogModulation, Color_t(255, 255, 255, 255), VISUAL)

		CVar(Line, Color_t(255, 255, 255, 0), VISUAL)
		CVar(LineClipped, Color_t(255, 255, 255, 255), VISUAL)

		CVar(PlayerPath, Color_t(255, 255, 255, 255), VISUAL)
		CVar(PlayerPathClipped, Color_t(255, 255, 255, 0), VISUAL)
		CVar(ProjectilePath, Color_t(255, 255, 255, 255), VISUAL)
		CVar(ProjectilePathClipped, Color_t(255, 255, 255, 0), VISUAL)
		CVar(TrajectoryPath, Color_t(255, 255, 255, 255), VISUAL)
		CVar(TrajectoryPathClipped, Color_t(255, 255, 255, 0), VISUAL)
		CVar(ShotPath, Color_t(255, 255, 255, 255), VISUAL)
		CVar(ShotPathClipped, Color_t(255, 255, 255, 0), VISUAL)
		CVar(SplashRadius, Color_t(255, 255, 255, 255), VISUAL)
		CVar(SplashRadiusClipped, Color_t(255, 255, 255, 0), VISUAL)

		CVar(BoneHitboxEdge, Color_t(255, 255, 255, 0), VISUAL)
		CVar(BoneHitboxEdgeClipped, Color_t(255, 255, 255, 255), VISUAL)
		CVar(BoneHitboxFace, Color_t(255, 255, 255, 0), VISUAL)
		CVar(BoneHitboxFaceClipped, Color_t(255, 255, 255, 0), VISUAL)
		CVar(TargetHitboxEdge, Color_t(255, 150, 150, 0), VISUAL)
		CVar(TargetHitboxEdgeClipped, Color_t(255, 150, 150, 255), VISUAL)
		CVar(TargetHitboxFace, Color_t(255, 150, 150, 0), VISUAL)
		CVar(TargetHitboxFaceClipped, Color_t(255, 150, 150, 0), VISUAL)
		CVar(BoundHitboxEdge, Color_t(255, 255, 255, 0), VISUAL)
		CVar(BoundHitboxEdgeClipped, Color_t(255, 255, 255, 255), VISUAL)
		CVar(BoundHitboxFace, Color_t(255, 255, 255, 0), VISUAL)
		CVar(BoundHitboxFaceClipped, Color_t(255, 255, 255, 0), VISUAL)

		CVar(SpellFootstep, Color_t(255, 255, 255, 0), VISUAL)
	NAMESPACE_END(Colors)

	NAMESPACE_BEGIN(Logging)
		CVarEnum(Logs, 0b0000011, NONE, VoteStart = 1 << 0, VoteCast = 1 << 1, ClassChanges = 1 << 2, Damage = 1 << 3, CheatDetection = 1 << 4, Tags = 1 << 5, Aliases = 1 << 6, Resolver = 1 << 7)
		Enum(LogTo, Toasts = 1 << 0, Chat = 1 << 1, Party = 1 << 2, Console = 1 << 3)
		CVar(Lifetime, 5.f, VISUAL)

		SUBNAMESPACE_BEGIN(VoteStart)
			CVar(LogTo, 0b0001)
		SUBNAMESPACE_END(VoteStart)

		SUBNAMESPACE_BEGIN(VoteCast)
			CVar(LogTo, 0b0001)
		SUBNAMESPACE_END(VoteCast)

		SUBNAMESPACE_BEGIN(ClassChange)
			CVar(LogTo, 0b0001)
		SUBNAMESPACE_END(ClassChange)

		SUBNAMESPACE_BEGIN(Damage)
			CVar(LogTo, 0b0001)
		SUBNAMESPACE_END(Damage)

		SUBNAMESPACE_BEGIN(CheatDetection)
			CVar(LogTo, 0b0001)
		SUBNAMESPACE_END(CheatDetection)

		SUBNAMESPACE_BEGIN(Tags)
			CVar(LogTo, 0b0001)
		SUBNAMESPACE_END(Tags)

		SUBNAMESPACE_BEGIN(Aliases)
			CVar(LogTo, 0b0001)
		SUBNAMESPACE_END(Aliases)

		SUBNAMESPACE_BEGIN(Resolver)
			CVar(LogTo, 0b0001)
		SUBNAMESPACE_END(Resolver)
	NAMESPACE_END(Logging)

	NAMESPACE_BEGIN(Debug)
		CVar(Info, false, NOSAVE)
		CVar(Logging, false, NOSAVE)
		CVar(ServerHitbox, false, NOSAVE)
		CVar(AntiAimLines, false)
		CVar(CrashLogging, true)
#ifdef DEBUG_TRACES
		CVar(VisualizeTraces, false, NOSAVE)
		CVar(VisualizeTraceHits, false, NOSAVE)
#endif
	NAMESPACE_END(Debug)
}