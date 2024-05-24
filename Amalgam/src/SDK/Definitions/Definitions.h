#pragma once
#include "Misc/BaseTypes.h"
#include "Types.h"

#define TICK_INTERVAL I::GlobalVars->interval_per_tick
#define TIME_TO_TICKS(dt) (static_cast<int>(0.5f + static_cast<float>(dt) / TICK_INTERVAL))
#define TICKS_TO_TIME(t) (TICK_INTERVAL * (t))

#define ANIMATION_CYCLE_BITS 10
#define ANIMATION_CYCLE_MINFRAC (1.0f / (1<<ANIMATION_CYCLE_BITS))

//TODO: add game rules
//#define VEC_VIEW g_pGameRules->GetViewVectors()->m_vView
//#define VEC_HULL_MIN g_pGameRules->GetViewVectors()->m_vHullMin
//#define VEC_HULL_MAX g_pGameRules->GetViewVectors()->m_vHullMax
//#define VEC_DUCK_HULL_MIN g_pGameRules->GetViewVectors()->m_vDuckHullMin
//#define VEC_DUCK_HULL_MAX g_pGameRules->GetViewVectors()->m_vDuckHullMax
//#define VEC_DUCK_VIEW g_pGameRules->GetViewVectors()->m_vDuckView
//#define VEC_OBS_HULL_MIN g_pGameRules->GetViewVectors()->m_vObsHullMin
//#define VEC_OBS_HULL_MAX g_pGameRules->GetViewVectors()->m_vObsHullMax
//#define VEC_DEAD_VIEWHEIGHT	g_pGameRules->GetViewVectors()->m_vDeadViewHeight

//#define VEC_VIEW_SCALED( player ) ( g_pGameRules->GetViewVectors()->m_vView * player->GetModelScale() )
//#define VEC_HULL_MIN_SCALED( player ) ( g_pGameRules->GetViewVectors()->m_vHullMin * player->GetModelScale() )
//#define VEC_HULL_MAX_SCALED( player ) ( g_pGameRules->GetViewVectors()->m_vHullMax * player->GetModelScale() )
//#define VEC_DUCK_HULL_MIN_SCALED( player ) ( g_pGameRules->GetViewVectors()->m_vDuckHullMin * player->GetModelScale() )
//#define VEC_DUCK_HULL_MAX_SCALED( player ) ( g_pGameRules->GetViewVectors()->m_vDuckHullMax * player->GetModelScale() )
//#define VEC_DUCK_VIEW_SCALED( player ) ( g_pGameRules->GetViewVectors()->m_vDuckView * player->GetModelScale() )
//#define VEC_OBS_HULL_MIN_SCALED( player ) ( g_pGameRules->GetViewVectors()->m_vObsHullMin * player->GetModelScale() )
//#define VEC_OBS_HULL_MAX_SCALED( player ) ( g_pGameRules->GetViewVectors()->m_vObsHullMax * player->GetModelScale() )
//#define VEC_DEAD_VIEWHEIGHT_SCALED( player ) ( g_pGameRules->GetViewVectors()->m_vDeadViewHeight * player->GetModelScale() )

#define WATERJUMP_HEIGHT			8

#define MAX_CLIMB_SPEED		200

#if defined(TF_DLL) || defined(TF_CLIENT_DLL)
#define TIME_TO_DUCK		0.2
#define TIME_TO_DUCK_MS		200.0f
#else
#define TIME_TO_DUCK 0.4f
#define TIME_TO_DUCK_MS	400.0f
#endif 
#define TIME_TO_UNDUCK 0.2f
#define TIME_TO_UNDUCK_MS 200.0f

#define MAX_WEAPON_SLOTS 6
#define MAX_WEAPON_POSITIONS 20
#define MAX_ITEM_TYPES 6
#define MAX_WEAPONS 48

#define MAX_ITEMS 5

#define WEAPON_NOCLIP -1

#define	MAX_AMMO_TYPES 32
#define MAX_AMMO_SLOTS 32

#define HUD_PRINTNOTIFY 1
#define HUD_PRINTCONSOLE 2
#define HUD_PRINTTALK 3
#define HUD_PRINTCENTER 4

#define MAX_VOTE_DETAILS_LENGTH 64
#define INVALID_ISSUE -1
#define MAX_VOTE_OPTIONS 5
#define DEDICATED_SERVER 99

#define TEAM_ANY -2
#define	TEAM_INVALID -1
#define TEAM_UNASSIGNED 0
#define TEAM_SPECTATOR 1
#define LAST_SHARED_TEAM TEAM_SPECTATOR
#define FIRST_GAME_TEAM (LAST_SHARED_TEAM+1)
#define MAX_TEAMS 32
#define MAX_TEAM_NAME_LENGTH 32

#define TRACER_FLAG_WHIZ 0x0001
#define TRACER_FLAG_USEATTACHMENT 0x0002
#define TRACER_DONT_USE_ATTACHMENT	-1

#define	HITGROUP_GENERIC 0
#define	HITGROUP_HEAD 1
#define	HITGROUP_CHEST 2
#define	HITGROUP_STOMACH 3
#define HITGROUP_LEFTARM 4	
#define HITGROUP_RIGHTARM 5
#define HITGROUP_LEFTLEG 6
#define HITGROUP_RIGHTLEG 7
#define HITGROUP_GEAR 10

#define DMG_GENERIC 0		
#define DMG_CRUSH (1 << 0)
#define DMG_BULLET (1 << 1)
#define DMG_SLASH (1 << 2)
#define DMG_BURN (1 << 3)
#define DMG_VEHICLE (1 << 4)
#define DMG_FALL (1 << 5)
#define DMG_BLAST (1 << 6)
#define DMG_CLUB (1 << 7)
#define DMG_SHOCK (1 << 8)
#define DMG_SONIC (1 << 9)
#define DMG_ENERGYBEAM (1 << 10)
#define DMG_PREVENT_PHYSICS_FORCE (1 << 11) 
#define DMG_NEVERGIB (1 << 12)
#define DMG_ALWAYSGIB (1 << 13)
#define DMG_DROWN (1 << 14)


#define DMG_PARALYZE (1 << 15)
#define DMG_NERVEGAS (1 << 16)
#define DMG_POISON (1 << 17)
#define DMG_RADIATION (1 << 18)
#define DMG_DROWNRECOVER (1 << 19)
#define DMG_ACID (1 << 20)
#define DMG_SLOWBURN (1 << 21)

#define DMG_REMOVENORAGDOLL	(1<<22)

#define DMG_PHYSGUN (1<<23)
#define DMG_PLASMA (1<<24)
#define DMG_AIRBOAT (1<<25)

#define DMG_DISSOLVE (1<<26)
#define DMG_BLAST_SURFACE (1<<27)
#define DMG_DIRECT (1<<28)
#define DMG_BUCKSHOT (1<<29)

#define DMG_LASTGENERICFLAG	DMG_BUCKSHOT

#define	DAMAGE_NO 0
#define DAMAGE_EVENTS_ONLY 1
#define	DAMAGE_YES 2
#define	DAMAGE_AIM 3

#define LAST_PLAYER_OBSERVERMODE OBS_MODE_ROAMING

#define TF_VISION_FILTER_NONE 0
#define TF_VISION_FILTER_PYRO (1<<0)
#define TF_VISION_FILTER_HALLOWEEN (1<<1)
#define TF_VISION_FILTER_ROME (1<<2)

#define	MAX_MVM_WAVE_STRING 256

#define TF_DAMAGE_CRIT_CHANCE 0.02f
#define TF_DAMAGE_CRIT_CHANCE_RAPID 0.02f
#define TF_DAMAGE_CRIT_DURATION_RAPID 2.0f
#define TF_DAMAGE_CRIT_CHANCE_MELEE 0.15f
#define TF_DAMAGE_CRITMOD_MAXTIME 20
#define TF_DAMAGE_CRITMOD_MINTIME 2
#define TF_DAMAGE_CRITMOD_DAMAGE 800
#define TF_DAMAGE_CRITMOD_MAXMULT 6
#define TF_DAMAGE_CRIT_MULTIPLIER 3.0f
#define TF_DAMAGE_MINICRIT_MULTIPLIER 1.35f

#define WEAPON_RANDOM_RANGE 10000

#define TF_HEALTH_UNDEFINED 1
#define TF_SPY_UNDEFINED TEAM_UNASSIGNED
#define TF_CLASS_COUNT ( TF_CLASS_COUNT_ALL )
#define TF_FIRST_NORMAL_CLASS ( TF_CLASS_UNDEFINED + 1 )
#define TF_LAST_NORMAL_CLASS ( TF_CLASS_CIVILIAN )
#define	TF_CLASS_MENU_BUTTONS ( TF_CLASS_RANDOM + 1 )

#define TF_TEAM_AUTOASSIGN (TF_TEAM_COUNT + 1 )
#define TF_TEAM_HALLOWEEN TF_TEAM_AUTOASSIGN
#define TF_TEAM_PVE_INVADERS TF_TEAM_BLUE
#define TF_TEAM_PVE_DEFENDERS TF_TEAM_RED	
#define TF_TEAM_PVE_INVADERS_GIANTS 4

#define COLOR_TF_SPECTATOR Color( 245, 229, 196, 255 )
#define COLOR_TF_RED Color( 175, 73, 73, 255 )
#define COLOR_TF_BLUE Color( 79, 117, 143, 255 )

#define TF_PLAYER_WEAPON_COUNT 6
#define TF_PLAYER_GRENADE_COUNT 2
#define TF_PLAYER_BUILDABLE_COUNT 3
#define TF_PLAYER_BLUEPRINT_COUNT 6
#define TF_WEAPON_PRIMARY_MODE 0
#define TF_WEAPON_SECONDARY_MODE 1
#define TF_WEAPON_GRENADE_FRICTION 0.6f
#define TF_WEAPON_GRENADE_GRAVITY 0.81f
#define TF_WEAPON_GRENADE_INITPRIME 0.8f
#define TF_WEAPON_GRENADE_CONCUSSION_TIME 15.0f
#define TF_WEAPON_GRENADE_MIRV_BOMB_COUNT 4
#define TF_WEAPON_GRENADE_CALTROP_TIME 8.0f
#define TF_WEAPON_PIPEBOMB_WORLD_COUNT 15
#define TF_WEAPON_PIPEBOMB_COUNT 8
#define TF_WEAPON_PIPEBOMB_INTERVAL 0.6f
#define TF_WEAPON_ROCKET_INTERVAL 0.8f
#define TF_WEAPON_FLAMETHROWER_INTERVAL 0.15f
#define TF_WEAPON_FLAMETHROWER_ROCKET_INTERVAL 0.8f
#define TF_WEAPON_ZOOM_FOV 20

#define TF_PLAYER_VIEW_OFFSET Vector( 0, 0, 64.0 )
#define TF_BURNING_FREQUENCY 0.5f
#define TF_BURNING_FLAME_LIFE 10.0
#define TF_BURNING_FLAME_LIFE_PYRO 0.25
#define TF_BURNING_FLAME_LIFE_FLARE 10.0
#define TF_BURNING_FLAME_LIFE_PLASMA 6.0
#define TF_BURNING_DMG 3
#define TF_BLEEDING_FREQUENCY 0.5f
#define TF_BLEEDING_DMG 4
#define TF_TIME_TO_DISGUISE 2.0f
#define TF_TIME_TO_QUICK_DISGUISE 0.5f
#define TF_TIME_TO_SHOW_DISGUISED_FINISHED_EFFECT 5.0
#define TF_SCOUT_NUMBEROFPHASEATTACHMENTS 5
#define SHOW_DISGUISE_EFFECT  
#define TF_DISGUISE_TARGET_INDEX_NONE ( MAX_PLAYERS + 1 )
#define TF_PLAYER_INDEX_NONE ( MAX_PLAYERS + 1 )

#define TF_FLAGINFO_HOME 0
#define TF_FLAGINFO_STOLEN (1<<0)
#define TF_FLAGINFO_DROPPED (1<<1)

#define TF_REGEN_TIME 1.0
#define TF_REGEN_AMOUNT 3 
#define TF_REGEN_TIME_RUNE 0.25
#define TF_TIME_ASSIST_KILL 3.0f
#define TF_TIME_SUICIDE_KILL_CREDIT 10.0f
#define TF_KILLS_DOMINATION 4

#define DMG_USE_HITLOCATIONS (DMG_AIRBOAT)
#define DMG_HALF_FALLOFF (DMG_RADIATION)
#define DMG_CRITICAL (DMG_ACID)
#define DMG_RADIUS_MAX (DMG_ENERGYBEAM)
#define DMG_IGNITE (DMG_PLASMA)
#define DMG_USEDISTANCEMOD (DMG_SLOWBURN)
#define DMG_NOCLOSEDISTANCEMOD (DMG_POISON)
#define DMG_FROM_OTHER_SAPPER (DMG_IGNITE)
#define DMG_MELEE (DMG_BLAST_SURFACE)
#define DMG_DONT_COUNT_DAMAGE_TOWARDS_CRIT_RATE	(DMG_DISSOLVE)
#define DMG_IGNORE_MAXHEALTH (DMG_BULLET)
#define DMG_IGNORE_DEBUFFS (DMG_SLASH)

#define TF_STUN_NONE 0
#define TF_STUN_MOVEMENT (1<<0)
#define	TF_STUN_CONTROLS (1<<1)
#define TF_STUN_MOVEMENT_FORWARD_ONLY (1<<2)
#define TF_STUN_SPECIAL_SOUND (1<<3)
#define TF_STUN_DODGE_COOLDOWN (1<<4)
#define TF_STUN_NO_EFFECTS (1<<5)
#define TF_STUN_LOSER_STATE (1<<6)
#define TF_STUN_BY_TRIGGER (1<<7)
#define TF_STUN_BOTH TF_STUN_MOVEMENT | TF_STUN_CONTROLS

#define SENTRYGUN_UPGRADE_COST 130
#define SENTRYGUN_UPGRADE_METAL 200
#define SENTRYGUN_EYE_OFFSET_LEVEL_1 Vector( 0, 0, 32 )
#define SENTRYGUN_EYE_OFFSET_LEVEL_2 Vector( 0, 0, 40 )
#define SENTRYGUN_EYE_OFFSET_LEVEL_3 Vector( 0, 0, 46 )
#define SENTRYGUN_MAX_SHELLS_1 150
#define SENTRYGUN_MAX_SHELLS_2 200
#define SENTRYGUN_MAX_SHELLS_3 200
#define SENTRYGUN_MAX_ROCKETS 20

#define DISPENSER_MAX_METAL_AMMO 400
#define	MAX_DISPENSER_HEALING_TARGETS 32
#define MINI_DISPENSER_MAX_METAL 200

#define BUILDING_MODE_ANY -1

#define TF_SCORE_KILL 1
#define TF_SCORE_DEATH 0
#define TF_SCORE_CAPTURE 2
#define TF_SCORE_DEFEND 1
#define TF_SCORE_DESTROY_BUILDING 1
#define TF_SCORE_HEADSHOT_DIVISOR 2
#define TF_SCORE_BACKSTAB 1
#define TF_SCORE_INVULN 1
#define TF_SCORE_REVENGE 1
#define TF_SCORE_KILL_ASSISTS_PER_POINT 2
#define TF_SCORE_TELEPORTS_PER_POINT 2
#define TF_SCORE_HEAL_HEALTHUNITS_PER_POINT 600
#define TF_SCORE_BONUS_POINT_DIVISOR 10
#define TF_SCORE_DAMAGE 250
#define TF_SCORE_CURRENCY_COLLECTED 20
#define TF_SCORE_CAPTURE_POWERUPMODE 10
#define TF_SCORE_FLAG_RETURN 4
#define TF_SCORE_KILL_RUNECARRIER 1

#define TELEPORTER_TYPE_ENTRANCE 0
#define TELEPORTER_TYPE_EXIT 1

#define TF_DEATH_DOMINATION 0x0001
#define TF_DEATH_ASSISTER_DOMINATION 0x0002
#define TF_DEATH_REVENGE 0x0004
#define TF_DEATH_ASSISTER_REVENGE 0x0008
#define TF_DEATH_FIRST_BLOOD 0x0010
#define TF_DEATH_FEIGN_DEATH 0x0020
#define TF_DEATH_INTERRUPTED 0x0040
#define TF_DEATH_GIBBED 0x0080
#define TF_DEATH_PURGATORY 0x0100
#define TF_DEATH_MINIBOSS 0x0200
#define TF_DEATH_AUSTRALIUM 0x0400

