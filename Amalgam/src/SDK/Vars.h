#pragma once
#include "../SDK/Definitions/Types.h"
#include "../Utils/Macros/Macros.h"
#include <windows.h>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <typeinfo>

#define DEFAULT_BIND -1

template <class T>
class ConfigVar;

class BaseVar
{
public:
	std::vector<const char*> m_vNames;
	int m_iFlags = 0;
	union {
		int i = 0;
		float f;
	} m_unMin;
	union {
		int i = 0;
		float f;
	} m_unMax;
	union {
		int i = 0;
		float f;
	} m_unStep;
	std::vector<const char*> m_vValues = {};
	const char* m_sExtra = nullptr;

protected:
	std::string m_sName;
	const char* m_sSection;

public:
	constexpr const char* Name() const
	{
		return m_sName.c_str();
	}
	constexpr const char* Section() const
	{
		return m_sSection;
	}

public:
	size_t m_iType;

	template <class T>
	inline ConfigVar<T>* As()
	{
		if (typeid(T).hash_code() != m_iType)
			return nullptr;

		return reinterpret_cast<ConfigVar<T>*>(this);
	}
};

namespace G
{
	inline std::vector<BaseVar*> Vars = {};
};

template <class T>
class ConfigVar : public BaseVar
{
public:
	T Value;
	T Default;
	std::unordered_map<int, T> Map = {};

	ConfigVar(T tValue, std::vector<const char*> vNames, const char* sName, const char* sSection, int iFlags = 0, std::vector<const char*> vValues = {}, const char* sNone = nullptr)
	{
		Value = Default = Map[DEFAULT_BIND] = tValue;
		m_iType = typeid(T).hash_code();

		m_vNames = vNames;
		m_sName = std::string(sName).replace(strlen(sName) - 1, 1, "");
		m_sSection = sSection;

		m_iFlags = iFlags;
		m_vValues = vValues;
		m_sExtra = sNone;

		G::Vars.push_back(this);
	}
	ConfigVar(T tValue, std::vector<const char*> vNames, const char* sName, const char* sSection, int iFlags, int iMin, int iMax, int iStep = 1, const char* sFormat = "%i")
	{
		Value = Default = Map[DEFAULT_BIND] = tValue;
		m_iType = typeid(T).hash_code();

		m_vNames = vNames;
		m_sName = std::string(sName).replace(strlen(sName) - 1, 1, "");
		m_sSection = sSection;

		m_iFlags = iFlags;
		m_unMin.i = iMin;
		m_unMax.i = iMax;
		m_unStep.i = iStep;
		m_sExtra = sFormat;

		G::Vars.push_back(this);
	}
	ConfigVar(T tValue, std::vector<const char*> vNames, const char* sName, const char* sSection, int iFlags, float flMin, float flMax, float flStep = 1.f, const char* sFormat = "%g")
	{
		Value = Default = Map[DEFAULT_BIND] = tValue;
		m_iType = typeid(T).hash_code();

		m_vNames = vNames;
		m_sName = std::string(sName).replace(strlen(sName) - 1, 1, "");
		m_sSection = sSection;

		m_iFlags = iFlags;
		m_unMin.f = flMin;
		m_unMax.f = flMax;
		m_unStep.f = flStep;
		m_sExtra = sFormat;

		G::Vars.push_back(this);
	}
	
	inline T& operator[](int i)
	{
		return Map[i];
	}
	inline bool contains(int i) const
	{
		return Map.contains(i);
	}
};

