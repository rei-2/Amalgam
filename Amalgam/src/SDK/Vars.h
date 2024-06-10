#pragma once
#include "../SDK/Definitions/Types.h"
#include <windows.h>
#include <unordered_map>
#include <typeinfo>

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
	ConfigVar<T>* GetVar()
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
	std::unordered_map<std::string, T> Map;
	ConfigVar(T value, std::string name, int iFlags = 0);
};

inline std::vector<CVarBase*> g_Vars;

template<class T>
inline ConfigVar<T>::ConfigVar(T value, std::string name, int iFlags)
{
	Default = value;
	Value = value;
	Map["default"] = value;
	m_iType = typeid(T).hash_code();
	m_sName = name;
	g_Vars.push_back(this);
	m_iFlags = iFlags;
}

#define NAMESPACE_BEGIN(name) \
	namespace name { \
		inline std::string GetNamespace() { return "Vars::" + std::string(#name) + "::"; } \
		inline std::string GetSubname() { return ""; } \

#define SUBNAMESPACE_BEGIN(name) \
	namespace name { \
		inline std::string GetSubname() { return std::string(#name) + "::"; } \

#define NAMESPACE_END(name) \
	}
#define SUBNAMESPACE_END(name) \
	}

#define CVar(name, value, ...) inline ConfigVar<decltype(value)> name{ value, GetNamespace() + GetSubname() + std::string(#name), __VA_ARGS__ };

#define VISUAL (1 << 0)
#define NOSAVE (1 << 1)
#define NOCOND (1 << 2)

namespace Vars
{
	NAMESPACE_BEGIN(Menu)
		CVar(CheatName, std::string("Amalgam"), VISUAL)
		CVar(CheatPrefix, std::string("[Amalgam]"), VISUAL)
		CVar(MenuPrimaryKey, VK_INSERT, NOCOND)
		CVar(MenuSecondaryKey, VK_F3, NOCOND)

		CVar(ShowBinds, false)
		CVar(BindsDisplay, DragBox_t(), NOCOND)
		CVar(MenuShowsBinds, false, NOCOND)

		CVar(Indicators, 0b0)
		//CVar(SpectatorAvatars, false, VISUAL)

		CVar(TicksDisplay, DragBox_t(), NOCOND)
		CVar(CritsDisplay, DragBox_t(), NOCOND)
		CVar(SpectatorsDisplay, DragBox_t(), NOCOND)
		CVar(PingDisplay, DragBox_t(), NOCOND)
		CVar(ConditionsDisplay, DragBox_t(), NOCOND)
		CVar(SeedPredictionDisplay, DragBox_t(), NOCOND)

		CVar(DPI, 1.f, NOCOND)

		SUBNAMESPACE_BEGIN(Theme) // possibly reduce the amount of theme vars
			CVar(Accent, Color_t(255, 101, 101, 255), VISUAL)
			CVar(Background, Color_t(23, 23, 23, 250), VISUAL)
			CVar(Foreground, Color_t(11, 11, 11, 250), VISUAL)
			CVar(Foremost, Color_t(23, 23, 23, 250), VISUAL)
			CVar(Inactive, Color_t(150, 150, 150, 255), VISUAL)
			CVar(Active, Color_t(255, 255, 255, 255), VISUAL)
		SUBNAMESPACE_END(Theme)
	NAMESPACE_END(Menu)

	NAMESPACE_BEGIN(Aimbot)
		SUBNAMESPACE_BEGIN(General)
			CVar(AimType, 0) // 0 - Off, 1 - Normal, 2 - Smooth, 3 - Silent
			CVar(TargetSelection, 0) // 0 - FOV, 1 - Distance
			CVar(Target, 0b0000001) // { Bombs, NPCs, Stickies, Teleporter, Dispenser, Sentry, Players }
			CVar(Ignore, 0b00000000) // { Taunting, Disguised, Unsimulated, Vaccinator, Friends, Dead Ringer, Cloaked, Invulnerable }
			CVar(AimFOV, 30.f)
			CVar(Smoothing, 25.f)
			CVar(MaxTargets, 2)
			CVar(IgnoreCloakPercentage, 100)
			CVar(TickTolerance, 7)
			CVar(AutoShoot, true)
			CVar(FOVCircle, true)
			CVar(NoSpread, false)

			CVar(HitscanPeek, 3, NOSAVE) // debug
			CVar(NoSpreadOffset, 0.f, NOSAVE) // debug
			CVar(NoSpreadAverage, 5, NOSAVE) // debug
		SUBNAMESPACE_END(Global)

		SUBNAMESPACE_BEGIN(Hitscan)
			CVar(Hitboxes, 0b00111) // { Legs, Arms, Body, Pelvis, Head }
			CVar(Modifiers, 0) // { Extinguish team, Bodyaim if lethal, Auto scope, Scoped only, Wait for charge, Wait for heatshot, Tapfire }
			CVar(PointScale, 0.f)
			CVar(TapFireDist, 1000.f)
		SUBNAMESPACE_END(HITSCAN)

		SUBNAMESPACE_BEGIN(Projectile)
			CVar(StrafePrediction, 0b11) // { Ground, Air }
			CVar(SplashPrediction, 0) // 0 - Off, 1 - Obstructed, 2 - Prefer, 3 - Only
			CVar(AutoDetonate, 0b00) // { Flares, Stickies }
			CVar(AutoAirblast, 0) // 0 - Off, 1 - Legit, 2 - Rage
			CVar(Modifiers, 0b010) // { Bodyaim if lethal, Cancel charge, Charge weapon }
			CVar(PredictionTime, 2.f)
			CVar(Hitchance, 0.f)
			CVar(AutodetRadius, 90.f)
			CVar(SplashRadius, 90.f)
			CVar(AutoRelease, 0.f)

			CVar(GroundSamples, 5, NOSAVE) // debug
			CVar(AirSamples, 5, NOSAVE) // debug
			CVar(VerticalShift, 5.f, NOSAVE) // debug
			CVar(LatencyOffset, 0.f, NOSAVE) // debug
			CVar(HullIncrease, 0.f, NOSAVE) // debug
			CVar(DragOverride, 0.f, NOSAVE) // debug
			CVar(TimeOverride, 0.f, NOSAVE) // debug
			CVar(HuntsmanLerp, 50.f, NOSAVE) // debug
			CVar(SplashPoints, 80, NOSAVE) // debug
			CVar(SplashCount, 5, NOSAVE) // debug
		SUBNAMESPACE_END(Projectile)

		SUBNAMESPACE_BEGIN(Melee)
			CVar(AutoBackstab, true)
			CVar(IgnoreRazorback, true)
			CVar(SwingPrediction, false)
			CVar(WhipTeam, false)

			CVar(SwingTicks, 13, NOSAVE) // debug
		SUBNAMESPACE_END(Melee)
	NAMESPACE_END(AIMBOT)
	
	NAMESPACE_BEGIN(CritHack)
		CVar(ForceCrits, false)
		CVar(AvoidRandom, false)
		CVar(AlwaysMelee, false)
	NAMESPACE_END(CritHack)

	NAMESPACE_BEGIN(Backtrack)
		CVar(Enabled, false)
		CVar(PreferOnShot, false)
		CVar(Latency, 0)
		CVar(Interp, 0)
		CVar(Window, 185)

		CVar(Offset, 0, NOSAVE) // debug
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
		SUBNAMESPACE_END(DoubleTap)

		SUBNAMESPACE_BEGIN(Fakelag)
			CVar(Fakelag, 0) // 0 - Off, 1 - Plain, 2 - Random, 3 - Adaptive
			CVar(PlainTicks, 12)
			CVar(RandomTicks, IntRange_t(14, 18));
			CVar(Options, 0b000) // { While Airborne, While Unducking, While Moving }
			CVar(UnchokeOnAttack, true)
			CVar(RetainBlastJump, false)
		SUBNAMESPACE_END(FakeLag)

		CVar(AutoPeek, false)

		CVar(SpeedEnabled, false)
		CVar(SpeedFactor, 1)
	NAMESPACE_END(CL_Move)

	NAMESPACE_BEGIN(AntiHack)
		SUBNAMESPACE_BEGIN(AntiAim)
			CVar(Enabled, false)
			CVar(PitchReal, 0) // 0 - None, 1 - Up, 2 - Down, 3 - Zero
			CVar(PitchFake, 0) // 0 - None, 1 - Up, 2 - Down
			CVar(YawReal, 0) // 0 - Forward, 1 - Left, 2 - Right, 3 - Backwards, 4 - Spin, 5 - Edge
			CVar(YawFake, 0) // 0 - Forward, 1 - Left, 2 - Right, 3 - Backwards, 4 - Spin, 5 - Edge
			CVar(RealYawMode, 0) // 0 - View, 1 - Target
			CVar(FakeYawMode, 0) // 0 - View, 1 - Target
			CVar(RealYawOffset, 0)
			CVar(FakeYawOffset, 0)
			CVar(SpinSpeed, 15.f)
			CVar(MinWalk, true)
			CVar(AntiOverlap, false)
			CVar(InvalidShootPitch, false)
		SUBNAMESPACE_END(AntiAim)

		SUBNAMESPACE_BEGIN(Resolver)
			CVar(Resolver, false)
			CVar(AutoResolveCheaters, false)
			CVar(IgnoreAirborne, false)
		SUBNAMESPACE_END(Resolver)
	NAMESPACE_END(AntiHack)

	NAMESPACE_BEGIN(CheaterDetection)
		CVar(Methods, 0b00001) // { Duck Speed, Aim flicking, Packet choking, Invalid pitch }
		CVar(DetectionsRequired, 10)
		CVar(MinimumChoking, 20)
		CVar(MinimumFlick, 20.f) // min flick size to suspect
		CVar(MaximumNoise, 5.f) // max different between angles before and after flick
	NAMESPACE_END(CheaterDetection)

	NAMESPACE_BEGIN(ESP)
		CVar(Draw, 0, VISUAL)
		CVar(Player, 0, VISUAL)
		CVar(Building, 0, VISUAL)

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
			
			CVar(VisibleMaterial, std::vector<std::string> { "Original" }, VISUAL)
			CVar(OccludedMaterial, std::vector<std::string> {}, VISUAL)
			CVar(VisibleColor, Color_t(255, 255, 255, 255), VISUAL)
			CVar(OccludedColor, Color_t(255, 255, 255, 255), VISUAL)
		SUBNAMESPACE_END(Friendly)

		SUBNAMESPACE_BEGIN(Enemy)
			CVar(Players, false, VISUAL)
			CVar(Buildings, false, VISUAL)
			CVar(Ragdolls, false, VISUAL)
			CVar(Projectiles, false, VISUAL)

			CVar(VisibleMaterial, std::vector<std::string> { "Original" }, VISUAL)
			CVar(OccludedMaterial, std::vector<std::string> {}, VISUAL)
			CVar(VisibleColor, Color_t(255, 255, 255, 255), VISUAL)
			CVar(OccludedColor, Color_t(255, 255, 255, 255), VISUAL)
		SUBNAMESPACE_END(Enemy)

		SUBNAMESPACE_BEGIN(Player)
			CVar(Local, false, VISUAL)
			CVar(Friend, false, VISUAL)
			CVar(Priority, false, VISUAL)
		
			CVar(VisibleMaterial, std::vector<std::string> { "Original" }, VISUAL)
			CVar(OccludedMaterial, std::vector<std::string> {}, VISUAL)
			CVar(VisibleColor, Color_t(255, 255, 255, 255), VISUAL)
			CVar(OccludedColor, Color_t(255, 255, 255, 255), VISUAL)
		SUBNAMESPACE_END(Player)

		SUBNAMESPACE_BEGIN(World)
			CVar(NPCs, false, VISUAL)
			CVar(Pickups, false, VISUAL)
			CVar(Bombs, false, VISUAL)
			CVar(Halloween, false, VISUAL)
		
			CVar(VisibleMaterial, std::vector<std::string> { "Original" }, VISUAL)
			CVar(OccludedMaterial, std::vector<std::string> {}, VISUAL)
			CVar(VisibleColor, Color_t(255, 255, 255, 255), VISUAL)
			CVar(OccludedColor, Color_t(255, 255, 255, 255), VISUAL)
		SUBNAMESPACE_END(World)

		SUBNAMESPACE_BEGIN(Backtrack)
			CVar(Enabled, false, VISUAL)
			CVar(Draw, 0, VISUAL)
				
			CVar(VisibleMaterial, std::vector<std::string> { "Original" }, VISUAL)
			CVar(OccludedMaterial, std::vector<std::string> {}, NOSAVE) // unused
			CVar(VisibleColor, Color_t(255, 255, 255, 255), VISUAL)
			CVar(OccludedColor, Color_t(255, 255, 255, 255), NOSAVE) // unused
		SUBNAMESPACE_END(Backtrack)

		SUBNAMESPACE_BEGIN(FakeAngle)
			CVar(Enabled, false, VISUAL)
		
			CVar(VisibleMaterial, std::vector<std::string> { "Original" }, VISUAL)
			CVar(OccludedMaterial, std::vector<std::string> {}, NOSAVE) // unused
			CVar(VisibleColor, Color_t(255, 255, 255, 255), VISUAL)
			CVar(OccludedColor, Color_t(255, 255, 255, 255), NOSAVE) // unused
		SUBNAMESPACE_END(FakeAngle)

		SUBNAMESPACE_BEGIN(Viewmodel)
			CVar(Weapon, false, VISUAL)
			CVar(Hands, false, VISUAL)
			
			CVar(VisibleMaterial, std::vector<std::string> { "Original" }, VISUAL)
			CVar(OccludedMaterial, std::vector<std::string> {}, NOSAVE) // unused
			CVar(VisibleColor, Color_t(255, 255, 255, 255), VISUAL)
			CVar(OccludedColor, Color_t(255, 255, 255, 255), NOSAVE) // unused
		SUBNAMESPACE_END(Viewmodel)
	NAMESPACE_END(Chams)

	NAMESPACE_BEGIN(Glow)
		SUBNAMESPACE_BEGIN(Friendly)
			CVar(Players, false, VISUAL)
			CVar(Buildings, false, VISUAL)
			CVar(Ragdolls, false, VISUAL)
			CVar(Projectiles, false, VISUAL)
				
			CVar(Stencil, false, VISUAL)
			CVar(Blur, false, VISUAL)
			CVar(StencilScale, 1, VISUAL)
			CVar(BlurScale, 1, VISUAL)
		SUBNAMESPACE_END(Friendly)

		SUBNAMESPACE_BEGIN(Enemy)
			CVar(Players, false, VISUAL)
			CVar(Buildings, false, VISUAL)
			CVar(Ragdolls, false, VISUAL)
			CVar(Projectiles, false, VISUAL)
				
			CVar(Stencil, false, VISUAL)
			CVar(Blur, false, VISUAL)
			CVar(StencilScale, 1, VISUAL)
			CVar(BlurScale, 1, VISUAL)
		SUBNAMESPACE_END(Enemy)

		SUBNAMESPACE_BEGIN(Player)
			CVar(Local, false, VISUAL)
			CVar(Friend, false, VISUAL)
			CVar(Priority, false, VISUAL)
				
			CVar(Stencil, false, VISUAL)
			CVar(Blur, false, VISUAL)
			CVar(StencilScale, 1, VISUAL)
			CVar(BlurScale, 1, VISUAL)
		SUBNAMESPACE_END(Player)

		SUBNAMESPACE_BEGIN(World)
			CVar(NPCs, false, VISUAL)
			CVar(Pickups, false, VISUAL)
			CVar(Bombs, false, VISUAL)
			CVar(Halloween, false, VISUAL)
				
			CVar(Stencil, false, VISUAL)
			CVar(Blur, false, VISUAL)
			CVar(StencilScale, 1, VISUAL)
			CVar(BlurScale, 1, VISUAL)
		SUBNAMESPACE_END(World)

		SUBNAMESPACE_BEGIN(Backtrack)
			CVar(Enabled, false, VISUAL)
			CVar(Draw, 0, VISUAL)
				
			CVar(Stencil, false, VISUAL)
			CVar(Blur, false, VISUAL)
			CVar(StencilScale, 1, VISUAL)
			CVar(BlurScale, 1, VISUAL)
		SUBNAMESPACE_END(Backtrack)

		SUBNAMESPACE_BEGIN(FakeAngle)
			CVar(Enabled, false, VISUAL)
				
			CVar(Stencil, false, VISUAL)
			CVar(Blur, false, VISUAL)
			CVar(StencilScale, 1, VISUAL)
			CVar(BlurScale, 1, VISUAL)
		SUBNAMESPACE_END(FakeAngle)

		SUBNAMESPACE_BEGIN(Viewmodel)
			CVar(Weapon, false, VISUAL)
			CVar(Hands, false, VISUAL)

			CVar(Stencil, false, VISUAL)
			CVar(Blur, false, VISUAL)
			CVar(StencilScale, 1, VISUAL)
			CVar(BlurScale, 1, VISUAL)
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
			CVar(FieldOfView, 0, VISUAL)
			CVar(ZoomFieldOfView, 0, VISUAL)
			CVar(RevealScoreboard, false, VISUAL)
			CVar(ScoreboardPlayerlist, false)
			CVar(CleanScreenshots, true)
			CVar(ScoreboardColors, false, VISUAL)
			CVar(SniperSightlines, false, VISUAL)
			CVar(PickupTimers, false, VISUAL)
		SUBNAMESPACE_END(Viewmodel)

		SUBNAMESPACE_BEGIN(Viewmodel)
			CVar(CrosshairAim, false, VISUAL)
			CVar(ViewmodelAim, false, VISUAL)
			CVar(OffsetX, 0, VISUAL)
			CVar(OffsetY, 0, VISUAL)
			CVar(OffsetZ, 0, VISUAL)
			CVar(Roll, 0, VISUAL)
			CVar(Sway, false, VISUAL)
			CVar(SwayScale, 5.f, VISUAL)
			CVar(SwayInterp, 0.05f, VISUAL)
		SUBNAMESPACE_END(Viewmodel)

		SUBNAMESPACE_BEGIN(Tracers)
			CVar(ParticleTracer, std::string("Off"), VISUAL)
			CVar(ParticleTracerCrits, std::string("Off"), VISUAL)
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
		SUBNAMESPACE_END(Beans)

		SUBNAMESPACE_BEGIN(Ragdolls)
			CVar(NoRagdolls, false, VISUAL)
			CVar(NoGib, false, VISUAL)
			CVar(Enabled, false, VISUAL)
			CVar(EnemyOnly, false, VISUAL)
			CVar(Effects, 0, VISUAL)
			CVar(Type, 0, VISUAL)
			CVar(Force, 1.f, VISUAL)
			CVar(ForceHorizontal, 1.f, VISUAL)
			CVar(ForceVertical, 1.f, VISUAL)
		SUBNAMESPACE_END(RagdollEffects)

		SUBNAMESPACE_BEGIN(Bullet)
			CVar(BulletTracer, false, VISUAL)
		SUBNAMESPACE_END(Bullet)

		SUBNAMESPACE_BEGIN(Simulation)
			CVar(Enabled, false, VISUAL)
			CVar(Timed, false, VISUAL)
			CVar(Separators, false, VISUAL)
			CVar(SeparatorLength, 12, VISUAL)
			CVar(SeparatorSpacing, 6, VISUAL)
			CVar(ProjectileTrajectory, false, VISUAL)
			CVar(ProjectileCamera, false, VISUAL)
			CVar(ProjectileWindow, WindowBox_t(), NOCOND)
			CVar(TrajectoryOnShot, false, VISUAL)
			CVar(SwingLines, false, VISUAL)
		SUBNAMESPACE_END(ProjectileTrajectory)

		SUBNAMESPACE_BEGIN(Trajectory)
			CVar(Overwrite, false, NOSAVE) // debug
			CVar(OffX, 16.f, NOSAVE) // debug
			CVar(OffY, 8.f, NOSAVE) // debug
			CVar(OffZ, -6.f, NOSAVE) // debug
			CVar(Pipes, true, NOSAVE) // debug
			CVar(Hull, 5.f, NOSAVE) // debug
			CVar(Speed, 1200.f, NOSAVE) // debug
			CVar(Gravity, 1.f, NOSAVE) // debug
			CVar(NoSpin, false, NOSAVE) // debug
			CVar(LifeTime, 2.2f, NOSAVE) // debug
			CVar(UpVelocity, 200.f, NOSAVE) // debug
			CVar(AngVelocityX, 600.f, NOSAVE) // debug
			CVar(AngVelocityY, -1200.f, NOSAVE) // debug
			CVar(AngVelocityZ, 0.f, NOSAVE) // debug
			CVar(Drag, 1.f, NOSAVE) // debug
			CVar(DragBasisX, 0.003902f, NOSAVE) // debug
			CVar(DragBasisY, 0.009962f, NOSAVE) // debug
			CVar(DragBasisZ, 0.009962f, NOSAVE) // debug
			CVar(AngDragBasisX, 0.003618f, NOSAVE) // debug
			CVar(AngDragBasisY, 0.001514f, NOSAVE) // debug
			CVar(AngDragBasisZ, 0.001514f, NOSAVE) // debug
			CVar(MaxVelocity, 2000.f, NOSAVE) // debug
			CVar(MaxAngularVelocity, 3600.f, NOSAVE) // debug
		SUBNAMESPACE_END(ProjectileTrajectory)

		SUBNAMESPACE_BEGIN(Hitbox)
			CVar(ShowHitboxes, false, VISUAL)
		SUBNAMESPACE_END(Hitbox)

		SUBNAMESPACE_BEGIN(ThirdPerson)
			CVar(Enabled, false, VISUAL)
			CVar(Distance, 200.f, VISUAL)
			CVar(Right, 0.f, VISUAL)
			CVar(Up, 0.f, VISUAL)
			CVar(Crosshair, false, VISUAL)
		SUBNAMESPACE_END(ThirdPerson)

		SUBNAMESPACE_BEGIN(FOVArrows)
			CVar(Enabled, false, VISUAL)
			CVar(Offset, 25, VISUAL)
			CVar(MaxDist, 1000.f, VISUAL)
		SUBNAMESPACE_END(Arrows)

		SUBNAMESPACE_BEGIN(World)
			CVar(Modulations, 0b00000, VISUAL) // { Fog, Particle, Prop, Sky, World }
			CVar(SkyboxChanger, std::string("Off"), VISUAL)
			CVar(WorldTexture, std::string("Default"), VISUAL)
			CVar(NearPropFade, false, VISUAL)
			CVar(NoPropFade, false, VISUAL)
		SUBNAMESPACE_END(World)
	NAMESPACE_END(Visuals)

	NAMESPACE_BEGIN(Radar)
		SUBNAMESPACE_BEGIN(Main)
			CVar(Enabled, false, VISUAL)
			CVar(AlwaysDraw, true, VISUAL)
			CVar(Style, 0, VISUAL) // 0 - Circle, 1 - Rectangle
			CVar(Window, WindowBox_t(), NOCOND)
			CVar(Range, 1500, VISUAL)
			CVar(BackAlpha, 128, VISUAL)
			CVar(LineAlpha, 255, VISUAL)
		SUBNAMESPACE_END(Main)

		SUBNAMESPACE_BEGIN(Players)
			CVar(Enabled, false, VISUAL)
			CVar(Background, true, VISUAL)
			CVar(IconType, 1, VISUAL) // 0 - Icons, 1 - Portraits, 2 - Avatars
			CVar(Draw, 0b101010, VISUAL) // { Cloaked, Priority, Friends, Team, Enemy, Local }
			CVar(IconSize, 24, VISUAL)
			CVar(Health, false, VISUAL)
			CVar(Height, false, VISUAL)
		SUBNAMESPACE_END(Players)

		SUBNAMESPACE_BEGIN(Buildings)
			CVar(Enabled, false, VISUAL)
			CVar(Background, true, VISUAL)
			CVar(Draw, 0b01011, VISUAL) // { Priority, Friends, Team, Enemy, Local }
			CVar(Health, false, VISUAL)
			CVar(IconSize, 18, VISUAL)
		SUBNAMESPACE_END(Buildings)

		SUBNAMESPACE_BEGIN(World)
			CVar(Enabled, false, VISUAL)
			CVar(Background, true, VISUAL)
			CVar(Draw, 0b00011, VISUAL) // { Halloween, Bombs, Money, Ammo, Health }
			CVar(IconSize, 14, VISUAL)
		SUBNAMESPACE_END(World)
	NAMESPACE_END(Radar)

	NAMESPACE_BEGIN(Misc)
		SUBNAMESPACE_BEGIN(Movement)
			CVar(AutoStrafe, 0)
			CVar(AutoStrafeTurnScale, 0.5f)
			CVar(Bunnyhop, false)
			CVar(AutoJumpbug, false)
			CVar(AutoRocketJump, false)
			CVar(AutoCTap, false)
			CVar(FastStop, false)
			CVar(FastAccel, false)
			CVar(FastStrafe, false)
			CVar(NoPush, false)
			CVar(CrouchSpeed, false)

			CVar(TimingOffset, -1, NOSAVE) // debug
			CVar(ApplyAbove, 0, NOSAVE) // debug
		SUBNAMESPACE_END(Movement)

		SUBNAMESPACE_BEGIN(Exploits)
			CVar(CheatsBypass, false)
			CVar(BypassPure, false)
			CVar(PingReducer, false)
			CVar(PingTarget, 1)
			CVar(EquipRegionUnlock, false)
		SUBNAMESPACE_END(Exploits)

		SUBNAMESPACE_BEGIN(Automation)
			CVar(AntiBackstab, false)
			CVar(AntiAFK, false)
			CVar(AntiAutobalance, false)
			CVar(AcceptItemDrops, false)
			CVar(TauntControl, false)
			CVar(KartControl, false)
			CVar(BackpackExpander, true)
		SUBNAMESPACE_END(Automation)

		SUBNAMESPACE_BEGIN(Sound)
			CVar(Block, 0)
			CVar(GiantWeaponSounds, false)
			CVar(HitsoundAlways, false)
		SUBNAMESPACE_END(Sound)

		SUBNAMESPACE_BEGIN(Game)
			CVar(NetworkFix, false)
			CVar(PredictionErrorJitterFix, false)
			CVar(SetupBonesOptimization, false)
		SUBNAMESPACE_END(Game)

		SUBNAMESPACE_BEGIN(Queueing)
			CVar(ForceRegions, 0)
			CVar(FreezeQueue, false)
			CVar(AutoCasualQueue, false)
		SUBNAMESPACE_END(Queueing)

		SUBNAMESPACE_BEGIN(MannVsMachine)
			CVar(InstantRespawn, false)
			CVar(InstantRevive, false)
		SUBNAMESPACE_END(Sound)

		SUBNAMESPACE_BEGIN(Chat)
			CVar(Tags, false)
		SUBNAMESPACE_END(Chat)

		SUBNAMESPACE_BEGIN(Steam)
			CVar(EnableRPC, false)
			CVar(OverrideMenu, false)
			CVar(MatchGroup, 0) // 0 - Special Event; 1 - MvM Mann Up; 2 - Competitive; 3 - Casual; 4 - MvM Boot Camp;
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
		CVar(Invulnerable, Color_t(125, 100, 175, 255), VISUAL)
		CVar(Cloak, Color_t(150, 175, 210, 255), VISUAL)
		CVar(Overheal, Color_t(75, 175, 255, 255), VISUAL)
		CVar(HealthBar, Gradient_t({ 255, 0, 0, 255 }, { 0, 200, 125, 255 }), VISUAL)
		CVar(UberBar, Color_t( 127, 255, 255, 255 ), VISUAL)
		CVar(Health, Color_t(0, 225, 75, 255), VISUAL)
		CVar(Ammo, Color_t(175, 175, 175, 255), VISUAL)
		CVar(NPC, Color_t(255, 255, 255, 255), VISUAL)
		CVar(Bomb, Color_t(255, 75, 0, 255), VISUAL)
		CVar(Money, Color_t(0, 150, 75, 255), VISUAL)
		CVar(Halloween, Color_t(100, 0, 255, 255), VISUAL)

		CVar(WorldModulation, Color_t(255, 255, 255, 255), VISUAL)
		CVar(SkyModulation, Color_t(255, 255, 255, 255), VISUAL)
		CVar(PropModulation, Color_t(255, 255, 255, 255), VISUAL)
		CVar(ParticleModulation, Color_t(255, 255, 255, 255), VISUAL)
		CVar(FogModulation, Color_t(255, 255, 255, 255), VISUAL)

		CVar(BulletTracer, Color_t(255, 255, 255, 255), VISUAL)
		CVar(PredictionColor, Color_t(255, 255, 255, 255), VISUAL)
		CVar(ProjectileColor, Color_t(255, 100, 100, 255), VISUAL)
		CVar(ClippedColor, Color_t(255, 255, 255, 0), VISUAL)
		CVar(HitboxEdge, Color_t(255, 255, 255, 255), VISUAL)
		CVar(HitboxFace, Color_t(255, 255, 255, 0), VISUAL)
	NAMESPACE_END(Colors)

	NAMESPACE_BEGIN(Logging)
		CVar(Logs, 0b0011) // { Damage, Class Changes, Vote cast, Vote start, Cheat Detection, Tags }
		// LogTo // { Console, Party, Chat, Toasts }
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
	NAMESPACE_END(Logging)

	NAMESPACE_BEGIN(Debug)
		CVar(Info, false, NOSAVE)
		CVar(Logging, false, NOSAVE)
		CVar(ServerHitbox, false, NOSAVE)
		CVar(AntiAimLines, false, NOSAVE)
	NAMESPACE_END(Debug)
}