#define MAX_DECAPITATIONS 4

#define PLAYER_ROLL_MIN			1
#define PLAYER_ROLL_MAX			500

#define TF_PLAYER_ROCKET_JUMPED ( 1 << 0 )
#define TF_PLAYER_STICKY_JUMPED ( 1 << 1 )
#define TF_PLAYER_ENEMY_BLASTED_ME ( 1 << 2 )

#define MVM_BUYBACK_COST_PER_SEC 5
#define MVM_CLASS_TYPES_PER_WAVE_MAX 12
#define MVM_CLASS_TYPES_PER_WAVE_MAX_NEW ( MVM_CLASS_TYPES_PER_WAVE_MAX * 2 )
#define MVM_CLASS_FLAG_NONE 0
#define MVM_CLASS_FLAG_NORMAL (1<<0)
#define MVM_CLASS_FLAG_SUPPORT (1<<1)
#define MVM_CLASS_FLAG_MISSION (1<<2)
#define MVM_CLASS_FLAG_MINIBOSS (1<<3)
#define MVM_CLASS_FLAG_ALWAYSCRIT (1<<4)
#define MVM_CLASS_FLAG_SUPPORT_LIMITED	(1<<5)

#define RD_MAX_ROBOT_GROUPS_PER_TEAM 6
#define MAX_RAIDMODE_UPGRADES 60

#define MAX_PLAYERS 33

#define STEAM_PARM "-steam"
#define AUTO_RESTART "-autoupdate"
#define INVALID_STEAM_TICKET "Invalid STEAM UserID Ticket\n"
#define INVALID_STEAM_VACBANSTATE "VAC banned from secure server\n"
#define INVALID_STEAM_LOGGED_IN_ELSEWHERE "This Steam account is being used in another location\n"
#define INVALID_STEAM_LOGON_NOT_CONNECTED "Client not connected to Steam\n"
#define INVALID_STEAM_LOGON_TICKET_CANCELED "Client left game (Steam auth ticket has been canceled)\n"
#define CLIENTNAME_TIMED_OUT "%s timed out"

#define DEFAULT_TICK_INTERVAL (0.015)
#define MINIMUM_TICK_INTERVAL (0.001)
#define MAXIMUM_TICK_INTERVAL (0.1)
#define ABSOLUTE_PLAYER_LIMIT 255
#define ABSOLUTE_PLAYER_LIMIT_DW ((ABSOLUTE_PLAYER_LIMIT/32) + 1)
#define MAX_PLAYER_NAME_LENGTH 32
#define MAX_PLAYERS_PER_CLIENT 1
#define MAX_MAP_NAME 96
#define MAX_MAP_NAME_SAVE 32
#define MAX_DISPLAY_MAP_NAME 32
#define	MAX_NETWORKID_LENGTH 64
#define SP_MODEL_INDEX_BITS 13
#define	MAX_EDICT_BITS 11
#define	MAX_EDICTS (1<<MAX_EDICT_BITS)
#define MAX_SERVER_CLASS_BITS 9
#define MAX_SERVER_CLASSES (1<<MAX_SERVER_CLASS_BITS)
#define SIGNED_GUID_LEN 32
#define NUM_ENT_ENTRY_BITS (MAX_EDICT_BITS + 1)
#define NUM_ENT_ENTRIES (1 << NUM_ENT_ENTRY_BITS)
#define ENT_ENTRY_MASK (NUM_ENT_ENTRIES - 1)
#define INVALID_EHANDLE_INDEX 0xFFFFFFFF
#define NUM_SERIAL_NUM_BITS (32 - NUM_ENT_ENTRY_BITS)
#define NUM_NETWORKED_EHANDLE_SERIAL_NUMBER_BITS 10
#define NUM_NETWORKED_EHANDLE_BITS (MAX_EDICT_BITS + NUM_NETWORKED_EHANDLE_SERIAL_NUMBER_BITS)
#define INVALID_NETWORKED_EHANDLE_VALUE ((1 << NUM_NETWORKED_EHANDLE_BITS) - 1)
#define MAX_PACKEDENTITY_DATA (16384)
#define MAX_PACKEDENTITY_PROPS (4096)
#define MAX_CUSTOM_FILES 4
#define MAX_CUSTOM_FILE_SIZE 524288

#define	FL_ONGROUND (1<<0)
#define FL_DUCKING (1<<1)
#define	FL_WATERJUMP (1<<2)
#define FL_ONTRAIN (1<<3)
#define FL_INRAIN (1<<4)
#define FL_FROZEN (1<<5)
#define FL_ATCONTROLS (1<<6)
#define	FL_CLIENT (1<<7)
#define FL_FAKECLIENT (1<<8)
#define	FL_INWATER (1<<9)
#define PLAYER_FLAG_BITS 10

#define	LIFE_ALIVE 0
#define	LIFE_DYING 1
#define	LIFE_DEAD 2
#define LIFE_RESPAWNABLE 3
#define LIFE_DISCARDBODY 4

#define EF_PARITY_BITS 3
#define EF_PARITY_MASK ((1<<EF_PARITY_BITS)-1)
#define EF_MUZZLEFLASH_BITS 2
#define	PLAT_LOW_TRIGGER 1
#define	SF_TRAIN_WAIT_RETRIGGER	1
#define SF_TRAIN_PASSABLE 8
#define FIXANGLE_NONE 0
#define FIXANGLE_ABSOLUTE 1
#define FIXANGLE_RELATIVE 2
#define BREAK_GLASS 0x01
#define BREAK_METAL 0x02
#define BREAK_FLESH 0x04
#define BREAK_WOOD 0x08
#define BREAK_SMOKE 0x10
#define BREAK_TRANS 0x20
#define BREAK_CONCRETE 0x40
#define BREAK_SLAVE 0x80
#define BOUNCE_GLASS BREAK_GLASS
#define	BOUNCE_METAL BREAK_METAL
#define BOUNCE_FLESH BREAK_FLESH
#define BOUNCE_WOOD	 BREAK_WOOD
#define BOUNCE_SHRAP 0x10
#define BOUNCE_SHELL 0x20
#define	BOUNCE_CONCRETE BREAK_CONCRETE
#define BOUNCE_SHOTSHELL 0x80
#define TE_BOUNCE_NULL 0
#define TE_BOUNCE_SHELL 1
#define TE_BOUNCE_SHOTSHELL	2

#define SOUND_NORMAL_CLIP_DIST	1000.0f
#define MAX_AREA_STATE_BYTES 32
#define MAX_AREA_PORTAL_STATE_BYTES 24
#define MAX_USER_MSG_DATA 255
#define MAX_ENTITY_MSG_DATA 255

#define STUDIO_NONE 0x00000000
#define STUDIO_RENDER 0x00000001
#define STUDIO_VIEWXFORMATTACHMENTS 0x00000002
#define STUDIO_DRAWTRANSLUCENTSUBMODELS 0x00000004
#define STUDIO_TWOPASS 0x00000008
#define STUDIO_STATIC_LIGHTING 0x00000010
#define STUDIO_WIREFRAME 0x00000020
#define STUDIO_ITEM_BLINK 0x00000040
#define STUDIO_NOSHADOWS 0x00000080
#define STUDIO_WIREFRAME_VCOLLIDE 0x00000100
#define STUDIO_NO_OVERRIDE_FOR_ATTACH 0x00000200
#define STUDIO_GENERATE_STATS 0x01000000
#define STUDIO_SSAODEPTHTEXTURE 0x08000000
#define STUDIO_SHADOWDEPTHTEXTURE 0x40000000
#define STUDIO_TRANSPARENCY 0x80000000

#define MAX_NEW_COMMANDS 15
#define MAX_BACKUP_COMMANDS	7
#define MAX_COMMANDS MAX_NEW_COMMANDS + MAX_BACKUP_COMMANDS

// Following values should be +16384, -16384, +15/16, -15/16
// NOTE THAT IF THIS GOES ANY BIGGER THEN DISK NODES/LEAVES CANNOT USE SHORTS TO STORE THE BOUNDS
#define MAX_COORD_INTEGER			(16384)
#define MIN_COORD_INTEGER			(-MAX_COORD_INTEGER)
#define MAX_COORD_FRACTION			(1.f - (1.f / 16))
#define MIN_COORD_FRACTION			(-1.f + (1.f / 16))

#define MAX_COORD_FLOAT				(16384.f)
#define MIN_COORD_FLOAT				(-MAX_COORD_FLOAT)

// Width of the coord system, which is TOO BIG to send as a client/server coordinate value
#define COORD_EXTENT				(2*MAX_COORD_INTEGER)

// Maximum traceable distance ( assumes cubic world and trace from one corner to opposite )
// COORD_EXTENT * sqrt(3)
#define MAX_TRACE_LENGTH			( 1.732050807569f * COORD_EXTENT )		

// This value is the LONGEST possible range (limited by max valid coordinate number, not 2x)
#define MAX_COORD_RANGE				(MAX_COORD_INTEGER)

#define ASSERT_COORD( v ) Assert( (v.x>=MIN_COORD_INTEGER*2) && (v.x<=MAX_COORD_INTEGER*2) && \
								  (v.y>=MIN_COORD_INTEGER*2) && (v.y<=MAX_COORD_INTEGER*2) && \
								  (v.z>=MIN_COORD_INTEGER*2) && (v.z<=MAX_COORD_INTEGER*2) ); \


enum MoveType_t
{
	MOVETYPE_NONE = 0,
	MOVETYPE_ISOMETRIC,
	MOVETYPE_WALK,
	MOVETYPE_STEP,
	MOVETYPE_FLY,
	MOVETYPE_FLYGRAVITY,
	MOVETYPE_VPHYSICS,
	MOVETYPE_PUSH,
	MOVETYPE_NOCLIP,
	MOVETYPE_LADDER,
	MOVETYPE_OBSERVER,
	MOVETYPE_CUSTOM,
	MOVETYPE_LAST = MOVETYPE_CUSTOM,
	MOVETYPE_MAX_BITS = 4
};

enum MoveCollide_t
{
	MOVECOLLIDE_DEFAULT = 0,
	MOVECOLLIDE_FLY_BOUNCE,
	MOVECOLLIDE_FLY_CUSTOM,
	MOVECOLLIDE_FLY_SLIDE,
	MOVECOLLIDE_COUNT,
	MOVECOLLIDE_MAX_BITS = 3
};

enum SolidType_t
{
	SOLID_NONE = 0,
	SOLID_BSP = 1,
	SOLID_BBOX = 2,
	SOLID_OBB = 3,
	SOLID_OBB_YAW = 4,
	SOLID_CUSTOM = 5,
	SOLID_VPHYSICS = 6,
	SOLID_LAST
};

enum SolidFlags_t
{
	FSOLID_CUSTOMRAYTEST = 0x0001,
	FSOLID_CUSTOMBOXTEST = 0x0002,
	FSOLID_NOT_SOLID = 0x0004,
	FSOLID_TRIGGER = 0x0008,
	FSOLID_NOT_STANDABLE = 0x0010,
	FSOLID_VOLUME_CONTENTS = 0x0020,
	FSOLID_FORCE_WORLD_ALIGNED = 0x0040,
	FSOLID_USE_TRIGGER_BOUNDS = 0x0080,
	FSOLID_ROOT_PARENT_ALIGNED = 0x0100,
	FSOLID_TRIGGER_TOUCH_DEBRIS = 0x0200,
	FSOLID_MAX_BITS = 10
};

inline bool IsSolid(SolidType_t solidType, int nSolidFlags)
{
	return (solidType != SOLID_NONE) && ((nSolidFlags & FSOLID_NOT_SOLID) == 0);
}

enum
{
	EF_BONEMERGE = 0x001,
	EF_BRIGHTLIGHT = 0x002,
	EF_DIMLIGHT = 0x004,
	EF_NOINTERP = 0x008,
	EF_NOSHADOW = 0x010,
	EF_NODRAW = 0x020,
	EF_NORECEIVESHADOW = 0x040,
	EF_BONEMERGE_FASTCULL = 0x080,
	EF_ITEM_BLINK = 0x100,
	EF_PARENT_ANIMATES = 0x200,
	EF_MAX_BITS = 10
};

enum RenderMode_t
{
	kRenderNormal = 0,
	kRenderTransColor,
	kRenderTransTexture,
	kRenderGlow,
	kRenderTransAlpha,
	kRenderTransAdd,
	kRenderEnvironmental,
	kRenderTransAddFrameBlend,
	kRenderTransAlphaAdd,
	kRenderWorldGlow,
	kRenderNone,
	kRenderModeCount
};

enum RenderFx_t
{
	kRenderFxNone = 0,
	kRenderFxPulseSlow,
	kRenderFxPulseFast,
	kRenderFxPulseSlowWide,
	kRenderFxPulseFastWide,
	kRenderFxFadeSlow,
	kRenderFxFadeFast,
	kRenderFxSolidSlow,
	kRenderFxSolidFast,
	kRenderFxStrobeSlow,
	kRenderFxStrobeFast,
	kRenderFxStrobeFaster,
	kRenderFxFlickerSlow,
	kRenderFxFlickerFast,
	kRenderFxNoDissipation,
	kRenderFxDistort,
	kRenderFxHologram,
	kRenderFxExplode,
	kRenderFxGlowShell,
	kRenderFxClampMinScale,
	kRenderFxEnvRain,
	kRenderFxEnvSnow,
	kRenderFxSpotlight,
	kRenderFxRagdoll,
	kRenderFxPulseFastWider,
	kRenderFxMax
};

enum Collision_Group_t
{
	COLLISION_GROUP_NONE = 0,
	COLLISION_GROUP_DEBRIS,
	COLLISION_GROUP_DEBRIS_TRIGGER,
	COLLISION_GROUP_INTERACTIVE_DEBRIS,
	COLLISION_GROUP_INTERACTIVE,
	COLLISION_GROUP_PLAYER,
	COLLISION_GROUP_BREAKABLE_GLASS,
	COLLISION_GROUP_VEHICLE,
	COLLISION_GROUP_PLAYER_MOVEMENT,
	COLLISION_GROUP_NPC,
	COLLISION_GROUP_IN_VEHICLE,
	COLLISION_GROUP_WEAPON,
	COLLISION_GROUP_VEHICLE_CLIP,
	COLLISION_GROUP_PROJECTILE,
	COLLISION_GROUP_DOOR_BLOCKER,
	COLLISION_GROUP_PASSABLE_DOOR,
	COLLISION_GROUP_DISSOLVING,
	COLLISION_GROUP_PUSHAWAY,
	COLLISION_GROUP_NPC_ACTOR,
	COLLISION_GROUP_NPC_SCRIPTED,
	LAST_SHARED_COLLISION_GROUP
};

class CThreadMutex;
typedef CThreadMutex CSourceMutex;

enum ETFHitboxes
{
	HITBOX_HEAD,
	HITBOX_NECK,
	HITBOX_LOWER_NECK,
	HITBOX_PELVIS,
	HITBOX_BODY,
	HITBOX_THORAX,
	HITBOX_CHEST,
	HITBOX_UPPER_CHEST,
	HITBOX_RIGHT_THIGH,
	HITBOX_LEFT_THIGH,
	HITBOX_RIGHT_CALF,
	HITBOX_LEFT_CALF,
	HITBOX_RIGHT_FOOT,
	HITBOX_LEFT_FOOT,
	HITBOX_RIGHT_HAND,
	HITBOX_LEFT_HAND,
	HITBOX_RIGHT_UPPER_ARM,
	HITBOX_RIGHT_FOREARM,
	HITBOX_LEFT_UPPER_ARM,
	HITBOX_LEFT_FOREARM,
	HITBOX_MAX
};

enum MinigunState_t
{
	AC_STATE_IDLE = 0,
	AC_STATE_STARTFIRING,
	AC_STATE_FIRING,
	AC_STATE_SPINNING,
	AC_STATE_DRYFIRE
};

enum minigun_weapontypes_t
{
	MINIGUN_STANDARD = 0,
	MINIGUN_STUN
};

typedef enum
{
	GROUND = 0,
	STUCK,
	LADDER
} IntervalType_t;

enum
{
	SPEED_CROPPED_RESET = 0,
	SPEED_CROPPED_DUCK = 1,
	SPEED_CROPPED_WEAPON = 2
};