#define NAMESPACE_BEGIN(name, ...) \
	namespace name { \
		constexpr inline const char* Section() { return !std::string(#__VA_ARGS__).empty() ? ""#__VA_ARGS__ : #name; }
#define NAMESPACE_END(name) \
	}

#define CVar(name, title, value, ...) \
	constexpr inline const char* name##_() { return __FUNCTION__; } \
	inline ConfigVar<decltype(value)> name = { value, { title }, name##_(), Section(), __VA_ARGS__ }
#define CVarValues(name, title, value, flags, none, ...) \
	constexpr inline const char* name##_() { return __FUNCTION__; } \
	inline ConfigVar<decltype(value)> name = { value, { title }, name##_(), Section(), flags, { __VA_ARGS__ }, none }
#define Enum(name, ...) \
	namespace name##Enum { enum name##Enum { __VA_ARGS__ }; }
#define CVarEnum(name, title, value, flags, none, values, ...) \
	CVarValues(name, title, value, flags, none, values); \
	Enum(name, __VA_ARGS__);

#define NONE 0
#define VISUAL (1 << 31)
#define NOSAVE (1 << 30)
#define NOBIND (1 << 29)
#define DEBUGVAR (1 << 28)

// flags to be automatically used in widgets. keep these as the same values as the flags in components, do not include visual flags
#define SLIDER_CLAMP (1 << 2)
#define SLIDER_MIN (1 << 3)
#define SLIDER_MAX (1 << 4)
#define SLIDER_PRECISION (1 << 5)
#define SLIDER_NOAUTOUPDATE (1 << 6)
#define DROPDOWN_MULTI (1 << 2)
#define DROPDOWN_MODIFIABLE (1 << 3)
#define DROPDOWN_NOSANITIZATION (1 << 4)
#define DROPDOWN_CUSTOM (1 << 2)
#define DROPDOWN_AUTOUPDATE (1 << 3)

NAMESPACE_BEGIN(Vars)
	NAMESPACE_BEGIN(Config)
		CVar(LoadDebugSettings, "Load debug settings", false);
	NAMESPACE_END(Config)
	
	NAMESPACE_BEGIN(Menu)
		CVar(CheatTitle, "Cheat title", std::string("unibox"), VISUAL | DROPDOWN_AUTOUPDATE);
		CVar(CheatTag, "Cheat tag", std::string("[unibox]"), VISUAL);
		CVar(PrimaryKey, "Primary key", VK_INSERT, NOBIND);
		CVar(SecondaryKey, "Secondary key", VK_F3, NOBIND);

		CVar(BindWindow, "Bind window", true);
		CVar(BindWindowTitle, "Bind window title", true);
		CVar(MenuShowsBinds, "Menu shows binds", false, NOBIND);

		CVarEnum(Indicators, "Indicators", 0b00000, VISUAL | DROPDOWN_MULTI, nullptr,
			VA_LIST("Ticks", "Crit hack", "Spectators", "Ping", "Conditions", "Seed prediction", "Navbot"),
			Ticks = 1 << 0, CritHack = 1 << 1, Spectators = 1 << 2, Ping = 1 << 3, Conditions = 1 << 4, SeedPrediction = 1 << 5, NavBot = 1 << 6);

		CVar(BindsDisplay, "Binds display", DragBox_t(100, 100), VISUAL | NOBIND);
		CVar(TicksDisplay, "Ticks display", DragBox_t(), VISUAL | NOBIND);
		CVar(CritsDisplay, "Crits display", DragBox_t(), VISUAL | NOBIND);
		CVar(SpectatorsDisplay, "Spectators display", DragBox_t(), VISUAL | NOBIND);
		CVar(PingDisplay, "Ping display", DragBox_t(), VISUAL | NOBIND);
		CVar(ConditionsDisplay, "Conditions display", DragBox_t(), VISUAL | NOBIND);
		CVar(SeedPredictionDisplay, "Seed prediction display", DragBox_t(), VISUAL | NOBIND);
		CVar(NavBotDisplay, "Navbot display", DragBox_t(), VISUAL | NOBIND);

		CVar(Scale, "Scale", 1.f, NOBIND | SLIDER_MIN | SLIDER_PRECISION | SLIDER_NOAUTOUPDATE, 0.75f, 2.f, 0.25f);
		CVar(CheapText, "Cheap text", false, NOBIND);

		NAMESPACE_BEGIN(Theme)
			CVar(Accent, "Accent color", Color_t(149, 139, 209, 255), VISUAL);
			CVar(Background, "Background color", Color_t(31, 29, 53, 250), VISUAL);
			CVar(Active, "Active color", Color_t(229, 226, 238, 255), VISUAL);
			CVar(Inactive, "Inactive color", Color_t(159, 159, 219, 255), VISUAL);
		NAMESPACE_END(Theme)
	NAMESPACE_END(Menu)

	NAMESPACE_BEGIN(Colors)
		CVar(IndicatorGood, "Indicator good", Color_t(0, 255, 100, 255), NOSAVE | DEBUGVAR);
		CVar(IndicatorMid, "Indicator mid", Color_t(255, 200, 0, 255), NOSAVE | DEBUGVAR);
		CVar(IndicatorBad, "Indicator bad", Color_t(255, 0, 0, 255), NOSAVE | DEBUGVAR);
		CVar(IndicatorMisc, "Indicator misc", Color_t(75, 175, 255, 255), NOSAVE | DEBUGVAR);
		CVar(IndicatorTextGood, "Indicator text good", Color_t(150, 255, 150, 255), NOSAVE | DEBUGVAR);
		CVar(IndicatorTextMid, "Indicator text mid", Color_t(255, 200, 0, 255), NOSAVE | DEBUGVAR);
		CVar(IndicatorTextBad, "Indicator text bad", Color_t(255, 150, 150, 255), NOSAVE | DEBUGVAR);
		CVar(IndicatorTextMisc, "Indicator text misc", Color_t(100, 255, 255, 255), NOSAVE | DEBUGVAR);

		CVar(Local, "Local color", Color_t(255, 255, 255), VISUAL);
		CVar(FOVCircle, "FOV circle color", Color_t(255, 255, 255, 100), VISUAL);
		CVar(NoSpread, "Nospread color", Color_t(75, 175, 255, 255), VISUAL);
		CVar(SpellFootstep, "Spell footstep color", Color_t(255, 255, 255, 255), VISUAL);

		CVar(WorldModulation, VA_LIST("World modulation", "World modulation color"), Color_t(255, 255, 255, 255), VISUAL);
		CVar(SkyModulation, VA_LIST("Sky modulation", "Sky modulation color"), Color_t(255, 255, 255, 255), VISUAL);
		CVar(PropModulation, VA_LIST("Prop modulation", "Prop modulation color"), Color_t(255, 255, 255, 255), VISUAL);
		CVar(ParticleModulation, VA_LIST("Particle modulation", "Particle modulation color"), Color_t(255, 255, 255, 255), VISUAL);
		CVar(FogModulation, VA_LIST("Fog modulation", "Fog modulation color"), Color_t(255, 255, 255, 255), VISUAL);

		CVar(Line, "Line color", Color_t(255, 255, 255, 255), VISUAL);
		CVar(LineIgnoreZ, "Line ignore Z color", Color_t(255, 255, 255, 0), VISUAL);

		CVar(BoneHitboxEdge, "Bone hitbox edge color", Color_t(255, 255, 255, 255), VISUAL);
		CVar(BoneHitboxEdgeIgnoreZ, "Bone hitbox edge ignore Z color", Color_t(255, 255, 255, 0), VISUAL);
		CVar(BoneHitboxFace, "Bone hitbox face color", Color_t(255, 255, 255, 0), VISUAL);
		CVar(BoneHitboxFaceIgnoreZ, "Bone hitbox face ignore Z color", Color_t(255, 255, 255, 0), VISUAL);
		CVar(TargetHitboxEdge, "Target hitbox edge color", Color_t(255, 150, 150, 255), VISUAL);
		CVar(TargetHitboxEdgeIgnoreZ, "Target hitbox edge ignore Z color", Color_t(255, 150, 150, 0), VISUAL);
		CVar(TargetHitboxFace, "Target hitbox face color", Color_t(255, 150, 150, 0), VISUAL);
		CVar(TargetHitboxFaceIgnoreZ, "Target hitbox face ignore Z color", Color_t(255, 150, 150, 0), VISUAL);
		CVar(BoundHitboxEdge, "Bound hitbox edge color", Color_t(255, 255, 255, 255), VISUAL);
		CVar(BoundHitboxEdgeIgnoreZ, "Bound hitbox edge ignore Z color", Color_t(255, 255, 255, 0), VISUAL);
		CVar(BoundHitboxFace, "Bound hitbox face color", Color_t(255, 255, 255, 0), VISUAL);
		CVar(BoundHitboxFaceIgnoreZ, "Bound hitbox face ignore Z color", Color_t(255, 255, 255, 0), VISUAL);

		CVar(PlayerPath, "Player path color", Color_t(255, 255, 255, 0), VISUAL);
		CVar(PlayerPathIgnoreZ, "Player path ignore Z color", Color_t(255, 255, 255, 255), VISUAL);
		CVar(ProjectilePath, "Projectile path color", Color_t(255, 255, 255, 0), VISUAL);
		CVar(ProjectilePathIgnoreZ, "Projectile path ignore Z color", Color_t(255, 255, 255, 255), VISUAL);
		CVar(TrajectoryPath, "Trajectory path color", Color_t(255, 255, 255, 0), VISUAL);
		CVar(TrajectoryPathIgnoreZ, "Trajectory path ignore Z color", Color_t(255, 255, 255, 255), VISUAL);
		CVar(ShotPath, "Shot path color", Color_t(255, 255, 255, 0), VISUAL);
		CVar(ShotPathIgnoreZ, "Shot path ignore Z color", Color_t(255, 255, 255, 255), VISUAL);
		CVar(SplashRadius, "Splash radius color", Color_t(255, 255, 255, 0), VISUAL);
		CVar(SplashRadiusIgnoreZ, "Splash radius ignore Z color", Color_t(255, 255, 255, 255), VISUAL);
		CVar(RealPath, "Real path color", Color_t(255, 255, 255, 0), NOSAVE | DEBUGVAR);
		CVar(RealPathIgnoreZ, "Real path ignore Z color", Color_t(255, 255, 255, 255), NOSAVE | DEBUGVAR);

		CVar(NavbotPath, "Navbot path color", Color_t(255, 255, 0, 255), VISUAL);
		CVar(NavbotArea, "Navbot area color", Color_t(0, 255, 0, 255), VISUAL);
		CVar(NavbotBlacklist, "Navbot blacklisted color", Color_t(255, 0, 0, 255), VISUAL);
		CVar(FollowbotPathLine, "Followbot path line color", Color_t(255, 255, 0, 255), VISUAL);
		CVar(FollowbotPathBox, "Followbot path box color", Color_t(255, 255, 0, 255), VISUAL);
		CVar(EdgebugPath, "Auto edgebug path color", Color_t(255, 255, 255, 255), VISUAL);

		CVar(HurtTrigger, "Hurt trigger color", Color_t(248, 155, 0, 80), VISUAL);
		CVar(IgniteTrigger, "Ignite trigger color", Color_t(248, 155, 0, 80), VISUAL);
		CVar(PushTrigger, "Push trigger color", Color_t(248, 155, 0, 80), VISUAL);
		CVar(RegenerateTrigger, "Regenerate trigger color", Color_t(248, 155, 0, 80), VISUAL);
		CVar(RespawnRoomTrigger, "Respawn room trigger color", Color_t(248, 155, 0, 80), VISUAL);
		CVar(CaptureAreaTrigger, "Capture area trigger color", Color_t(248, 155, 0, 80), VISUAL);
		CVar(CatapultTrigger, "Catapult trigger color", Color_t(248, 155, 0, 80), VISUAL);
		CVar(ApplyImpulseTrigger, "Apply impulse trigger color", Color_t(248, 155, 0, 80), VISUAL);
		CVar(TriggerAngle, "Trigger angle color", Color_t(50, 100, 200, 255), VISUAL);
		CVar(TriggerSurfaceCenter, "Trigger surface center color", Color_t(60, 200, 10, 80), VISUAL);
	NAMESPACE_END(Colors)

	NAMESPACE_BEGIN(Aimbot)
		NAMESPACE_BEGIN(General, Aimbot)
			CVarEnum(AimType, "Aim type", 0, NONE, nullptr,
				VA_LIST("Off", "Plain", "Smooth", "Silent", "Locking", "Assistive", "Legit", "SmoothVelocity"),
				Off, Plain, Smooth, Silent, Locking, Assistive, Legit, SmoothVelocity);
			CVarEnum(TargetSelection, "Target selection", 0, NONE, nullptr,
				VA_LIST("FOV", "Distance", "Hybrid", "Most health", "Least health"),
				FOV, Distance, Hybrid, MostHealth, LeastHealth);
			CVarEnum(Target, "Target", 0b0000001, DROPDOWN_MULTI, nullptr,
				VA_LIST("Players", "Sentries", "Dispensers", "Teleporters", "Stickies", "NPCs", "Bombs"),
				Players = 1 << 0, Sentry = 1 << 1, Dispenser = 1 << 2, Teleporter = 1 << 3, Stickies = 1 << 4, NPCs = 1 << 5, Bombs = 1 << 6,
				All = Players | Sentry | Dispenser | Teleporter | Stickies | NPCs | Bombs, Building = Sentry | Dispenser | Teleporter);
			CVarEnum(Ignore, "Ignore", 0b00000001000, DROPDOWN_MULTI, nullptr,
				VA_LIST("Friends", "Party", "Unprioritized", "Invulnerable", "Invisible", "Unsimulated", "Dead ringer", "Vaccinator", "Disguised", "Taunting", "Team", "Sentry busters"),
				Friends = 1 << 0, Party = 1 << 1, Unprioritized = 1 << 2, Invulnerable = 1 << 3, Invisible = 1 << 4, Unsimulated = 1 << 5, DeadRinger = 1 << 6, Vaccinator = 1 << 7, Disguised = 1 << 8, Taunting = 1 << 9, Team = 1 << 10, SentryBusters = 1 << 11);
			CVar(AimFOV, "Aim FOV", 180.f, SLIDER_CLAMP | SLIDER_PRECISION, 0.f, 360.f);
			CVar(MaxTargets, "Max targets", 2, SLIDER_MIN, 1, 6);
			CVar(IgnoreInvisible, "Ignore invisible", 50.f, SLIDER_CLAMP | SLIDER_PRECISION, 0.f, 100.f, 10.f, "%g%%");
			CVar(AssistStrength, "Assist strength", 25.f, SLIDER_CLAMP | SLIDER_PRECISION, 0.f, 100.f, 1.f, "%g%%");
			CVarEnum(SmoothCurve, "Smooth curve", 0, NONE, nullptr,
				VA_LIST("Linear", "Fast start", "Fast end", "Slow start", "Slow end"),
				Linear, FastStart, FastEnd, SlowStart, SlowEnd);
			CVar(SmoothCurveAmount, "Smooth curve amount", 100.f, SLIDER_CLAMP | SLIDER_PRECISION, 0.f, 200.f, 5.f, "%g%%");
			CVar(TickTolerance, "Tick tolerance", 4, SLIDER_CLAMP, 0, 21);
			CVar(AutoShoot, "Auto shoot", true);
			CVar(FOVCircle, "FOV Circle", true, VISUAL);
			CVar(LeadAndRestrict, "Lead and restrict", false, VISUAL);
			CVar(NoSpread, "No spread", false);

			CVarEnum(AimHoldsFire, "Aim holds fire", 2, NOSAVE | DEBUGVAR, nullptr,
				VA_LIST("False", "Minigun only", "Always"),
				False, MinigunOnly, Always);
			CVar(NoSpreadOffset, "No spread offset", 0.f, NOSAVE | DEBUGVAR | SLIDER_PRECISION, -1.f, 1.f, 0.1f);
			CVar(NoSpreadAverage, "No spread average", 5, NOSAVE | DEBUGVAR | SLIDER_MIN, 1, 25);
			CVar(NoSpreadInterval, "No spread interval", 0.1f, NOSAVE | DEBUGVAR | SLIDER_MIN, 0.05f, 5.f, 0.1f, "%gs");
			CVar(NoSpreadBackupInterval, "No spread backup interval", 2.f, NOSAVE | DEBUGVAR | SLIDER_MIN, 2.f, 10.f, 0.1f, "%gs");
		NAMESPACE_END(General)

		NAMESPACE_BEGIN(Hitscan)
			CVarEnum(Hitboxes, VA_LIST("Hitboxes", "Hitscan hitboxes"), 0b000111, DROPDOWN_MULTI, nullptr,
				VA_LIST("Head", "Body", "Pelvis", "Arms", "Legs", "##Divider", "Bodyaim if lethal", "Headshot only"),
				Head = 1 << 0, Body = 1 << 1, Pelvis = 1 << 2, Arms = 1 << 3, Legs = 1 << 4, BodyaimIfLethal = 1 << 5, HeadshotOnly = 1 << 6);
			CVarValues(MultipointHitboxes, "Multipoint hitboxes", 0b00000, DROPDOWN_MULTI, "All",
				VA_LIST("Head", "Body", "Pelvis", "Arms", "Legs"));
			CVarEnum(Modifiers, VA_LIST("Modifiers", "Hitscan modifiers"), 0b0100000, DROPDOWN_MULTI, nullptr,
				VA_LIST("Tapfire", "Wait for headshot", "Wait for charge", "Scoped only", "Auto scope", "Auto rev minigun", "Extinguish team", "Prefer medics"),
				Tapfire = 1 << 0, WaitForHeadshot = 1 << 1, WaitForCharge = 1 << 2, ScopedOnly = 1 << 3, AutoScope = 1 << 4, AutoRev = 1 << 5, ExtinguishTeam = 1 << 6, PreferMedics = 1 << 7);
			CVar(MultipointScale, "Multipoint scale", 0.f, SLIDER_CLAMP | SLIDER_PRECISION, 0.f, 100.f, 5.f, "%g%%");
			CVar(TapfireDistance, "Tapfire distance", 1000.f, SLIDER_MIN | SLIDER_PRECISION, 250.f, 1000.f, 50.f);

			CVarEnum(PeekCheck, "Peek check", 1, NOSAVE | DEBUGVAR, nullptr,
				VA_LIST("Off", "Doubletap only", "Always"),
				Off, DoubletapOnly, Always);
			CVar(PeekAmount, "Peek amount", 1, NOSAVE | DEBUGVAR, 0, 5);
			CVar(BoneSizeSubtract, "Bone size subtract", 1.f, NOSAVE | DEBUGVAR | SLIDER_MIN, 0.f, 4.f, 0.25f);
			CVar(BoneSizeMinimumScale, "Bone size minimum scale", 1.f, NOSAVE | DEBUGVAR | SLIDER_CLAMP, 0.f, 1.f, 0.1f);
		NAMESPACE_END(Hitscan)

		NAMESPACE_BEGIN(Projectile)
			CVarEnum(StrafePrediction, VA_LIST("Predict", "Strafe prediction"), 0b11, DROPDOWN_MULTI, "Off",
				VA_LIST("Air strafing", "Ground strafing"),
				Air = 1 << 0, Ground = 1 << 1);
			CVarEnum(SplashPrediction, VA_LIST("Splash", "Splash prediction"), 2, NONE, nullptr,
				VA_LIST("Off", "Include", "Prefer", "Only"),
				Off, Include, Prefer, Only);
			CVarEnum(AutoDetonate, "Auto detonate", 0b00, DROPDOWN_MULTI, "Off",
				VA_LIST("Stickies", "Flares", "##Divider", "Damage priority", "Prevent self damage", "Ignore invisible"),
				Stickies = 1 << 0, Flares = 1 << 1, MaxDamage = 1 << 2, PreventSelfDamage = 1 << 3, IgnoreInvisible = 1 << 4);
			CVarEnum(AutoAirblast, "Auto airblast", 0b000, DROPDOWN_MULTI, "Off", // todo: implement advanced redirect!!
				VA_LIST("Enabled", "##Divider", "Redirect", "Ignore FOV"),
				Enabled = 1 << 0, Redirect = 1 << 1, IgnoreFOV = 1 << 2);
			CVarEnum(Hitboxes, VA_LIST("Hitboxes", "Projectile hitboxes"), 0b001111, DROPDOWN_MULTI, nullptr,
				VA_LIST("Auto", "##Divider", "Head", "Body", "Feet", "##Divider", "Bodyaim if lethal", "Prioritize feet"),
				Auto = 1 << 0, Head = 1 << 1, Body = 1 << 2, Feet = 1 << 3, BodyaimIfLethal = 1 << 4, PrioritizeFeet = 1 << 5);
			CVarEnum(Modifiers, VA_LIST("Modifiers", "Projectile modifiers"), 0b011111, DROPDOWN_MULTI, nullptr,
				VA_LIST("Charge weapon", "Cancel charge", "Use arm time", "Air splash", "Lob angles", "Target dormant"),
				ChargeWeapon = 1 << 0, CancelCharge = 1 << 1, UseArmTime = 1 << 2, AirSplash = 1 << 3, LobAngles = 1 << 4, TargetDormant = 1 << 5);
			CVar(MaxSimulationTime, "Max simulation time", 2.5f, SLIDER_MIN | SLIDER_PRECISION, 0.1f, 2.5f, 0.25f, "%gs");
			CVar(HitChance, "Hit chance", 0.f, SLIDER_CLAMP | SLIDER_PRECISION, 0.f, 100.f, 10.f, "%g%%");
			CVar(AutodetRadius, "Autodet radius", 90.f, SLIDER_CLAMP | SLIDER_PRECISION, 0.f, 100.f, 10.f, "%g%%");
			CVar(SplashRadius, "Splash radius", 100.f, SLIDER_CLAMP | SLIDER_PRECISION, 0.f, 100.f, 10.f, "%g%%");
			CVar(AutoRelease, "Auto release", 0.f, SLIDER_CLAMP | SLIDER_PRECISION, 0.f, 100.f, 5.f, "%g%%");
			CVar(GrapplingHookAim, "Grappling hook aim", false);
			CVar(AutoSCMetalAmount, "Auto short circuit min metal", 0, SLIDER_CLAMP, 0, 195, 65);

			CVar(GroundSamples, "Samples", 33, NOSAVE | DEBUGVAR, 3, 66);
			CVar(GroundStraightFuzzyValue, "Straight fuzzy value", 100.f, NOSAVE | DEBUGVAR | SLIDER_PRECISION, 0.f, 500.f, 25.f);
			CVar(GroundLowMinimumSamples, "Low min samples", 16, NOSAVE | DEBUGVAR, 3, 66);
			CVar(GroundHighMinimumSamples, "High min samples", 33, NOSAVE | DEBUGVAR, 3, 66);
			CVar(GroundLowMinimumDistance, "Low min distance", 0.f, NOSAVE | DEBUGVAR | SLIDER_MIN | SLIDER_PRECISION, 0.f, 2500.f, 100.f);
			CVar(GroundHighMinimumDistance, "High min distance", 1000.f, NOSAVE | DEBUGVAR | SLIDER_MIN | SLIDER_PRECISION, 0.f, 2500.f, 100.f);
			CVar(GroundMaxChanges, "Max changes", 0, NOSAVE | DEBUGVAR | SLIDER_MIN | SLIDER_PRECISION, 0, 5);
			CVar(GroundMaxChangeTime, "Max change time", 0, NOSAVE | DEBUGVAR | SLIDER_MIN | SLIDER_PRECISION, 0, 66);

			CVar(AirSamples, "Samples", 33, NOSAVE | DEBUGVAR, 3, 66);
			CVar(AirStraightFuzzyValue, "Straight fuzzy value", 0.f, NOSAVE | DEBUGVAR | SLIDER_PRECISION, 0.f, 500.f, 25.f);
			CVar(AirLowMinimumSamples, "Low min samples", 16, NOSAVE | DEBUGVAR, 3, 66);
			CVar(AirHighMinimumSamples, "High min samples", 16, NOSAVE | DEBUGVAR, 3, 66);
			CVar(AirLowMinimumDistance, "Low min distance", 100000.f, NOSAVE | DEBUGVAR | SLIDER_MIN | SLIDER_PRECISION, 0.f, 2500.f, 100.f);
			CVar(AirHighMinimumDistance, "High min distance", 100000.f, NOSAVE | DEBUGVAR | SLIDER_MIN | SLIDER_PRECISION, 0.f, 2500.f, 100.f);
			CVar(AirMaxChanges, "Max changes", 2, NOSAVE | DEBUGVAR | SLIDER_MIN | SLIDER_PRECISION, 0, 5);
			CVar(AirMaxChangeTime, "Max change time", 16, NOSAVE | DEBUGVAR | SLIDER_MIN | SLIDER_PRECISION, 0, 66);

			CVar(VelocityAverageCount, "Velocity average count", 5, NOSAVE | DEBUGVAR, 1, 10);
			CVar(VerticalShift, "Vertical shift", 5.f, NOSAVE | DEBUGVAR | SLIDER_MIN | SLIDER_PRECISION, 0.f, 10.f, 0.5f);
			CVar(DragOverride, "Drag override", 0.f, NOSAVE | DEBUGVAR | SLIDER_MIN | SLIDER_PRECISION, 0.f, 1.f, 0.01f);
			CVar(TimeOverride, "Time override", 0.f, NOSAVE | DEBUGVAR | SLIDER_MIN | SLIDER_PRECISION, 0.f, 2.f, 0.01f);
			CVar(LobAnglesUnderpredict, "Lob angles underpredict", true, NOSAVE | DEBUGVAR);
			CVar(HuntsmanLerp, "Huntsman lerp", 50.f, NOSAVE | DEBUGVAR | SLIDER_CLAMP | SLIDER_PRECISION, 0.f, 100.f, 1.f, "%g%%");
			CVar(HuntsmanLerpLow, "Huntsman lerp low", 100.f, NOSAVE | DEBUGVAR | SLIDER_CLAMP | SLIDER_PRECISION, 0.f, 100.f, 1.f, "%g%%");
			CVar(HuntsmanAdd, "Huntsman add", 0.f, NOSAVE | DEBUGVAR | SLIDER_CLAMP | SLIDER_PRECISION, 0.f, 20.f);
			CVar(HuntsmanAddLow, "Huntsman add low", 0.f, NOSAVE | DEBUGVAR | SLIDER_CLAMP | SLIDER_PRECISION, 0.f, 20.f);
			CVar(HuntsmanClamp, "Huntsman clamp", 5.f, NOSAVE | DEBUGVAR | SLIDER_CLAMP | SLIDER_PRECISION, 0.f, 10.f, 0.5f);
			CVar(HuntsmanPullPoint, "Huntsman pull point", false, NOSAVE | DEBUGVAR);
			CVar(HuntsmanPullNoZ, "Pull no Z", false, NOSAVE | DEBUGVAR);

			CVarEnum(SplashMode, "Splash mode", 1, NOSAVE | DEBUGVAR, nullptr,
				VA_LIST("Trace", "Face"),
				Trace, Face);
			CVar(SplashAirCount, "Splash air count", 4, NOSAVE | DEBUGVAR | SLIDER_MIN, 0, 10);
			CVar(SplashPointsDirect, "Splash points direct", 100, NOSAVE | DEBUGVAR | SLIDER_MIN | SLIDER_PRECISION, 0, 400, 5);
			CVar(SplashPointsArc, "Splash points arc", 100, NOSAVE | DEBUGVAR | SLIDER_MIN | SLIDER_PRECISION, 0, 400, 5);
			CVar(SplashRotateX, "Splash Rx", -1.f, NOSAVE | DEBUGVAR | SLIDER_MIN | SLIDER_PRECISION, -1.f, 360.f);
			CVar(SplashRotateY, "Splash Ry", -1.f, NOSAVE | DEBUGVAR | SLIDER_MIN | SLIDER_PRECISION, -1.f, 360.f);
			CVar(SplashDensityDirect, "Splash density direct", 55.f, NOSAVE | DEBUGVAR | SLIDER_MIN | SLIDER_PRECISION, 0.f, 100.f, 1.f);
			CVar(SplashDensityArc, "Splash density arc", 65.f, NOSAVE | DEBUGVAR | SLIDER_MIN | SLIDER_PRECISION, 0.f, 100.f, 1.f);
			CVar(SplashSamplesCutoff, "Splash samples cutoff", 0.0000001f, NOSAVE | DEBUGVAR | SLIDER_MIN | SLIDER_PRECISION, 0.f, 0.000001f, 0.00000001f);
			CVar(SplashRestrictDirect, "Splash restrict direct", 100, NOSAVE | DEBUGVAR | SLIDER_MIN | SLIDER_PRECISION, 1, 400, 5);
			CVar(SplashRestrictArc, "Splash restrict arc", 25, NOSAVE | DEBUGVAR | SLIDER_MIN | SLIDER_PRECISION, 1, 400, 5);
			CVar(SplashRestrictFirst, "Splash restrict first", 50, NOSAVE | DEBUGVAR | SLIDER_MIN | SLIDER_PRECISION, 1, 400, 5);
			CVar(DirectTraceInterval, "Direct trace interval", 1, NOSAVE | DEBUGVAR | SLIDER_MIN, 1, 20);
			CVar(SplashTraceInterval, "Splash trace interval", 5, NOSAVE | DEBUGVAR | SLIDER_MIN, 1, 20);
			CVar(LobTraceInterval, "Lob trace interval", 10, NOSAVE | DEBUGVAR | SLIDER_MIN, 1, 20);
			CVar(IntervalRetest, "Interval retest", true, NOSAVE | DEBUGVAR);

			CVar(DeltaCount, "Delta count", 5, NOSAVE | DEBUGVAR, 1, 5);
			CVarEnum(DeltaMode, "Delta mode", 0, NOSAVE | DEBUGVAR, nullptr,
				VA_LIST("Average", "Max"),
				Average, Max);
			CVarEnum(MovesimFrictionFlags, "Movesim friction flags", 0b01, NOSAVE | DEBUGVAR | DROPDOWN_MULTI, nullptr,
				VA_LIST("Run reduce", "Calculate increase"),
				RunReduce = 1 << 0, CalculateIncrease = 1 << 1);
			CVar(AutodetAccountPing, "Auto detonate account for ping", true, DEBUGVAR);
		NAMESPACE_END(Projectile)
		
		NAMESPACE_BEGIN(AutoEngie)
			CVarEnum(AutoRepair, "Auto repair", 0b111, DROPDOWN_MULTI, "Off",
				VA_LIST("Sentrygun","Dispenser", "Teleporter"),
				Sentry = 1 << 0, Dispenser = 1 << 1, Teleporter = 1 << 2);
			CVarEnum(AutoUpgrade, "Auto upgrade", 0b111, DROPDOWN_MULTI, "Off",
				VA_LIST("Sentrygun", "Dispenser", "Teleporter"),
				Sentry = 1 << 0, Dispenser = 1 << 1, Teleporter = 1 << 2);

			CVar(AutoUpgradeSentryLVL, "Sentry LVL", 3, SLIDER_CLAMP, 1, 3);
			CVar(AutoUpgradeDispenserLVL, "Dispenser LVL", 3, SLIDER_CLAMP, 1, 3);
			CVar(AutoUpgradeTeleporterLVL, "Teleporter LVL", 2, SLIDER_CLAMP, 1, 3);
		NAMESPACE_END(AutoEngie)

		NAMESPACE_BEGIN(Melee)
			CVar(AutoBackstab, "Auto backstab", true);
			CVar(IgnoreRazorback, "Ignore razorback", false);
			CVar(SwingPrediction, "Swing prediction", true);
			CVar(WhipTeam, "Whip team", false);

			CVar(SwingTicks, "Swing ticks", 13, NOSAVE | DEBUGVAR | SLIDER_MIN, 0, 14);
			CVar(SwingPredictLag, "Swing predict lag", true, NOSAVE | DEBUGVAR);
			CVarEnum(SwingValidateMode, "Swing validate mode", 0, NOSAVE | DEBUGVAR, nullptr,
				VA_LIST("Both", "Swing", "Simulated"),
				Both, Swing, Simulated);
			CVarEnum(BackstabFlags, "Backstab flags", 0b11, NOSAVE | DEBUGVAR | DROPDOWN_MULTI, nullptr,
				VA_LIST("Account ping", "Double test"),
				AccountPing = 1 << 0, DoubleTest = 1 << 1);
		NAMESPACE_END(Melee)

		NAMESPACE_BEGIN(Healing)
			CVarEnum(HealPriority, "Heal priority", 0, NONE, nullptr,
				VA_LIST("None", "Prioritize team", "Prioritize friends", "Prioritize party", "Friends only"),
				None, PrioritizeTeam, PrioritizeFriends, PrioritizeParty, FriendsOnly);
			CVarEnum(DangerIgnore, "Danger ignore", 0b1000, DROPDOWN_MULTI, nullptr,
				VA_LIST("Friends", "Party", "Unprioritized", "Ignored"),
				Friends = 1 << 0, Party = 1 << 1, Unprioritized = 1 << 2, Ignored = 1 << 3,
				Shared = Friends | Party | Unprioritized);
			CVar(AutoHeal, "Auto heal", false);
			CVar(AutoArrow, "Auto arrow", false);
			CVar(AutoSwitch, "Auto switch", false);
			CVar(AutoSandvich, "Auto sandvich", false);
			CVar(AutoVaccinator, "Auto vaccinator", false);
			CVar(ActivateOnVoice, "Activate on voice", false);
			CVar(ActivateFriendsOnly, "Activate friends only", false);
			CVar(ActivationHealthPercent, "Activate at health", 0.f, SLIDER_MIN | SLIDER_PRECISION, 0.f, 100.f, 5.f, "%g%%");
			CVar(AutoSwitchHealth, "Switch at", 75, SLIDER_CLAMP, 1, 100, 1, "%i%%");

			CVar(AutoVaccinatorBulletScale, "Auto vaccinator bullet scale", 100.f, NOSAVE | DEBUGVAR | SLIDER_MIN | SLIDER_PRECISION, 0.f, 200.f, 10.f, "%g%%");
			CVar(AutoVaccinatorBlastScale, "Auto vaccinator blast scale", 100.f, NOSAVE | DEBUGVAR | SLIDER_MIN | SLIDER_PRECISION, 0.f, 200.f, 10.f, "%g%%");
			CVar(AutoVaccinatorFireScale, "Auto vaccinator fire scale", 100.f, NOSAVE | DEBUGVAR | SLIDER_MIN | SLIDER_PRECISION, 0.f, 200.f, 10.f, "%g%%");
			CVar(AutoVaccinatorFlamethrowerDamageOnly, "Auto vaccinator flamethrower damage only", false, NOSAVE | DEBUGVAR);
		NAMESPACE_END(Healing)
	NAMESPACE_END(Aimbot)
	
	NAMESPACE_BEGIN(CritHack, Crit Hack)
		CVar(ForceCrits, "Force crits", false);
		CVar(AvoidRandomCrits, "Avoid random crits", false);
		CVar(AlwaysMeleeCrit, "Always melee crit", false);
		CVar(CritEffects, "Crit effects", false);
	NAMESPACE_END(CritHack)

	NAMESPACE_BEGIN(Backtrack)
		CVar(Latency, "Fake latency", 0, SLIDER_CLAMP, 0, 1000, 5);
		CVar(Interp, "Fake interp", 0, SLIDER_CLAMP | SLIDER_PRECISION, 0, 1000, 5);
		CVar(Window, VA_LIST("Window", "Backtrack window"), 185, SLIDER_CLAMP | SLIDER_PRECISION, 0, 200, 5);
		CVar(PreferOnShot, "Prefer on shot", false);

		CVar(Offset, "Offset", 0, NOSAVE | DEBUGVAR, -1, 1);
	NAMESPACE_END(Backtrack)

	NAMESPACE_BEGIN(Doubletap)
		CVar(Doubletap, "Doubletap", false);
		CVar(Warp, "Warp", false);
		CVar(RechargeTicks, "Recharge ticks", false);
		CVar(AntiWarp, "Anti-warp", true);
		CVar(TickLimit, "Tick limit", 22, SLIDER_CLAMP, 2, 22);
		CVar(WarpRate, "Warp rate", 22, SLIDER_CLAMP, 2, 22);
		CVar(RechargeLimit, "Recharge limit", 24, SLIDER_MIN, 1, 24);
		CVar(PassiveRecharge, "Passive recharge", 0, SLIDER_CLAMP, 0, 67);
	NAMESPACE_END(Doubletap)

	NAMESPACE_BEGIN(Fakelag)
		CVarEnum(Fakelag, "Fakelag", 0, NONE, nullptr,
			VA_LIST("Off", "Plain", "Random", "Adaptive"),
			Off, Plain, Random, Adaptive);
		CVarEnum(Options, VA_LIST("Options", "Fakelag options"), 0b000, DROPDOWN_MULTI, nullptr,
			VA_LIST("Only moving", "On unduck", "Not airborne"),
			OnlyMoving = 1 << 0, OnUnduck = 1 << 1, NotAirborne = 1 << 2);
		CVar(PlainTicks, "Plain ticks", 12, SLIDER_CLAMP, 1, 22);
		CVar(RandomTicks, "Random ticks", IntRange_t(14, 18), SLIDER_CLAMP, 1, 22, 1, "%i - %i");
		CVar(UnchokeOnAttack, "Unchoke on attack", true);
		CVar(RetainBlastJump, "Retain blastjump", false);

		CVar(RetainSoldierOnly, "Retain blastjump soldier only", true, NOSAVE | DEBUGVAR);
	NAMESPACE_END(Fakelag)

	NAMESPACE_BEGIN(AutoPeek, Auto Peek)
		CVar(Enabled, VA_LIST("Enabled", "Auto peek"), false);
	NAMESPACE_END(AutoPeek)

	NAMESPACE_BEGIN(AntiAim, Antiaim)
		CVar(Enabled, VA_LIST("Enabled", "Antiaim enabled"), false);
		CVarEnum(PitchReal, "Real pitch", 0, NONE, nullptr,
			VA_LIST("None", "Up", "Down", "Zero", "Jitter", "Reverse jitter", "Half up", "Half down", "Random", "Spin", "Ultra Random", "Heck", "Saw", "Moonwalk", "Timed flip (3s)", "Timed flip (3-10s)"),
			None, Up, Down, Zero, Jitter, ReverseJitter, HalfUp, HalfDown, Random, Spin, UltraRandom, Heck, Saw, Moonwalk, TimedFlip, TimedFlipRandom);
		CVarEnum(PitchFake, "Fake pitch", 0, NONE, nullptr,
			VA_LIST("None", "Up", "Down", "Jitter", "Reverse jitter", "Half up", "Half down", "Random", "Spin", "Ultra Random", "Inverse", "Mirror"),
			None, Up, Down, Jitter, ReverseJitter, HalfUp, HalfDown, Random, Spin, UltraRandom, Inverse, Mirror);
		Enum(Yaw, Forward, Left, Right, Backwards, Edge, Jitter, Spin, Random, Wiggle, Mercedes, Star, UltraRandom, Sideways, Omega, RandomUnclamped, Heck, Tornado, Pulse, Helix, Quantum);
		CVarValues(YawReal, "Real yaw", 0, NONE, nullptr,
			"Forward", "Left", "Right", "Backwards", "Edge", "Jitter", "Spin", "Random", "Wiggle", "Mercedes", "Star", "Ultra Random", "Sideways", "Omega", "Random Unclamped", "Heck", "Tornado", "Pulse", "Helix", "Quantum");
		CVarValues(YawFake, "Fake yaw", 0, NONE, nullptr,
			"Forward", "Left", "Right", "Backwards", "Edge", "Jitter", "Spin", "Random", "Wiggle", "Mercedes", "Star", "Ultra Random", "Sideways", "Omega", "Random Unclamped", "Heck", "Tornado", "Pulse", "Helix", "Quantum");
		Enum(YawMode, View, Target);
		CVarValues(RealYawBase, "Real base", 0, NONE, nullptr,
			"View", "Target");
		CVarValues(FakeYawBase, "Fake base", 0, NONE, nullptr,
			"View", "Target");
		CVar(RealYawOffset, "Real offset", 0.f, SLIDER_CLAMP | SLIDER_PRECISION, -180.f, 180.f, 5.f);
		CVar(FakeYawOffset, "Fake offset", 0.f, SLIDER_CLAMP | SLIDER_PRECISION, -180.f, 180.f, 5.f);
		CVar(RealYawValue, "Real value", 90.f, SLIDER_CLAMP | SLIDER_PRECISION, -180.f, 180.f, 5.f);
		CVar(FakeYawValue, "Fake value", -90.f, SLIDER_CLAMP | SLIDER_PRECISION, -180.f, 180.f, 5.f);
		CVar(SpinSpeed, "Spin speed", 15.f, SLIDER_PRECISION, -30.f, 30.f);
		CVar(MinWalk, "Minwalk", true);

		CVar(HidePitchOnShot, "Hide pitch on shot", false);
		CVar(AntiOverlap, "Anti-overlap", false);
		CVar(TauntSpin, "Taunt Spin", false);
		
		CVar(AntiAimLines, "Antiaim lines", false, NOSAVE);
	NAMESPACE_END(AntiAim)

	NAMESPACE_BEGIN(Resolver)
		CVar(Enabled, VA_LIST("Enabled", "Resolver enabled"), false);
		CVar(AutoResolve, "Auto resolve", false);
		CVar(AutoResolveCheatersOnly, "Auto resolve cheaters only", false);
		CVar(AutoResolveHeadshotOnly, "Auto resolve headshot only", false);
		CVar(AutoResolveYawAmount, "Auto resolve yaw", 90.f, SLIDER_CLAMP | SLIDER_PRECISION, -180.f, 180.f, 45.f);
		CVar(AutoResolvePitchAmount, "Auto resolve pitch", 90.f, SLIDER_CLAMP, -180.f, 180.f, 90.f);
		CVar(CycleYaw, "Cycle yaw", 0.f, SLIDER_CLAMP | SLIDER_PRECISION, -180.f, 180.f, 45.f);
		CVar(CyclePitch, "Cycle pitch", 0.f, SLIDER_CLAMP, -180.f, 180.f, 90.f);
		CVar(CycleView, "Cycle view", false);
		CVar(CycleMinwalk, "Cycle minwalk", false);
	NAMESPACE_END(Resolver)

	NAMESPACE_BEGIN(ESP)
		CVarValues(ActiveGroups, "Active groups", int(0b11111111111111111111111111111111), VISUAL | DROPDOWN_MULTI | DROPDOWN_NOSANITIZATION, nullptr);
	NAMESPACE_END(ESP)

	NAMESPACE_BEGIN(Visuals)
		NAMESPACE_BEGIN(UI)
			CVarEnum(StreamerMode, "Streamer mode", 0, VISUAL, nullptr,
				VA_LIST("Off", "Local", "Friends", "Party", "All"),
				Off, Local, Friends, Party, All);
			CVarEnum(ChatTags, "Chat tags", 0b000, VISUAL | DROPDOWN_MULTI, nullptr,
				VA_LIST("Local", "Friends", "Party", "Assigned"),
				Local = 1 << 0, Friends = 1 << 1, Party = 1 << 2, Assigned = 1 << 3);
			CVar(FieldOfView, "Field of view## FOV", 0.f, VISUAL | SLIDER_MIN | SLIDER_PRECISION, 0.f, 160.f, 5.f);
			CVar(ZoomFieldOfView, "Zoomed field of view## Zoomed FOV", 0.f, VISUAL | SLIDER_MIN | SLIDER_PRECISION, 0.f, 160.f, 5.f);
			CVar(AspectRatio, "Aspect ratio", 0.f, VISUAL | SLIDER_MIN | SLIDER_PRECISION, 0.f, 5.f, 0.05f);
			CVar(RevealScoreboard, "Reveal scoreboard", false, VISUAL);
			CVar(ScoreboardUtility, "Scoreboard utility", false);
			CVar(ScoreboardColors, "Scoreboard colors", false, VISUAL);
			CVar(CleanScreenshots, "Clean screenshots", true);
		NAMESPACE_END(UI)

		NAMESPACE_BEGIN(Thirdperson)
			CVar(Enabled, "Thirdperson", false, VISUAL);
			CVar(Crosshair, VA_LIST("Crosshair", "Thirdperson crosshair"), false, VISUAL);
			CVar(Distance, "Thirdperson distance", 150.f, VISUAL | SLIDER_PRECISION, 0.f, 400.f, 10.f);
			CVar(Right, "Thirdperson right", 0.f, VISUAL | SLIDER_PRECISION, -100.f, 100.f, 5.f);
			CVar(Up, "Thirdperson up", 0.f, VISUAL | SLIDER_PRECISION, -100.f, 100.f, 5.f);

			CVar(Scale, "Thirdperson scales", true, NOSAVE | DEBUGVAR);
			CVar(Collision, "Thirdperson collision", true, NOSAVE | DEBUGVAR);
		NAMESPACE_END(ThirdPerson)
		
		NAMESPACE_BEGIN(Removals)
			CVar(Interpolation, VA_LIST("Interpolation", "Remove interpolation"), false);
			CVar(Lerp, VA_LIST("Lerp", "Remove lerp"), true);
			CVar(Disguises, VA_LIST("Disguises", "Remove disguises"), false, VISUAL);
			CVar(Taunts, VA_LIST("Taunts", "Remove taunts"), false, VISUAL);
			CVar(Scope, VA_LIST("Scope", "Remove scope"), false, VISUAL);
			CVar(PostProcessing, VA_LIST("Post processing", "Remove post processing"), false, VISUAL);
			CVar(ScreenOverlays, VA_LIST("Screen overlays", "Remove screen overlays"), false, VISUAL);
			CVar(ScreenEffects, VA_LIST("Screen effects", "Remove screen effects"), false, VISUAL);
			CVar(ViewPunch, VA_LIST("View punch", "Remove view punch"), false, VISUAL);
			CVar(AngleForcing, VA_LIST("Angle forcing", "Remove angle forcing"), false, VISUAL);
			CVar(Ragdolls, VA_LIST("Ragdolls", "Remove ragdoll"), false, VISUAL);
			CVar(Gibs, VA_LIST("Gibs", "Remove gibs"), false, VISUAL);
			CVar(MOTD, VA_LIST("MOTD", "Remove MOTD"), false, VISUAL);
		NAMESPACE_END(Removals)

		NAMESPACE_BEGIN(Effects)
			CVarValues(BulletTracer, "Bullet tracer", std::string("Default"), VISUAL | DROPDOWN_CUSTOM, nullptr,
				"Default", "None", "Big nasty", "Distortion trail", "Machina", "Sniper rail", "Short circuit", "C.A.P.P.E.R", "Merasmus ZAP", "Merasmus ZAP 2", "Black ink", "Line", "Line ignore Z", "Beam");
			CVarValues(CritTracer, "Crit tracer", std::string("Default"), VISUAL | DROPDOWN_CUSTOM, nullptr,
				"Default", "None", "Big nasty", "Distortion trail", "Machina", "Sniper rail", "Short circuit", "C.A.P.P.E.R", "Merasmus ZAP", "Merasmus ZAP 2", "Black ink", "Line", "Line ignore Z", "Beam");
			CVarValues(MedigunBeam, "Medigun beam", std::string("Default"), VISUAL | DROPDOWN_CUSTOM, nullptr,
				"Default", "None", "Uber", "Dispenser", "Passtime", "Bombonomicon", "White", "Orange");
			CVarValues(MedigunCharge, "Medigun charge", std::string("Default"), VISUAL | DROPDOWN_CUSTOM, nullptr,
				"Default", "None", "Electrocuted", "Halloween", "Fireball", "Teleport", "Burning", "Scorching", "Purple energy", "Green energy", "Nebula", "Purple stars", "Green stars", "Sunbeams", "Spellbound", "Purple sparks", "Yellow sparks", "Green zap", "Yellow zap", "Plasma", "Frostbite", "Time warp", "Purple souls", "Green souls", "Bubbles", "Hearts");
			CVarValues(ProjectileTrail, "Projectile trail", std::string("Default"), VISUAL | DROPDOWN_CUSTOM, nullptr,
				"Default", "None", "Rocket", "Critical", "Energy", "Charged", "Ray", "Fireball", "Teleport", "Fire", "Flame", "Sparks", "Flare", "Trail", "Health", "Smoke", "Bubbles", "Halloween", "Monoculus", "Sparkles", "Rainbow");
			CVarEnum(SpellFootsteps, "Spell footsteps", 0, VISUAL, nullptr,
				VA_LIST("Off", "Color", "Team", "Halloween"),
				Off, Color, Team, Halloween);
			CVarEnum(RagdollEffects, "Ragdoll effects", 0b000000, VISUAL | DROPDOWN_MULTI, nullptr,
				VA_LIST("Burning", "Electrocuted", "Ash", "Dissolve", "##Divider", "Gold", "Ice"),
				Burning = 1 << 0, Electrocuted = 1 << 1, Ash = 1 << 2, Dissolve = 1 << 3, Gold = 1 << 4, Ice = 1 << 5);
			CVar(DrawIconsThroughWalls, "Draw icons through walls", false, VISUAL);
			CVar(DrawDamageNumbersThroughWalls, "Draw damage numbers through walls", false, VISUAL);
		NAMESPACE_END(Effects)

		NAMESPACE_BEGIN(Viewmodel)
			CVar(CrosshairAim, "Crosshair aim", false, VISUAL);
			CVar(ViewmodelAim, "Viewmodel aim", false, VISUAL);
			CVar(OffsetX, VA_LIST("Offset X", "Viewmodel offset X"), 0.f, VISUAL | SLIDER_PRECISION, -45.f, 45.f, 5.f);
			CVar(OffsetY, VA_LIST("Offset Y", "Viewmodel offset Y"), 0.f, VISUAL | SLIDER_PRECISION, -45.f, 45.f, 5.f);
			CVar(OffsetZ, VA_LIST("Offset Z", "Viewmodel offset Z"), 0.f, VISUAL | SLIDER_PRECISION, -45.f, 45.f, 5.f);
			CVar(Pitch, VA_LIST("Pitch", "Viewmodel pitch"), 0.f, VISUAL | SLIDER_CLAMP | SLIDER_PRECISION, -180.f, 180.f, 5.f);
			CVar(Yaw, VA_LIST("Yaw", "Viewmodel yaw"), 0.f, VISUAL | SLIDER_CLAMP | SLIDER_PRECISION, -180.f, 180.f, 5.f);
			CVar(Roll, VA_LIST("Roll", "Viewmodel roll"), 0.f, VISUAL | SLIDER_CLAMP | SLIDER_PRECISION, -180.f, 180.f, 5.f);
			CVar(SwayScale, VA_LIST("Sway scale", "Viewmodel sway scale"), 0.f, VISUAL | SLIDER_MIN | SLIDER_PRECISION, 0.f, 5.f, 0.5f);
			CVar(SwayInterp, VA_LIST("Sway interp", "Viewmodel sway interp"), 0.f, VISUAL | SLIDER_MIN | SLIDER_PRECISION, 0.f, 1.f, 0.1f);
		NAMESPACE_END(Viewmodel)

		NAMESPACE_BEGIN(World)
			CVarEnum(Modulations, "Modulations", 0b00000, VISUAL | DROPDOWN_MULTI, nullptr,
				VA_LIST("World", "Sky", "Prop", "Particle", "Fog"),
				World = 1 << 0, Sky = 1 << 1, Prop = 1 << 2, Particle = 1 << 3, Fog = 1 << 4);
			CVarValues(SkyboxChanger, "Skybox changer", std::string("Off"), VISUAL | DROPDOWN_CUSTOM, nullptr,
				VA_LIST("Off"));
			CVarValues(WorldTexture, "World texture", std::string("Default"), VISUAL | DROPDOWN_CUSTOM, nullptr,
				"Default", "Dev", "Camo", "Black", "White", "Gray", "Flat");
			CVar(NearPropFade, "Near prop fade", false, VISUAL);
			CVar(NoPropFade, "No prop fade", false, VISUAL);
			CVarEnum(ShowTriggers, "Show triggers", 0b00000, VISUAL | DROPDOWN_MULTI, nullptr,
				VA_LIST("Hurt", "Ignite", "Push", "Regenerate", "Respawn room", "Capture area", "Catapult", "Apply impulse", "##Divider", "Show angles", "Show surface centers", "Ignore Z"),
				Hurt = 1 << 0, Ignite = 1 << 1, Push = 1 << 2, Regenerate = 1 << 3, RespawnRoom = 1 << 4, CaptureArea = 1 << 5, Catapult = 1 << 6, ApplyImpulse = 1 << 7, ShowAngles = 1 << 8, ShowSurfaceCenters = 1 << 9, IgnoreZ = 1 << 10);
		NAMESPACE_END(World)

		NAMESPACE_BEGIN(Beams) // as of now, these will stay out of the menu
			CVar(Model, "Model", std::string("sprites/physbeam.vmt"), VISUAL);
			CVar(Life, "Life", 2.f, VISUAL | SLIDER_MIN | SLIDER_PRECISION, 0.f, 10.f);
			CVar(Width, "Width", 2.f, VISUAL | SLIDER_MIN | SLIDER_PRECISION, 0.f, 10.f);
			CVar(EndWidth, "End width", 2.f, VISUAL | SLIDER_MIN | SLIDER_PRECISION, 0.f, 10.f);
			CVar(FadeLength, "Fade length", 10.f, VISUAL | SLIDER_MIN | SLIDER_PRECISION, 0.f, 30.f);
			CVar(Amplitude, "Amplitude", 2.f, VISUAL | SLIDER_MIN | SLIDER_PRECISION, 0.f, 10.f);
			CVar(Brightness, "Brightness", 255.f, VISUAL | SLIDER_MIN | SLIDER_PRECISION, 0.f, 255.f);
			CVar(Speed, "Speed", 0.2f, VISUAL | SLIDER_MIN | SLIDER_PRECISION, 0.f, 5.f);
			CVar(Segments, "Segments", 2, VISUAL | SLIDER_MIN, 1, 10);
			CVar(Color, "Color", Color_t(255, 255, 255, 255), VISUAL);
			CVarEnum(Flags, "Flags", 0b10000000100000000, VISUAL | DROPDOWN_MULTI, nullptr,
				VA_LIST("Start entity", "End entity", "Fade in", "Fade out", "Sine noise", "Solid", "Shade in", "Shade out", "Only noise once", "No tile", "Use hitboxes", "Start visible", "End visible", "Is active", "Forever", "Halobeam", "Reverse"),
				StartEntity = 1 << 0, EndEntity = 1 << 1, FadeIn = 1 << 2, FadeOut = 1 << 3, SineNoise = 1 << 4, Solid = 1 << 5, ShadeIn = 1 << 6, ShadeOut = 1 << 7, OnlyNoiseOnce = 1 << 8, NoTile = 1 << 9, UseHitboxes = 1 << 10, StartVisible = 1 << 11, EndVisible = 1 << 12, IsActive = 1 << 13, Forever = 1 << 14, Halobeam = 1 << 15, Reverse = 1 << 16);
		NAMESPACE_END(Beams)

		NAMESPACE_BEGIN(Line)
			CVar(TracersEnabled, "Tracers enabled", false, VISUAL);
			CVar(DrawDuration, VA_LIST("Draw duration", "Line draw duration"), 5.f, VISUAL | SLIDER_MIN | SLIDER_PRECISION, 0.f, 10.f);
		NAMESPACE_END(Line)

		NAMESPACE_BEGIN(Hitbox)
			CVarEnum(BonesEnabled, VA_LIST("Bones enabled", "Hitbox bones enabled"), 0b00, VISUAL | DROPDOWN_MULTI, "Off",
				VA_LIST("On shot", "On hit"),
				OnShot = 1 << 0, OnHit = 1 << 1);
			CVarEnum(BoundsEnabled, VA_LIST("Bounds enabled", "Hitbox bounds enabled"), 0b000, VISUAL | DROPDOWN_MULTI, "Off",
				VA_LIST("On shot", "On hit", "Aim point"),
				OnShot = 1 << 0, OnHit = 1 << 1, AimPoint = 1 << 2);
			CVar(DrawDuration, VA_LIST("Draw duration", "Hitbox draw duration"), 5.f, VISUAL | SLIDER_MIN | SLIDER_PRECISION, 0.f, 10.f);
		NAMESPACE_END(Hitbox)

		NAMESPACE_BEGIN(Prediction)
			CVarValues(PlayerPath, "Player path", 0, VISUAL, nullptr,
				"Off", "Line", "Separators", "Spaced", "Arrows", "Boxes");
			CVarValues(ProjectilePath, "Projectile path", 0, VISUAL, nullptr,
				"Off", "Line", "Separators", "Spaced", "Arrows", "Boxes");
			CVar(SwingLines, "Swing lines", false, VISUAL);
			CVar(PlayerDrawDuration, VA_LIST("Draw duration", "Player path draw duration"), 5.f, VISUAL | SLIDER_MIN | SLIDER_PRECISION, 0.f, 10.f);
			CVar(ProjectileDrawDuration, VA_LIST("Draw duration", "Projectile path draw duration"), 5.f, VISUAL | SLIDER_MIN | SLIDER_PRECISION, 0.f, 10.f);

			CVarValues(RealPath, "Real path", 0, NOSAVE | DEBUGVAR, nullptr,
				"Off", "Line", "Separators", "Spaced", "Arrows", "Boxes");
		NAMESPACE_END(Prediction)

		NAMESPACE_BEGIN(Simulation)
			CVarValues(TrajectoryPath, "Trajectory path", 0, VISUAL, nullptr,
				"Off", "Line", "Separators", "Spaced", "Arrows", "Boxes");
			CVarValues(ShotPath, "Shot path", 0, VISUAL, nullptr,
				"Off", "Line", "Separators", "Spaced", "Arrows", "Boxes");
			CVarEnum(SplashRadius, "Splash radius", 0b0, VISUAL | DROPDOWN_MULTI, "Off",
				VA_LIST("Rockets", "Stickies", "Pipes", "Flares", "##Divider", "Trace", "Sphere"),
				Rockets = 1 << 0, Stickies = 1 << 1, Pipes = 1 << 2, Flares = 1 << 3, Trace = 1 << 4, Sphere = 1 << 5,
				Enabled = Rockets | Stickies | Pipes | Flares);
			CVar(ProjectileCamera, "Projectile camera", false, VISUAL);
			CVar(ProjectileWindow, "Projectile window", WindowBox_t(), VISUAL | NOBIND);
			CVar(Box, VA_LIST("Box", "Path box"), true, VISUAL);
		NAMESPACE_END(Simulation)

		NAMESPACE_BEGIN(Path)
			Enum(Style, Off, Line, Separators, Spaced, Arrows, Boxes);
			CVar(SeparatorSpacing, "Separator spacing", 4, NOSAVE | DEBUGVAR, 1, 16);
			CVar(SeparatorLength, "Separator length", 12.f, NOSAVE | DEBUGVAR, 2.f, 16.f);
		NAMESPACE_END(Path)
		
		NAMESPACE_BEGIN(Other, Other Visuals)
			CVar(KillstreakWeapons, "Killstreak weapons", false, VISUAL);
		NAMESPACE_END(Other);

		NAMESPACE_BEGIN(Trajectory)
			CVar(Override, "Simulation override", false, NOSAVE | DEBUGVAR);
			CVar(Type, "Type", std::string("custom"), NOSAVE | DEBUGVAR);
			CVar(OffsetX, "Offset X", 23.5f, NOSAVE | DEBUGVAR | SLIDER_PRECISION, -25.f, 25.f, 0.5f);
			CVar(OffsetY, "Offset Y", 12.f, NOSAVE | DEBUGVAR | SLIDER_PRECISION, -25.f, 25.f, 0.5f);
			CVar(OffsetZ, "Offset Z", -3.f, NOSAVE | DEBUGVAR | SLIDER_PRECISION, -25.f, 25.f, 0.5f);
			CVar(ForwardRedirect, "Forward redirect", 2000.f, NOSAVE | DEBUGVAR | SLIDER_PRECISION, 0.f, 2000.f, 100.f);
			CVar(ForwardCutoff, "Forward cutoff", 0.1f, NOSAVE | DEBUGVAR | SLIDER_PRECISION, 0.f, 1.f, 0.1f);
			CVar(Hull, "Hull", 0.f, NOSAVE | DEBUGVAR | SLIDER_MIN | SLIDER_PRECISION, 0.f, 10.f, 0.5f);
			CVar(Speed, "Speed", 1100.f, NOSAVE | DEBUGVAR | SLIDER_MIN | SLIDER_PRECISION, 0.f, 5000.f, 50.f);
			CVar(Gravity, "Gravity", 0.f, NOSAVE | DEBUGVAR | SLIDER_PRECISION, 0.f, 800.f, 100.f);
			CVar(LifeTime, "Life time", 10.f, NOSAVE | DEBUGVAR | SLIDER_MIN | SLIDER_PRECISION, 0.f, 10.f, 0.1f);
			CVar(UpVelocity, "Up velocity", 0.f, NOSAVE | DEBUGVAR | SLIDER_PRECISION, 0.f, 1000.f, 50.f);
			CVar(AngularVelocityX, "Angular velocity X", 0.f, NOSAVE | DEBUGVAR | SLIDER_PRECISION, -1000.f, 1000.f, 50.f);
			CVar(AngularVelocityY, "Angular velocity Y", 0.f, NOSAVE | DEBUGVAR | SLIDER_PRECISION, -1000.f, 1000.f, 50.f);
			CVar(AngularVelocityZ, "Angular velocity Z", 0.f, NOSAVE | DEBUGVAR | SLIDER_PRECISION, -1000.f, 1000.f, 50.f);
			CVar(Drag, "Drag", 0.f, NOSAVE | DEBUGVAR | SLIDER_PRECISION, 0.f, 2.f, 0.1f);
			CVar(DragX, "Drag X", 0.f, NOSAVE | DEBUGVAR | SLIDER_PRECISION, 0.f, 0.1f, 0.01f, "%.15g");
			CVar(DragY, "Drag Y", 0.f, NOSAVE | DEBUGVAR | SLIDER_PRECISION, 0.f, 0.1f, 0.01f, "%.15g");
			CVar(DragZ, "Drag Z", 0.f, NOSAVE | DEBUGVAR | SLIDER_PRECISION, 0.f, 0.1f, 0.01f, "%.15g");
			CVar(AngularDragX, "Angular drag X", 0.f, NOSAVE | DEBUGVAR | SLIDER_PRECISION, 0.f, 0.1f, 0.01f, "%.15g");
			CVar(AngularDragY, "Angular drag Y", 0.f, NOSAVE | DEBUGVAR | SLIDER_PRECISION, 0.f, 0.1f, 0.01f, "%.15g");
			CVar(AngularDragZ, "Angular drag Z", 0.f, NOSAVE | DEBUGVAR | SLIDER_PRECISION, 0.f, 0.1f, 0.01f, "%.15g");
			CVar(MaxVelocity, "Max velocity", 0.f, NOSAVE | DEBUGVAR | SLIDER_MIN | SLIDER_PRECISION, 0.f, 4000.f, 50.f);
			CVar(MaxAngularVelocity, "Max angular velocity", 0.f, NOSAVE | DEBUGVAR | SLIDER_MIN | SLIDER_PRECISION, 0.f, 7200.f, 50.f);
		NAMESPACE_END(ProjectileTrajectory)
	NAMESPACE_END(Visuals)

	NAMESPACE_BEGIN(Misc)

/*
I dont think this is a good idea to disable simulations completely:
	1. Proj aim still runs and fails because there is no projectile/movement prediction (if you dont want it to run just dont give your bots projectile weapons)
	2. Auto-scope also partially breaks if you disable simulations
	3. In general a lot of features still partially run despite there being no point
	4. People will start to complain because they might not even know this exists
*/
/*
		NAMESPACE_BEGIN(Performance)
#ifdef TEXTMODE
			CVar(DisableSimulations, "Disable simulations", true);
#else
			CVar(DisableSimulations, "Disable simulations", false);
#endif
		NAMESPACE_END(Performance)
*/

		NAMESPACE_BEGIN(Movement)
			CVarEnum(AutoStrafe, "Auto strafe", 0, NONE, nullptr,
				VA_LIST("Off", "Legit", "Directional"),
				Off, Legit, Directional);
			CVar(AutoStrafeTurnScale, VA_LIST("Turn scale", "Auto strafe turn scale"), 0.5f, SLIDER_CLAMP | SLIDER_PRECISION, 0.f, 1.f, 0.1f);
			CVar(AutoStrafeMaxDelta, VA_LIST("Max delta", "Auto strafe max delta"), 180.f, SLIDER_CLAMP | SLIDER_PRECISION, 0.f, 180.f, 5.f);
			CVar(Bunnyhop, "Bunnyhop", false);
			CVar(EdgeJump, "Edge jump", false);
			CVar(BreakJump, "Break jump", false);
			CVar(AutoJumpbug, "Auto jumpbug", false);
			CVarEnum(AutoEdgebug, "Auto edgebug", 0, NONE, nullptr,
				VA_LIST("Off", "Legit", "Strafe", "Strafe silent"),
				Off, Legit, Strafe, StrafeSilent);
			CVar(AutoRocketJump, "Auto rocket jump", false);
			CVar(AutoCTap, "Auto ctap", false);
			CVar(AutoFaNJump, "Auto FaN jump", false);
			CVar(AutoRevJump, "Auto rev jump", false);
			CVar(FastStop, "Fast stop", false);
			CVar(FastAccelerate, "Fast accelerate", false);
			CVar(DuckSpeed, "Duck speed", false);
			CVar(ShieldTurnRate, "Shield turn rate", false);
			CVar(ChargeBot, "Charge bot", false);
			CVar(ChargeBotDelay, "Charge bot delay", 0.5f, SLIDER_MIN, 0.f, 3.f, 0.1f, "%gs");
			CVar(ChargeBotTurnRate, "Charge bot turn rate", 10.f, SLIDER_MIN, 1.f, 45.f, 1.f, "%g°");
			CVar(AutoAirStickyPogo, "Auto air sticky pogo", false);
			CVar(AutoAirStickyPogoDelay, "Auto air sticky pogo delay", 0.5f, SLIDER_MIN, 0.f, 3.f, 0.1f, "%gs");
			CVar(NoPush, "No push", false);
			CVar(MovementLock, "Movement lock", false);

			NAMESPACE_BEGIN(NavEngine)
				CVar(Enabled, VA_LIST("Enabled", "Nav engine enabled"), false);
				CVar(PathInSetup, "Path in setup time", false);
				CVarEnum(Draw, "Draw", 0b011, VISUAL | DROPDOWN_MULTI, nullptr,
					VA_LIST("Path", "Areas", "Blacklisted zones"),
					Path = 1 << 0, Area = 1 << 1, Blacklist = 1 << 2);
				CVarEnum(LookAtPath, "Look at path", 0, NONE, nullptr,
					VA_LIST("Off", "Plain", "Silent", "Legit", "Legit silent"),
					Off, Plain, Silent, Legit, LegitSilent);

				CVar(StickyIgnoreTime, "Sticky ignore time", 15, SLIDER_MIN, 15, 100, 5, "%is");
				CVar(StuckDetectTime, "Stuck detect time", 1, SLIDER_MIN, 1, 26, 1, "%is");
				CVar(StuckBlacklistTime, "Stuck blacklist time", 90, SLIDER_MIN, 20, 600, 20, "%is");
				CVar(StuckExpireTime, "Stuck expire time", 20, SLIDER_MIN, 5, 100, 5, "%is");
				CVar(StuckTime, "Stuck time", 0.25f, SLIDER_MIN, 0.25f, 0.9f, 0.05f, "%gs");

				CVar(VischeckEnabled, "Vischeck enabled", true);
				CVar(VischeckTime, "Vischeck time", 0.25f, SLIDER_MIN, 0.005f, 3.f, 0.005f, "%gs");
				CVar(VischeckCacheTime, "Vischeck cache time", 45, SLIDER_MIN, 10, 500, 10, "%is");
			NAMESPACE_END(NavEngine)

			NAMESPACE_BEGIN(BotUtils)
				CVar(LookAtPathSpeed, "Look at path speed", 25, SLIDER_CLAMP, 0, 120);
				CVarEnum(WeaponSlot, "Force weapon", 0, NONE, nullptr,
					VA_LIST("Off", "Best", "Primary", "Secondary", "Melee", "PDA"),
					Off, Best, Primary, Secondary, Melee, PDA);
			
				CVarEnum(AutoScope, "Auto scope", 0, NONE, nullptr,
					VA_LIST("Off", "Simple", "MoveSim"),
					Off, Simple, MoveSim);
				CVar(AutoScopeCancelTime, "Auto scope cancel time", 3, SLIDER_MIN, 1, 5, 1, "%is");
				CVar(AutoScopeUseCachedResults, "Auto scope use cached results", true, NOSAVE | DEBUGVAR);
				CVar(LookAtPathDebug, "Look at path debug", false, NOSAVE | DEBUGVAR);
			NAMESPACE_END(BotUtils)

			NAMESPACE_BEGIN(NavBot)
				CVar(Enabled, VA_LIST("Enabled", "Navbot enabled"), false);
				CVarEnum(Blacklist, "Blacklist", 0b0111111, DROPDOWN_MULTI, "None",
					VA_LIST("Normal threats", "Dormant threats", "##Divider", "Players", "Stickies", "Projectiles", "Sentries"),
					NormalThreats = 1 << 0, DormantThreats = 1 << 1, Players = 1 << 2, Stickies = 1 << 3, Projectiles = 1 << 4, Sentries = 1 << 5);

				CVar(BlacklistDelay, "Blacklist normal scan delay", 0.5f, SLIDER_MIN, 0.1f, 1.f, 0.1f, "%gs");
				CVar(BlacklistDormantDelay, "Blacklist dormant scan delay", 1.f, SLIDER_MIN, 0.5f, 5.f, 0.5f, "%gs");
				CVar(BlacklistSlightDangerLimit, "Blacklist slight danger limit", 2, SLIDER_MIN, 1, 10);

				CVar(SmartJump, "Smart jump", true);
				CVar(EscapeSpawn, "Escape spawn", true);

				CVarEnum(RechargeDT, "Recharge DT", 0, NONE, nullptr,
					VA_LIST("Off", "On", "If not fakelagging"),
					Off, Always, WaitForFL);
				CVar(RechargeDTDelay, "Recharge DT delay", 5, SLIDER_MIN, 0, 10, 1, "%is");

				CVarEnum(Preferences, "Preferences", 0b100001111110111, DROPDOWN_MULTI, nullptr,
					VA_LIST("Get health", "Get ammo", "Reload weapons", "Stalk enemies", "Defend objectives", "Capture objectives", "Help capture objectives", "Escape danger", "Safe capping", "Target sentries", "Auto engie", "##Divider", "Target sentries low range", "Help capture objective friend only", "Dont escape danger with intel", "Group with others"),
					SearchHealth = 1 << 0, SearchAmmo = 1 << 1, ReloadWeapons = 1 << 2, StalkEnemies = 1 << 3, DefendObjectives = 1 << 4, CaptureObjectives = 1 << 5, HelpCaptureObjectives = 1 << 6, EscapeDanger = 1 << 7, SafeCapping = 1 << 8, TargetSentries = 1 << 9, AutoEngie = 1 << 10, TargetSentriesLowRange = 1 << 11, HelpFriendlyCaptureObjectives = 1 << 12, DontEscapeDangerIntel = 1 << 13, GroupWithOthers = 1 << 14);
				CVar(MeleeTargetRange, "Melee target range", 600, NONE, 150, 4000, 50);
				CVar(DangerOverlay, "Danger overlay", false);
				CVar(DangerOverlayMaxDist, "Danger overlay max distance", 2000.f, SLIDER_MIN, 500.f, 6000.f, 250.f, "%0.0f");

				CVar(StickyDangerRange, "Sticky danger range", 600, NOSAVE | DEBUGVAR, 50, 1500, 50);
				CVar(ProjectileDangerRange, "Projectile danger range", 600, NOSAVE | DEBUGVAR, 50, 1500, 50);
			NAMESPACE_END(NavBot)

			NAMESPACE_BEGIN(FollowBot)
				CVar(Enabled, VA_LIST("Enabled", "Followbot enabled"), false);

				CVarEnum(UseNav, "Use nav mesh on", 0, NONE, nullptr,
					VA_LIST("Off", "Normal", "Normal + Dormant"),
					Off, Normal, Dormant);

				CVarEnum(Targets, "Targets", 0b01, DROPDOWN_MULTI, nullptr,
					VA_LIST("Teammates", "Enemies"),
					Teammates = 1 << 0, Enemies = 1 << 1);

				CVarEnum(Prefer, "Prefer", 0, NONE, nullptr,
					VA_LIST("Off", "Prefer friends", "Party members"),
					Off, PreferFriends, PartyMembers);

				CVarEnum(LookAtPath, "Look at path", 0, NONE, nullptr,
					VA_LIST("Off", "Plain", "Silent"),
					Off, Plain, Silent);
				CVarEnum(LookAtPathMode, "Look at path mode", 0, NONE, nullptr,
					VA_LIST("Path", "Copy target", "Copy target immediate", "At target"),
					Path, Copy, CopyImmediate, AtTarget);
				CVar(LookAtPathNoSnap, "Avoid view snap", false);

				CVar(DrawPath, "Draw path nodes", false);
				CVar(MinPriority, "Min follow priority", 0, SLIDER_CLAMP, 0, 10);
				CVar(MaxNodes, "Max path nodes", 300, SLIDER_CLAMP, 50, 500);
				CVar(ActivationDistance, "Activation distance", 300, SLIDER_CLAMP, 10, 1200);
				CVar(FollowDistance, "Follow distance", 60, SLIDER_CLAMP, 10, 150);
				CVar(AbandonDistance, "Abandon distance", 1500, SLIDER_CLAMP, 250, 1500);
				CVar(NavAbandonDistance, "Nav abandon distance", 1500, SLIDER_CLAMP, 2000, 8000);
			NAMESPACE_END(FollowBot)

			CVar(AutoRocketJumpChokeGrounded, "Choke grounded", 1, NOSAVE | DEBUGVAR, 0, 3);
			CVar(AutoRocketJumpChokeAir, "Choke air", 1, NOSAVE | DEBUGVAR, 0, 3);
			CVar(AutoRocketJumpSkipGround, "Skip grounded", 0, NOSAVE | DEBUGVAR, 0, 3);
			CVar(AutoRocketJumpSkipAir, "Skip air", 1, NOSAVE | DEBUGVAR, 0, 3);
			CVar(AutoRocketJumpTimingOffset, "Timing offset", 0, NOSAVE | DEBUGVAR, 0, 3);
			CVar(AutoRocketJumpApplyAbove, "Apply offset above", 0, NOSAVE | DEBUGVAR, 0, 10);

			CVar(AutoFaNJumpOnSolidTicks, "On solid ticks", 8, NOSAVE | DEBUGVAR, 2, 30);
			CVar(AutoFaNJumpCheckCeiling, "Check ceiling", true, NOSAVE | DEBUGVAR);

			CVar(AutoEdgebugStrafeSamples, "Strafe samples", 6, NOSAVE | DEBUGVAR | SLIDER_CLAMP, 3, 15);
			CVar(AutoEdgebugStrafeMaxDelta, "Max delta", 180, NOSAVE | DEBUGVAR | SLIDER_CLAMP, 1, 360);
			CVar(AutoEdgebugTryNegativeDir, "Try negative dir", true, NOSAVE | DEBUGVAR);
			CVar(AutoEdgebugTryRandomMove, "Try random move", true, NOSAVE | DEBUGVAR);
		NAMESPACE_END(Movement)

		NAMESPACE_BEGIN(Automation)
			CVarEnum(AntiBackstab, "Anti-backstab", 0, NONE, nullptr,
				VA_LIST("Off", "Yaw", "Pitch", "Fake"),
				Off, Yaw, Pitch, Fake);
			CVar(AcceptItemDrops, "Auto accept item drops", false);
			CVar(AntiAFK, "Anti-AFK", false);
			CVarEnum(AntiAutobalance, "Anti-autobalance", 0, NONE, nullptr,
				VA_LIST("Off", "Retry", "Retry on death"),
				Off, Retry, RetryOnDeath);
			CVar(AutoRetry, "Auto retry", false);
			CVar(AutoRetryHealth, "Retry below health", 35, SLIDER_CLAMP, 1, 100, 1, "%i%%");
			CVar(TauntControl, "Taunt control", false);
			CVar(KartControl, "Kart control", false);
			CVar(AutoDisguise, "Auto disguise", false);
			CVar(AutoTaunt, "Auto taunt on kill", false);
			CVar(AutoTauntChance, "Auto taunt chance", 100, SLIDER_CLAMP, 0, 100, 1, "%i%%");
			CVar(AutoTauntSlot, "Auto taunt slot", 0, SLIDER_CLAMP, 0, 8);
			CVar(AchievementSpam, "Achievement spam", false);
			CVar(AchievementSpamID, "Achievement spam id", 2332);
			struct AchievementSpamEntry_t
			{
				int m_iID;
				const char* m_sName;
			};
			inline const std::vector<AchievementSpamEntry_t>& GetAchievementSpamEntries()
			{
				static const std::vector<AchievementSpamEntry_t> vEntries =
				{
					{127, "Sentry Gunner"}, {128, "Nemesis"}, {129, "Hard To Kill"}, {130, "Master of Disguise"},
					{131, "With Friends Like These"}, {132, "Dynasty"}, {133, "Hardcore"}, {134, "Powerhouse Offense"},
					{135, "Lightning Offense"}, {136, "Relentless Offense"}, {137, "Impenetrable Defense"}, {138, "Impossible Defense"},
					{139, "Head of the Class"}, {140, "World Traveler"}, {141, "Team Doctor"}, {142, "Flamethrower"},
					{145, "Grey Matter"}, {150, "Riftwalker"}, {152, "Escape the Heat"}, {154, "BFF2"},
					{155, "Mass Hysteria"}, {156, "A Fresh Pair of Eyes"},

					{1001, "First Blood"}, {1002, "First Blood, Part 2"}, {1003, "Quick Hook"}, {1004, "A Year to Remember"},
					{1005, "The Cycle"}, {1006, "Closer"}, {1007, "If You Build it"}, {1008, "Gun Down"},
					{1009, "Batter Up"}, {1010, "Doctoring the ball"}, {1011, "Dodgers 1, Giants 0"}, {1012, "Batting the Doctor"},
					{1013, "I'm Bat Man"}, {1014, "Triple Steal"}, {1015, "Pop Fly"}, {1016, "Round-Tripper"},
					{1017, "Artful Dodger"}, {1018, "Fall Classic"}, {1019, "Strike Zone"}, {1020, "Foul Territory"},
					{1021, "The Big Hurt"}, {1022, "Brushback"}, {1023, "Moon Shot"}, {1024, "Beanball"},
					{1025, "Retire the Runner"}, {1026, "Caught Napping"}, {1027, "Side Retired"}, {1028, "Triple Play"},
					{1029, "Stealing Home"}, {1030, "Set the Table"}, {1031, "Block the Plate"}, {1032, "Belittled Beleaguer"},
					{1033, "No-Hitter"}, {1034, "Race for the Pennant"}, {1035, "Out of the Park"}, {1036, "Scout Milestone 1"},
					{1037, "Scout Milestone 2"}, {1038, "Scout Milestone 3"},

					{1101, "Rode Hard, Put Away Wet"}, {1102, "Be polite"}, {1103, "Be Efficient"}, {1104, "Have a Plan"},
					{1105, "Kill Everyone You Meet"}, {1106, "Triple Prey"}, {1107, "Self-Destruct Sequence"}, {1108, "De-sentry-lized"},
					{1109, "Shoot the Breeze"}, {1110, "Dropped Dead"}, {1111, "the last wave"}, {1112, "australian rules"},
					{1113, "Kook the Spook"}, {1114, "Socket to Him"}, {1115, "Jumper Stumper"}, {1116, "Not a Crazed Gunman, Dad"},
					{1117, "Trust Your Feelings"}, {1118, "Uberectomy"}, {1119, "Consolation Prize"}, {1120, "Enemy at the Gate"},
					{1121, "Parting Shot"}, {1122, "My Brilliant Career"}, {1123, "Shock Treatment"}, {1124, "Saturation Bombing"},
					{1125, "Rain on Their Parade"}, {1126, "Jarring Transition"}, {1127, "Friendship is Golden"}, {1128, "William Tell Overkill"},
					{1129, "Beaux and Arrows"}, {1130, "Robbin' Hood"}, {1131, "Pincushion"}, {1132, "Number One Assistant"},
					{1133, "Jarate Chop"}, {1134, "Shafted"}, {1135, "Dead Reckoning"}, {1136, "Sniper Milestone 1"},
					{1137, "Sniper Milestone 2"}, {1138, "Sniper Milestone 3"},

					{1201, "Duty Bound"}, {1202, "The Boostie Boys"}, {1203, "Out, Damned Scot!"}, {1204, "Engineer to Eternity"},
					{1205, "Backdraft Dodger"}, {1206, "Trench Warfare"}, {1207, "Bomb Squaddie"}, {1208, "Where Eagles Dare"},
					{1209, "Ain't Got Time to Bleed"}, {1210, "Banner of Brothers"}, {1211, "Tri-Splatteral Damage"}, {1212, "Death from Above"},
					{1213, "Spray of Defeat"}, {1214, "War Crime and Punishment"}, {1215, "Near Death Experience"}, {1216, "Wings of Glory"},
					{1217, "For Whom the Shell Trolls"}, {1218, "Death From Below"}, {1219, "Mutually Assured Destruction"}, {1220, "Guns of the Navar0wned"},
					{1221, "Brothers in Harms"}, {1222, "Medals of Honor"}, {1223, "S*M*A*S*H"}, {1224, "Crockets Are Such B.S."},
					{1225, "Geneva Contravention"}, {1226, "Semper Fry"}, {1227, "Worth a Thousand Words"}, {1228, "Gore-a! Gore-a! Gore-a!"},
					{1229, "War Crime Spybunal"}, {1230, "Frags of our Fathers"}, {1231, "Dominator"}, {1232, "Ride of the Valkartie"},
					{1233, "Screamin' Eagle"}, {1234, "The Longest Daze"}, {1235, "Hamburger Hill"}, {1236, "Soldier Milestone 1"},
					{1237, "Soldier Milestone 2"}, {1238, "Soldier Milestone 3"},

					{1301, "Kilt in Action"}, {1302, "Tam O'Shatter"}, {1303, "Shorn Connery"}, {1304, "Laddy Macdeth"},
					{1305, "Caber Toss"}, {1306, "Double Mauled Scotch"}, {1307, "Loch Ness Bombster"}, {1308, "Three Times a Laddy"},
					{1309, "Blind Fire"}, {1310, "Brainspotting"}, {1311, "Left 4 Heads"}, {1312, "Well Plaid!"},
					{1313, "The Scottish Play"}, {1314, "The Argyle Sap"}, {1315, "Slammy Slayvis Woundya"}, {1316, "There Can Be Only One"},
					{1317, "Tartan Spartan"}, {1318, "Scotch Guard"}, {1319, "Bravehurt"}, {1320, "Cry Some Moor!"},
					{1321, "The Stickening"}, {1322, "Glasg0wned"}, {1323, "Scotch Tap"}, {1324, "The Targe Charge"},
					{1325, "Beat Me Up, Scotty"}, {1326, "Something Stickied This Way Comes"}, {1327, "The High Road"}, {1328, "Bloody Merry"},
					{1329, "Second Eye"}, {1330, "He Who Celt It"}, {1331, "Robbed Royal"}, {1332, "Highland Fling"},
					{1333, "Pipebagger"}, {1334, "Spynal Tap"}, {1335, "Sticky Thump"}, {1336, "Demoman Milestone 1"},
					{1337, "Demoman Milestone 2"}, {1338, "Demoman Milestone 3"},

					{1401, "First Do No Harm"}, {1402, "Quadruple Bypass"}, {1403, "Group Health"}, {1404, "Surgical Prep"},
					{1405, "Trauma Queen"}, {1406, "Double Blind Trial"}, {1407, "Play Doctor"}, {1408, "Triage"},
					{1409, "Preventative Medicine"}, {1410, "Consultation"}, {1411, "Does It Hurt When I Do This?"}, {1412, "Peer Review"},
					{1413, "Big Pharma"}, {1414, "You'll Feel a Little Prick"}, {1415, "Autoclave"}, {1416, "Blunt Trauma"},
					{1417, "Medical Breakthrough"}, {1418, "Blast Assist"}, {1419, "Midwife Crisis"}, {1420, "Ubi concordia, ibi victoria"},
					{1421, "Grand Rounds"}, {1422, "Infernal Medicine"}, {1423, "Doctor Assisted Homicide"}, {1424, "Placebo Effect"},
					{1425, "Sawbones"}, {1426, "Intern"}, {1427, "Specialist"}, {1428, "Chief of Staff"},
					{1429, "Hypocritical Oath"}, {1430, "Medical Intervention"}, {1431, "Second Opinion"}, {1432, "Autopsy Report"},
					{1433, "FYI I am A Medic"}, {1434, "Family Practice"}, {1435, "House Call"}, {1436, "Bedside Manner"},
					{1437, "Medic Milestone 1"}, {1438, "Medic Milestone 2"}, {1439, "Medic Milestone 3"},

					{1501, "Iron Kurtain"}, {1502, "Party Loyalty"}, {1503, "Division of Labor"}, {1504, "Red Oktoberfest"},
					{1505, "Show Trial"}, {1506, "Crime and Punishment"}, {1507, "Class Struggle"}, {1508, "Soviet Block"},
					{1509, "Stalin the Kart"}, {1510, "Supreme Soviet"}, {1511, "Factory Worker"}, {1512, "Soviet Union"},
					{1513, "Own the Means of Production"}, {1514, "Krazy Ivan"}, {1515, "Rasputin"}, {1516, "Icing on the Cake"},
					{1517, "Crock Block"}, {1518, "Kollectivization"}, {1519, "Spyalectical Materialism"}, {1520, "Permanent Revolution"},
					{1521, "Heavy Industry"}, {1522, "Communist Mani-Fisto"}, {1523, "Redistribution of Health"}, {1524, "Rationing"},
					{1525, "Vanguard Party"}, {1527, "Pushkin the Kart"}, {1528, "Marxman"}, {1529, "Gorky Parked"},
					{1530, "Purge"}, {1531, "Lenin A Hand"}, {1532, "Five Second Plan"}, {1533, "Photostroika"},
					{1534, "Konspicuous Konsumption"}, {1535, "Don't Touch Sandvich"}, {1536, "Borscht Belt"}, {1537, "Heavy Milestone 1"},
					{1538, "Heavy Milestone 2"}, {1539, "Heavy Milestone 3"},

					{1601, "Combined Fire"}, {1602, "Weenie Roast"}, {1603, "Baptism By Fire"}, {1604, "Fire and Forget"},
					{1605, "Firewall"}, {1606, "Cooking the Books"}, {1607, "Spontaneous Combustion"}, {1608, "Trailblazer"},
					{1609, "Camp Fire"}, {1610, "Lumberjack"}, {1611, "Clearcutter"}, {1612, "Hot on Your Heels"},
					{1613, "I Fry"}, {1614, "Firewatch"}, {1615, "Burn Ward"}, {1616, "Hot Potato"},
					{1617, "Makin' Bacon"}, {1618, "Plan B"}, {1619, "Pyrotechnics"}, {1620, "Arsonist"},
					{1621, "Controlled Burn"}, {1622, "Firefighter"}, {1623, "Pyromancer"}, {1624, "Next of Kindling"},
					{1625, "OMGWTFBBQ"}, {1626, "Second Degree Burn"}, {1627, "Got A Light?"}, {1628, "BarbeQueQ"},
					{1629, "Hotshot"}, {1630, "Dance Dance Immolation"}, {1631, "Dead Heat"}, {1632, "Pilot Light"},
					{1633, "Freezer Burn"}, {1634, "Fire Chief"}, {1635, "Attention Getter"}, {1637, "Pyro Milestone 1"},
					{1638, "Pyro Milestone 2"}, {1639, "Pyro Milestone 3"},

					{1701, "Triplecrossed"}, {1702, "For Your Eyes Only"}, {1703, "Counter Espionage"}, {1704, "Identity Theft"},
					{1705, "The Man from P.U.N.C.T.U.R.E."}, {1706, "FYI I am a Spy"}, {1707, "The Man with the Broken Guns"}, {1708, "Sapsucker"},
					{1709, "May I Cut In?"}, {1710, "Agent Provocateur"}, {1711, "The Melbourne Supremacy"}, {1712, "Spies Like Us"},
					{1713, "A Cut Above"}, {1714, "Burn Notice"}, {1715, "Die Another Way"}, {1716, "Constructus Interruptus"},
					{1717, "On Her Majesty's Secret Surface"}, {1718, "Insurance Fraud"}, {1719, "Point Breaker"}, {1720, "High Value Target"},
					{1721, "Come in From the Cold"}, {1722, "Wetwork"}, {1723, "You Only Shiv Thrice"}, {1724, "Spymaster"},
					{1725, "Sap Auteur"}, {1726, "Joint Operation"}, {1727, "Dr. Nooooo"}, {1728, "Is It Safe?"},
					{1729, "Slash and Burn"}, {1730, "Biplomacy"}, {1731, "Skullpluggery"}, {1732, "Sleeper Agent"},
					{1733, "Who's Your Daddy?"}, {1734, "Deep Undercover"}, {1735, "Spy Milestone 1"}, {1736, "Spy Milestone 2"},
					{1737, "Spy Milestone 3"},

					{1801, "Engineer Milestone 1"}, {1802, "Engineer Milestone 2"}, {1803, "Engineer Milestone 3"}, {1804, "Revengineering"},
					{1805, "Battle Rustler"}, {1806, "The Extinguished Gentleman"}, {1807, "Search Engine"}, {1808, "Unforgiven"},
					{1809, "Building Block"}, {1810, "Pownd on the Range"}, {1811, "Silent Pardner"}, {1812, "Doc Holiday"},
					{1813, "Best Little Slaughterhouse In Texas"}, {1814, "Death Metal"}, {1815, "Trade Secrets"}, {1816, "The Wrench Connection"},
					{1817, "Land Grab"}, {1818, "Six-String Stringer"}, {1819, "Uncivil Engineer"}, {1820, "Texas Two-Step"},
					{1821, "Frontier Justice"}, {1822, "Doc, Stock, and Barrel"}, {1823, "No Man's Land"}, {1824, "Fistful of Sappers"},
					{1825, "Quick Draw"}, {1826, "Get Along!"}, {1827, "Honky Tonky Man"}, {1828, "How the Pests Was Gunned"},
					{1829, "Rio Grind"}, {1830, "Breaking Morant"}, {1831, "Patent Protection"}, {1832, "If You Build It, They Will Die"},
					{1833, "Texas Ranger"}, {1834, "Deputized"}, {1835, "Drugstore Cowboy"}, {1836, "Circle the Wagons"},
					{1837, "Build to Last"}, {1838, "(Not so) Lonely Are the Brave"},

					{1901, "Candy Cornoner"}, {1902, "Ghastly Gibus Grab"}, {1903, "Scared Stiff"}, {1904, "Attack 'o Latern"},
					{1905, "Costume Contest"}, {1906, "Sleepy Holl0WND"}, {1907, "Masked Mann"}, {1908, "Sackston Hale"},
					{1909, "Gored!"}, {1910, "Optical Defusion"}, {1911, "Dive Into A Good Book"}, {1912, "A Lovely Vacation Spot"},
					{1913, "Wizards Never Prosper"}, {1914, "Helltower: Hell's Spells"}, {1915, "Helltower: Competitive Spirit (+1)"},
					{1916, "Helltower: Mine Games (+1)"}, {1917, "Helltower: Skeleton Coup (+1)"}, {1918, "HellTower: Spelling Spree (+1)"},
					{1919, "Helltower: Hell on Wheels"}, {1920, "The Mann-tastic Four"}, {1921, "Hat Out of Hell"},

					{2001, "That's a Wrap"}, {2002, "We Can Fix It In Post"}, {2003, "Time For Your Close-Up, Mr. Hale"}, {2004, "Star of My Own Show"},
					{2005, "Home Movie"}, {2006, "Local Cinema Star"}, {2007, "Indie Film Sensation"}, {2008, "Blockbuster"},

					{2101, "Gift Grab"},

					{2201, "Cap Trap"}, {2202, "Foundry Force Five"}, {2203, "Two Minute Warring"}, {2204, "The Crucible"},
					{2205, "Five the Fast Way"}, {2206, "Claim Jumper"}, {2207, "Terminated, Too"}, {2208, "Real Steal"},
					{2209, "Classassin"}, {2210, "Raze the Roof"}, {2211, "Dead Heat"}, {2212, "Foundry Milestone"},

					{2301, "Steel Fragnolias"}, {2302, "Wage Against the Machine"}, {2303, "Frags to Riches"}, {2304, "Fast Cache"},
					{2305, "T-1000000"}, {2306, "Brotherhood of Steel"}, {2307, "Hack of All Trades"}, {2308, "Clockwork Carnage"},
					{2309, "Balls-E"}, {2310, "Clockwork Conqueror"}, {2311, "Spam Blocker"}, {2312, ".executioner"},
					{2313, "Deus Ex Machina"}, {2314, "Raid Array"}, {2315, "Ghost in the Machine"}, {2316, "Kritical Terror"},
					{2317, "German Engineering"}, {2318, "Undelete"}, {2319, "Shell Extension"}, {2320, "System Upgrade"},
					{2321, "Maximum Performance"}, {2322, "Engine Block"}, {2323, "Negative Charge"}, {2324, "Silicon Slaughter"},
					{2325, "Metal Massacre"}, {2326, "Ctrl + Assault + Delete"}, {2327, "Sly Voltage"}, {2328, "Turbocharger"},
					{2329, "Heavy Mettle"}, {2330, "Vial Sharing"}, {2331, "Tech Wrecker"}, {2332, "Do Androids Dream?"},
					{2333, "Spark Plugger"}, {2334, "Hard Reset"}, {2335, "Real Steal"},

					{2401, "Mission Control"}, {2402, "Flight Crew"}, {2403, "The Fight Stuff"}, {2404, "Plan Nine to Outer Space"},
					{2405, "Failure to Launch"}, {2406, "Rocket Booster"}, {2407, "Best Case Scenario"}, {2408, "Cap-ogee"},
					{2409, "Space Camp"}, {2410, "Lift-offed"}, {2411, "Escape Ferocity"}, {2412, "Doomsday Milestone"}
				};
				return vEntries;
			}
			inline const std::vector<const char*>& GetAchievementSpamDropdownEntries()
			{
				static std::vector<std::string> vAchievementSpamNames = {};
				static std::vector<const char*> vAchievementSpamEntries = {};

				if (vAchievementSpamEntries.empty())
				{
					const auto& vAchievements = GetAchievementSpamEntries();
					vAchievementSpamNames.reserve(vAchievements.size());
					vAchievementSpamEntries.reserve(vAchievements.size());

					for (const auto& tAchievement : vAchievements)
						vAchievementSpamNames.push_back(std::format("{} - {}", tAchievement.m_sName, tAchievement.m_iID));

					for (const auto& sEntry : vAchievementSpamNames)
						vAchievementSpamEntries.push_back(sEntry.c_str());
				}
				return vAchievementSpamEntries;
			}
			inline const std::vector<int>& GetAchievementSpamDropdownIDs()
			{
				static std::vector<int> vAchievementSpamIDs = {};
				if (vAchievementSpamIDs.empty())
				{
					const auto& vAchievements = GetAchievementSpamEntries();
					vAchievementSpamIDs.reserve(vAchievements.size());

					for (const auto& tAchievement : vAchievements)
						vAchievementSpamIDs.push_back(tAchievement.m_iID);
				}
				return vAchievementSpamIDs;
			}
			inline const std::vector<int>& GetItemAchievementIDs()
			{
				static const std::vector<int> vItemAchievementIDs =
				{
					1036, 1037, 1038, 1136, 1137, 1138, 1236, 1237, 1238, 1336, 1337, 1338, 1437, 1438, 1439, 1537,
					1538, 1539, 1637, 1638, 1639, 1735, 1736, 1737, 1801, 1802, 1803, 1901, 1902,
					1906, 1909, 1910, 1911, 1912, 1928, 2006, 2212, 2412
				};
				return vItemAchievementIDs;
			}
			CVarEnum(AutoVote, "Auto vote", 0b0, DROPDOWN_MULTI, "Off",
				VA_LIST("Defend", "Assist", "Kick", "##Divider", "Kick all", "Wait for cooldown"),
				Defend = 1 << 0, Assist = 1 << 1, Kick = 1 << 2, KickAll = 1 << 3, Cooldown = 1 << 4);
			CVar(AutoVoteDelay, "Random vote delay", false);
			CVar(AutoVoteDelayInterval, "Delay interval", FloatRange_t(1.f, 6.5f), SLIDER_MIN, 1.f, 6.5f, 0.5f, "%0.1fs - %0.1fs");
			CVar(RandomClass, "Random class", false);
			CVarEnum(RandomClassExclude, "Random class exclude", 0b0, DROPDOWN_MULTI, "None",
				VA_LIST("Scout", "Sniper", "Soldier", "Demoman", "Medic", "Heavy", "Pyro", "Spy", "Engineer"),
				Scout = 1 << 0, Sniper = 1 << 1, Soldier = 1 << 2, Demoman = 1 << 3, Medic = 1 << 4, Heavy = 1 << 5, Pyro = 1 << 6, Spy = 1 << 7, Engineer = 1 << 8);
			CVar(RandomClassInterval, "Random class interval", FloatRange_t(3.f, 5.f), SLIDER_MIN | SLIDER_PRECISION, 0.5f, 30.f, 0.5f, "%g - %gm");
			CVar(ForceClass, "Autojoin class", 0);
			CVar(Micspam, "Micspam", false);
			CVar(NoiseSpam, "Noise spam", false);
			CVar(CallVoteSpam, "Callvote spam", false);
			CVarEnum(VoiceCommandSpam, "Voice command spam", 0, NONE, nullptr,
				VA_LIST("Off", "Random", "Medic", "Thanks", "Nice Shot", "Cheers", "Jeers", "Go Go Go", "Move Up", "Go Left", "Go Right", "Yes", "No", "Incoming", "Spy", "Sentry Ahead", "Need Teleporter", "Pootis", "Need Sentry", "Activate Charge", "Help", "Battle Cry", "Good Job"),
				Off, Random, Medic, Thanks, NiceShot, Cheers, Jeers, GoGoGo, MoveUp, GoLeft, GoRight, Yes, No, Incoming, Spy, Sentry, NeedTeleporter, Pootis, NeedSentry, ActivateCharge, Help, BattleCry, GoodJob);
			CVar(AutoVoteMap, "Auto vote map", true);
			CVar(AutoVoteMapOption, "", 2, SLIDER_CLAMP, 0, 2, 1, "%i");
			CVar(AutoReport, "Auto report players", false);

			NAMESPACE_BEGIN(ChatSpam)
				CVar(Enable, "Chat spam", false);
				CVar(Interval, "Interval", 3.0f, SLIDER_CLAMP | SLIDER_PRECISION, 0.5f, 10.0f, 0.5f, "%0.1fs");
				CVar(TeamChat, "Team chat", false);
				CVar(Randomize, "Randomize", false);
				CVar(AutoReply, "Auto reply", false);
				CVar(VoteKickReply, "Vote kick reply", false);
			NAMESPACE_END(ChatSpam)

			NAMESPACE_BEGIN(KillSay)
				CVar(Enable, "Kill say", false);
				CVar(Chance, "Kill say chance", 100, SLIDER_CLAMP, 0, 100, 1, "%i%%");
				CVar(TeamChat, "Team chat", false);
			NAMESPACE_END(KillSay)

			NAMESPACE_BEGIN(AutoItem)
				CVarEnum(Enable, "Enable", 0b0, DROPDOWN_MULTI, nullptr,
					VA_LIST("Waapons", "Hats", "Noisemaker"),
					Weapons = 1 << 0, Hats = 1 << 1, Noisemaker = 1 << 2);
				CVar(Interval, "Interval", 30, SLIDER_CLAMP, 2, 60, 1, "%is");

				CVar(Primary, "Weapon primary", std::string("-1"));
				CVar(Secondary, "Weapon secondary", std::string("-1"));
				CVar(Melee, "Weapon melee", std::string("-1"));

				CVar(FirstHat, "Hat 1", 940);
				CVar(SecondHat, "Hat 2", 941);
				CVar(ThirdHat, "Hat 3", 302);
			NAMESPACE_END(AutoItem)
		NAMESPACE_END(Automation)

		NAMESPACE_BEGIN(Exploits)
			CVar(PureBypass, "Pure bypass", false);
			CVar(CheatsBypass, "Cheats bypass", false);
			CVar(UnlockCVars, "Unlock CVars", true);
			CVar(EquipRegionUnlock, "Equip region unlock", false);
			CVar(BreakShootSound, "Break shoot sound", false);
			CVar(BackpackExpander, "Backpack expander", false);
			CVar(PingReducer, "Ping reducer", false);
			CVar(PingTarget, "Ping", 1, SLIDER_CLAMP, 1, 100, 1);
		NAMESPACE_END(Exploits)

		NAMESPACE_BEGIN(Game)
			CVar(NetworkFix, "Network fix", false);
			CVar(SetupBonesOptimization, "Bones optimization", false);
			CVar(InsecureBypass, "-insecure bypass", false);
			CVar(AntiCheatCompatibility, "Anti-cheat compatibility", false);
			CVar(AntiCheatCritHack, "Anti-cheat crit hack", false, NOSAVE | DEBUGVAR);
		NAMESPACE_END(Game)

		NAMESPACE_BEGIN(Queueing)
			CVarEnum(ForceRegions, "Force regions", 0b0, DROPDOWN_MULTI, nullptr, // i'm not sure all of these are actually used for tf2 servers (they are)
				VA_LIST("Atlanta", "Chicago", "Dallas", "Los Angeles", "Seattle", "Virginia", "##Divider", "Amsterdam", "Falkenstein", "Frankfurt", "Helsinki", "London", "Madrid", "Paris", "Stockholm", "Vienna", "Warsaw", "##Divider", "Buenos Aires", "Lima", "Santiago", "Sao Paulo", "##Divider", "Chennai", "Dubai", "Hong Kong", "Mumbai", "Seoul", "Singapore", "Tokyo", "##Divider", "Sydney", "##Divider", "Johannesburg"),
				// North America
				ATL = 1 << 0, // Atlanta
				ORD = 1 << 1, // Chicago
				DFW = 1 << 2, // Dallas
				LAX = 1 << 3, // Los Angeles
				SEA = 1 << 4, // Seattle (+DC_EAT?)
				IAD = 1 << 5, // Virginia
				// Europe
				AMS = 1 << 6, // Amsterdam
				FSN = 1 << 7, // Falkenstein
				FRA = 1 << 8, // Frankfurt
				HEL = 1 << 9, // Helsinki
				LHR = 1 << 10, // London
				MAD = 1 << 11, // Madrid
				PAR = 1 << 12, // Paris
				STO = 1 << 13, // Stockholm
				VIE = 1 << 14, // Vienna
				WAW = 1 << 15, // Warsaw
				// South America
				EZE = 1 << 16, // Buenos Aires
				LIM = 1 << 17, // Lima
				SCL = 1 << 18, // Santiago
				GRU = 1 << 19, // Sao Paulo
				// Asia
				MAA = 1 << 20, // Chennai
				DXB = 1 << 21, // Dubai
				HKG = 1 << 22, // Hong Kong
				BOM = 1 << 23, // Mumbai
				SEO = 1 << 24, // Seoul
				SGP = 1 << 25, // Singapore
				TYO = 1 << 26, // Tokyo
				// Australia
				SYD = 1 << 27, // Sydney
				// Africa
				JNB = 1 << 28, // Johannesburg
			);
			CVarEnum(ExtendQueue, "Extend queue", 0, NONE, nullptr, 
				VA_LIST("Off", "Join", "Abandon"),
				Off, Join, Abandon);
			CVar(AutoCasualQueue, "Auto casual queue", false);
			CVar(AutoCasualJoin, "Auto casual join", false);
			CVar(AutoCompetitiveQueue, "Auto competitive queue", false);
			CVar(AutoAbandonIfNoNavmesh, "Auto abandon if no navmesh", false);
			CVar(AutoDumpProfiles, "Auto dump profiles", false);
			CVar(AutoDumpDelay, "Auto dump delay", 15, SLIDER_CLAMP, 0, 120, 1, "%is");
			CVar(QueueDelay, "Queue delay", 5, SLIDER_MIN, 0, 10, 1, "%im");
			CVar(RQif, "Requeue if...", false); // Dropdown?
			CVar(RQplt, "Players LT", 12, SLIDER_MIN, 0, 100, 1, "%i");
			CVar(RQpgt, "Players GT", 0, SLIDER_MIN, 0, 100, 1, "%i");
			CVar(RQkick, "Kicked", false);
			CVar(RQLTM, "dont RQLTM", false);
			CVar(RQIgnoreFriends, "Ignore Friends", false);
			CVar(RQnoAbandon, "RQ w/o abandon", false);
			CVar(AutoMannUpQueue, "Auto MannUp queue", false);
		NAMESPACE_END(Queueing)

		NAMESPACE_BEGIN(MannVsMachine, Mann vs. Machine)
			CVar(InstantRespawn, "Instant respawn", false);
			CVar(InstantRevive, "Instant revive", false);
			CVar(AllowInspect, "Allow inspect", false);
			CVar(AutoMvmReadyUp, "Auto MvM ready up", false);
			CVar(AutoAbandonMannUp, "Auto abandon Mann Up", false);
			CVar(BuyBot, "Buy Bot", false);
			CVar(MaxCash, "Turn off buybot at cash", 15000, SLIDER_CLAMP | SLIDER_MIN, 0, 100000, 1000, "%i");
			CVar(BuyBotAutoClass, "Buy Bot auto class", false);
			CVar(BuyBotClass, "Buy Bot class", 6);
		NAMESPACE_END(MannVsMachine)

		NAMESPACE_BEGIN(Sound)
			CVarEnum(Block, VA_LIST("Block", "Sound block"), 0b0000, DROPDOWN_MULTI, nullptr,
				VA_LIST("Footsteps", "Noisemaker", "Frying pan", "Water"),
				Footsteps = 1 << 0, Noisemaker = 1 << 1, FryingPan = 1 << 2, Water = 1 << 3);
			CVar(HitsoundAlways, "Hitsound always", false);
			CVar(RemoveDSP, "Remove DSP", false);
			CVar(GiantWeaponSounds, "Giant weapon sounds", false);
		NAMESPACE_END(Sound)
	NAMESPACE_END(Misc)

	NAMESPACE_BEGIN(Logging)
		CVarEnum(Logs, "Logs", 0b0000011, DROPDOWN_MULTI, "Off",
			VA_LIST("Vote start", "Vote cast", "Class changes", "Damage", "Cheat detection", "Tags", "Aliases", "Resolver"),
			VoteStart = 1 << 0, VoteCast = 1 << 1, ClassChanges = 1 << 2, Damage = 1 << 3, CheatDetection = 1 << 4, Tags = 1 << 5, Aliases = 1 << 6, Resolver = 1 << 7);
		Enum(LogTo, Toasts = 1 << 0, Chat = 1 << 1, Party = 1 << 2, Console = 1 << 3, Menu = 1 << 4, Debug = 1 << 5);
		CVarEnum(NotificationPosition, "Notification position", 0, VISUAL, nullptr,
			VA_LIST("Top left", "Top right", "Bottom left", "Bottom right"),
			TopLeft, TopRight, BottomLeft, BottomRight);
		CVar(NotificationTime, "Notification time", 5.f, VISUAL, 0.5f, 5.f, 0.5f);
		CVar(MaxNotifications, "Max notifications", 10, VISUAL | SLIDER_MIN, 1, 10);

		NAMESPACE_BEGIN(VoteStart, Logging)
			CVarValues(LogTo, "Vote start log to", 0b000001, DROPDOWN_MULTI, nullptr,
				"Toasts", "Chat", "Party", "Console", "Menu", "Debug");
		NAMESPACE_END(VoteStart)

		NAMESPACE_BEGIN(VoteCast, Logging)
			CVarValues(LogTo, "Vote cast log to", 0b000001, DROPDOWN_MULTI, nullptr,
				"Toasts", "Chat", "Party", "Console", "Menu", "Debug");
		NAMESPACE_END(VoteCast)

		NAMESPACE_BEGIN(ClassChange, Logging)
			CVarValues(LogTo, "Class change log to", 0b000001, DROPDOWN_MULTI, nullptr,
				"Toasts", "Chat", "Party", "Console", "Menu", "Debug");
		NAMESPACE_END(ClassChange)

		NAMESPACE_BEGIN(Damage, Logging)
			CVarValues(LogTo, "Damage log to", 0b000001, DROPDOWN_MULTI, nullptr,
				"Toasts", "Chat", "Party", "Console", "Menu", "Debug");
		NAMESPACE_END(Damage)

		NAMESPACE_BEGIN(CheatDetection, Logging)
			CVarValues(LogTo, "Cheat detection log to", 0b000001, DROPDOWN_MULTI, nullptr,
				"Toasts", "Chat", "Party", "Console", "Menu", "Debug");
		NAMESPACE_END(CheatDetection)

		NAMESPACE_BEGIN(Tags, Logging)
			CVarValues(LogTo, "Tags log to", 0b000001, DROPDOWN_MULTI, nullptr,
				"Toasts", "Chat", "Party", "Console", "Menu", "Debug");
		NAMESPACE_END(Tags)

		NAMESPACE_BEGIN(Aliases, Logging)
			CVarValues(LogTo, "Aliases log to", 0b000001, DROPDOWN_MULTI, nullptr,
				"Toasts", "Chat", "Party", "Console", "Menu", "Debug");
		NAMESPACE_END(Aliases)

		NAMESPACE_BEGIN(Resolver, Logging)
			CVarValues(LogTo, "Resolver log to", 0b000001, DROPDOWN_MULTI, nullptr,
				"Toasts", "Chat", "Party", "Console", "Menu", "Debug");
		NAMESPACE_END(Resolver)
	NAMESPACE_END(Logging)

	NAMESPACE_BEGIN(CheatDetection, Cheat Detection)
		CVarEnum(Methods, "Detection methods", 0b000000, DROPDOWN_MULTI, nullptr,
			VA_LIST("Invalid pitch", "Packet choking", "Aim flicking", "Duck Speed", "Lagcomp abuse", "Critbucket"),
			InvalidPitch = 1 << 0, PacketChoking = 1 << 1, AimFlicking = 1 << 2, DuckSpeed = 1 << 3, LagCompAbuse = 1 << 4, CritManipulation = 1 << 5);
		CVar(DetectionsRequired, "Detections required", 10, SLIDER_MIN, 0, 50);
		CVar(MinChoking, "Min choking", 20, SLIDER_MIN, 4, 22);
		CVar(MinFlick, "Min flick angle", 20.f, SLIDER_PRECISION, 10.f, 30.f); // min flick size to suspect
		CVar(MaxNoise, "Max flick noise", 1.f, SLIDER_PRECISION, 1.f, 10.f); // max difference between angles before and after flick
		CVar(LagCompMinimumDelta, "Lag burst delta", 3, SLIDER_MIN, 2, 8);
		CVar(LagCompWindow, "Lag burst window", 1.f, SLIDER_PRECISION, 0.25f, 2.f, 0.05f);
		CVar(LagCompBurstCount, "Lag burst count", 3, SLIDER_MIN, 1, 6);
		CVar(CritWindow, "Crit window size", 12, SLIDER_MIN, 6, 30);
		CVar(CritThreshold, "Crit rate threshold", 85.f, SLIDER_PRECISION, 50.f, 100.f, 5.f);
	NAMESPACE_END(CheatDetection)

	NAMESPACE_BEGIN(Debug)
		CVar(Info, "Debug info", false, NOSAVE);
		CVar(Logging, "Debug logging", false, NOSAVE);
		CVar(Options, "Debug options", false, NOSAVE);
		CVar(CrashLogging, "Crash logging", true, NOBIND);

#ifdef DEBUG_TRACES
		CVar(VisualizeTraces, "Visualize traces", false, NOSAVE);
		CVar(VisualizeTraceHits, "Visualize trace hits", false, NOSAVE);
#endif

		CVar(DrawHitboxes, "Show hitboxes", false, NOSAVE);
	NAMESPACE_END(Debug)
NAMESPACE_END(Vars)