typedef enum
{
	PREDICTION_SIMULATION_RESULTS_ARRIVING_ON_SEND_FRAME = 0,
	PREDICTION_NORMAL
} PREDICTION_REASON;

enum view_id_t
{
	VIEW_ILLEGAL = -2,
	VIEW_NONE = -1,
	VIEW_MAIN = 0,
	VIEW_3DSKY = 1,
	VIEW_MONITOR = 2,
	VIEW_REFLECTION = 3,
	VIEW_REFRACTION = 4,
	VIEW_INTRO_PLAYER = 5,
	VIEW_INTRO_CAMERA = 6,
	VIEW_SHADOW_DEPTH_TEXTURE = 7,
	VIEW_SSAO = 8,
	VIEW_ID_COUNT
};

enum medigun_weapontypes_t
{
	MEDIGUN_STANDARD = 0,
	MEDIGUN_UBER,
	MEDIGUN_QUICKFIX,
	MEDIGUN_RESIST
};

enum medigun_resist_types_t
{
	MEDIGUN_BULLET_RESIST = 0,
	MEDIGUN_BLAST_RESIST,
	MEDIGUN_FIRE_RESIST,
	MEDIGUN_NUM_RESISTS
};

enum PrecipitationType_t
{
	PRECIPITATION_TYPE_RAIN = 0,
	PRECIPITATION_TYPE_SNOW,
	PRECIPITATION_TYPE_ASH,
	PRECIPITATION_TYPE_SNOWFALL,
	NUM_PRECIPITATION_TYPES
};

class CViewVectors
{
public:
	Vector m_vView;
	Vector m_vHullMin;
	Vector m_vHullMax;
	Vector m_vDuckHullMin;
	Vector m_vDuckHullMax;
	Vector m_vDuckView;
	Vector m_vObsHullMin;
	Vector m_vObsHullMax;
	Vector m_vDeadViewHeight;
};

typedef enum
{
	VOTE_FAILED_GENERIC = 0,
	VOTE_FAILED_TRANSITIONING_PLAYERS,
	VOTE_FAILED_RATE_EXCEEDED,
	VOTE_FAILED_YES_MUST_EXCEED_NO,
	VOTE_FAILED_QUORUM_FAILURE,
	VOTE_FAILED_ISSUE_DISABLED,
	VOTE_FAILED_MAP_NOT_FOUND,
	VOTE_FAILED_MAP_NAME_REQUIRED,
	VOTE_FAILED_ON_COOLDOWN,
	VOTE_FAILED_TEAM_CANT_CALL,
	VOTE_FAILED_WAITINGFORPLAYERS,
	VOTE_FAILED_PLAYERNOTFOUND,
	VOTE_FAILED_CANNOT_KICK_ADMIN,
	VOTE_FAILED_SCRAMBLE_IN_PROGRESS,
	VOTE_FAILED_SPECTATOR,
	VOTE_FAILED_NEXTLEVEL_SET,
	VOTE_FAILED_MAP_NOT_VALID,
	VOTE_FAILED_CANNOT_KICK_FOR_TIME,
	VOTE_FAILED_CANNOT_KICK_DURING_ROUND,
	VOTE_FAILED_VOTE_IN_PROGRESS,
	VOTE_FAILED_KICK_LIMIT_REACHED,
	VOTE_FAILED_KICK_DENIED_BY_GC,
	VOTE_FAILED_MODIFICATION_ALREADY_ACTIVE
} vote_create_failed_t;

enum UserMessageType
{
	Geiger = 0,
	Train,
	HudText,
	SayText,
	SayText2,
	TextMsg,
	ResetHUD,
	GameTitle,
	ItemPickup,
	ShowMenu,
	Shake,
	Fade,
	VGUIMenu,
	Rumble,
	CloseCaption,
	SendAudio,
	VoiceMask,
	RequestState,
	Damage,
	HintText,
	KeyHintText,
	HudMsg,
	AmmoDenied,
	AchievementEvent,
	UpdateRadar,
	VoiceSubtitle,
	HudNotify,
	HudNotifyCustom,
	PlayerStatsUpdate,
	MapStatsUpdate,
	PlayerIgnited,
	PlayerIgnitedInv,
	HudArenaNotify,
	UpdateAchievement,
	TrainingMsg,
	TrainingObjective,
	DamageDodged,
	PlayerJarated,
	PlayerExtinguished,
	PlayerJaratedFade,
	PlayerShieldBlocked,
	BreakModel,
	CheapBreakModel,
	BreakModel_Pumpkin,
	BreakModelRocketDud,
	CallVoteFailed,
	VoteStart,
	VotePass,
	VoteFailed,
	VoteSetup,
	PlayerBonusPoints,
	RDTeamPointsChanged,
	SpawnFlyingBird,
	PlayerGodRayEffect,
	PlayerTeleportHomeEffect,
	MVMStatsReset,
	MVMPlayerEvent,
	MVMResetPlayerStats,
	MVMWaveFailed,
	MVMAnnouncement,
	MVMPlayerUpgradedEvent,
	MVMVictory,
	MVMWaveChange,
	MVMLocalPlayerUpgradesClear,
	MVMLocalPlayerUpgradesValue,
	MVMResetPlayerWaveSpendingStats,
	MVMLocalPlayerWaveSpendingValue,
	MVMResetPlayerUpgradeSpending,
	MVMServerKickTimeUpdate,
	PlayerLoadoutUpdated,
	PlayerTauntSoundLoopStart,
	PlayerTauntSoundLoopEnd,
	ForcePlayerViewAngles,
	BonusDucks,
	EOTLDuckEvent,
	PlayerPickupWeapon,
	QuestObjectiveCompleted,
	SPHapWeapEvent,
	HapDmg,
	HapPunch,
	HapSetDrag,
	HapSetConst,
	HapMeleeContact
};

enum
{
	SERVER_MODIFICATION_ITEM_DURATION_IN_MINUTES = 120
};

enum CastVote
{
	VOTE_OPTION1,
	VOTE_OPTION2,
	VOTE_OPTION3,
	VOTE_OPTION4,
	VOTE_OPTION5,
	VOTE_UNCAST
};

enum
{
	ENTITY_DISSOLVE_NORMAL = 0,
	ENTITY_DISSOLVE_ELECTRICAL,
	ENTITY_DISSOLVE_ELECTRICAL_LIGHT,
	ENTITY_DISSOLVE_CORE,
	ENTITY_DISSOLVE_BITS = 3
};

enum EObserverModes
{
	OBS_MODE_NONE = 0,		// not in spectator mode
	OBS_MODE_DEATHCAM,		// special mode for death cam animation
	OBS_MODE_FREEZECAM,		// zooms to a target, and freeze-frames on them
	OBS_MODE_FIXED,			// view from a fixed camera position
	OBS_MODE_FIRSTPERSON,	// follow a player in first person view
	OBS_MODE_THIRDPERSON,	// follow a player in third person view
	OBS_MODE_ROAMING,		// free roaming
};

enum
{
	OBS_ALLOW_ALL = 0,
	OBS_ALLOW_TEAM,
	OBS_ALLOW_NONE,
	OBS_ALLOW_NUM_MODES
};

enum
{
	TYPE_TEXT = 0,
	TYPE_INDEX,
	TYPE_URL,
	TYPE_FILE
};

enum
{
	EFL_KILLME = (1 << 0),
	EFL_DORMANT = (1 << 1),
	EFL_NOCLIP_ACTIVE = (1 << 2),
	EFL_SETTING_UP_BONES = (1 << 3),
	EFL_KEEP_ON_RECREATE_ENTITIES = (1 << 4),
	EFL_HAS_PLAYER_CHILD = (1 << 4),
	EFL_DIRTY_SHADOWUPDATE = (1 << 5),
	EFL_NOTIFY = (1 << 6),
	EFL_FORCE_CHECK_TRANSMIT = (1 << 7),
	EFL_BOT_FROZEN = (1 << 8),
	EFL_SERVER_ONLY = (1 << 9),
	EFL_NO_AUTO_EDICT_ATTACH = (1 << 10),
	EFL_DIRTY_ABSTRANSFORM = (1 << 11),
	EFL_DIRTY_ABSVELOCITY = (1 << 12),
	EFL_DIRTY_ABSANGVELOCITY = (1 << 13),
	EFL_DIRTY_SURROUNDING_COLLISION_BOUNDS = (1 << 14),
	EFL_DIRTY_SPATIAL_PARTITION = (1 << 15),
	EFL_IN_SKYBOX = (1 << 17),
	EFL_USE_PARTITION_WHEN_NOT_SOLID = (1 << 18),
	EFL_TOUCHING_FLUID = (1 << 19),
	EFL_IS_BEING_LIFTED_BY_BARNACLE = (1 << 20),
	EFL_NO_ROTORWASH_PUSH = (1 << 21),
	EFL_NO_THINK_FUNCTION = (1 << 22),
	EFL_NO_GAME_PHYSICS_SIMULATION = (1 << 23),
	EFL_CHECK_UNTOUCH = (1 << 24),
	EFL_DONTBLOCKLOS = (1 << 25),
	EFL_DONTWALKON = (1 << 26),
	EFL_NO_DISSOLVE = (1 << 27),
	EFL_NO_MEGAPHYSCANNON_RAGDOLL = (1 << 28),
	EFL_NO_WATER_VELOCITY_CHANGE = (1 << 29),
	EFL_NO_PHYSCANNON_INTERACTION = (1 << 30),
	EFL_NO_DAMAGE_FORCES = (1 << 31)
};

class CBaseEntity;

struct FireBulletsInfo_t
{
	int m_iShots;
	Vector m_vecSrc;
	Vector m_vecDirShooting;
	Vector m_vecSpread;
	float m_flDistance;
	int m_iAmmoType;
	int m_iTracerFreq;
	float m_flDamage;
	int m_iPlayerDamage;
	int m_nFlags;
	float m_flDamageForceScale;
	CBaseEntity* m_pAttacker;
	CBaseEntity* m_pAdditionalIgnoreEnt;
	bool m_bPrimaryAttack;
	bool m_bUseServerRandomSeed;
};

enum
{
	VISION_MODE_NONE = 0,
	VISION_MODE_PYRO,
	VISION_MODE_HALLOWEEN,
	VISION_MODE_ROME,
	MAX_VISION_MODES
};

class CHudTexture
{
public:
	void* vftp;
	char szShortName[64];
	char szTextureFile[64];
	bool bRenderUsingFont;
	bool bPrecached;
	char cCharacterInFont;
	unsigned long hFont;
	int textureId;
	float texCoords[4];
	wrect_t rc;

public:
	int Width() const
	{
		return rc.right - rc.left;
	}
	int Height() const
	{
		return rc.bottom - rc.top;
	}
};

enum class ETFClassID
{
	CTFWearableRazorback = 341,
	CTFWearableDemoShield = 338,
	CTFWearableLevelableItem = 340,
	CTFWearableCampaignItem = 337,
	CTFBaseRocket = 185,
	CTFWeaponBaseMerasmusGrenade = 325,
	CTFWeaponBaseMelee = 324,
	CTFWeaponBaseGun = 323,
	CTFWeaponBaseGrenadeProj = 322,
	CTFWeaponBase = 321,
	CTFWearableRobotArm = 342,
	CTFRobotArm = 287,
	CTFWrench = 344,
	CTFProjectile_ThrowableBreadMonster = 279,
	CTFProjectile_ThrowableBrick = 280,
	CTFProjectile_ThrowableRepel = 281,
	CTFProjectile_Throwable = 278,
	CTFThrowable = 319,
	CTFSyringeGun = 315,
	CTFKatana = 225,
	CTFSword = 314,
	CSniperDot = 118,
	CTFSniperRifleClassic = 308,
	CTFSniperRifleDecap = 309,
	CTFSniperRifle = 307,
	CTFChargedSMG = 197,
	CTFSMG = 306,
	CTFSlap = 305,
	CTFShovel = 304,
	CTFShotgunBuildingRescue = 303,
	CTFPEPBrawlerBlaster = 241,
	CTFSodaPopper = 310,
	CTFShotgun_Revenge = 301,
	CTFScatterGun = 297,
	CTFShotgun_Pyro = 300,
	CTFShotgun_HWG = 299,
	CTFShotgun_Soldier = 302,
	CTFShotgun = 298,
	CTFRocketPack = 296,
	CTFCrossbow = 201,
	CTFRocketLauncher_Mortar = 295,
	CTFRocketLauncher_AirStrike = 293,
	CTFRocketLauncher_DirectHit = 294,
	CTFRocketLauncher = 292,
	CTFRevolver = 286,
	CTFDRGPomson = 202,
	CTFRaygun = 284,
	CTFPistol_ScoutSecondary = 246,
	CTFPistol_ScoutPrimary = 245,
	CTFPistol_Scout = 244,
	CTFPistol = 243,
	CTFPipebombLauncher = 242,
	CTFWeaponPDA_Spy = 332,
	CTFWeaponPDA_Engineer_Destroy = 331,
	CTFWeaponPDA_Engineer_Build = 330,
	CTFWeaponPDAExpansion_Teleporter = 334,
	CTFWeaponPDAExpansion_Dispenser = 333,
	CTFWeaponPDA = 329,
	CTFParticleCannon = 239,
	CTFParachute_Secondary = 238,
	CTFParachute_Primary = 237,
	CTFParachute = 236,
	CTFMinigun = 234,
	CTFMedigunShield = 231,
	CWeaponMedigun = 352,
	CTFProjectile_MechanicalArmOrb = 263,
	CTFMechanicalArm = 230,
	CTFLunchBox_Drink = 229,
	CTFLunchBox = 228,
	CLaserDot = 78,
	CTFLaserPointer = 227,
	CTFKnife = 226,
	CTFGasManager = 212,
	CTFProjectile_JarGas = 261,
	CTFJarGas = 223,
	CTFProjectile_Cleaver = 254,
	CTFProjectile_JarMilk = 262,
	CTFProjectile_Jar = 260,
	CTFCleaver = 198,
	CTFJarMilk = 224,
	CTFJar = 222,
	CTFWeaponInvis = 328,
	CTFCannon = 196,
	CTFGrenadeLauncher = 216,
	CTFGrenadePipebombProjectile = 217,
	CTFGrapplingHook = 215,
	CTFFlareGun_Revenge = 210,
	CTFFlareGun = 209,
	CTFFlameRocket = 207,
	CTFFlameThrower = 208,
	CTFFists = 205,
	CTFFireAxe = 204,
	CTFWeaponFlameBall = 327,
	CTFCompoundBow = 200,
	CTFClub = 199,
	CTFBuffItem = 195,
	CTFStickBomb = 312,
	CTFBreakableSign = 194,
	CTFBottle = 192,
	CTFBreakableMelee = 193,
	CTFBonesaw = 190,
	CTFBall_Ornament = 182,
	CTFStunBall = 313,
	CTFBat_Giftwrap = 188,
	CTFBat_Wood = 189,
	CTFBat_Fish = 187,
	CTFBat = 186,
	CTFProjectile_EnergyRing = 256,
	CTFDroppedWeapon = 203,
	CTFWeaponSapper = 335,
	CTFWeaponBuilder = 326,
	C_TFWeaponBuilder = 0,
	CTFProjectile_Rocket = 264,
	CTFProjectile_Flare = 257,
	CTFProjectile_EnergyBall = 255,
	CTFProjectile_GrapplingHook = 258,
	CTFProjectile_HealingBolt = 259,
	CTFProjectile_Arrow = 252,
	CMannVsMachineStats = 80,
	CTFTankBoss = 316,
	CTFBaseBoss = 183,
	CBossAlpha = 0,
	NextBotCombatCharacter = 357,
	CTFProjectile_SpellKartBats = 268,
	CTFProjectile_SpellKartOrb = 269,
	CTFHellZap = 220,
	CTFProjectile_SpellLightningOrb = 270,
	CTFProjectile_SpellTransposeTeleport = 277,
	CTFProjectile_SpellMeteorShower = 271,
	CTFProjectile_SpellSpawnBoss = 274,
	CTFProjectile_SpellMirv = 272,
	CTFProjectile_SpellPumpkin = 273,
	CTFProjectile_SpellSpawnHorde = 275,
	CTFProjectile_SpellSpawnZombie = 276,
	CTFProjectile_SpellBats = 266,
	CTFProjectile_SpellFireball = 267,
	CTFSpellBook = 311,
	CHightower_TeleportVortex = 74,
	CTeleportVortex = 160,
	CZombie = 354,
	CMerasmusDancer = 83,
	CMerasmus = 82,
	CHeadlessHatman = 73,
	CEyeballBoss = 48,
	CTFBotHintEngineerNest = 191,
	CBotNPCMinion = 0,
	CBotNPC = 0,
	CPasstimeGun = 94,
	CTFViewModel = 320,
	CRobotDispenser = 112,
	CTFRobotDestruction_Robot = 288,
	CTFReviveMarker = 285,
	CTFPumpkinBomb = 282,
	CTFProjectile_BallOfFire = 253,
	CTFBaseProjectile = 184,
	CTFPointManager = 250,
	CBaseObjectUpgrade = 11,
	CTFRobotDestructionLogic = 291,
	CTFRobotDestruction_RobotGroup = 289,
	CTFRobotDestruction_RobotSpawn = 290,
	CTFPlayerDestructionLogic = 248,
	CPlayerDestructionDispenser = 101,
	CTFMinigameLogic = 233,
	CTFHalloweenMinigame_FallingPlatforms = 219,
	CTFHalloweenMinigame = 218,
	CTFMiniGame = 232,
	CTFPowerupBottle = 251,
	CTFItem = 221,
	CHalloweenSoulPack = 71,
	CTFGenericBomb = 213,
	CBonusRoundLogic = 23,
	CTFGameRulesProxy = 211,
	CTETFParticleEffect = 179,
	CTETFExplosion = 178,
	CTETFBlood = 177,
	CTFFlameManager = 206,
	CHalloweenGiftPickup = 69,
	CBonusDuckPickup = 21,
	CHalloweenPickup = 70,
	CCaptureFlagReturnIcon = 27,
	CCaptureFlag = 26,
	CBonusPack = 22,
	CTFTeam = 318,
	CTFTauntProp = 317,
	CTFPlayerResource = 249,
	CTFPlayer = 247,
	CTFRagdoll = 283,
	CTEPlayerAnimEvent = 165,
	CTFPasstimeLogic = 240,
	CPasstimeBall = 93,
	CTFObjectiveResource = 235,
	CTFGlow = 214,
	CTEFireBullets = 152,
	CTFBuffBanner = 0,
	CTFAmmoPack = 181,
	CObjectTeleporter = 89,
	CObjectSentrygun = 88,
	CTFProjectile_SentryRocket = 265,
	CObjectSapper = 87,
	CObjectCartDispenser = 85,
	CObjectDispenser = 86,
	CMonsterResource = 84,
	CFuncRespawnRoomVisualizer = 64,
	CFuncRespawnRoom = 63,
	CFuncPasstimeGoal = 61,
	CFuncForceField = 57,
	CCaptureZone = 28,
	CCurrencyPack = 31,
	CBaseObject = 10,
	CTestTraceline = 176,
	CTEWorldDecal = 180,
	CTESpriteSpray = 174,
	CTESprite = 173,
	CTESparks = 172,
	CTESmoke = 171,
	CTEShowLine = 169,
	CTEProjectedDecal = 167,
	CTEPlayerDecal = 166,
	CTEPhysicsProp = 164,
	CTEParticleSystem = 163,
	CTEMuzzleFlash = 162,
	CTELargeFunnel = 159,
	CTEKillPlayerAttachments = 158,
	CTEImpact = 157,
	CTEGlowSprite = 156,
	CTEShatterSurface = 168,
	CTEFootprintDecal = 154,
	CTEFizz = 153,
	CTEExplosion = 151,
	CTEEnergySplash = 150,
	CTEEffectDispatch = 149,
	CTEDynamicLight = 148,
	CTEDecal = 146,
	CTEClientProjectile = 145,
	CTEBubbleTrail = 144,
	CTEBubbles = 143,
	CTEBSPDecal = 142,
	CTEBreakModel = 141,
	CTEBloodStream = 140,
	CTEBloodSprite = 139,
	CTEBeamSpline = 138,
	CTEBeamRingPoint = 137,
	CTEBeamRing = 136,
	CTEBeamPoints = 135,
	CTEBeamLaser = 134,
	CTEBeamFollow = 133,
	CTEBeamEnts = 132,
	CTEBeamEntPoint = 131,
	CTEBaseBeam = 130,
	CTEArmorRicochet = 129,
	CTEMetalSparks = 161,
	CSteamJet = 123,
	CSmokeStack = 117,
	DustTrail = 355,
	CFireTrail = 50,
	SporeTrail = 362,
	SporeExplosion = 361,
	RocketTrail = 359,
	SmokeTrail = 360,
	CPropVehicleDriveable = 108,
	ParticleSmokeGrenade = 358,
	CParticleFire = 90,
	MovieExplosion = 356,
	CTEGaussExplosion = 155,
	CEnvQuadraticBeam = 43,
	CEmbers = 36,
	CEnvWind = 47,
	CPrecipitation = 107,
	CBaseTempEntity = 17,
	CWeaponIFMSteadyCam = 351,
	CWeaponIFMBaseCamera = 350,
	CWeaponIFMBase = 349,
	CTFWearableVM = 343,
	CTFWearable = 336,
	CTFWearableItem = 339,
	CEconWearable = 35,
	CBaseAttributableItem = 3,
	CEconEntity = 34,
	CHandleTest = 72,
	CTeamplayRoundBasedRulesProxy = 126,
	CTeamRoundTimer = 127,
	CSpriteTrail = 122,
	CSpriteOriented = 121,
	CSprite = 120,
	CRagdollPropAttached = 111,
	CRagdollProp = 110,
	CPoseController = 106,
	CGameRulesProxy = 68,
	CInfoLadderDismount = 75,
	CFuncLadder = 58,
	CEnvDetailController = 40,
	CWorld = 353,
	CWaterLODControl = 348,
	CWaterBullet = 347,
	CVoteController = 346,
	CVGuiScreen = 345,
	CPropJeep = 0,
	CPropVehicleChoreoGeneric = 0,
	CTest_ProxyToggle_Networkable = 175,
	CTesla = 170,
	CTeamTrainWatcher = 128,
	CBaseTeamObjectiveResource = 16,
	CTeam = 125,
	CSun = 124,
	CParticlePerformanceMonitor = 91,
	CSpotlightEnd = 119,
	CSlideshowDisplay = 116,
	CShadowControl = 115,
	CSceneEntity = 114,
	CRopeKeyframe = 113,
	CRagdollManager = 109,
	CPhysicsPropMultiplayer = 98,
	CPhysBoxMultiplayer = 96,
	CBasePropDoor = 15,
	CDynamicProp = 33,
	CPointWorldText = 105,
	CPointCommentaryNode = 104,
	CPointCamera = 103,
	CPlayerResource = 102,
	CPlasma = 100,
	CPhysMagnet = 99,
	CPhysicsProp = 97,
	CPhysBox = 95,
	CParticleSystem = 92,
	CMaterialModifyControl = 81,
	CLightGlow = 79,
	CInfoOverlayAccessor = 77,
	CFuncTrackTrain = 67,
	CFuncSmokeVolume = 66,
	CFuncRotating = 65,
	CFuncReflectiveGlass = 62,
	CFuncOccluder = 60,
	CFuncMonitor = 59,
	CFunc_LOD = 54,
	CTEDust = 147,
	CFunc_Dust = 53,
	CFuncConveyor = 56,
	CBreakableSurface = 25,
	CFuncAreaPortalWindow = 55,
	CFish = 51,
	CEntityFlame = 38,
	CFireSmoke = 49,
	CEnvTonemapController = 46,
	CEnvScreenEffect = 44,
	CEnvScreenOverlay = 45,
	CEnvProjectedTexture = 42,
	CEnvParticleScript = 41,
	CFogController = 52,
	CEntityParticleTrail = 39,
	CEntityDissolve = 37,
	CDynamicLight = 32,
	CColorCorrectionVolume = 30,
	CColorCorrection = 29,
	CBreakableProp = 24,
	CBasePlayer = 13,
	CBaseFlex = 8,
	CBaseEntity = 7,
	CBaseDoor = 6,
	CBaseCombatCharacter = 4,
	CBaseAnimatingOverlay = 2,
	CBoneFollower = 20,
	CBaseAnimating = 1,
	CInfoLightingRelative = 76,
	CAI_BaseNPC = 0,
	CBeam = 19,
	CBaseViewModel = 18,
	CBaseProjectile = 14,
	CBaseParticleEntity = 12,
	CBaseGrenade = 9,
	CBaseCombatWeapon = 5
};

enum
{
	TF_STATE_ACTIVE = 0,
	TF_STATE_WELCOME,
	TF_STATE_OBSERVER,
	TF_STATE_DYING,
	TF_STATE_COUNT
};

enum
{
	TF_CLASS_UNDEFINED = 0,
	TF_CLASS_SCOUT,
	TF_CLASS_SNIPER,
	TF_CLASS_SOLDIER,
	TF_CLASS_DEMOMAN,
	TF_CLASS_MEDIC,
	TF_CLASS_HEAVY,
	TF_CLASS_PYRO,
	TF_CLASS_SPY,
	TF_CLASS_ENGINEER,
	TF_CLASS_CIVILIAN,
	TF_CLASS_COUNT_ALL,
	TF_CLASS_RANDOM
};

enum
{
	TF_TEAM_RED = LAST_SHARED_TEAM + 1,
	TF_TEAM_BLUE,
	TF_TEAM_COUNT
};

enum
{
	TEAM_ROLE_NONE = 0,
	TEAM_ROLE_DEFENDERS,
	TEAM_ROLE_ATTACKERS,
	NUM_TEAM_ROLES
};

enum ETFWeaponType
{
	TF_WEAPON_NONE,
	TF_WEAPON_BAT,
	TF_WEAPON_BAT_WOOD,
	TF_WEAPON_BOTTLE,
	TF_WEAPON_FIREAXE,
	TF_WEAPON_CLUB,
	TF_WEAPON_CROWBAR,
	TF_WEAPON_KNIFE,
	TF_WEAPON_FISTS,
	TF_WEAPON_SHOVEL,
	TF_WEAPON_WRENCH,
	TF_WEAPON_BONESAW,
	TF_WEAPON_SHOTGUN_PRIMARY,
	TF_WEAPON_SHOTGUN_SOLDIER,
	TF_WEAPON_SHOTGUN_HWG,
	TF_WEAPON_SHOTGUN_PYRO,
	TF_WEAPON_SCATTERGUN,
	TF_WEAPON_SNIPERRIFLE,
	TF_WEAPON_MINIGUN,
	TF_WEAPON_SMG,
	TF_WEAPON_SYRINGEGUN_MEDIC,
	TF_WEAPON_TRANQ,
	TF_WEAPON_ROCKETLAUNCHER,
	TF_WEAPON_GRENADELAUNCHER,
	TF_WEAPON_PIPEBOMBLAUNCHER,
	TF_WEAPON_FLAMETHROWER,
	TF_WEAPON_GRENADE_NORMAL,
	TF_WEAPON_GRENADE_CONCUSSION,
	TF_WEAPON_GRENADE_NAIL,
	TF_WEAPON_GRENADE_MIRV,
	TF_WEAPON_GRENADE_MIRV_DEMOMAN,
	TF_WEAPON_GRENADE_NAPALM,
	TF_WEAPON_GRENADE_GAS,
	TF_WEAPON_GRENADE_EMP,
	TF_WEAPON_GRENADE_CALTROP,
	TF_WEAPON_GRENADE_PIPEBOMB,
	TF_WEAPON_GRENADE_SMOKE_BOMB,
	TF_WEAPON_GRENADE_HEAL,
	TF_WEAPON_GRENADE_STUNBALL,
	TF_WEAPON_GRENADE_JAR,
	TF_WEAPON_GRENADE_JAR_MILK,
	TF_WEAPON_PISTOL,
	TF_WEAPON_PISTOL_SCOUT,
	TF_WEAPON_REVOLVER,
	TF_WEAPON_NAILGUN,
	TF_WEAPON_PDA,
	TF_WEAPON_PDA_ENGINEER_BUILD,
	TF_WEAPON_PDA_ENGINEER_DESTROY,
	TF_WEAPON_PDA_SPY,
	TF_WEAPON_BUILDER,
	TF_WEAPON_MEDIGUN,
	TF_WEAPON_GRENADE_MIRVBOMB,
	TF_WEAPON_FLAMETHROWER_ROCKET,
	TF_WEAPON_GRENADE_DEMOMAN,
	TF_WEAPON_SENTRY_BULLET,
	TF_WEAPON_SENTRY_ROCKET,
	TF_WEAPON_DISPENSER,
	TF_WEAPON_INVIS,
	TF_WEAPON_FLAREGUN,
	TF_WEAPON_LUNCHBOX,
	TF_WEAPON_JAR,
	TF_WEAPON_COMPOUND_BOW,
	TF_WEAPON_BUFF_ITEM,
	TF_WEAPON_PUMPKIN_BOMB,
	TF_WEAPON_SWORD,
	TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT,
	TF_WEAPON_LIFELINE,
	TF_WEAPON_LASER_POINTER,
	TF_WEAPON_DISPENSER_GUN,
	TF_WEAPON_SENTRY_REVENGE,
	TF_WEAPON_JAR_MILK,
	TF_WEAPON_HANDGUN_SCOUT_PRIMARY,
	TF_WEAPON_BAT_FISH,
	TF_WEAPON_CROSSBOW,
	TF_WEAPON_STICKBOMB,
	TF_WEAPON_HANDGUN_SCOUT_SECONDARY,
	TF_WEAPON_SODA_POPPER,
	TF_WEAPON_SNIPERRIFLE_DECAP,
	TF_WEAPON_RAYGUN,
	TF_WEAPON_PARTICLE_CANNON,
	TF_WEAPON_MECHANICAL_ARM,
	TF_WEAPON_DRG_POMSON,
	TF_WEAPON_BAT_GIFTWRAP,
	TF_WEAPON_GRENADE_ORNAMENT_BALL,
	TF_WEAPON_FLAREGUN_REVENGE,
	TF_WEAPON_PEP_BRAWLER_BLASTER,
	TF_WEAPON_CLEAVER,
	TF_WEAPON_GRENADE_CLEAVER,
	TF_WEAPON_STICKY_BALL_LAUNCHER,
	TF_WEAPON_GRENADE_STICKY_BALL,
	TF_WEAPON_SHOTGUN_BUILDING_RESCUE,
	TF_WEAPON_CANNON,
	TF_WEAPON_THROWABLE,
	TF_WEAPON_GRENADE_THROWABLE,
	TF_WEAPON_PDA_SPY_BUILD,
	TF_WEAPON_GRENADE_WATERBALLOON,
	TF_WEAPON_HARVESTER_SAW,
	TF_WEAPON_SPELLBOOK,
	TF_WEAPON_SPELLBOOK_PROJECTILE,
	TF_WEAPON_SNIPERRIFLE_CLASSIC,
	TF_WEAPON_PARACHUTE,
	TF_WEAPON_GRAPPLINGHOOK,
	TF_WEAPON_PASSTIME_GUN,
	TF_WEAPON_CHARGED_SMG,
	TF_WEAPON_BREAKABLE_SIGN,
	TF_WEAPON_ROCKETPACK,
	TF_WEAPON_SLAP,
	TF_WEAPON_JAR_GAS,
	TF_WEAPON_GRENADE_JAR_GAS,
	TF_WEAPON_FLAME_BALL
};

enum ETFWeapons
{
	Scout_m_Scattergun = 13,
	Scout_m_ScattergunR = 200,
	Scout_m_ForceANature = 45,
	Scout_m_TheShortstop = 220,
	Scout_m_TheSodaPopper = 448,
	Scout_m_FestiveScattergun = 669,
	Scout_m_BabyFacesBlaster = 772,
	Scout_m_SilverBotkillerScattergunMkI = 799,
	Scout_m_GoldBotkillerScattergunMkI = 808,
	Scout_m_RustBotkillerScattergunMkI = 888,
	Scout_m_BloodBotkillerScattergunMkI = 897,
	Scout_m_CarbonadoBotkillerScattergunMkI = 906,
	Scout_m_DiamondBotkillerScattergunMkI = 915,
	Scout_m_SilverBotkillerScattergunMkII = 964,
	Scout_m_GoldBotkillerScattergunMkII = 973,
	Scout_m_FestiveForceANature = 1078,
	Scout_m_TheBackScatter = 1103,
	Scout_m_NightTerror = 15002,
	Scout_m_TartanTorpedo = 15015,
	Scout_m_CountryCrusher = 15021,
	Scout_m_BackcountryBlaster = 15029,
	Scout_m_SpruceDeuce = 15036,
	Scout_m_CurrentEvent = 15053,
	Scout_m_MacabreWeb = 15065,
	Scout_m_Nutcracker = 15069,
	Scout_m_BlueMew = 15106,
	Scout_m_FlowerPower = 15107,
	Scout_m_ShottoHell = 15108,
	Scout_m_CoffinNail = 15131,
	Scout_m_KillerBee = 15151,
	Scout_m_Corsair = 15157,
	Scout_s_ScoutsPistol = 23,
	Scout_s_PistolR = 209,
	Scout_s_BonkAtomicPunch = 46,
	Scout_s_VintageLugermorph = 160,
	Scout_s_CritaCola = 163,
	Scout_s_MadMilk = 222,
	Scout_s_Lugermorph = 294,
	Scout_s_TheWinger = 449,
	Scout_s_PrettyBoysPocketPistol = 773,
	Scout_s_TheFlyingGuillotine = 812,
	Scout_s_TheFlyingGuillotineG = 833,
	Scout_s_MutatedMilk = 1121,
	Scout_s_FestiveBonk = 1145,
	Scout_s_RedRockRoscoe = 15013,
	Scout_s_HomemadeHeater = 15018,
	Scout_s_HickoryHolepuncher = 15035,
	Scout_s_LocalHero = 15041,
	Scout_s_BlackDahlia = 15046,
	Scout_s_SandstoneSpecial = 15056,
	Scout_s_MacabreWeb = 15060,
	Scout_s_Nutcracker = 15061,
	Scout_s_BlueMew = 15100,
	Scout_s_BrainCandy = 15101,
	Scout_s_ShottoHell = 15102,
	Scout_s_DressedToKill = 15126,
	Scout_s_Blitzkrieg = 15148,
	Scout_s_TheCAPPER = 30666,
	Scout_t_Bat = 0,
	Scout_t_BatR = 190,
	Scout_t_TheSandman = 44,
	Scout_t_TheHolyMackerel = 221,
	Scout_t_TheCandyCane = 317,
	Scout_t_TheBostonBasher = 325,
	Scout_t_SunonaStick = 349,
	Scout_t_TheFanOWar = 355,
	Scout_t_TheAtomizer = 450,
	Scout_t_ThreeRuneBlade = 452,
	Scout_t_TheConscientiousObjector = 474,
	Scout_t_UnarmedCombat = 572,
	Scout_t_TheWrapAssassin = 648,
	Scout_t_FestiveBat = 660,
	Scout_t_TheFreedomStaff = 880,
	Scout_t_TheBatOuttaHell = 939,
	Scout_t_TheMemoryMaker = 954,
	Scout_t_FestiveHolyMackerel = 999,
	Scout_t_TheHamShank = 1013,
	Scout_t_TheNecroSmasher = 1123,
	Scout_t_TheCrossingGuard = 1127,
	Scout_t_Batsaber = 30667,
	Scout_t_PrinnyMachete = 30758,
	Soldier_m_RocketLauncher = 18,
	Soldier_m_RocketLauncherR = 205,
	Soldier_m_TheDirectHit = 127,
	Soldier_m_TheBlackBox = 228,
	Soldier_m_RocketJumper = 237,
	Soldier_m_TheLibertyLauncher = 414,
	Soldier_m_TheCowMangler5000 = 441,
	Soldier_m_TheOriginal = 513,
	Soldier_m_FestiveRocketLauncher = 658,
	Soldier_m_TheBeggarsBazooka = 730,
	Soldier_m_SilverBotkillerRocketLauncherMkI = 800,
	Soldier_m_GoldBotkillerRocketLauncherMkI = 809,
	Soldier_m_RustBotkillerRocketLauncherMkI = 889,
	Soldier_m_BloodBotkillerRocketLauncherMkI = 898,
	Soldier_m_CarbonadoBotkillerRocketLauncherMkI = 907,
	Soldier_m_DiamondBotkillerRocketLauncherMkI = 916,
	Soldier_m_SilverBotkillerRocketLauncherMkII = 965,
	Soldier_m_GoldBotkillerRocketLauncherMkII = 974,
	Soldier_m_FestiveBlackBox = 1085,
	Soldier_m_TheAirStrike = 1104,
	Soldier_m_WoodlandWarrior = 15006,
	Soldier_m_SandCannon = 15014,
	Soldier_m_AmericanPastoral = 15028,
	Soldier_m_SmalltownBringdown = 15043,
	Soldier_m_ShellShocker = 15052,
	Soldier_m_AquaMarine = 15057,
	Soldier_m_Autumn = 15081,
	Soldier_m_BlueMew = 15104,
	Soldier_m_BrainCandy = 15105,
	Soldier_m_CoffinNail = 15129,
	Soldier_m_HighRollers = 15130,
	Soldier_m_Warhawk = 15150,
	Soldier_s_SoldiersShotgun = 10,
	Soldier_s_ShotgunR = 199,
	Soldier_s_TheBuffBanner = 129,
	Soldier_s_Gunboats = 133,
	Soldier_s_TheBattalionsBackup = 226,
	Soldier_s_TheConcheror = 354,
	Soldier_s_TheReserveShooter = 415,
	Soldier_s_TheRighteousBison = 442,
	Soldier_s_TheMantreads = 444,
	Soldier_s_FestiveBuffBanner = 1001,
	Soldier_s_TheBASEJumper = 1101,
	Soldier_s_FestiveShotgun = 1141,
	Soldier_s_PanicAttack = 1153,
	Soldier_s_BackwoodsBoomstick = 15003,
	Soldier_s_RusticRuiner = 15016,
	Soldier_s_CivicDuty = 15044,
	Soldier_s_LightningRod = 15047,
	Soldier_s_Autumn = 15085,
	Soldier_s_FlowerPower = 15109,
	Soldier_s_CoffinNail = 15132,
	Soldier_s_DressedtoKill = 15133,
	Soldier_s_RedBear = 15152,
	Soldier_t_Shovel = 6,
	Soldier_t_ShovelR = 196,
	Soldier_t_TheEqualizer = 128,
	Soldier_t_ThePainTrain = 154,
	Soldier_t_TheHalfZatoichi = 357,
	Soldier_t_TheMarketGardener = 416,
	Soldier_t_TheDisciplinaryAction = 447,
	Soldier_t_TheConscientiousObjector = 474,
	Soldier_t_TheEscapePlan = 775,
	Soldier_t_TheFreedomStaff = 880,
	Soldier_t_TheBatOuttaHell = 939,
	Soldier_t_TheMemoryMaker = 954,
	Soldier_t_TheHamShank = 1013,
	Soldier_t_TheNecroSmasher = 1123,
	Soldier_t_TheCrossingGuard = 1127,
	Soldier_t_PrinnyMachete = 30758,
	Pyro_m_FlameThrower = 21,
	Pyro_m_FlameThrowerR = 208,
	Pyro_m_TheBackburner = 40,
	Pyro_m_TheDegreaser = 215,
	Pyro_m_ThePhlogistinator = 594,
	Pyro_m_FestiveFlameThrower = 659,
	Pyro_m_TheRainblower = 741,
	Pyro_m_SilverBotkillerFlameThrowerMkI = 798,
	Pyro_m_GoldBotkillerFlameThrowerMkI = 807,
	Pyro_m_RustBotkillerFlameThrowerMkI = 887,
	Pyro_m_BloodBotkillerFlameThrowerMkI = 896,
	Pyro_m_CarbonadoBotkillerFlameThrowerMkI = 905,
	Pyro_m_DiamondBotkillerFlameThrowerMkI = 914,
	Pyro_m_SilverBotkillerFlameThrowerMkII = 963,
	Pyro_m_GoldBotkillerFlameThrowerMkII = 972,
	Pyro_m_FestiveBackburner = 1146,
	Pyro_m_DragonsFury = 1178,
	Pyro_m_ForestFire = 15005,
	Pyro_m_BarnBurner = 15017,
	Pyro_m_BovineBlazemaker = 15030,
	Pyro_m_EarthSkyandFire = 15034,
	Pyro_m_FlashFryer = 15049,
	Pyro_m_TurbineTorcher = 15054,
	Pyro_m_Autumn = 15066,
	Pyro_m_PumpkinPatch = 15067,
	Pyro_m_Nutcracker = 15068,
	Pyro_m_Balloonicorn = 15089,
	Pyro_m_Rainbow = 15090,
	Pyro_m_CoffinNail = 15115,
	Pyro_m_Warhawk = 15141,
	Pyro_m_NostromoNapalmer = 30474,
	Pyro_s_PyrosShotgun = 12,
	Pyro_s_ShotgunR = 199,
	Pyro_s_TheFlareGun = 39,
	Pyro_s_TheDetonator = 351,
	Pyro_s_TheReserveShooter = 415,
	Pyro_s_TheManmelter = 595,
	Pyro_s_TheScorchShot = 740,
	Pyro_s_FestiveFlareGun = 1081,
	Pyro_s_FestiveShotgun = 1141,
	Pyro_s_PanicAttack = 1153,
	Pyro_s_ThermalThruster = 1179,
	Pyro_s_GasPasser = 1180,
	Pyro_s_BackwoodsBoomstick = 15003,
	Pyro_s_RusticRuiner = 15016,
	Pyro_s_CivicDuty = 15044,
	Pyro_s_LightningRod = 15047,
	Pyro_s_Autumn = 15085,
	Pyro_s_FlowerPower = 15109,
	Pyro_s_CoffinNail = 15132,
	Pyro_s_DressedtoKill = 15133,
	Pyro_s_RedBear = 15152,
	Pyro_t_FireAxe = 2,
	Pyro_t_FireAxeR = 192,
	Pyro_t_TheAxtinguisher = 38,
	Pyro_t_Homewrecker = 153,
	Pyro_t_ThePowerjack = 214,
	Pyro_t_TheBackScratcher = 326,
	Pyro_t_SharpenedVolcanoFragment = 348,
	Pyro_t_ThePostalPummeler = 457,
	Pyro_t_TheMaul = 466,
	Pyro_t_TheConscientiousObjector = 474,
	Pyro_t_TheThirdDegree = 593,
	Pyro_t_TheLollichop = 739,
	Pyro_t_NeonAnnihilator = 813,
	Pyro_t_NeonAnnihilatorG = 834,
	Pyro_t_TheFreedomStaff = 880,
	Pyro_t_TheBatOuttaHell = 939,
	Pyro_t_TheMemoryMaker = 954,
	Pyro_t_TheFestiveAxtinguisher = 1000,
	Pyro_t_TheHamShank = 1013,
	Pyro_t_TheNecroSmasher = 1123,
	Pyro_t_TheCrossingGuard = 1127,
	Pyro_t_HotHand = 1181,
	Pyro_t_PrinnyMachete = 30758,
	Demoman_m_GrenadeLauncher = 19,
	Demoman_m_GrenadeLauncherR = 206,
	Demoman_m_TheLochnLoad = 308,
	Demoman_m_AliBabasWeeBooties = 405,
	Demoman_m_TheBootlegger = 608,
	Demoman_m_TheLooseCannon = 996,
	Demoman_m_FestiveGrenadeLauncher = 1007,
	Demoman_m_TheBASEJumper = 1101,
	Demoman_m_TheIronBomber = 1151,
	Demoman_m_Autumn = 15077,
	Demoman_m_MacabreWeb = 15079,
	Demoman_m_Rainbow = 15091,
	Demoman_m_SweetDreams = 15092,
	Demoman_m_CoffinNail = 15116,
	Demoman_m_TopShelf = 15117,
	Demoman_m_Warhawk = 15142,
	Demoman_m_ButcherBird = 15158,
	Demoman_s_StickybombLauncher = 20,
	Demoman_s_StickybombLauncherR = 207,
	Demoman_s_TheScottishResistance = 130,
	Demoman_s_TheCharginTarge = 131,
	Demoman_s_StickyJumper = 265,
	Demoman_s_TheSplendidScreen = 406,
	Demoman_s_FestiveStickybombLauncher = 661,
	Demoman_s_SilverBotkillerStickybombLauncherMkI = 797,
	Demoman_s_GoldBotkillerStickybombLauncherMkI = 806,
	Demoman_s_RustBotkillerStickybombLauncherMkI = 886,
	Demoman_s_BloodBotkillerStickybombLauncherMkI = 895,
	Demoman_s_CarbonadoBotkillerStickybombLauncherMkI = 904,
	Demoman_s_DiamondBotkillerStickybombLauncherMkI = 913,
	Demoman_s_SilverBotkillerStickybombLauncherMkII = 962,
	Demoman_s_GoldBotkillerStickybombLauncherMkII = 971,
	Demoman_s_TheTideTurner = 1099,
	Demoman_s_FestiveTarge = 1144,
	Demoman_s_TheQuickiebombLauncher = 1150,
	Demoman_s_SuddenFlurry = 15009,
	Demoman_s_CarpetBomber = 15012,
	Demoman_s_BlastedBombardier = 15024,
	Demoman_s_RooftopWrangler = 15038,
	Demoman_s_LiquidAsset = 15045,
	Demoman_s_PinkElephant = 15048,
	Demoman_s_Autumn = 15082,
	Demoman_s_PumpkinPatch = 15083,
	Demoman_s_MacabreWeb = 15084,
	Demoman_s_SweetDreams = 15113,
	Demoman_s_CoffinNail = 15137,
	Demoman_s_DressedtoKill = 15138,
	Demoman_s_Blitzkrieg = 15155,
	Demoman_t_Bottle = 1,
	Demoman_t_BottleR = 191,
	Demoman_t_TheEyelander = 132,
	Demoman_t_ThePainTrain = 154,
	Demoman_t_TheScotsmansSkullcutter = 172,
	Demoman_t_HorselessHeadlessHorsemannsHeadtaker = 266,
	Demoman_t_UllapoolCaber = 307,
	Demoman_t_TheClaidheamhMor = 327,
	Demoman_t_TheHalfZatoichi = 357,
	Demoman_t_ThePersianPersuader = 404,
	Demoman_t_TheConscientiousObjector = 474,
	Demoman_t_NessiesNineIron = 482,
	Demoman_t_TheScottishHandshake = 609,
	Demoman_t_TheFreedomStaff = 880,
	Demoman_t_TheBatOuttaHell = 939,
	Demoman_t_TheMemoryMaker = 954,
	Demoman_t_TheHamShank = 1013,
	Demoman_t_FestiveEyelander = 1082,
	Demoman_t_TheNecroSmasher = 1123,
	Demoman_t_TheCrossingGuard = 1127,
	Demoman_t_PrinnyMachete = 30758,
	Heavy_m_Minigun = 15,
	Heavy_m_MinigunR = 202,
	Heavy_m_Natascha = 41,
	Heavy_m_IronCurtain = 298,
	Heavy_m_TheBrassBeast = 312,
	Heavy_m_Tomislav = 424,
	Heavy_m_FestiveMinigun = 654,
	Heavy_m_SilverBotkillerMinigunMkI = 793,
	Heavy_m_GoldBotkillerMinigunMkI = 802,
	Heavy_m_TheHuoLongHeater = 811,
	Heavy_m_TheHuoLongHeaterG = 832,
	Heavy_m_Deflector_mvm = 850,
	Heavy_m_RustBotkillerMinigunMkI = 882,
	Heavy_m_BloodBotkillerMinigunMkI = 891,
	Heavy_m_CarbonadoBotkillerMinigunMkI = 900,
	Heavy_m_DiamondBotkillerMinigunMkI = 909,
	Heavy_m_SilverBotkillerMinigunMkII = 958,
	Heavy_m_GoldBotkillerMinigunMkII = 967,
	Heavy_m_KingoftheJungle = 15004,
	Heavy_m_IronWood = 15020,
	Heavy_m_AntiqueAnnihilator = 15026,
	Heavy_m_WarRoom = 15031,
	Heavy_m_CitizenPain = 15040,
	Heavy_m_BrickHouse = 15055,
	Heavy_m_MacabreWeb = 15086,
	Heavy_m_PumpkinPatch = 15087,
	Heavy_m_Nutcracker = 15088,
	Heavy_m_BrainCandy = 15098,
	Heavy_m_MisterCuddles = 15099,
	Heavy_m_CoffinNail = 15123,
	Heavy_m_DressedtoKill = 15124,
	Heavy_m_TopShelf = 15125,
	Heavy_m_ButcherBird = 15147,
	Heavy_s_HeavysShotgun = 11,
	Heavy_s_ShotgunR = 199,
	Heavy_s_Sandvich = 42,
	Heavy_s_TheDalokohsBar = 159,
	Heavy_s_TheBuffaloSteakSandvich = 311,
	Heavy_s_TheFamilyBusiness = 425,
	Heavy_s_Fishcake = 433,
	Heavy_s_RoboSandvich = 863,
	Heavy_s_FestiveSandvich = 1002,
	Heavy_s_FestiveShotgun = 1141,
	Heavy_s_PanicAttack = 1153,
	Heavy_s_SecondBanana = 1190,
	Heavy_s_BackwoodsBoomstick = 15003,
	Heavy_s_RusticRuiner = 15016,
	Heavy_s_CivicDuty = 15044,
	Heavy_s_LightningRod = 15047,
	Heavy_s_Autumn = 15085,
	Heavy_s_FlowerPower = 15109,
	Heavy_s_CoffinNail = 15132,
	Heavy_s_DressedtoKill = 15133,
	Heavy_s_RedBear = 15152,
	Heavy_t_Fists = 5,
	Heavy_t_FistsR = 195,
	Heavy_t_TheKillingGlovesofBoxing = 43,
	Heavy_t_GlovesofRunningUrgently = 239,
	Heavy_t_WarriorsSpirit = 310,
	Heavy_t_FistsofSteel = 331,
	Heavy_t_TheEvictionNotice = 426,
	Heavy_t_TheConscientiousObjector = 474,
	Heavy_t_ApocoFists = 587,
	Heavy_t_TheHolidayPunch = 656,
	Heavy_t_TheFreedomStaff = 880,
	Heavy_t_TheBatOuttaHell = 939,
	Heavy_t_TheMemoryMaker = 954,
	Heavy_t_TheHamShank = 1013,
	Heavy_t_FestiveGlovesofRunningUrgently = 1084,
	Heavy_t_TheBreadBite = 1100,
	Heavy_t_TheNecroSmasher = 1123,
	Heavy_t_TheCrossingGuard = 1127,
	Heavy_t_GlovesofRunningUrgentlyMvM = 1184,
	Heavy_t_PrinnyMachete = 30758,
	Engi_m_EngineersShotgun = 9,
	Engi_m_ShotgunR = 199,
	Engi_m_TheFrontierJustice = 141,
	Engi_m_TheWidowmaker = 527,
	Engi_m_ThePomson6000 = 588,
	Engi_m_TheRescueRanger = 997,
	Engi_m_FestiveFrontierJustice = 1004,
	Engi_m_FestiveShotgun = 1141,
	Engi_m_PanicAttack = 1153,
	Engi_m_BackwoodsBoomstick = 15003,
	Engi_m_RusticRuiner = 15016,
	Engi_m_CivicDuty = 15044,
	Engi_m_LightningRod = 15047,
	Engi_m_Autumn = 15085,
	Engi_m_FlowerPower = 15109,
	Engi_m_CoffinNail = 15132,
	Engi_m_DressedtoKill = 15133,
	Engi_m_RedBear = 15152,
	Engi_s_EngineersPistol = 22,
	Engi_s_PistolR = 209,
	Engi_s_TheWrangler = 140,
	Engi_s_VintageLugermorph = 160,
	Engi_s_Lugermorph = 294,
	Engi_s_TheShortCircuit = 528,
	Engi_s_FestiveWrangler = 1086,
	Engi_s_RedRockRoscoe = 15013,
	Engi_s_HomemadeHeater = 15018,
	Engi_s_HickoryHolepuncher = 15035,
	Engi_s_LocalHero = 15041,
	Engi_s_BlackDahlia = 15046,
	Engi_s_SandstoneSpecial = 15056,
	Engi_s_MacabreWeb = 15060,
	Engi_s_Nutcracker = 15061,
	Engi_s_BlueMew = 15100,
	Engi_s_BrainCandy = 15101,
	Engi_s_ShottoHell = 15102,
	Engi_s_DressedToKill = 15126,
	Engi_s_Blitzkrieg = 15148,
	Engi_s_TheCAPPER = 30666,
	Engi_s_TheGigarCounter = 30668,
	Engi_t_Wrench = 7,
	Engi_t_WrenchR = 197,
	Engi_t_TheGunslinger = 142,
	Engi_t_TheSouthernHospitality = 155,
	Engi_t_GoldenWrench = 169,
	Engi_t_TheJag = 329,
	Engi_t_TheEurekaEffect = 589,
	Engi_t_FestiveWrench = 662,
	Engi_t_SilverBotkillerWrenchMkI = 795,
	Engi_t_GoldBotkillerWrenchMkI = 804,
	Engi_t_RustBotkillerWrenchMkI = 884,
	Engi_t_BloodBotkillerWrenchMkI = 893,
	Engi_t_CarbonadoBotkillerWrenchMkI = 902,
	Engi_t_DiamondBotkillerWrenchMkI = 911,
	Engi_t_SilverBotkillerWrenchMkII = 960,
	Engi_t_GoldBotkillerWrenchMkII = 969,
	Engi_t_TheNecroSmasher = 1123,
	Engi_t_Nutcracker = 15073,
	Engi_t_Autumn = 15074,
	Engi_t_Boneyard = 15075,
	Engi_t_DressedtoKill = 15139,
	Engi_t_TopShelf = 15140,
	Engi_t_TorquedtoHell = 15114,
	Engi_t_Airwolf = 15156,
	Engi_t_PrinnyMachete = 30758,
	Engi_p_ConstructionPDA = 25,
	Engi_p_ConstructionPDAR = 737,
	Engi_p_DestructionPDA = 26,
	Engi_p_PDA = 28,
	Medic_m_SyringeGun = 17,
	Medic_m_SyringeGunR = 204,
	Medic_m_TheBlutsauger = 36,
	Medic_m_CrusadersCrossbow = 305,
	Medic_m_TheOverdose = 412,
	Medic_m_FestiveCrusadersCrossbow = 1079,
	Medic_s_MediGun = 29,
	Medic_s_MediGunR = 211,
	Medic_s_TheKritzkrieg = 35,
	Medic_s_TheQuickFix = 411,
	Medic_s_FestiveMediGun = 663,
	Medic_s_SilverBotkillerMediGunMkI = 796,
	Medic_s_GoldBotkillerMediGunMkI = 805,
	Medic_s_RustBotkillerMediGunMkI = 885,
	Medic_s_BloodBotkillerMediGunMkI = 894,
	Medic_s_CarbonadoBotkillerMediGunMkI = 903,
	Medic_s_DiamondBotkillerMediGunMkI = 912,
	Medic_s_SilverBotkillerMediGunMkII = 961,
	Medic_s_GoldBotkillerMediGunMkII = 970,
	Medic_s_TheVaccinator = 998,
	Medic_s_MaskedMender = 15008,
	Medic_s_WrappedReviver = 15010,
	Medic_s_ReclaimedReanimator = 15025,
	Medic_s_CivilServant = 15039,
	Medic_s_SparkofLife = 15050,
	Medic_s_Wildwood = 15078,
	Medic_s_FlowerPower = 15097,
	Medic_s_DressedToKill = 15121,
	Medic_s_HighRollers = 15122,
	Medic_s_CoffinNail = 15123,
	Medic_s_Blitzkrieg = 15145,
	Medic_s_Corsair = 15146,
	Medic_t_Bonesaw = 8,
	Medic_t_BonesawR = 198,
	Medic_t_TheUbersaw = 37,
	Medic_t_TheVitaSaw = 173,
	Medic_t_Amputator = 304,
	Medic_t_TheSolemnVow = 413,
	Medic_t_TheConscientiousObjector = 474,
	Medic_t_TheFreedomStaff = 880,
	Medic_t_TheBatOuttaHell = 939,
	Medic_t_TheMemoryMaker = 954,
	Medic_t_FestiveUbersaw = 1003,
	Medic_t_TheHamShank = 1013,
	Medic_t_TheNecroSmasher = 1123,
	Medic_t_TheCrossingGuard = 1127,
	Medic_t_FestiveBonesaw = 1143,
	Medic_t_PrinnyMachete = 30758,
	Sniper_m_SniperRifle = 14,
	Sniper_m_SniperRifleR = 201,
	Sniper_m_TheHuntsman = 56,
	Sniper_m_TheSydneySleeper = 230,
	Sniper_m_TheBazaarBargain = 402,
	Sniper_m_TheMachina = 526,
	Sniper_m_FestiveSniperRifle = 664,
	Sniper_m_TheHitmansHeatmaker = 752,
	Sniper_m_SilverBotkillerSniperRifleMkI = 792,
	Sniper_m_GoldBotkillerSniperRifleMkI = 801,
	Sniper_m_TheAWPerHand = 851,
	Sniper_m_RustBotkillerSniperRifleMkI = 881,
	Sniper_m_BloodBotkillerSniperRifleMkI = 890,
	Sniper_m_CarbonadoBotkillerSniperRifleMkI = 899,
	Sniper_m_DiamondBotkillerSniperRifleMkI = 908,
	Sniper_m_SilverBotkillerSniperRifleMkII = 957,
	Sniper_m_GoldBotkillerSniperRifleMkII = 966,
	Sniper_m_FestiveHuntsman = 1005,
	Sniper_m_TheFortifiedCompound = 1092,
	Sniper_m_TheClassic = 1098,
	Sniper_m_NightOwl = 15000,
	Sniper_m_PurpleRange = 15007,
	Sniper_m_LumberFromDownUnder = 15019,
	Sniper_m_ShotintheDark = 15023,
	Sniper_m_Bogtrotter = 15033,
	Sniper_m_Thunderbolt = 15059,
	Sniper_m_PumpkinPatch = 15070,
	Sniper_m_Boneyard = 15071,
	Sniper_m_Wildwood = 15072,
	Sniper_m_Balloonicorn = 15111,
	Sniper_m_Rainbow = 15112,
	Sniper_m_CoffinNail = 15135,
	Sniper_m_DressedtoKill = 15136,
	Sniper_m_Airwolf = 15154,
	Sniper_m_ShootingStar = 30665,
	Sniper_s_SMG = 16,
	Sniper_s_SMGR = 203,
	Sniper_s_TheRazorback = 57,
	Sniper_s_Jarate = 58,
	Sniper_s_DarwinsDangerShield = 231,
	Sniper_s_CozyCamper = 642,
	Sniper_s_TheCleanersCarbine = 751,
	Sniper_s_FestiveJarate = 1083,
	Sniper_s_TheSelfAwareBeautyMark = 1105,
	Sniper_s_FestiveSMG = 1149,
	Sniper_s_WoodsyWidowmaker = 15001,
	Sniper_s_PlaidPotshotter = 15022,
	Sniper_s_TreadplateTormenter = 15032,
	Sniper_s_TeamSprayer = 15037,
	Sniper_s_LowProfile = 15058,
	Sniper_s_Wildwood = 15076,
	Sniper_s_BlueMew = 15110,
	Sniper_s_HighRollers = 15134,
	Sniper_s_Blitzkrieg = 15153,
	Sniper_t_Kukri = 3,
	Sniper_t_KukriR = 193,
	Sniper_t_TheTribalmansShiv = 171,
	Sniper_t_TheBushwacka = 232,
	Sniper_t_TheShahanshah = 401,
	Sniper_t_TheConscientiousObjector = 474,
	Sniper_t_TheFreedomStaff = 880,
	Sniper_t_TheBatOuttaHell = 939,
	Sniper_t_TheMemoryMaker = 954,
	Sniper_t_TheHamShank = 1013,
	Sniper_t_TheNecroSmasher = 1123,
	Sniper_t_TheCrossingGuard = 1127,
	Sniper_t_PrinnyMachete = 30758,
	Spy_m_Revolver = 24,
	Spy_m_RevolverR = 210,
	Spy_m_TheAmbassador = 61,
	Spy_m_BigKill = 161,
	Spy_m_LEtranger = 224,
	Spy_m_TheEnforcer = 460,
	Spy_m_TheDiamondback = 525,
	Spy_m_FestiveAmbassador = 1006,
	Spy_m_FestiveRevolver = 1142,
	Spy_m_PsychedelicSlugger = 15011,
	Spy_m_OldCountry = 15027,
	Spy_m_Mayor = 15042,
	Spy_m_DeadReckoner = 15051,
	Spy_m_Boneyard = 15062,
	Spy_m_Wildwood = 15063,
	Spy_m_MacabreWeb = 15064,
	Spy_m_FlowerPower = 15103,
	Spy_m_TopShelf = 15128,
	Spy_m_CoffinNail = 15129,
	Spy_m_Blitzkrieg = 15149,
	Spy_s_Sapper = 735,
	Spy_s_SapperR = 736,
	Spy_s_TheRedTapeRecorder = 810,
	Spy_s_TheRedTapeRecorderG = 831,
	Spy_s_TheApSapG = 933,
	Spy_s_FestiveSapper = 1080,
	Spy_s_TheSnackAttack = 1102,
	Spy_t_Knife = 4,
	Spy_t_KnifeR = 194,
	Spy_t_YourEternalReward = 225,
	Spy_t_ConniversKunai = 356,
	Spy_t_TheBigEarner = 461,
	Spy_t_TheWangaPrick = 574,
	Spy_t_TheSharpDresser = 638,
	Spy_t_TheSpycicle = 649,
	Spy_t_FestiveKnife = 665,
	Spy_t_TheBlackRose = 727,
	Spy_t_SilverBotkillerKnifeMkI = 794,
	Spy_t_GoldBotkillerKnifeMkI = 803,
	Spy_t_RustBotkillerKnifeMkI = 883,
	Spy_t_BloodBotkillerKnifeMkI = 892,
	Spy_t_CarbonadoBotkillerKnifeMkI = 901,
	Spy_t_DiamondBotkillerKnifeMkI = 910,
	Spy_t_SilverBotkillerKnifeMkII = 959,
	Spy_t_GoldBotkillerKnifeMkII = 968,
	Spy_t_Boneyard = 15062,
	Spy_t_BlueMew = 15094,
	Spy_t_BrainCandy = 15095,
	Spy_t_StabbedtoHell = 15096,
	Spy_t_DressedtoKill = 15118,
	Spy_t_TopShelf = 15119,
	Spy_t_Blitzkrieg = 15143,
	Spy_t_Airwolf = 15144,
	Spy_t_PrinnyMachete = 30758,
	Spy_d_DisguiseKitPDA = 27,
	Spy_w_InvisWatch = 30,
	Spy_w_InvisWatchR = 212,
	Spy_w_TheDeadRinger = 59,
	Spy_w_TheCloakandDagger = 60,
	Spy_w_EnthusiastsTimepiece = 297,
	Spy_w_TheQuackenbirdt = 947,
	Misc_t_FryingPan = 264,
	Misc_t_GoldFryingPan = 1071,
	Misc_t_Saxxy = 423
};

enum struct EWeaponType
{
	UNKNOWN,
	HITSCAN,
	PROJECTILE,
	MELEE
};

enum EWeaponSlot
{
	SLOT_PRIMARY,
	SLOT_SECONDARY,
	SLOT_MELEE
};

enum ProjectileType_t
{
	TF_PROJECTILE_NONE,
	TF_PROJECTILE_BULLET,
	TF_PROJECTILE_ROCKET,
	TF_PROJECTILE_PIPEBOMB,
	TF_PROJECTILE_PIPEBOMB_REMOTE,
	TF_PROJECTILE_SYRINGE,
	TF_PROJECTILE_FLARE,
	TF_PROJECTILE_JAR,
	TF_PROJECTILE_ARROW,
	TF_PROJECTILE_FLAME_ROCKET,
	TF_PROJECTILE_JAR_MILK,
	TF_PROJECTILE_HEALING_BOLT,
	TF_PROJECTILE_ENERGY_BALL,
	TF_PROJECTILE_ENERGY_RING,
	TF_PROJECTILE_PIPEBOMB_PRACTICE,
	TF_PROJECTILE_CLEAVER,
	TF_PROJECTILE_STICKY_BALL,
	TF_PROJECTILE_CANNONBALL,
	TF_PROJECTILE_BUILDING_REPAIR_BOLT,
	TF_PROJECTILE_FESTIVE_ARROW,
	TF_PROJECTILE_THROWABLE,
	TF_PROJECTILE_SPELL,
	TF_PROJECTILE_FESTIVE_JAR,
	TF_PROJECTILE_FESTIVE_HEALING_BOLT,
	TF_PROJECTILE_BREADMONSTER_JARATE,
	TF_PROJECTILE_BREADMONSTER_MADMILK,
	TF_PROJECTILE_GRAPPLINGHOOK,
	TF_PROJECTILE_SENTRY_ROCKET,
	TF_PROJECTILE_BREAD_MONSTER,
	TF_PROJECTILE_JAR_GAS,
	TF_PROJECTILE_BALLOFFIRE,
	TF_NUM_PROJECTILES
};

enum ArrowModels
{
	MODEL_ARROW_REGULAR,
	MODEL_ARROW_BUILDING_REPAIR,
	MODEL_SNOWBALL,
	MODEL_FESTIVE_ARROW_REGULAR,
	MODEL_SYRINGE,
	MODEL_FESTIVE_HEALING_BOLT,
	MODEL_BREAD_MONSTER,
	MODEL_GRAPPLINGHOOK,
	MODEL_THROWING_KNIFE,
	TF_ARROW_MODEL_COUNT
};

enum MedigunChargeTypes
{
	MEDIGUN_CHARGE_INVALID = -1,
	MEDIGUN_CHARGE_INVULN = 0,
	MEDIGUN_CHARGE_CRITICALBOOST,
	MEDIGUN_CHARGE_MEGAHEAL,
	MEDIGUN_CHARGE_BULLET_RESIST,
	MEDIGUN_CHARGE_BLAST_RESIST,
	MEDIGUN_CHARGE_FIRE_RESIST,
	MEDIGUN_NUM_CHARGE_TYPES
};

enum MedicCallerType
{
	CALLER_TYPE_NORMAL,
	CALLER_TYPE_AUTO,
	CALLER_TYPE_REVIVE_EASY,
	CALLER_TYPE_REVIVE_MEDIUM,
	CALLER_TYPE_REVIVE_HARD
};

enum ETFCond
{
	TF_COND_INVALID = -1,
	TF_COND_AIMING,
	TF_COND_ZOOMED,
	TF_COND_DISGUISING,
	TF_COND_DISGUISED,
	TF_COND_STEALTHED,
	TF_COND_INVULNERABLE,
	TF_COND_TELEPORTED,
	TF_COND_TAUNTING,
	TF_COND_INVULNERABLE_WEARINGOFF,
	TF_COND_STEALTHED_BLINK,
	TF_COND_SELECTED_TO_TELEPORT,
	TF_COND_CRITBOOSTED,
	TF_COND_TMPDAMAGEBONUS,
	TF_COND_FEIGN_DEATH,
	TF_COND_PHASE,
	TF_COND_STUNNED,
	TF_COND_OFFENSEBUFF,
	TF_COND_SHIELD_CHARGE,
	TF_COND_DEMO_BUFF,
	TF_COND_ENERGY_BUFF,
	TF_COND_RADIUSHEAL,
	TF_COND_HEALTH_BUFF,
	TF_COND_BURNING,
	TF_COND_HEALTH_OVERHEALED,
	TF_COND_URINE,
	TF_COND_BLEEDING,
	TF_COND_DEFENSEBUFF,
	TF_COND_MAD_MILK,
	TF_COND_MEGAHEAL,
	TF_COND_REGENONDAMAGEBUFF,
	TF_COND_MARKEDFORDEATH,
	TF_COND_NOHEALINGDAMAGEBUFF,
	TF_COND_SPEED_BOOST,
	TF_COND_CRITBOOSTED_PUMPKIN,
	TF_COND_CRITBOOSTED_USER_BUFF,
	TF_COND_CRITBOOSTED_DEMO_CHARGE,
	TF_COND_SODAPOPPER_HYPE,
	TF_COND_CRITBOOSTED_FIRST_BLOOD,
	TF_COND_CRITBOOSTED_BONUS_TIME,
	TF_COND_CRITBOOSTED_CTF_CAPTURE,
	TF_COND_CRITBOOSTED_ON_KILL,
	TF_COND_CANNOT_SWITCH_FROM_MELEE,
	TF_COND_DEFENSEBUFF_NO_CRIT_BLOCK,
	TF_COND_REPROGRAMMED,
	TF_COND_CRITBOOSTED_RAGE_BUFF,
	TF_COND_DEFENSEBUFF_HIGH,
	TF_COND_SNIPERCHARGE_RAGE_BUFF,
	TF_COND_DISGUISE_WEARINGOFF,
	TF_COND_MARKEDFORDEATH_SILENT,
	TF_COND_DISGUISED_AS_DISPENSER,
	TF_COND_SAPPED,
	TF_COND_INVULNERABLE_HIDE_UNLESS_DAMAGED,
	TF_COND_INVULNERABLE_USER_BUFF,
	TF_COND_HALLOWEEN_BOMB_HEAD,
	TF_COND_HALLOWEEN_THRILLER,
	TF_COND_RADIUSHEAL_ON_DAMAGE,
	TF_COND_CRITBOOSTED_CARD_EFFECT,
	TF_COND_INVULNERABLE_CARD_EFFECT,
	TF_COND_MEDIGUN_UBER_BULLET_RESIST,
	TF_COND_MEDIGUN_UBER_BLAST_RESIST,
	TF_COND_MEDIGUN_UBER_FIRE_RESIST,
	TF_COND_MEDIGUN_SMALL_BULLET_RESIST,
	TF_COND_MEDIGUN_SMALL_BLAST_RESIST,
	TF_COND_MEDIGUN_SMALL_FIRE_RESIST,
	TF_COND_STEALTHED_USER_BUFF,
	TF_COND_MEDIGUN_DEBUFF,
	TF_COND_STEALTHED_USER_BUFF_FADING,
	TF_COND_BULLET_IMMUNE,
	TF_COND_BLAST_IMMUNE,
	TF_COND_FIRE_IMMUNE,
	TF_COND_PREVENT_DEATH,
	TF_COND_MVM_BOT_STUN_RADIOWAVE,
	TF_COND_HALLOWEEN_SPEED_BOOST,
	TF_COND_HALLOWEEN_QUICK_HEAL,
	TF_COND_HALLOWEEN_GIANT,
	TF_COND_HALLOWEEN_TINY,
	TF_COND_HALLOWEEN_IN_HELL,
	TF_COND_HALLOWEEN_GHOST_MODE,
	TF_COND_MINICRITBOOSTED_ON_KILL,
	TF_COND_OBSCURED_SMOKE,
	TF_COND_PARACHUTE_ACTIVE,
	TF_COND_BLASTJUMPING,
	TF_COND_HALLOWEEN_KART,
	TF_COND_HALLOWEEN_KART_DASH,
	TF_COND_BALLOON_HEAD,
	TF_COND_MELEE_ONLY,
	TF_COND_SWIMMING_CURSE,
	TF_COND_FREEZE_INPUT,
	TF_COND_HALLOWEEN_KART_CAGE,
	TF_COND_HASRUNE, //TF_COND_DONOTUSE_0
	TF_COND_RUNE_STRENGTH,
	TF_COND_RUNE_HASTE,
	TF_COND_RUNE_REGEN,
	TF_COND_RUNE_RESIST,
	TF_COND_RUNE_VAMPIRE,
	TF_COND_RUNE_REFLECT,
	TF_COND_RUNE_PRECISION,
	TF_COND_RUNE_AGILITY,
	TF_COND_GRAPPLINGHOOK,
	TF_COND_GRAPPLINGHOOK_SAFEFALL,
	TF_COND_GRAPPLINGHOOK_LATCHED,
	TF_COND_GRAPPLINGHOOK_BLEEDING,
	TF_COND_AFTERBURN_IMMUNE,
	TF_COND_RUNE_KNOCKOUT,
	TF_COND_RUNE_IMBALANCE,
	TF_COND_CRITBOOSTED_RUNE_TEMP,
	TF_COND_PASSTIME_INTERCEPTION,
	TF_COND_SWIMMING_NO_EFFECTS,
	TF_COND_PURGATORY,
	TF_COND_RUNE_KING,
	TF_COND_RUNE_PLAGUE,
	TF_COND_RUNE_SUPERNOVA,
	TF_COND_PLAGUE,
	TF_COND_KING_BUFFED,
	TF_COND_TEAM_GLOWS,
	TF_COND_KNOCKED_INTO_AIR,
	TF_COND_COMPETITIVE_WINNER,
	TF_COND_COMPETITIVE_LOSER,
	TF_COND_HEALING_DEBUFF,
	TF_COND_PASSTIME_PENALTY_DEBUFF,
	TF_COND_GRAPPLED_TO_PLAYER,
	TF_COND_GRAPPLED_BY_PLAYER,
	TF_COND_PARACHUTE_DEPLOYED,
	TF_COND_GAS,
	TF_COND_BURNING_PYRO,
	TF_COND_ROCKETPACK,
	TF_COND_LOST_FOOTING,
	TF_COND_AIR_CURRENT,
	TF_COND_HALLOWEEN_HELL_HEAL,
	TF_COND_POWERUPMODE_DOMINANT
};

enum ETFConds
{
	TFCond_Slowed = (1 << 0), // 0
	TFCond_Zoomed = (1 << 1), // 1
	TFCond_Disguised = (1 << 3), // 3
	TFCond_Stealthed = (1 << 4), // 4
	TFCond_Ubercharged = (1 << 5), // 5
	TFCond_Taunting = (1 << 7), // 7
	TFCond_Bonked = (1 << 14), // 14
	TFCond_Stunned = (1 << 15), // 15
	TFCond_Charging = (1 << 17), // 17
	TFCond_OnFire = (1 << 22), // 22
	TFCond_Jarated = (1 << 24), // 24
	TFCond_Bleeding = (1 << 25), // 25
	TFCond_Milked = (1 << 27), // 27
	TFCond_MegaHeal = (1 << 28), // 28

	TFCondEx_PyroCrits = (1 << 12), // 44
	TFCondEx_BulletCharge = (1 << 26), // 58
	TFCondEx_ExplosiveCharge = (1 << 27), // 59
	TFCondEx_FireCharge = (1 << 28), // 60

	TFCondEx2_BulletImmune = (1 << 3), // 67
	TFCondEx2_BlastImmune = (1 << 4), // 68
	TFCondEx2_FireImmune = (1 << 5), // 69
	TFCondEx2_HalloweenGhostMode = (1 << 13), // 77
	TFCondEx2_InKart = (1 << 18), // 82
	TFCondEx2_StrengthRune = (1 << 26), // 90
	TFCondEx2_HasteRune = (1 << 27), // 91
	TFCondEx2_RegenRune = (1 << 28), // 92
	TFCondEx2_ResistRune = (1 << 29), // 93
	TFCondEx2_VampireRune = (1 << 30), // 94
	TFCondEx2_ReflectRune = (1 << 31), // 95

	TFCondEx3_PrecisionRune = (1 << 0), // 96
	TFCondEx3_AgilityRune = (1 << 1), // 97
	TFCondEx3_KnockoutRune = (1 << 7), // 103
	TFCondEx3_ImbalanceRune = (1 << 8), // 104
	TFCondEx3_CritboostedTempRune = (1 << 9), // 105
	TFCondEx3_KingRune = (1 << 13), // 109
	TFCondEx3_PlagueRune = (1 << 14), // 110
	TFCondEx3_SupernovaRune = (1 << 15), // 111
	TFCondEx3_KingBuff = (1 << 17), // 113
};

enum ETFDmgCustom
{
	TF_DMG_CUSTOM_NONE = 0,
	TF_DMG_CUSTOM_HEADSHOT,
	TF_DMG_CUSTOM_BACKSTAB,
	TF_DMG_CUSTOM_BURNING,
	TF_DMG_WRENCH_FIX,
	TF_DMG_CUSTOM_MINIGUN,
	TF_DMG_CUSTOM_SUICIDE,
	TF_DMG_CUSTOM_TAUNTATK_HADOUKEN,
	TF_DMG_CUSTOM_BURNING_FLARE,
	TF_DMG_CUSTOM_TAUNTATK_HIGH_NOON,
	TF_DMG_CUSTOM_TAUNTATK_GRAND_SLAM,
	TF_DMG_CUSTOM_PENETRATE_MY_TEAM,
	TF_DMG_CUSTOM_PENETRATE_ALL_PLAYERS,
	TF_DMG_CUSTOM_TAUNTATK_FENCING,
	TF_DMG_CUSTOM_PENETRATE_NONBURNING_TEAMMATE,
	TF_DMG_CUSTOM_TAUNTATK_ARROW_STAB,
	TF_DMG_CUSTOM_TELEFRAG,
	TF_DMG_CUSTOM_BURNING_ARROW,
	TF_DMG_CUSTOM_FLYINGBURN,
	TF_DMG_CUSTOM_PUMPKIN_BOMB,
	TF_DMG_CUSTOM_DECAPITATION,
	TF_DMG_CUSTOM_TAUNTATK_GRENADE,
	TF_DMG_CUSTOM_BASEBALL,
	TF_DMG_CUSTOM_CHARGE_IMPACT,
	TF_DMG_CUSTOM_TAUNTATK_BARBARIAN_SWING,
	TF_DMG_CUSTOM_AIR_STICKY_BURST,
	TF_DMG_CUSTOM_DEFENSIVE_STICKY,
	TF_DMG_CUSTOM_PICKAXE,
	TF_DMG_CUSTOM_ROCKET_DIRECTHIT,
	TF_DMG_CUSTOM_TAUNTATK_UBERSLICE,
	TF_DMG_CUSTOM_PLAYER_SENTRY,
	TF_DMG_CUSTOM_STANDARD_STICKY,
	TF_DMG_CUSTOM_SHOTGUN_REVENGE_CRIT,
	TF_DMG_CUSTOM_TAUNTATK_ENGINEER_GUITAR_SMASH,
	TF_DMG_CUSTOM_BLEEDING,
	TF_DMG_CUSTOM_GOLD_WRENCH,
	TF_DMG_CUSTOM_CARRIED_BUILDING,
	TF_DMG_CUSTOM_COMBO_PUNCH,
	TF_DMG_CUSTOM_TAUNTATK_ENGINEER_ARM_KILL,
	TF_DMG_CUSTOM_FISH_KILL,
	TF_DMG_CUSTOM_TRIGGER_HURT,
	TF_DMG_CUSTOM_DECAPITATION_BOSS,
	TF_DMG_CUSTOM_STICKBOMB_EXPLOSION,
	TF_DMG_CUSTOM_AEGIS_ROUND,
	TF_DMG_CUSTOM_FLARE_EXPLOSION,
	TF_DMG_CUSTOM_BOOTS_STOMP,
	TF_DMG_CUSTOM_PLASMA,
	TF_DMG_CUSTOM_PLASMA_CHARGED,
	TF_DMG_CUSTOM_PLASMA_GIB,
	TF_DMG_CUSTOM_PRACTICE_STICKY,
	TF_DMG_CUSTOM_EYEBALL_ROCKET,
	TF_DMG_CUSTOM_HEADSHOT_DECAPITATION,
	TF_DMG_CUSTOM_TAUNTATK_ARMAGEDDON,
	TF_DMG_CUSTOM_FLARE_PELLET,
	TF_DMG_CUSTOM_CLEAVER,
	TF_DMG_CUSTOM_CLEAVER_CRIT,
	TF_DMG_CUSTOM_SAPPER_RECORDER_DEATH,
	TF_DMG_CUSTOM_MERASMUS_PLAYER_BOMB,
	TF_DMG_CUSTOM_MERASMUS_GRENADE,
	TF_DMG_CUSTOM_MERASMUS_ZAP,
	TF_DMG_CUSTOM_MERASMUS_DECAPITATION,
	TF_DMG_CUSTOM_CANNONBALL_PUSH,
	TF_DMG_CUSTOM_TAUNTATK_ALLCLASS_GUITAR_RIFF,
	TF_DMG_CUSTOM_THROWABLE,
	TF_DMG_CUSTOM_THROWABLE_KILL,
	TF_DMG_CUSTOM_SPELL_TELEPORT,
	TF_DMG_CUSTOM_SPELL_SKELETON,
	TF_DMG_CUSTOM_SPELL_MIRV,
	TF_DMG_CUSTOM_SPELL_METEOR,
	TF_DMG_CUSTOM_SPELL_LIGHTNING,
	TF_DMG_CUSTOM_SPELL_FIREBALL,
	TF_DMG_CUSTOM_SPELL_MONOCULUS,
	TF_DMG_CUSTOM_SPELL_BLASTJUMP,
	TF_DMG_CUSTOM_SPELL_BATS,
	TF_DMG_CUSTOM_SPELL_TINY,
	TF_DMG_CUSTOM_KART,
	TF_DMG_CUSTOM_GIANT_HAMMER,
	TF_DMG_CUSTOM_RUNE_REFLECT,
	TF_DMG_CUSTOM_END
};

enum HalloweenBossType
{
	HALLOWEEN_BOSS_INVALID = 0,
	HALLOWEEN_BOSS_HHH = 1,
	HALLOWEEN_BOSS_MONOCULUS = 2,
	HALLOWEEN_BOSS_MERASMUS = 3
};

enum PowerupBottleType_t
{
	POWERUP_BOTTLE_NONE,
	POWERUP_BOTTLE_CRITBOOST,
	POWERUP_BOTTLE_UBERCHARGE,
	POWERUP_BOTTLE_RECALL,
	POWERUP_BOTTLE_REFILL_AMMO,
	POWERUP_BOTTLE_BUILDINGS_INSTANT_UPGRADE,
	POWERUP_BOTTLE_RADIUS_STEALTH,
	POWERUP_BOTTLE_TOTAL
};

enum ETFFlagType
{
	TF_FLAGTYPE_CTF = 0,
	TF_FLAGTYPE_ATTACK_DEFEND,
	TF_FLAGTYPE_TERRITORY_CONTROL,
	TF_FLAGTYPE_INVADE,
	TF_FLAGTYPE_RESOURCE_CONTROL,
	TF_FLAGTYPE_ROBOT_DESTRUCTION,
	TF_FLAGTYPE_PLAYER_DESTRUCTION
};

enum ETFGameType
{
	TF_GAMETYPE_UNDEFINED = 0,
	TF_GAMETYPE_CTF,
	TF_GAMETYPE_CP,
	TF_GAMETYPE_ESCORT,
	TF_GAMETYPE_ARENA,
	TF_GAMETYPE_MVM,
	TF_GAMETYPE_RD,
	TF_GAMETYPE_PASSTIME,
	TF_GAMETYPE_PD,
	TF_GAMETYPE_COUNT
};

enum gamerules_roundstate_t
{
	// initialize the game, create teams
	GR_STATE_INIT = 0,

	//Before players have joined the game. Periodically checks to see if enough players are ready
	//to start a game. Also reverts to this when there are no active players
	GR_STATE_PREGAME,

	//The game is about to start, wait a bit and spawn everyone
	GR_STATE_STARTGAME,

	//All players are respawned, frozen in place
	GR_STATE_PREROUND,

	//Round is on, playing normally
	GR_STATE_RND_RUNNING,

	//Someone has won the round
	GR_STATE_TEAM_WIN,

	//Noone has won, manually restart the game, reset scores
	GR_STATE_RESTART,

	//Noone has won, restart the game
	GR_STATE_STALEMATE,

	//Game is over, showing the scoreboard etc
	GR_STATE_GAME_OVER,

	//Game is in a bonus state, transitioned to after a round ends
	GR_STATE_BONUS,

	//Game is awaiting the next wave/round of a multi round experience
	GR_STATE_BETWEEN_RNDS,

	GR_NUM_ROUND_STATES
};

enum
{
	TF_ARENA_NOTIFICATION_CAREFUL = 0,
	TF_ARENA_NOTIFICATION_SITOUT,
	TF_ARENA_NOTIFICATION_NOPLAYERS
};

enum
{
	MVM_EVENT_POPFILE_NONE = 0,
	MVM_EVENT_POPFILE_HALLOWEEN,
	MVM_EVENT_POPFILE_MAX_TYPES
};

enum
{
	DRAW_ARROW_UP,
	DRAW_ARROW_LEFT,
	DRAW_ARROW_RIGHT
};

enum
{
	TF_HUDTYPE_UNDEFINED = 0,
	TF_HUDTYPE_CTF,
	TF_HUDTYPE_CP,
	TF_HUDTYPE_ESCORT,
	TF_HUDTYPE_ARENA,
	TF_HUDTYPE_TRAINING
};

enum
{
	TF_BUILDING_SENTRY = (1 << 0),
	TF_BUILDING_DISPENSER = (1 << 1),
	TF_BUILDING_TELEPORT = (1 << 2)
};

enum
{
	TF_ITEM_UNDEFINED = 0,
	TF_ITEM_CAPTURE_FLAG = (1 << 0),
	TF_ITEM_HEALTH_KIT = (1 << 1),
	TF_ITEM_ARMOR = (1 << 2),
	TF_ITEM_AMMO_PACK = (1 << 3),
	TF_ITEM_GRENADE_PACK = (1 << 4)
};

enum ETFFlagEventTypes
{
	TF_FLAGEVENT_PICKUP = 1,
	TF_FLAGEVENT_CAPTURE,
	TF_FLAGEVENT_DEFEND,
	TF_FLAGEVENT_DROPPED,
	TF_FLAGEVENT_RETURNED,
	TF_NUM_FLAG_EVENTS
};

enum TauntAttack_t
{
	TAUNTATK_NONE = 0,
	TAUNTATK_PYRO_HADOUKEN,
	TAUNTATK_HEAVY_EAT,
	TAUNTATK_HEAVY_RADIAL_BUFF,
	TAUNTATK_HEAVY_HIGH_NOON,
	TAUNTATK_SCOUT_DRINK,
	TAUNTATK_SCOUT_GRAND_SLAM,
	TAUNTATK_MEDIC_INHALE,
	TAUNTATK_SPY_FENCING_SLASH_A,
	TAUNTATK_SPY_FENCING_SLASH_B,
	TAUNTATK_SPY_FENCING_STAB,
	TAUNTATK_RPS_KILL,
	TAUNTATK_SNIPER_ARROW_STAB_IMPALE,
	TAUNTATK_SNIPER_ARROW_STAB_KILL,
	TAUNTATK_SOLDIER_GRENADE_KILL,
	TAUNTATK_DEMOMAN_BARBARIAN_SWING,
	TAUNTATK_MEDIC_UBERSLICE_IMPALE,
	TAUNTATK_MEDIC_UBERSLICE_KILL,
	TAUNTATK_FLIP_LAND_PARTICLE,
	TAUNTATK_RPS_PARTICLE,
	TAUNTATK_HIGHFIVE_PARTICLE,
	TAUNTATK_ENGINEER_GUITAR_SMASH,
	TAUNTATK_ENGINEER_ARM_IMPALE,
	TAUNTATK_ENGINEER_ARM_KILL,
	TAUNTATK_ENGINEER_ARM_BLEND,
	TAUNTATK_SOLDIER_GRENADE_KILL_WORMSIGN,
	TAUNTATK_SHOW_ITEM,
	TAUNTATK_MEDIC_RELEASE_DOVES,
	TAUNTATK_PYRO_ARMAGEDDON,
	TAUNTATK_PYRO_SCORCHSHOT,
	TAUNTATK_ALLCLASS_GUITAR_RIFF,
	TAUNTATK_MEDIC_HEROIC_TAUNT,
	TAUNTATK_COUNT
};

enum ObjectType_t
{
	OBJ_DISPENSER = 0,
	OBJ_TELEPORTER,
	OBJ_SENTRYGUN,
	OBJ_ATTACHMENT_SAPPER,
	OBJ_LAST
};

typedef enum
{
	BUILDING_HUD_ALERT_NONE = 0,
	BUILDING_HUD_ALERT_LOW_AMMO,
	BUILDING_HUD_ALERT_LOW_HEALTH,
	BUILDING_HUD_ALERT_VERY_LOW_AMMO,
	BUILDING_HUD_ALERT_VERY_LOW_HEALTH,
	BUILDING_HUD_ALERT_SAPPER,
	MAX_BUILDING_HUD_ALERT_LEVEL
} BuildingHudAlert_t;

typedef enum
{
	BUILDING_DAMAGE_LEVEL_NONE = 0,
	BUILDING_DAMAGE_LEVEL_LIGHT,
	BUILDING_DAMAGE_LEVEL_MEDIUM,
	BUILDING_DAMAGE_LEVEL_HEAVY,
	BUILDING_DAMAGE_LEVEL_CRITICAL,
	MAX_BUILDING_DAMAGE_LEVEL
} BuildingDamageLevel_t;

enum
{
	MODE_TELEPORTER_ENTRANCE = 0,
	MODE_TELEPORTER_EXIT
};

enum
{
	MODE_SENTRYGUN_NORMAL = 0,
	MODE_SENTRYGUN_DISPOSABLE
};

enum
{
	MODE_SAPPER_NORMAL = 0,
	MODE_SAPPER_ANTI_ROBOT,
	MODE_SAPPER_ANTI_ROBOT_RADIUS
};

enum ESpyTrapType_t
{
	MODE_SPY_TRAP_RADIUS_STEALTH = 0,
	MODE_SPY_TRAP_REPROGRAM,
	MODE_SPY_TRAP_MAGNET
};

enum
{
	DISPENSER_STATE_IDLE,
	DISPENSER_STATE_UPGRADING
};

enum
{
	TELEPORTER_STATE_BUILDING = 0,
	TELEPORTER_STATE_IDLE,
	TELEPORTER_STATE_READY,
	TELEPORTER_STATE_SENDING,
	TELEPORTER_STATE_RECEIVING,
	TELEPORTER_STATE_RECEIVING_RELEASE,
	TELEPORTER_STATE_RECHARGING,
	TELEPORTER_STATE_UPGRADING,
};
enum
{
	SENTRY_STATE_INACTIVE = 0,
	SENTRY_STATE_SEARCHING,
	SENTRY_STATE_ATTACKING,
	SENTRY_STATE_UPGRADING,
	SENTRY_NUM_STATES
};

enum
{
	OF_ALLOW_REPEAT_PLACEMENT = 0x01,
	OF_MUST_BE_BUILT_ON_ATTACHMENT = 0x02,
	OF_DOESNT_HAVE_A_MODEL = 0x04,
	OF_PLAYER_DESTRUCTION = 0x08,
	OF_BIT_COUNT = 4
};

enum
{
	BS_IDLE = 0,
	BS_SELECTING,
	BS_PLACING,
	BS_PLACING_INVALID
};

enum
{
	BUILDER_OBJECT_BITS = 8,
	BUILDER_INVALID_OBJECT = ((1 << BUILDER_OBJECT_BITS) - 1)
};

enum CurrencyRewards_t
{
	TF_CURRENCY_KILLED_PLAYER,
	TF_CURRENCY_KILLED_OBJECT,
	TF_CURRENCY_ASSISTED_PLAYER,
	TF_CURRENCY_BONUS_POINTS,
	TF_CURRENCY_CAPTURED_OBJECTIVE,
	TF_CURRENCY_ESCORT_REWARD,
	TF_CURRENCY_PACK_SMALL,
	TF_CURRENCY_PACK_MEDIUM,
	TF_CURRENCY_PACK_LARGE,
	TF_CURRENCY_PACK_CUSTOM,
	TF_CURRENCY_TIME_REWARD,
	TF_CURRENCY_WAVE_COLLECTION_BONUS
};

enum MVMAnnouncement_t
{
	TF_MVM_ANNOUNCEMENT_WAVE_COMPLETE,
	TF_MVM_ANNOUNCEMENT_WAVE_FAILED,
	TF_MVM_ANNOUNCEMENT_TOTAL
};

enum MVMUpgradeUIGroups_t
{
	UIGROUP_UPGRADE_ATTACHED_TO_ITEM = 0,
	UIGROUP_UPGRADE_ATTACHED_TO_PLAYER,
	UIGROUP_POWERUPBOTTLE
};

enum
{
	MVM_UPGRADE_QUALITY_LOW = 1,
	MVM_UPGRADE_QUALITY_NORMAL,
	MVM_UPGRADE_QAULITY_HIGH
};

enum
{
	TF_SAPEVENT_NONE = 0,
	TF_SAPEVENT_PLACED,
	TF_SAPEVENT_DONE
};

enum RuneTypes_t
{
	RUNE_NONE = -1,
	RUNE_STRENGTH,
	RUNE_HASTE,
	RUNE_REGEN,
	RUNE_RESIST,
	RUNE_VAMPIRE,
	RUNE_REFLECT,
	RUNE_PRECISION,
	RUNE_AGILITY,
	RUNE_KNOCKOUT,
	RUNE_KING,
	RUNE_PLAGUE,
	RUNE_SUPERNOVA,
	RUNE_TYPES_MAX
};

enum ETFMatchGroup
{
	k_eTFMatchGroup_Invalid = -1,
	k_eTFMatchGroup_MvM_Practice = 0,
	k_eTFMatchGroup_MvM_MannUp = 1,
	k_eTFMatchGroup_First = 0,
	k_eTFMatchGroup_MvM_Default = 0,
	k_eTFMatchGroup_MvM_First = 0,
	k_eTFMatchGroup_MvM_Last = 1,
	k_eTFMatchGroup_Ladder_6v6 = 2,
	k_eTFMatchGroup_Ladder_9v9 = 3,
	k_eTFMatchGroup_Ladder_12v12 = 4,
	k_eTFMatchGroup_Ladder_Default = 2,
	k_eTFMatchGroup_Ladder_First = 2,
	k_eTFMatchGroup_Ladder_Last = 4,
	k_eTFMatchGroup_Casual_6v6 = 5,
	k_eTFMatchGroup_Casual_9v9 = 6,
	k_eTFMatchGroup_Casual_12v12 = 7,
	k_eTFMatchGroup_Casual_Default = 7,
	k_eTFMatchGroup_Casual_First = 5,
	k_eTFMatchGroup_Casual_Last = 7,
	k_eTFMatchGroup_Event_Placeholder = 8,
	k_eTFMatchGroup_Event_Default = 8,
	k_eTFMatchGroup_Event_First = 8,
	k_eTFMatchGroup_Event_Last = 8,
};

enum
{
	PARTITION_ENGINE_SOLID_EDICTS = (1 << 0),
	PARTITION_ENGINE_TRIGGER_EDICTS = (1 << 1),
	PARTITION_CLIENT_SOLID_EDICTS = (1 << 2),
	PARTITION_CLIENT_RESPONSIVE_EDICTS = (1 << 3),
	PARTITION_ENGINE_NON_STATIC_EDICTS = (1 << 4),
	PARTITION_CLIENT_STATIC_PROPS = (1 << 5),
	PARTITION_ENGINE_STATIC_PROPS = (1 << 6),
	PARTITION_CLIENT_NON_STATIC_EDICTS = (1 << 7)
};

enum soundlevel_t
{
	SNDLVL_NONE = 0,
	SNDLVL_20dB = 20,
	SNDLVL_25dB = 25,
	SNDLVL_30dB = 30,
	SNDLVL_35dB = 35,
	SNDLVL_40dB = 40,
	SNDLVL_45dB = 45,
	SNDLVL_50dB = 50,
	SNDLVL_55dB = 55,
	SNDLVL_IDLE = 60,
	SNDLVL_60dB = 60,
	SNDLVL_65dB = 65,
	SNDLVL_STATIC = 66,
	SNDLVL_70dB = 70,
	SNDLVL_NORM = 75,
	SNDLVL_75dB = 75,
	SNDLVL_80dB = 80,
	SNDLVL_TALKING = 80,
	SNDLVL_85dB = 85,
	SNDLVL_90dB = 90,
	SNDLVL_95dB = 95,
	SNDLVL_100dB = 100,
	SNDLVL_105dB = 105,
	SNDLVL_110dB = 110,
	SNDLVL_120dB = 120,
	SNDLVL_130dB = 130,
	SNDLVL_GUNFIRE = 140,
	SNDLVL_140dB = 140,
	SNDLVL_150dB = 150,
	SNDLVL_180dB = 180
};

enum
{
	WL_NotInWater = 0,
	WL_Feet,
	WL_Waist,
	WL_Eyes
};

struct StartSoundParams_t
{
	bool			staticsound;
	int				userdata;
	int				soundsource;
	int				entchannel;
	void* pSfx; //CSfxTable
	Vec3			origin;
	Vec3			direction;
	bool			bUpdatePositions;
	float			fvol;
	soundlevel_t	soundlevel;
	int				flags;
	int				pitch;
	int				specialdsp;
	bool			fromserver;
	float			delay;
	int				speakerentity;
	bool			suppressrecording;
	int				initialStreamPosition;
};